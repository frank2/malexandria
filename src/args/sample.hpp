#ifndef __MALEXANDRIA_ARGS_SAMPLE_HPP
#define __MALEXANDRIA_ARGS_SAMPLE_HPP

#include <set>

#include "exception.hpp"
#include "utility.hpp"

#include "args/module.hpp"
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

            Logger::DebugN("sample add:");

            if (alias.has_value())
               Logger::DebugN("   alias: {}", *alias);

            if (tags.has_value())
               for (auto &tag : *tags)
                  Logger::DebugN("   tag: {}", tag);

            if (families.has_value())
               for (auto &family : *families)
                  Logger::DebugN("   family: {}", family);

            if (parents.has_value())
               for (auto &parent : *parents)
                  Logger::DebugN("   parent: {}", parent);

            if (children.has_value())
               for (auto &child : *children)
                  Logger::DebugN("   child: {}", child);

            Logger::DebugN("   sample: {}", sample);
            
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
                     Logger::DebugN("fatal exception, deleting sample...");
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
                     Logger::DebugN("fatal exception, deleting sample...");

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
                  Logger::DebugN("fatal exception, deleting sample...");
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

            Logger::DebugN("sample modify:");
            Logger::DebugN("   delete: {}", del);

            if (alias.has_value())
               Logger::DebugN("   alias: {}", *alias);
            else if (del && this->is_used("--alias"))
               Logger::DebugN("   deleting alias");

            if (tags.has_value())
            {
               for (auto &tag : *tags)
                  Logger::DebugN("   tag: {}", tag);
            }
            else if (del && this->is_used("--tag"))
               Logger::DebugN("   deleting tags");

            if (families.has_value())
            {
               for (auto &family : *families)
                  Logger::DebugN("   family: {}", family);
            }
            else if (del && this->is_used("--family"))
               Logger::DebugN("   deleting families");

            if (parents.has_value())
            {
               for (auto &parent : *parents)
                  Logger::DebugN("   parent: {}", parent);
            }
            else if (del && this->is_used("--parent"))
               Logger::DebugN("   deleting parents");

            if (children.has_value())
            {
               for (auto &child : *children)
                  Logger::DebugN("   child: {}", child);
            }
            else if (del && this->is_used("--child"))
               Logger::DebugN("   deleting children");

            if (filename.has_value())
               Logger::DebugN("   filename: {}", *filename);
           
            Logger::DebugN("   sample: {}", sample);
            
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

      struct RemoveFunction : public Module
      {
         RemoveFunction()
            : Module("remove",
                     "Remove samples from the database.",
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

                     Logger::DebugN("getting children of {}", descendant.label());
                     auto children = descendant.children();

                     Logger::DebugN("adding {} children", children.size());
                     visiting.insert(visiting.end(), children.begin(), children.end());
                  }

                  if (visited.size() > 0)
                  {
                     Logger::Debug("adding {} visited samples to erase set...", visited.size());
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

                     Logger::DebugN("getting parents of {}", ancestor.label());
                     auto parents = ancestor.parents();

                     Logger::DebugN("adding {} parents", parents.size());
                     visiting.insert(visiting.end(), parents.begin(), parents.end());
                  }

                  if (visited.size() > 0)
                  {
                     Logger::Debug("adding {} visited samples to erase set...", visited.size());
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

                  Logger::DebugN("visiting {}", sample.label());
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

                     Logger::DebugN("json_result: {}", json_result[hash_id].dump(4));
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
                           Logger::DebugN("adding parent {} to queue", parent.label());
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
                           Logger::DebugN("adding child {} to queue", child.label());
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
      
      struct ExportFunction : public Module
      {
         ExportFunction()
            : Module("export",
                     "Export samples from the database into a portable file.")
         {
            this->add_argument("-f", "--filename")
               .help("The filename to export the archive to. The default is \"[active directory]/export.mlxsample\"");
            
            this->add_argument("-c", "--children")
               .help("Export the children of this sample as well.")
               .default_value(false)
               .implicit_value(true);

            this->add_argument("-p", "--parents")
               .help("Export the parents of this sample as well.")
               .default_value(false)
               .implicit_value(true);

            this->add_argument("-d", "--descendants")
               .help("Recursively export children and all their children.")
               .default_value(false)
               .implicit_value(true);

            this->add_argument("-a", "--ancestors")
               .help("Recursively export parents and all their parents.")
               .default_value(false)
               .implicit_value(true);
            
            this->add_argument("sample")
               .help("The sample identifiers to export.")
               .remaining()
               .required();
         }

         virtual bool execute(Module &root) {
            auto filename = this->present("--filename");
            auto export_children = this->get<bool>("--children");
            auto export_parents = this->get<bool>("--parents");
            auto export_descendants = this->get<bool>("--descendants");
            auto export_ancestors = this->get<bool>("--ancestors");
            auto sample_ids = this->get<std::vector<std::string>>("sample");

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
                           Logger::DebugN("adding parent {} to queue", parent.label());
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
                           Logger::DebugN("adding child {} to queue", child.label());
                           queue.push_back(child);
                        }
                     }
                  }
               }

               ++paternal_depth;
               ++child_depth;
            }

            Logger::InfoN("exporting {} samples", exporting.size());

            if (filename.has_value())
               Logger::InfoN("exporting to {}", *filename);

            auto result = Sample::Export(exporting, filename);

            Logger::InfoN("samples exported to {}", result.string());
                           
            return true;
         }
      };

      struct ImportFunction : public Module
      {
         ImportFunction()
            : Module("import",
                     "Import a sample archive into the database.")
         {
            this->add_argument("-s", "--singleton")
               .help("Import an archive that only contains a malware sample.")
               .default_value(false)
               .implicit_value(true);

            this->add_argument("-m", "--mlx")
               .help("Import a Malexandria sample archive (.mlxsample). Fields such as --filename and --tag are ignored, as they are present in the metadata.")
               .default_value(false)
               .implicit_value(true);

            this->add_argument("-p", "--password")
               .help("The password to use on the sample archive. If no password is supplied, the config's password is used.");

            this->add_argument("-a", "--alias")
               .help("The alias to give this imported sample.")
               .metavar("NAME");
                     
            this->add_argument("-F", "--filename")
               .help("The filename to give the imported sample.")
               .metavar("FILENAME");

            this->add_argument("-t", "--tag")
               .help("The tags to give the imported sample.")
               .metavar("NAME")
               .append();

            this->add_argument("-f", "--family")
               .help("The families to associate with the imported sample.")
               .metavar("NAME")
               .append();

            this->add_argument("-p", "--parent")
               .help("The parent identifiers to give the imported sample.")
               .metavar("IDENTIFIER")
               .append();

            this->add_argument("-c", "--child")
               .help("The child identifiers to give the imported sample.")
               .metavar("IDENTIFIER")
               .append();
            
            this->add_argument("archive")
               .help("The sample archive to import.")
               .required();
         }

         virtual bool execute(Module &root) {
            auto is_singleton = this->get<bool>("--singleton");
            auto is_mlx = this->get<bool>("--mlx");
            auto is_auto = !is_singleton && !is_mlx;
            auto password = this->present("--password");
            auto alias = this->present("--alias");
            auto filename = this->present("--filename");
            auto tags = this->present<std::vector<std::string>>("--tag");
            auto families = this->present<std::vector<std::string>>("--family");
            auto parents = this->present<std::vector<std::string>>("--parent");
            auto children = this->present<std::vector<std::string>>("--child");
            auto archive = this->get("archive");

            auto zip_file = Zip(archive, ZIP_RDONLY);

            if (!password.has_value())
               password = MainConfig::GetInstance().zip_password();
            
            zip_file.set_password(*password);

            if (is_auto)
            {
               Logger::Info("detecting the type of archive file...");
               
               if (zip_file.locate("metadata.json") >= 0)
               {
                  Logger::Raw(Logger::Level::Info, "mlx\n");
                  is_mlx = true;
               }
               else if (zip_file.entries() == 1)
               {
                  Logger::Raw(Logger::Level::Info, "isolated sample\n");
                  is_singleton = true;
               }
               else
               {
                  Logger::Raw(Logger::Level::Info, "error\n");
                  throw exception::Exception("Archive is neither a single isolated sample nor a Malexandria archive.");
               }
            }

            if (is_mlx)
            {
               if (zip_file.locate("metadata.json") < 0)
                  throw exception::Exception("Archive is not a Malexandria archive.");

               Logger::Info("importing mlx archive...");
               zip_file.close();
               Sample::Import(archive, password);
               Logger::Raw(Logger::Level::Info, "done.\n");
            }
            else if (is_singleton)
            {
               auto entries = zip_file.entries();
               
               if (entries != 1)
                  throw exception::Exception("More than one file present in the archive, not an isolated sample.");

               Logger::Info("loading sample data...");
               auto sample = Sample::FromData(zip_file.extract_to_memory(static_cast<std::uint64_t>(0)));
               Logger::Raw(Logger::Level::Info, "done.\n");

               if (sample.is_saved())
               {
                  Logger::InfoN("sample {} already present.", sample.label());
                  return true;
               }

               if (!filename.has_value())
                  filename = zip_file.get_name(0);

               Logger::InfoN("   filename: {}", zip_file.get_name(0));
               Logger::InfoN("   md5:      {}", to_hex_string(sample.md5()));
               Logger::InfoN("   sha1:     {}", to_hex_string(sample.sha1()));
               Logger::InfoN("   sha256:   {}", to_hex_string(sample.sha256()));
               Logger::InfoN("   sha3-384: {}", to_hex_string(sample.sha3_384()));

               if (alias.has_value())
               {
                  Logger::Info("setting alias to \"{}\"...", *alias);
                  sample.set_alias(*alias);
                  Logger::Raw(Logger::Level::Info, "done.\n");
               }

               if (filename.has_value())
               {
                  Logger::Info("setting filename to \"{}\"...", *filename);
                  sample.set_filename(*filename);
                  Logger::Raw(Logger::Level::Info, "done.\n");
               }

               if (tags.has_value())
               {
                  for (auto &tag : *tags)
                  {
                     Logger::InfoN("setting tag \"{}\"", tag);
                     sample.add_tag(tag);
                  }
               }

               if (families.has_value())
               {
                  for (auto &family : *families)
                  {
                     Logger::InfoN("setting family \"{}\"", family);
                     sample.add_family(family);
                  }
               }

               Logger::Info("saving sample to database...");
               sample.save();
               Logger::Raw(Logger::Level::Info, "done.\n");
               
               if (parents.has_value() || children.has_value())
               {
                  try {
                     if (parents.has_value())
                     {
                        for (auto &parent : *parents)
                        {
                           Logger::InfoN("setting parent \"{}\"", parent);
                           sample.add_parent(Sample::FromIdentifier(parent));
                        }
                     }

                     if (children.has_value())
                     {
                        for (auto &child : *children)
                        {
                           Logger::InfoN("setting child \"{}\"", child);
                           sample.add_child(Sample::FromIdentifier(child));
                        }
                     }
                  }
                  catch (std::exception &exc)
                  {
                     Logger::Info("encountered an error, erasing sample...");
                     sample.erase();
                     Logger::Raw(Logger::Level::Info, "done.\n");

                     throw exc;
                  }
               }
            }
                                        
            return true;
         }
      };

      struct UploadFunction : public Module
      {
         UploadFunction()
            : Module("upload",
                     "Upload samples in the database to various targets.")
         {
            this->add_argument("-c", "--children")
               .help("Upload the children of this sample as well.")
               .default_value(false)
               .implicit_value(true);

            this->add_argument("-p", "--parents")
               .help("Upload the parents of this sample as well.")
               .default_value(false)
               .implicit_value(true);

            this->add_argument("-d", "--descendants")
               .help("Recursively select children and all their children to upload.")
               .default_value(false)
               .implicit_value(true);

            this->add_argument("-a", "--ancestors")
               .help("Recursively select parents and all their parents to upload.")
               .default_value(false)
               .implicit_value(true);

            this->add_argument("-s", "--ssh")
               .help("The SSH site to upload the samples to.")
               .metavar("SSH_URI");

            this->add_argument("sample")
               .help("The sample identifiers to upload to the given site.")
               .required()
               .remaining();
         }

         bool execute(Module &root) {
            auto children = this->get<bool>("--children");
            auto parents = this->get<bool>("--parents");
            auto descendants = this->get<bool>("--descendants");
            auto ancestors = this->get<bool>("--ancestors");
            auto ssh = this->present("--ssh");
            auto samples = this->get<std::vector<std::string>>("sample");

            Logger::DebugN("testing ssh");
            auto session = SSHSession("teal@localhost");
            auto verbosity = SSH_LOG_FUNCTIONS;
            //session.set_option(SSH_OPTIONS_LOG_VERBOSITY, &verbosity);

            auto key = SSHKeyPair(std::filesystem::path("C:/cygwin64/home/purple/.ssh/id_dongs"));
            session.add_key(key);
            
            Logger::DebugN("connecting to SSH site.");
            session.connect();
            Logger::DebugN("authenticating with SSH site.");
            session.authenticate();

            Logger::DebugN("connected. executing a command...");
            int exit_code;
            std::vector<std::uint8_t> out_bytes, err_bytes;
            std::string out_string, err_string;
            
            std::tie(exit_code, out_bytes, err_bytes) = session.exec("cat - > dongs && cat dongs && sha256sum dongs && echo pricks 1>&2", "dingleberry\n");
            out_string = std::string(out_bytes.begin(), out_bytes.end());
            err_string = std::string(err_bytes.begin(), err_bytes.end());
            
            Logger::DebugN("exit code: {}", exit_code);
            Logger::DebugN("stdout: {}", out_string);
            Logger::DebugN("stderr: {}", err_string);

            Logger::DebugN("disconnecting...");
            session.disconnect();

            return true;
         }
      };

      SampleModule()
         : Module("sample",
                  "Add, configure, remove and transport stored malware samples.",
                  {
                     std::make_shared<AddFunction>(),
                     std::make_shared<ModifyFunction>(),
                     std::make_shared<RemoveFunction>(),
                     std::make_shared<ExistsFunction>(),
                     std::make_shared<InfoFunction>(),
                     std::make_shared<ExportFunction>(),
                     std::make_shared<ImportFunction>(),
                     std::make_shared<UploadFunction>()
                  })
      {}
   };
}

#endif
