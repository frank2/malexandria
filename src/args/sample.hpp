#ifndef __MALEXANDRIA_ARGS_SAMPLE_HPP
#define __MALEXANDRIA_ARGS_SAMPLE_HPP

#include <cstdlib>
#include <set>

#include "exception.hpp"
#include "utility.hpp"

#include "args/module.hpp"
#include "export.hpp"
#include "logger.hpp"
#include "../sample.hpp"
#include "ssh.hpp"
#include "zip.hpp"

namespace malexandria
{
   struct SampleModule : public Module
   {
      struct AddFunction : public Module
      {
         AddFunction()
            : Module("add",
                     "Add a sample to the database.",
                     {})
         {
            this->add_argument("-a", "--alias")
               .help("Set the corresponding sample's alias.")
               .metavar("NAME");
       
            this->add_argument("-t", "--tag")
               .help("Give this sample one (or many) tags.")
               .metavar("NAME")
               .append();

            this->add_argument("-f", "--family")
               .help("Give this sample one (or many) families.")
               .metavar("NAME")
               .append();

            this->add_argument("-p", "--parent")
               .help("Give this sample one (or many) parents.")
               .metavar("IDENTIFIER")
               .append();

            this->add_argument("-c", "--child")
               .help("Give this sample one (or many) children.")
               .metavar("IDENTIFIER")
               .append();

            this->add_argument("sample")
               .help("The filename of the sample to import.")
               .required();
         }

         virtual bool execute(Module &root) {
            auto alias = this->present("--alias");
            auto tags = this->present<std::vector<std::string>>("--tag");
            auto families = this->present<std::vector<std::string>>("--family");
            auto parents = this->present<std::vector<std::string>>("--parent");
            auto children = this->present<std::vector<std::string>>("--child");
            auto sample = this->get<std::string>("sample");

            MLX_DEBUGN("sample add:");

            if (alias.has_value())
               MLX_DEBUGN("   alias: {}", *alias);

            if (tags.has_value())
               for (auto &tag : *tags)
                  MLX_DEBUGN("   tag: {}", tag);

            if (families.has_value())
               for (auto &family : *families)
                  MLX_DEBUGN("   family: {}", family);

            if (parents.has_value())
               for (auto &parent : *parents)
                  MLX_DEBUGN("   parent: {}", parent);

            if (children.has_value())
               for (auto &child : *children)
                  MLX_DEBUGN("   child: {}", child);

            MLX_DEBUGN("   sample: {}", sample);
            
            Logger::Info("loading {}...", sample);
            auto sample_object = Sample::FromFile(sample);
            Logger::Raw(Logger::Level::Info, "got {}\n", sample_object.label());

            if (sample_object.is_saved())
               throw exception::SampleExists(to_hex_string(sample_object.sha256()));

            Logger::Info("saving sample to database...");
            sample_object.save();
            Logger::Raw(Logger::Level::Info, "sample id: {}\n", sample_object.row_id());
            
            Logger::InfoN("sample info:");
            Logger::InfoN("   filename: {}", sample_object.filename());
            Logger::InfoN("   md5:      {}", to_hex_string(sample_object.md5()));
            Logger::InfoN("   sha1:     {}", to_hex_string(sample_object.sha1()));
            Logger::InfoN("   sha256:   {}", to_hex_string(sample_object.sha256()));
            Logger::InfoN("   sha3-384: {}", to_hex_string(sample_object.sha3_384()));

            bool aux = false;

            // paternal relationships need to be established first.
            if (parents.has_value())
            {
               aux = true;
               
               for (auto &parent : *parents)
               {
                  Logger::Info("loading parent identifier {}...", parent);
                  auto parent_object = Sample::FromIdentifier(parent);
                  Logger::Raw(Logger::Level::Info, "got {}\n", parent_object.label());

                  if (!parent_object.is_saved())
                  {
                     MLX_DEBUGN("fatal exception, deleting sample...");
                     sample_object.erase();
                     throw exception::SampleNotSaved();
                  }

                  Logger::Info("declaring {} a parent of {}...", parent_object.label(), sample_object.label());
                  sample_object.add_parent(parent_object);
                  Logger::Raw(Logger::Level::Info, "done.\n");
               }
            }

            // paternal relationships need to be established first.
            if (children.has_value())
            {
               aux = true;
               
               for (auto &child : *children)
               {
                  Logger::Info("loading child identifier {}...", child);
                  auto child_object = Sample::FromIdentifier(child);
                  Logger::Raw(Logger::Level::Info, "got {}\n", child_object.label());

                  if (!child_object.is_saved())
                  {
                     MLX_DEBUGN("fatal exception, deleting sample...");

                     sample_object.erase();
                     throw exception::SampleNotSaved();
                  }

                  Logger::Info("declaring {} a child of {}", child_object.label(), sample_object.label());
                  sample_object.add_child(child_object);
                  Logger::Raw(Logger::Level::Info, "done\n");
               }
            }

            if (alias.has_value())
            {
               aux = true;
               
               if (Sample::IDFromAlias(*alias).has_value())
               {
                  MLX_DEBUGN("fatal exception, deleting sample...");
                  sample_object.erase();
                  throw exception::AliasAlreadyExists(*alias);
               }

               Logger::InfoN("setting alias for {}", to_hex_string(sample_object.sha256()));
               sample_object.set_alias(*alias);
            }

            if (tags.has_value())
            {
               aux = true;
               
               for (auto &tag : *tags)
               {
                  Logger::InfoN("adding tag {} to {}", tag, sample_object.label());
                  sample_object.add_tag(tag);
               }
            }

            if (families.has_value())
            {
               aux = true;
               
               for (auto &family : *families)
               {
                  Logger::InfoN("adding family {} to {}", family, sample_object.label());
                  sample_object.add_family(family);
               }
            }

            if (aux)
            {
               Logger::InfoN("saving changes to database...");
               sample_object.save();
               Logger::InfoN("done.");
            }

            std::cout << sample_object.label() << std::endl;

            return true;
         }
      };

