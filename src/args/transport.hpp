#ifndef __MALEXANDRIA_ARGS_TRANSPORT_HPP
#define __MALEXANDRIA_ARGS_TRANSPORT_HPP

#include <cstdlib>
#include <set>

#include "exception.hpp"
#include "utility.hpp"

#include "args/module.hpp"
#include "export.hpp"
#include "http.hpp"
#include "logger.hpp"
#include "../sample.hpp"
#include "ssh.hpp"
#include "zip.hpp"

namespace malexandria
{
   struct TransportModule : public Module
   {
      struct ExportFunction : public Module
      {
         ExportFunction()
            : Module("export",
                     "Export samples and analyses from the database into a portable file.")
         {
            this->add_argument("-t", "--type")
               .help("The type of export to write. Default value is mlx. Options are mlx and singleton. Singleton may only be used for a single sample.")
               .default_value("mlx");
            
            this->add_argument("-f", "--filename")
               .help("The filename to export to. If none is specified, the default is \"[active directory]/export.mlx\" or \"export.zip\" depending on type. Specify - to write to stdout.");
            
            this->add_argument("-c", "--children")
               .help("Export the children of the given samples as well.")
               .default_value(false)
               .implicit_value(true);

            this->add_argument("-P", "--parents")
               .help("Export the parents of the given samples as well.")
               .default_value(false)
               .implicit_value(true);

            this->add_argument("-d", "--descendants")
               .help("Recursively export children and all their children.")
               .default_value(false)
               .implicit_value(true);

            this->add_argument("-A", "--ancestors")
               .help("Recursively export parents and all their parents.")
               .default_value(false)
               .implicit_value(true);

            this->add_argument("-p", "--password")
               .help("The password to give to the export archive. If none is specified, the default config password is used.");
            
            this->add_argument("-s", "--sample")
               .help("The sample identifiers to export.")
               .metavar("IDENTIFIER")
               .append();

            this->add_argument("-a", "--analysis")
               .help("The analysis identifiers to export.")
               .metavar("IDENTIFIER")
               .append();
         }

         virtual bool execute(Module &root) {
            auto export_type = this->get("--type");
            auto filename = this->present("--filename");
            auto export_children = this->get<bool>("--children");
            auto export_parents = this->get<bool>("--parents");
            auto export_descendants = this->get<bool>("--descendants");
            auto export_ancestors = this->get<bool>("--ancestors");
            auto password = this->present("--password");
            auto sample_ids = this->get<std::vector<std::string>>("--sample");
            auto analysis_ids = this->get<std::vector<std::string>>("--analysis");

            if (export_type != "mlx" && export_type != "singleton")
               throw exception::Exception("Invalid export type: must be \"mlx\" or \"singleton\".");

            if (export_type == "singleton" && sample_ids.size() != 1)
               throw exception::Exception("Exactly one sample must be present for a singleton archive.");

            if (!password.has_value())
               password = MainConfig::GetInstance().zip_password();

            std::size_t child_depth = 0, paternal_depth = 0;
            std::vector<Sample> visiting, queue, exporting;
            std::set<std::int64_t> visited, parent_set, child_set;
            
            for (auto &id : sample_ids)
               queue.push_back(Sample::FromIdentifier(id));

            while (queue.size() > 0)
            {
               visiting = queue;
               queue.clear();

               for (auto &sample : visiting)
               {
                  if (visited.find(sample.row_id()) != visited.end())
                     continue;
                  
                  visited.insert(sample.row_id());

                  Logger::InfoN("adding {} to export set", sample.label());
                  exporting.push_back(sample);

                  auto parents = sample.parents();
                  auto children = sample.children();
                  
                  if ((parent_set.size() == 0 || parent_set.find(sample.row_id()) != parent_set.end()) && ((export_parents && paternal_depth < 1) || export_ancestors))
                  {
                     for (auto &parent : parents)
                     {
                        parent_set.insert(parent.row_id());
                        
                        if (visited.find(parent.row_id()) == visited.end())
                        {
                           MLX_DEBUGN("adding parent {} to queue", parent.label());
                           queue.push_back(parent);
                        }
                     }
                  }
                  
                  if ((child_set.size() == 0 || child_set.find(sample.row_id()) != child_set.end()) && ((export_children && child_depth < 1) || export_descendants))
                  {
                     for (auto &child : children)
                     {
                        child_set.insert(child.row_id());
                        
                        if (visited.find(child.row_id()) == visited.end())
                        {
                           MLX_DEBUGN("adding child {} to queue", child.label());
                           queue.push_back(child);
                        }
                     }
                  }
               }

               ++paternal_depth;
               ++child_depth;
            }
            
            std::optional<std::filesystem::path> archive_file;

            if (!filename.has_value())
            {
               archive_file = std::filesystem::path(MainConfig::GetInstance().active_path());

               if (export_type == "mlx")
                  *archive_file /= std::string("export.mlx");
               else
                  *archive_file /= std::string("export.zip");
            }
            else if (*filename != "-")
               archive_file = *filename;
            else
               archive_file = std::nullopt;

            if (archive_file.has_value())
            {
               archive_file = archive_file->make_preferred();
               Logger::InfoN("exporting to {}", archive_file->string());
            }
            else
               Logger::InfoN("exporting to stdout");

            if (export_type == "singleton")
            {
               auto &sample = exporting[0];
               Zip result;

               if (archive_file.has_value())
                  result = Zip(*archive_file);
               else
                  result = Zip(std::nullopt);
               
               result.set_password(*password);
               result.insert_buffer(sample.extract_to_memory(), sample.benign_file());
               result.close();

               if (!archive_file.has_value())
               {
                  auto data = result.read();
                  binary_io();
                  std::cout.write(reinterpret_cast<char *>(data.data()), data.size());
               }
            }
            else
            {
               Export result;

               if (archive_file.has_value())
                  result = Export(*archive_file);
               else
                  result = Export(std::nullopt);

               result.set_password(*password);

               for (auto &sample : exporting)
                  result.add_sample(sample);

               for (auto &id : analysis_ids)
                  result.add_analysis(Analysis::FromIdentifier(id));

               result.close();

               if (!archive_file.has_value())
               {
                  auto data = result.read();
                  binary_io();
                  std::cout.write(reinterpret_cast<char *>(data.data()), data.size());
               }
            }

            if (archive_file.has_value())
            {
               Logger::InfoN("data exported to {}", archive_file->string());
               std::cout << archive_file->string() << std::endl;
            }
            else
               Logger::InfoN("data exported to stdout");
                           
            return true;
         }
      };

