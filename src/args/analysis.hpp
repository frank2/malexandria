#ifndef __MALEXANDRIA_ARGS_ANALYSIS_HPP
#define __MALEXANDRIA_ARGS_ANALYSIS_HPP

#include "args/module.hpp"
#include "../analysis.hpp"

namespace malexandria
{
   struct AnalysisModule : public Module
   {
      struct CreateFunction : public Module
      {
         CreateFunction()
            : Module("create",
                     "Create an analysis repository.")
         {
            this->add_argument("-a", "--alias")
               .help("The alias to give the analysis.");
            
            this->add_argument("-s", "--sample")
               .help("The samples to add to the analysis.")
               .metavar("IDENTIFIER")
               .append();
            
            this->add_argument("path")
               .help("The path to create the analysis in. If a path is not provided, a new directory in the active directory will be created.")
               .nargs(0, 1);
         }

         bool execute(Module &root) {
            auto alias = this->present("--alias");
            auto path = this->present("path");
            auto samples = this->present<std::vector<std::string>>("--sample");
            auto analysis = Analysis();
            std::filesystem::path open_path;

            if (alias.has_value())
               analysis.set_alias(*alias);

            if (path.has_value())
               open_path = analysis.create(*path);
            else
               open_path = analysis.create(std::nullopt);

            if (samples.has_value())
            {
               for (auto &ident : *samples)
               {
                  auto sample = Sample::FromIdentifier(ident);

                  if (!sample.is_saved())
                     sample.save();

                  analysis.add_sample(sample, std::nullopt);
               }

               analysis.save_config();
            }

            std::cout << open_path.make_preferred().string() << std::endl;

            return true;
         }
      };

      class AddFunction : public Module
      {
      public:
         AddFunction()
            : Module("add",
                     "Add files, artifacts, samples, and links to other analysis files.")
         {
            this->add_argument("-a", "--analysis")
               .help("The identifier of the analysis to add to. If not present, the directory of the file argument will be searched.")
               .metavar("IDENTIFIER");
            
            this->add_argument("-s", "--sample")
               .help("Add a sample to the analysis. If the filename argument is present, it becomes the active filename of the added sample.")
               .metavar("IDENTIFIER");
               
            this->add_argument("-A", "--artifact")
               .help("The sample identifier the given file is an artifact of.")
               .metavar("IDENTIFIER");

            this->add_argument("-l", "--link")
               .help("The analysis identifier and analysis-local filename to link to the given file.")
               .metavar("IDENTIFIER FILENAME")
               .nargs(2);

            this->add_argument("filename")
               .help("The file to add to the analysis.")
               .nargs(0, 1);
         }

         bool execute(Module &root) {
            auto analysis = this->present("--analysis");
            auto artifact = this->present("--artifact");
            auto link = this->present<std::vector<std::string>>("--link");
            auto sample = this->present("--sample");
            auto filename = this->present("filename");

            Analysis found_analysis;

            if (analysis.has_value())
               found_analysis = Analysis::FromIdentifier(*analysis);
            else {
               auto found_config = Analysis::FindAnalysis(filename);

               if (!found_config.has_value())
                  throw exception::AnalysisNotFound();

               found_analysis = Analysis(*found_config);
            }

            if (!found_analysis.is_open())
               throw exception::AnalysisNotOpen(uuids::to_string(found_analysis.id()));
         
            if (sample.has_value())
            {
               auto sample_obj = Sample::FromIdentifier(*sample);

               if (!sample_obj.is_saved())
                  throw exception::SampleNotSaved();

               if (!filename.has_value())
                  found_analysis.add_sample(sample_obj, std::nullopt);
               else
                  found_analysis.add_sample(sample_obj, found_analysis.disk_to_relative(*filename));
            }
            else if (link.has_value())
               throw exception::Exception("not yet");
            else {
               if (!filename.has_value())
                  throw exception::Exception("No filename provided.");

               if (artifact.has_value())
               {
                  auto sample_obj = Sample::FromIdentifier(*artifact);

                  if (!sample_obj.is_saved())
                     throw exception::SampleNotSaved();

                  found_analysis.add_file(found_analysis.disk_to_relative(*filename), sample_obj);
               }
               else
                  found_analysis.add_file(found_analysis.disk_to_relative(*filename));
            }

            found_analysis.save_config();

            return true;
         }
      };

      class SaveFunction : public Module
      {
      public:
         SaveFunction()
            : Module("save",
                     "Save a given analysis to the Malexandria database.")
         {
            this->add_argument("analysis")
               .help("The identifier of the analysis to add to. If not present, the current directory is searched.")
               .nargs(0, 1);
         }