      struct ModifyFunction : public Module
      {
         ModifyFunction()
            : Module("modify",
                     "Modify a sample's attributes in the database.",
                     {})
         {
            this->add_argument("-d", "--delete")
               .help("Delete the given attributes instead of adding them.")
               .default_value(false)
               .implicit_value(true);
            
            this->add_argument("-a", "--alias")
               .help("Set the corresponding sample's alias. Give no name when deleting.")
               .metavar("NAME")
               .nargs(0, 1);
                         
            this->add_argument("-t", "--tag")
               .help("The tags to add or delete. Give no tag when deleting to delete all tags.")
               .metavar("NAME")
               .nargs(0, 1)
               .append();
                              
            this->add_argument("-f", "--family")
               .help("The families to add or delete. Give no family when deleting to delete all families.")
               .metavar("NAME")
               .nargs(0, 1)
               .append();
               
            this->add_argument("-p", "--parent")
               .help("The parents to add or delete. Give no parent when deleting to delete all parents.")
               .metavar("IDENTIFIER")
               .nargs(0, 1)
               .append();

            this->add_argument("-c", "--child")
               .help("The children to add or delete. Give no child when deleting to delete all children.")
               .metavar("IDENTIFIER")
               .nargs(0, 1)
               .append();

            this->add_argument("-F", "--filename")
               .help("The filename to give the sample when it is active. It cannot be deleted.")
               .metavar("FILENAME");

            this->add_argument("sample")
               .help("The identifier of the sample to modify.")
               .required();
         }