      struct ImportFunction : public Module
      {
         ImportFunction()
            : Module("import",
                     "Import a sample or analysis into the database.")
         {
            this->add_argument("-p", "--password")
               .help("The password to use on the sample archive. If no password is supplied, the config's password is used.");

            this->add_argument("-i", "--ignore-known")
               .help("Skip importing already known samples and analyses.")
               .default_value(false)
               .implicit_value(true);
            
            this->add_argument("archive")
               .help("The sample archive to import. Specify - to import from stdin.")
               .required();
         }

         virtual bool execute(Module &root) {
            auto password = this->present("--password");
            auto ignore = this->get<bool>("--ignore-known");
            auto archive = this->get("archive");

            std::optional<std::filesystem::path> archive_file;
            std::optional<std::vector<std::uint8_t>> archive_data;

            if (archive == "-")
            {
               archive_file = std::nullopt;
               archive_data = std::vector<std::uint8_t>();

               std::vector<std::uint8_t> buffer(1024*1024);
               binary_io();

               do
               {
                  std::cin.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
                  archive_data->insert(archive_data->end(), buffer.begin(), buffer.begin()+std::cin.gcount());
               } while (std::cin.gcount() == buffer.size());
            }
            else
            {
               archive_file = archive;
               archive_data = std::nullopt;
            }

            Zip zip_check;

            if (archive_data.has_value())
               zip_check = Zip(*archive_data, ZIP_RDONLY);
            else
               zip_check = Zip(*archive_file, ZIP_RDONLY);

            if (!password.has_value())
               password = MainConfig::GetInstance().zip_password();
            
            zip_check.set_password(*password);

            Logger::Info("detecting the type of archive file...");
               
            if (zip_check.locate("metadata.json") >= 0)
            {
               Logger::Raw(Logger::Level::Info, "mlx\n");
               zip_check.discard();

               Export mlx;

               if (archive_data.has_value())
                  mlx = Export(*archive_data, ZIP_RDONLY);
               else
                  mlx = Export(*archive_file, ZIP_RDONLY);

               mlx.set_password(*password);
               
               if (!ignore)
               {
                  for (auto &analysis : mlx.import_analyses())
                     std::cout << analysis.label() << std::endl;
                  
                  for (auto &sample : mlx.import_samples())
                     std::cout << sample.label() << std::endl;
               }
               else
               {
                  for (auto &analysis : mlx.analyses())
                  {
                     if (Database::GetInstance().query("SELECT id FROM mlx_analysis WHERE analysis_id = ?",
                                                       uuids::to_string(analysis)).size() > 0)
                     {
                        auto analysis_obj = mlx.import_analysis(analysis);
                        std::cout << analysis_obj.label() << std::endl;
                     }
                  }
                  
                  for (auto &sample : mlx.samples())
                  {
                     if (!Sample::IDFromHash(sample).has_value())
                     {
                        auto sample_obj = mlx.import_sample(sample);
                        std::cout << sample_obj.label() << std::endl;
                     }
                  }
               }
            }
            else if (zip_check.entries() == 1)
            {
               Logger::Raw(Logger::Level::Info, "singleton\n");

               auto filename = zip_check.get_name(0);
               Sample sample;

               sample.load_data(zip_check.extract_to_memory(static_cast<std::uint64_t>(0)));
               sample.set_filename(zip_check.get_name(static_cast<std::uint64_t>(0)));

               if (!sample.is_saved())
               {
                  sample.save();
                  std::cout << sample.label() << std::endl;
               }
            }
            else
            {
               Logger::Raw(Logger::Level::Info, "error\n");
               throw exception::Exception("Archive is neither a single isolated sample nor a Malexandria archive.");
            }
                                        
            return true;
         }
      };