         bool execute(Module &root) {
            auto analysis = this->present("analysis");
            Analysis found_analysis;

            if (analysis.has_value())
               found_analysis = Analysis::FromIdentifier(*analysis);
            else {
               auto config_file = Analysis::FindAnalysis();

               if (!config_file.has_value())
                  throw exception::AnalysisNotFound();

               found_analysis = Analysis(*config_file);
            }

            found_analysis.save();

            return true;
         }
      };
          
      struct CloseFunction : public Module
      {
         CloseFunction()
            : Module("close",
                     "Close an open analysis.")
         {
            this->add_argument("analysis")
               .help("The analysis identifier for the analysis to close. If one is not specified, the current directory will be searched.")
               .nargs(0, 1);
         }

         bool execute(Module &root) {
            auto analysis = this->present("analysis");
            Analysis found_analysis;

            if (analysis.has_value())
               found_analysis = Analysis::FromIdentifier(*analysis);
            else
            {
               auto config_file = Analysis::FindAnalysis();

               if (!config_file.has_value())
                  throw exception::AnalysisNotFound();

               found_analysis = Analysis(*config_file);
            }

            found_analysis.close();
            
            return true;
         }
      };

      struct EraseFunction : public Module
      {
         EraseFunction()
            : Module("erase",
                     "Remove an analysis from the database.")
         {
            this->add_argument("analysis")
               .help("The analysis identifier for the analysis to close. If one is not specified, the current directory will be searched.")
               .nargs(0, 1);
         }

         bool execute(Module &root) {
            auto analysis = this->present("analysis");
            Analysis found_analysis;

            if (analysis.has_value())
               found_analysis = Analysis::FromIdentifier(*analysis);
            else
            {
               auto config_file = Analysis::FindAnalysis();

               if (!config_file.has_value())
                  throw exception::AnalysisNotFound();

               found_analysis = Analysis(*config_file);
            }

            found_analysis.erase();
            
            return true;
         }
      };

      struct RemoveFunction : public Module
      {
         RemoveFunction()
            : Module("remove",
                     "Remove a file from the analysis.")
         {
            this->add_argument("-a", "--analysis")
               .help("The analysis identifier for the analysis to close. If one is not specified, the directory of the filename will be searched.")
               .metavar("IDENTIFIER");

            this->add_argument("-p", "--preserve")
               .help("Preserve the underlying file instead of deleting it. Does not apply to samples.")
               .default_value(false)
               .implicit_value(true);

            this->add_argument("filename")
               .help("The file to remove from the analysis. This can also be a sample identifier to remove a sample.");
         }

         bool execute(Module &root) {
            auto analysis = this->present("--analysis");
            auto preserve = this->get<bool>("--preserve");
            auto filename = this->get("filename");
            Analysis found_analysis;

            if (analysis.has_value())
               found_analysis = Analysis::FromIdentifier(*analysis);
            else
            {
               auto config_file = Analysis::FindAnalysis(filename);

               if (!config_file.has_value())
                  throw exception::AnalysisNotFound();

               found_analysis = Analysis(*config_file);
            }

            if (path_exists(filename))
            {
               auto local_file = found_analysis.disk_to_relative(filename);

               if (found_analysis.is_sample(local_file))
               {
                  auto active_map = found_analysis.active_map();
                  auto taint_map = found_analysis.taint_map();
                  std::vector<std::uint8_t> hash;

                  if (active_map.find(local_file) != active_map.end())
                     hash = active_map[local_file];
                  else
                     hash = taint_map[local_file];

                  found_analysis.remove_sample(found_analysis.get_sample(hash));
               }
               else if (found_analysis.has_file(local_file))
                  found_analysis.remove_file(local_file);
               else
                  throw exception::FileNotAnalyzed(local_file.string());

               if (!preserve)
               {
                  auto absolute = found_analysis.relative_to_disk(local_file);

                  if (path_exists(absolute))
                     erase_file(absolute);
               }
            }
            else
            {
               auto sample = Sample::FromIdentifier(filename);
               found_analysis.remove_sample(sample);
            }

            found_analysis.save_config();
            
            return true;
         }
      };

      struct RenameFunction : public Module
      {
         RenameFunction()
            : Module("rename",
                     "Rename a file in an analysis.")
         {
            this->add_argument("-a", "--analysis")
               .help("The analysis identifier for the analysis to close. If one is not specified, the directory of the original filename will be searched.")
               .metavar("IDENTIFIER");

            this->add_argument("original_filename")
               .help("The file to rename.");

            this->add_argument("new_filename")
               .help("The new filename to give the file. When renaming active samples, this argument not being present resets it to the database name.")
               .nargs(0, 1);
         }