         virtual bool execute(Module &root) {
            auto del = this->get<bool>("--delete");
            auto alias = this->present("--alias");
            auto tags = this->present<std::vector<std::string>>("--tag");
            auto families = this->present<std::vector<std::string>>("--family");
            auto parents = this->present<std::vector<std::string>>("--parent");
            auto children = this->present<std::vector<std::string>>("--child");
            auto filename = this->present<std::string>("--filename");
            auto sample = this->get<std::string>("sample");

            MLX_DEBUGN("sample modify:");
            MLX_DEBUGN("   delete: {}", del);

            if (alias.has_value())
               MLX_DEBUGN("   alias: {}", *alias);
            else if (del && this->is_used("--alias"))
               MLX_DEBUGN("   deleting alias");

            if (tags.has_value())
            {
               for (auto &tag : *tags)
                  MLX_DEBUGN("   tag: {}", tag);
            }
            else if (del && this->is_used("--tag"))
               MLX_DEBUGN("   deleting tags");

            if (families.has_value())
            {
               for (auto &family : *families)
                  MLX_DEBUGN("   family: {}", family);
            }
            else if (del && this->is_used("--family"))
               MLX_DEBUGN("   deleting families");

            if (parents.has_value())
            {
               for (auto &parent : *parents)
                  MLX_DEBUGN("   parent: {}", parent);
            }
            else if (del && this->is_used("--parent"))
               MLX_DEBUGN("   deleting parents");

            if (children.has_value())
            {
               for (auto &child : *children)
                  MLX_DEBUGN("   child: {}", child);
            }
            else if (del && this->is_used("--child"))
               MLX_DEBUGN("   deleting children");

            if (filename.has_value())
               MLX_DEBUGN("   filename: {}", *filename);
           
            MLX_DEBUGN("   sample: {}", sample);
            
            Logger::Info("loading {}...", sample);
            auto sample_object = Sample::FromIdentifier(sample);
            Logger::Raw(Logger::Level::Info, "got {}\n", sample_object.label());
            
            Logger::InfoN("sample info:");

            if (sample_object.has_alias())
               Logger::InfoN("   alias:    {}", sample_object.alias());

            Logger::InfoN("   filename: {}", sample_object.filename());
            Logger::InfoN("   md5:      {}", to_hex_string(sample_object.md5()));
            Logger::InfoN("   sha1:     {}", to_hex_string(sample_object.sha1()));
            Logger::InfoN("   sha256:   {}", to_hex_string(sample_object.sha256()));
            Logger::InfoN("   sha3-384: {}", to_hex_string(sample_object.sha3_384()));

            for (auto &tag : sample_object.tags())
               Logger::InfoN("   tag:      {}", *tag);

            for (auto &family : sample_object.families())
               Logger::InfoN("   family:   {}", *family);

            for (auto &parent : sample_object.parents())
               Logger::InfoN("   parent:   {}", parent.label());

            for (auto &child : sample_object.children())
               Logger::InfoN("   child:    {}", child.label());

            if (!sample_object.is_saved())
               throw exception::SampleNotSaved();

            bool opt = false;

            // paternal relationships need to be established first.
            if (parents.has_value())
            {
               opt = true;
               
               for (auto &parent : *parents)
               {
                  Logger::Info("loading parent identifier {}...", parent);
                  auto parent_object = Sample::FromIdentifier(parent);
                  Logger::Raw(Logger::Level::Info, "got {}\n", parent_object.label());

                  if (!parent_object.is_saved())
                     throw exception::SampleNotSaved();

                  if (del)
                  {
                     Logger::Info("removing {} as parent from {}...", parent_object.label(), sample_object.label());
                     sample_object.remove_parent(parent_object);
                     Logger::Raw(Logger::Level::Info, "done.\n");
                  }
                  else
                  {
                     Logger::Info("declaring {} a parent of {}", parent_object.label(), sample_object.label());
                     sample_object.add_parent(parent_object);
                     Logger::Raw(Logger::Level::Info, "done.\n");
                  }
               }
            }
            else if (del && this->is_used("--parent"))
            {
               opt = true;
               
               Logger::Info("removing all parents from {}...", sample_object.label());
               sample_object.remove_parents();
               Logger::Raw(Logger::Level::Info, "done.\n");
            }

            if (children.has_value())
            {
               opt = true;
               
               for (auto &child : *children)
               {
                  Logger::Info("loading child identifier {}...", child);
                  auto child_object = Sample::FromIdentifier(child);
                  Logger::Raw(Logger::Level::Info, "got {}", child_object.label());

                  if (!child_object.is_saved())
                     throw exception::SampleNotSaved();

                  if (del)
                  {
                     Logger::Info("removing {} as child from {}...", child_object.label(), sample_object.label());
                     sample_object.remove_child(child_object);
                     Logger::Raw(Logger::Level::Info, "done.\n");
                  }
                  else
                  {
                     Logger::Info("declaring {} a child of {}...", child_object.label(), sample_object.label());
                     sample_object.add_child(child_object);
                     Logger::Raw(Logger::Level::Info, "done.\n");
                  }
               }
            }
            else if (del && this->is_used("--child"))
            {
               opt = true;
               
               Logger::Info("removing all children from {}...", sample_object.label());
               sample_object.remove_children();
               Logger::Raw(Logger::Level::Info, "done.\n");
            }

            if (alias.has_value())
            {
               opt = true;

               if (Sample::IDFromAlias(*alias))
                  throw exception::AliasAlreadyExists(*alias);

               Logger::Info("setting alias for {}...", to_hex_string(sample_object.sha256()));
               sample_object.set_alias(*alias);
               Logger::Raw(Logger::Level::Info, "done.\n");
            }
            else if (del && this->is_used("--alias"))
            {
               opt = true;

               Logger::Info("removing alias for {}...", sample_object.label());
               sample_object.remove_alias();
               Logger::Raw(Logger::Level::Info, "done.\n");
            }
            
            if (tags.has_value())
            {
               opt = true;
               
               for (auto &tag : *tags)
               {
                  if (del)
                  {
                     Logger::InfoN("deleting tag {} from {}", tag, sample_object.label());
                     sample_object.remove_tag(tag);
                  }
                  else
                  {
                     Logger::InfoN("adding tag {} to {}", tag, sample_object.label());
                     sample_object.add_tag(tag);
                  }
               }
            }
            else if (del && this->is_used("--tag"))
            {
               opt = true;
               
               Logger::Info("removing all tags from {}...", sample_object.label());
               sample_object.remove_tags();
               Logger::Raw(Logger::Level::Info, "done.\n");
            }

            if (families.has_value())
            {
               opt = true;
               
               for (auto &family : *families)
               {
                  if (del)
                  {
                     Logger::InfoN("deleting family {} from {}", family, sample_object.label());
                     sample_object.remove_family(family);
                  }
                  else
                  {
                     Logger::InfoN("adding family {} to {}", family, sample_object.label());
                     sample_object.add_family(family);
                  }
               }
            }
            else if (del && this->is_used("--family"))
            {
               opt = true;
               
               Logger::Info("removing all families from {}...", sample_object.label());
               sample_object.remove_families();
               Logger::Raw(Logger::Level::Info, "done.\n");
            }

            if (filename.has_value())
            {
               if (del)
                  throw exception::Exception("Can't delete filename.");

               opt = true;
               
               Logger::Info("setting filename to {}...", *filename);
               sample_object.set_filename(*filename);
               Logger::Raw(Logger::Level::Info, "done.\n");
            }

            if (opt)
            {
               Logger::Info("saving changes to database...");
               sample_object.save();
               Logger::Raw(Logger::Level::Info, "done.\n");
            }
            else
            {
               throw exception::Exception("No options selected.");
            }

            return true;
         }
      };