      struct PushFunction : public Module
      {
         PushFunction()
            : Module("push",
                     "Push samples and analyses in the database to remote SSH targets.")
         {
            this->add_argument("-p", "--password")
               .help("The password to use on the export archive. If none is set, the default config password is used.");
            
            this->add_argument("-c", "--children")
               .help("Upload the children of the samples as well.")
               .default_value(false)
               .implicit_value(true);

            this->add_argument("-P", "--parents")
               .help("Upload the parents of the samples as well.")
               .default_value(false)
               .implicit_value(true);

            this->add_argument("-d", "--descendants")
               .help("Recursively select children and all their children to upload.")
               .default_value(false)
               .implicit_value(true);

            this->add_argument("-A", "--ancestors")
               .help("Recursively select parents and all their parents to upload.")
               .default_value(false)
               .implicit_value(true);

            this->add_argument("-s", "--sample")
               .help("The sample identifiers to push to the given site. Can be specified multiple times.")
               .metavar("IDENTIFIER")
               .append();

            this->add_argument("-a", "--analysis")
               .help("The analysis identifiers to push to the given site. Can be specified multiple times.")
               .metavar("IDENTIFIER")
               .append();

            this->add_argument("site")
               .help("The Malexandria site to upload the samples and analyses to.");
         }