         bool execute(Module &root) {
            auto analysis = this->present("--analysis");
            auto original = this->get("original_filename");
            auto new_filename = this->present("new_filename");
            
            Analysis found_analysis;
            
            if (analysis.has_value())
               found_analysis = Analysis::FromIdentifier(*analysis);
            else
            {
               auto config_file = Analysis::FindAnalysis(original);

               if (!config_file.has_value())
                  throw exception::AnalysisNotFound();

               found_analysis = Analysis(*config_file);
            }

            auto relative_old = found_analysis.disk_to_relative(original);

            if (!found_analysis.is_sample(relative_old) && !new_filename.has_value())
               throw exception::Exception("Original file is not a sample and new filename is not present.");
            else if (found_analysis.is_sample(relative_old))
            {
               auto active_map = found_analysis.active_map();
               auto taint_map = found_analysis.taint_map();

               if (taint_map.find(relative_old) != taint_map.end())
                  throw exception::TaintFileRename();

               auto &hash = active_map[relative_old];
               auto &sample = found_analysis.get_sample(hash);
               found_analysis.rename_active_file(sample, new_filename);
            }
            else if (found_analysis.has_file(relative_old))
            {
               auto absolute_old = found_analysis.relative_to_disk(relative_old);
               auto relative_new = found_analysis.disk_to_relative(*new_filename);
               auto absolute_new = found_analysis.relative_to_disk(relative_new);
               found_analysis.rename_file(relative_old, relative_new);
            }

            found_analysis.save_config();

            return true;
         }
      };

      struct OpenFunction : public Module
      {
         OpenFunction()
            : Module("open",
                     "Open an analysis for inspection or modification.")
         {
            this->add_argument("analysis")
               .help("The analysis identifier for the analysis to open.")
               .required();

            this->add_argument("path")
               .help("The path to extract the analysis to. If not present, the label of the analysis will be opened in the active directory.")
               .nargs(0, 1);

            /* TODO link strategies */
         }

         bool execute(Module &root) {
            auto analysis = this->get("analysis");
            auto path = this->present("path");
            
            Analysis found_analysis;

            found_analysis = Analysis::FromIdentifier(analysis);
            auto result = found_analysis.open(path);
            
            std::cout << result.string() << std::endl;
            
            return true;
         }
      };

      struct RestoreFunction : public Module
      {
         RestoreFunction()
            : Module("restore",
                     "Restore an analysis file from the database.")
         {
            this->add_argument("-a", "--analysis")
               .help("The analysis identifier to restore the file from. If one is not specified, the directory of the filename will be searched.")
               .metavar("IDENTIFIER");

            this->add_argument("-s", "--sample")
               .help("Restores the file from the sample database instead of the analysis database. Only applicable to sample files.")
               .default_value(false)
               .implicit_value(true);

            this->add_argument("filename")
               .help("The file in the analysis to restore. This can also be a sample identifier to restore a sample file.");
         }

         bool execute(Module &root) {
            auto analysis = this->present("--analysis");
            auto sample = this->get<bool>("--sample");
            auto filename = this->get("filename");
            Analysis found_analysis;

            if (analysis.has_value())
               found_analysis = Analysis::FromIdentifier(*analysis);
            else
            {
               auto config_file = Analysis::FindAnalysis(filename);

               if (!config_file.has_value())
                  throw exception::AnalysisNotFound();

               found_analysis = Analysis(*config_file);
            }

            auto relative = found_analysis.disk_to_relative(filename);

            if (found_analysis.has_file(relative) || found_analysis.is_sample(relative))
               found_analysis.restore_file(relative);
            else if (found_analysis.is_sample(relative))
            {
               auto active_map = found_analysis.active_map();
               auto taint_map = found_analysis.taint_map();
               std::vector<std::uint8_t> hash;

               if (active_map.find(relative) != active_map.end())
                  hash = active_map[relative];
               else
                  hash = taint_map[relative];

               if (sample)
                  found_analysis.restore_sample(found_analysis.get_sample(hash));
               else
                  found_analysis.restore_taintable_sample(found_analysis.get_sample(hash));
            }
            else
            {
               auto sample_obj = Sample::FromIdentifier(filename);

               if (sample)
                  found_analysis.restore_sample(sample_obj);
               else
                  found_analysis.restore_taintable_sample(sample_obj);
            }

            return true;
         }
      };

      AnalysisModule()
         : Module("analysis",
                  "Manage data related to the analysis of a sample.",
                  {
                     std::make_shared<CreateFunction>(),
                     std::make_shared<AddFunction>(),
                     std::make_shared<SaveFunction>(),
                     std::make_shared<OpenFunction>(),
                     std::make_shared<CloseFunction>(),
                     std::make_shared<EraseFunction>(),
                     std::make_shared<RemoveFunction>(),
                     std::make_shared<RestoreFunction>(),
                     std::make_shared<RenameFunction>()
                  })
      {}
   };
}

#endif