      struct EraseFunction : public Module
      {
         EraseFunction()
            : Module("erase",
                     "Erase samples from the database.",
                     {})
         {
            this->add_argument("-c", "--children")
               .help("Remove the children of this sample as well.")
               .default_value(false)
               .implicit_value(true);

            this->add_argument("-p", "--parents")
               .help("Remove the parents of this sample as well.")
               .default_value(false)
               .implicit_value(true);

            this->add_argument("-d", "--descendants")
               .help("Recursively remove children and all their children.")
               .default_value(false)
               .implicit_value(true);

            this->add_argument("-a", "--ancestors")
               .help("Recursively remove parents and all their parents.")
               .default_value(false)
               .implicit_value(true);

            this->add_argument("sample")
               .help("The sample identifiers to remove from the database.")
               .required()
               .remaining();
         }

         virtual bool execute(Module &root)
         {
            auto remove_children = this->get<bool>("--children");
            auto remove_parents = this->get<bool>("--parents");
            auto remove_descendants = this->get<bool>("--descendants");
            auto remove_ancestors = this->get<bool>("--ancestors");
            auto samples = this->get<std::vector<std::string>>("sample");

            if (samples.size() == 0)
               throw exception::Exception("No samples to remove");

            std::set<std::vector<std::uint8_t>> delete_set;

            for (auto &sample : samples)
            {
               Logger::Info("loading sample {}...", sample);
               auto sample_object = Sample::FromIdentifier(sample);
               Logger::Raw(Logger::Level::Info, "got {}\n", sample_object.label());

               if (!sample_object.is_saved())
                  throw exception::SampleNotSaved();

               Logger::InfoN("adding {} to delete set...", sample_object.label());
               delete_set.insert(sample_object.sha256());

               if (remove_descendants)
               {
                  std::set<std::vector<std::uint8_t>> visited;
                  std::vector<Sample> visiting;

                  Logger::InfoN("visiting all descendants of {}", sample_object.label());
                  visiting = sample_object.children();

                  while (visiting.size() > 0)
                  {
                     auto descendant = visiting.front();
                     visiting.erase(visiting.begin());

                     if (visited.find(descendant.sha256()) != visited.end())
                        continue;

                     Logger::InfoN("visiting {}", descendant.label());
                     visited.insert(descendant.sha256());

                     MLX_DEBUGN("getting children of {}", descendant.label());
                     auto children = descendant.children();

                     MLX_DEBUGN("adding {} children", children.size());
                     visiting.insert(visiting.end(), children.begin(), children.end());
                  }

                  if (visited.size() > 0)
                  {
                     MLX_DEBUG("adding {} visited samples to erase set...", visited.size());
                     delete_set.insert(visited.begin(), visited.end());
                     Logger::Raw(Logger::Level::Debug, "done.\n");
                  }
               }
               else if (remove_children)
               {
                  auto children = sample_object.children();
                  Logger::InfoN("removing {} children", children.size());
                  
                  for (auto &child : children)
                     delete_set.insert(child.sha256());
               }

               if (remove_ancestors)
               {
                  std::set<std::vector<std::uint8_t>> visited;
                  std::vector<Sample> visiting;

                  Logger::InfoN("visiting all ancestors of {}", sample_object.label());
                  visiting = sample_object.parents();

                  while (visiting.size() > 0)
                  {
                     auto ancestor = visiting.front();
                     visiting.erase(visiting.begin());

                     if (visited.find(ancestor.sha256()) != visited.end())
                        continue;

                     Logger::InfoN("visiting {}", ancestor.label());
                     visited.insert(ancestor.sha256());

                     MLX_DEBUGN("getting parents of {}", ancestor.label());
                     auto parents = ancestor.parents();

                     MLX_DEBUGN("adding {} parents", parents.size());
                     visiting.insert(visiting.end(), parents.begin(), parents.end());
                  }

                  if (visited.size() > 0)
                  {
                     MLX_DEBUG("adding {} visited samples to erase set...", visited.size());
                     delete_set.insert(visited.begin(), visited.end());
                     Logger::Raw(Logger::Level::Debug, "done.\n");
                  }
               }
               else if (remove_parents)
               {
                  auto parents = sample_object.parents();
                  Logger::InfoN("removing {} parents", parents.size());
                  
                  for (auto &parent : parents)
                     delete_set.insert(parent.sha256());
               }
            }

            Logger::InfoN("found {} candidates for deletion.", delete_set.size());

            for (auto &hash : delete_set)
            {
               Logger::InfoN("loading {}...", to_hex_string(hash));
               auto sample = Sample::ByHash(hash);

               Logger::Info("deleting {}...", sample.label());
               sample.erase();
               Logger::Raw(Logger::Level::Info, "done.\n");
            }

            return true;
         }
      };