         bool execute(Module &root) {
            auto password = this->present("--password");
            auto export_children = this->get<bool>("--children");
            auto export_parents = this->get<bool>("--parents");
            auto export_descendants = this->get<bool>("--descendants");
            auto export_ancestors = this->get<bool>("--ancestors");
            auto sample_ids = this->present<std::vector<std::string>>("--sample");
            auto analysis_ids = this->present<std::vector<std::string>>("--analysis");
            auto ssh = this->get("site");

            if (!sample_ids.has_value() && !analysis_ids.has_value())
               throw exception::Exception("Must specify either a sample or an analysis.");
            
            std::size_t child_depth = 0, paternal_depth = 0;
            std::vector<Sample> visiting, queue, exporting;
            std::set<std::int64_t> visited, parent_set, child_set;

            if (!password.has_value())
               password = MainConfig::GetInstance().zip_password();

            if (sample_ids.has_value())
               for (auto &id : *sample_ids)
                  queue.push_back(Sample::FromIdentifier(id));

            while (queue.size() > 0)
            {
               visiting = queue;
               queue.clear();

               for (auto &sample : visiting)
               {
                  if (visited.find(sample.row_id()) != visited.end())
                     continue;
                  
                  visited.insert(sample.row_id());

                  Logger::InfoN("adding {} to export set", sample.label());
                  exporting.push_back(sample);

                  auto parents = sample.parents();
                  auto children = sample.children();
                  
                  if ((parent_set.size() == 0 || parent_set.find(sample.row_id()) != parent_set.end()) && ((export_parents && paternal_depth < 1) || export_ancestors))
                  {
                     for (auto &parent : parents)
                     {
                        parent_set.insert(parent.row_id());
                        
                        if (visited.find(parent.row_id()) == visited.end())
                        {
                           MLX_DEBUGN("adding parent {} to queue", parent.label());
                           queue.push_back(parent);
                        }
                     }
                  }
                  
                  if ((child_set.size() == 0 || child_set.find(sample.row_id()) != child_set.end()) && ((export_children && child_depth < 1) || export_descendants))
                  {
                     for (auto &child : children)
                     {
                        child_set.insert(child.row_id());
                        
                        if (visited.find(child.row_id()) == visited.end())
                        {
                           MLX_DEBUGN("adding child {} to queue", child.label());
                           queue.push_back(child);
                        }
                     }
                  }
               }

               ++paternal_depth;
               ++child_depth;
            }

            Logger::InfoN("exporting {} samples and {} analyses", exporting.size(), (analysis_ids.has_value()) ? analysis_ids->size() : 0);
            auto mlx = Export(std::nullopt);

            for (auto &sample : exporting)
               mlx.add_sample(sample);

            if (analysis_ids.has_value())
               for (auto &id : *analysis_ids)
                  mlx.add_analysis(Analysis::FromIdentifier(id));

            mlx.close();
            
            MLX_DEBUGN("finished exporting samples.");

            Logger::InfoN("connecting to {}...", ssh);
            auto session = SSHSession(ssh);
            session.connect();

            Logger::InfoN("authenticating...");
            session.authenticate();

            Logger::InfoN("successfully connected to remote server.");
            MLX_DEBUGN("checking for malexandria...");
               
            if (!session.which("malexandria").has_value())
               throw exception::Exception(fmt::format("Malexandria not installed at {}.", ssh));

            MLX_DEBUGN("malexandria installed!");
               
            Logger::InfoN("importing into remote malexandria instance...");
            auto debug_data = mlx.read();
            std::ofstream debug_fp("debug.mlx", std::ios::binary);
            debug_fp.write(reinterpret_cast<char *>(debug_data.data()), debug_data.size());
            auto import_cmd = fmt::format("malexandria transport import -p \"{}\" -", *password);
            auto result = session.exec(import_cmd, debug_data);
            MLX_DEBUGN("exit code: {}", result.exit_code);
            MLX_DEBUGN("stdout: {}", std::string(result.output.begin(), result.output.end()));
            MLX_DEBUGN("stderr: {}", std::string(result.error.begin(), result.error.end()));

            if (result.exit_code != 0)
               throw exception::RemoteCommandFailure(import_cmd,
                                                     std::string(result.error.begin(), result.error.end()));
               
            Logger::InfoN("imported!");
         
            return true;
         }
      };

      struct PullFunction : public Module
      {
         PullFunction()
            : Module("pull",
                     "Pull samples and analyses from remote SSH sources.")
         {
            this->add_argument("-p", "--password")
               .help("The password to use on the export from the SSH server. If none is specified, the default password from the config is used.");
            
            this->add_argument("-c", "--children")
               .help("Get the children of the given samples as well.")
               .default_value(false)
               .implicit_value(true);

            this->add_argument("-P", "--parents")
               .help("Get the parents of the given samples as well.")
               .default_value(false)
               .implicit_value(true);

            this->add_argument("-d", "--descendants")
               .help("Recursively get children and all their children.")
               .default_value(false)
               .implicit_value(true);

            this->add_argument("-A", "--ancestors")
               .help("Recursively get parents and all their parents.")
               .default_value(false)
               .implicit_value(true);

            this->add_argument("-s", "--sample")
               .help("The sample identifiers to pull from the remote repository. Can be specified multiple times.")
               .metavar("IDENTIFIER")
               .append();

            this->add_argument("-a", "--analysis")
               .help("The analysis identifiers to pull from the remote repository. Can be specified multiple times.")
               .metavar("IDENTIFIER")
               .append();
            
            this->add_argument("site")
               .help("The SSH site to pull from.");
         }

         virtual bool execute(Module &root) {
            auto password = this->present("--password");
            auto download_children = this->get<bool>("--children");
            auto download_parents = this->get<bool>("--parents");
            auto download_descendants = this->get<bool>("--descendants");
            auto download_ancestors = this->get<bool>("--ancestors");
            auto sample_ids = this->present<std::vector<std::string>>("--sample");
            auto analysis_ids = this->present<std::vector<std::string>>("--analysis");
            auto ssh = this->get("site");

            if (!sample_ids.has_value() && !analysis_ids.has_value())
               throw exception::Exception("Must specify a sample or an analysis to pull.");
            
            Logger::InfoN("connecting to mlx site {}...", ssh);
            auto session = SSHSession(ssh);
            session.connect();

            Logger::InfoN("authenticating with site...");
            session.authenticate();

            Logger::InfoN("checking Malexandria installation...");
            
            if (!session.which("malexandria").has_value())
               throw exception::Exception("Malexandria not found on remote site.");

            MLX_DEBUGN("building export commandline...");

            if (!password.has_value())
               password = MainConfig::GetInstance().zip_password();
            
            auto remote_temp = session.temp_file();
            std::string commandline = fmt::format("malexandria transport export --filename - --password \"{}\"",
                                                  *password);

            if (download_children)
               commandline = fmt::format("{} --children", commandline);

            if (download_parents)
               commandline = fmt::format("{} --parents", commandline);

            if (download_descendants)
               commandline = fmt::format("{} --descendants", commandline);

            if (download_ancestors)
               commandline = fmt::format("{} --ancestors", commandline);

            if (sample_ids.has_value())
               for (auto &id : *sample_ids)
                  commandline = fmt::format("{} --sample \"{}\"", commandline, id);

            if (analysis_ids.has_value())
               for (auto &analysis : *analysis_ids)
                  commandline = fmt::format("{} --analysis \"{}\"", commandline, analysis);

            MLX_DEBUGN("built commandline: {}", commandline);
            Logger::InfoN("exporting samples in remote site...");
            auto result = session.exec(commandline);
            MLX_DEBUGN("exit code: {}", result.exit_code);
            MLX_DEBUGN("stderr: {}", std::string(result.error.begin(), result.error.end()));

            if (result.exit_code != 0)
               throw exception::RemoteCommandFailure(commandline, std::string(result.error.begin(), result.error.end()));

            auto export_archive = Export(result.output);
            export_archive.set_password(*password);

            Logger::InfoN("disconnecting from ssh site...");
            session.disconnect();

            Logger::InfoN("importing downloaded samples...");
            export_archive.import_samples();

            Logger::InfoN("importing downloaded analyses...");
            export_archive.import_analyses();

            return true;
         }
      };

      struct DownloadFunction : public Module
      {
         DownloadFunction()
            : Module("download",
                     "Download samples from various sources.")
         {
            this->add_argument("-m", "--malware-bazaar")
               .help("The SHA256 hash to download from Malware Bazaar")
               .metavar("SHA256")
               .required();
         }

         virtual bool execute(Module &root) {
            auto sample = this->get("--malware-bazaar");

            HTTPRequest request("https://mb-api.abuse.ch/api/v1");
            request.set_post_data(fmt::format("query=get_file&sha256_hash={}", sample));

            auto result = request.perform();

            if (result != CURLE_OK)
               throw exception::Exception(fmt::format("CURL error: {}", curl_easy_strerror(result)));

            auto headers = request.received_headers_map();

            if (headers.find("Content-Type") != headers.end() && headers["Content-Type"] != "application/zip")
               throw exception::Exception(fmt::format("Hash not found on Malware Bazaar: {}", sample));

            auto archive = Zip(request.get_data(), ZIP_RDONLY);
            archive.set_password("infected");

            Sample sample_obj;

            sample_obj.load_data(archive.extract_to_memory(static_cast<std::uint64_t>(0)));
            sample_obj.set_filename(archive.get_name(static_cast<std::uint64_t>(0)));

            if (!sample_obj.is_saved())
            {
               sample_obj.save();
               std::cout << sample_obj.label() << std::endl;
            }

            return true;
         }
      };

      TransportModule()
         : Module("transport",
                  "Download, upload, import and export samples and analyses in various ways.",
                  {
                     std::make_shared<ExportFunction>(),
                     std::make_shared<ImportFunction>(),
                     std::make_shared<PushFunction>(),
                     std::make_shared<PullFunction>(),
                     std::make_shared<DownloadFunction>()
                  })
      {}
   };
}

#endif