      struct ExistsFunction : public Module
      {
         ExistsFunction()
            : Module("exists",
                     "Check if a sample exists in the database.")
         {
            this->add_argument("sample")
               .help("The sample identifier to check for existence in the database.")
               .required();
         }

         virtual bool execute(Module &root) {
            auto sample_ident = this->get("sample");

            try {
               auto sample = Sample::FromIdentifier(sample_ident);

               if (sample.is_saved())
                  Logger::InfoN("sample exists");
               else
                  Logger::InfoN("sample does not exist");

               return sample.is_saved();
            }
            catch (exception::InvalidIdentifier &exc)
            {
               Logger::InfoN("sample does not exist");
               return false;
            }
         }
      };

      struct InfoFunction : public Module
      {
         InfoFunction()
            : Module("info",
                     "Print information related to the given sample identifiers.")
         {
            this->add_argument("-j", "--json")
               .help("Dump the corresponding info in json format.")
               .default_value(false)
               .implicit_value(true);

            this->add_argument("-c", "--children")
               .help("Dump the children of this sample as well.")
               .default_value(false)
               .implicit_value(true);

            this->add_argument("-p", "--parents")
               .help("Dump the parents of this sample as well.")
               .default_value(false)
               .implicit_value(true);

            this->add_argument("-d", "--descendants")
               .help("Recursively dump children and all their children.")
               .default_value(false)
               .implicit_value(true);

            this->add_argument("-a", "--ancestors")
               .help("Recursively dump parents and all their parents.")
               .default_value(false)
               .implicit_value(true);

            /*
            this->add_argument("-n", "--notes")
               .help("If a sample contains analysis, print its notes.")
               .default_value(false)
               .implicit_value(true);
            */
            
            this->add_argument("sample")
               .help("The sample identifiers to print the info of.")
               .remaining()
               .required();
         }

         virtual bool execute(Module &root) {
            auto in_json = this->get<bool>("--json");
            auto print_children = this->get<bool>("--children");
            auto print_parents = this->get<bool>("--parents");
            auto print_descendants = this->get<bool>("--descendants");
            auto print_ancestors = this->get<bool>("--ancestors");
            auto sample_ids = this->get<std::vector<std::string>>("sample");

            json json_result;
            std::size_t child_depth = 0, paternal_depth = 0;
            std::vector<Sample> visiting, queue;
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

                  MLX_DEBUGN("visiting {}", sample.label());
                  visited.insert(sample.row_id());

                  auto parents = sample.parents();
                  auto children = sample.children();

                  if (in_json)
                  {
                     auto hash_id = to_hex_string(sample.sha256());

                     if (sample.has_alias())
                        json_result[hash_id]["alias"] = sample.alias();

                     json_result[hash_id]["filename"] = sample.filename();
                     json_result[hash_id]["hashes"]["md5"] = to_hex_string(sample.md5());
                     json_result[hash_id]["hashes"]["sha1"] = to_hex_string(sample.sha1());
                     json_result[hash_id]["hashes"]["sha256"] = to_hex_string(sample.sha256());
                     json_result[hash_id]["hashes"]["sha3_384"] = to_hex_string(sample.sha3_384());
                     json_result[hash_id]["tags"] = std::vector<std::string>();

                     for (auto &tag : sample.tags())
                        json_result[hash_id]["tags"].push_back(*tag);

                     json_result[hash_id]["families"] = std::vector<std::string>();

                     for (auto &family : sample.families())
                        json_result[hash_id]["families"].push_back(*family);

                     json_result[hash_id]["parents"] = std::vector<std::string>();

                     for (auto &parent : parents)
                        json_result[hash_id]["parents"].push_back(to_hex_string(parent.sha256()));

                     json_result[hash_id]["children"] = std::vector<std::string>();

                     for (auto &child : children)
                        json_result[hash_id]["children"].push_back(to_hex_string(child.sha256()));

                     MLX_DEBUGN("json_result: {}", json_result[hash_id].dump(4));
                  }
                  else
                  {
                     std::cout << fmt::format("[Sample ID: {}]", to_hex_string(sample.sha256())) << std::endl;

                     if (sample.has_alias())
                        std::cout << fmt::format("   alias:             {}", sample.alias()) << std::endl;

                     std::cout << fmt::format("   original filename: {}", sample.filename()) << std::endl;
                     std::cout << fmt::format("   md5:               {}", to_hex_string(sample.md5())) << std::endl;
                     std::cout << fmt::format("   sha1:              {}", to_hex_string(sample.sha1())) << std::endl;
                     std::cout << fmt::format("   sha256:            {}", to_hex_string(sample.sha256())) << std::endl;
                     std::cout << fmt::format("   sha3-384:          {}", to_hex_string(sample.sha3_384())) << std::endl;

                     for (auto &tag : sample.tags())
                        std::cout << fmt::format("   tag:               {}", *tag) << std::endl;

                     for (auto &family : sample.families())
                        std::cout << fmt::format("   family:            {}", *family) << std::endl;

                     for (auto &parent : parents)
                        std::cout << fmt::format("   parent:            {}", parent.label()) << std::endl;

                     for (auto &child : children)
                        std::cout << fmt::format("   child:             {}", child.label()) << std::endl;

                     std::cout << std::endl;
                  }
                  
                  if ((parent_set.size() == 0 || parent_set.find(sample.row_id()) != parent_set.end()) && ((print_parents && paternal_depth < 1) || print_ancestors))
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
                  
                  if ((child_set.size() == 0 || child_set.find(sample.row_id()) != child_set.end()) && ((print_children && child_depth < 1) || print_descendants))
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

            if (in_json)
               std::cout << std::setw(4) << json_result << std::endl;
                           
            return true;
         }
      };

      SampleModule()
         : Module("sample",
                  "Add, configure, remove and list samples stored in the database.",
                  {
                     std::make_shared<AddFunction>(),
                     std::make_shared<ModifyFunction>(),
                     std::make_shared<EraseFunction>(),
                     std::make_shared<ExistsFunction>(),
                     std::make_shared<InfoFunction>()
                  })
      {}
   };
}

#endif
