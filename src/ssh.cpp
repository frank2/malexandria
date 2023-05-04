#include "ssh.hpp"

using namespace malexandria;

std::string malexandria::password_prompt(std::string prompt, bool echo) {
   Logger::Fatal(prompt);

   char c;
   std::string result;

   do
   {
      c = GETCH();

      switch (c)
      {
      case 0x3:
      case 0x4:
      case 0x1A:
         throw exception::KeyboardInterrupt();
         
      case '\r':
      case '\n':
         break;

      case '\b':
         if (result.size() > 0)
            result.pop_back();
         
         break;

      default:
         result.push_back(c);

         if (echo)
            Logger::Raw(Logger::Level::Fatal, "{}", c);
         
         break;
      }
   } while (c != '\r' && c != '\n');

   Logger::Raw(Logger::Level::Fatal, "\n");

   return result;
}

int malexandria::password_callback(const char *prompt, char *buf, std::size_t len, int echo, int verify, void *userdata)
{
   if (std::strcmp("Passphrase", prompt) == 0) // this prompt is very clunky, replace it
      prompt = "Password: ";
   
   auto result = password_prompt(prompt, static_cast<bool>(echo));

   if (verify)
   {
      std::string again;

      do
      {
         if (again.size() > 0)
         {
            Logger::FatalN("Passwords do not match.");
            result = password_prompt(prompt, static_cast<bool>(echo));
         }
         again = password_prompt("Verify: ", static_cast<bool>(echo));
      } while (result != again);
   }

   if (result.size()+1 > len)
      throw exception::OutOfBounds(result.size()+1, len);

   std::memcpy(buf, result.c_str(), result.size()+1);

   return 0;
}

void SSHKey::allocate(void) {
   auto key_obj = ssh_key_new();

   if (key_obj == nullptr)
      throw exception::SSHAllocationFailure();

   this->set_key(key_obj);
}

SSHKey &SSHKeyPair::public_key(void) {
   if (!this->_pub_key.has_key())
   {
      Logger::Debug("importing pubkey {}...", this->_pub_key_file.string());
      ssh_key key = nullptr;
      auto result = ssh_pki_import_pubkey_file(this->_pub_key_file.string().c_str(), &key);
      Logger::Raw(Logger::Level::Debug, "result = {}.\n", result);
      
      if (key != nullptr)
         this->_pub_key = key;

      if (result == SSH_EOF)
         throw exception::OpenFileFailure(this->_pub_key_file.string());
      else if (result == SSH_ERROR)
         throw exception::SSHKeyImportFailure(this->_pub_key_file.string());
   }
   
   return this->_pub_key;
}

SSHKey &SSHKeyPair::private_key(void) {
   if (!this->_priv_key.has_key())
   {
      Logger::FatalN("importing private key {}...", this->_priv_key_file.string());
      ssh_key key = nullptr;
      auto result = ssh_pki_import_privkey_file(this->_priv_key_file.string().c_str(),
                                                nullptr,
                                                password_callback,
                                                nullptr,
                                                &key);
      Logger::DebugN("import result={}.", result);
      
      if (key != nullptr)
         this->_priv_key = key;

      if (result == SSH_EOF)
         throw exception::OpenFileFailure(this->_pub_key_file.string());
      else if (result == SSH_ERROR)
         throw exception::SSHKeyImportFailure(this->_priv_key_file.string());
   }

   return this->_priv_key;
}

void SSHKeyPair::load(const std::filesystem::path &private_key, std::optional<std::filesystem::path> public_key) {
   if (!public_key.has_value())
   {
      public_key = private_key;
      *public_key += std::string(".pub");
   }

   this->_priv_key_file = private_key;
   this->_pub_key_file = *public_key;
}

HostParseResult malexandria::parse_host_string(const std::string &host)
{
   char *parsed_host, *parsed_username, *parsed_port;
   std::string host_result;
   std::optional<std::string> user_result, port_result;

   if (ssh_config_parse_uri(host.c_str(), &parsed_username, &parsed_host, &parsed_port) != SSH_OK)
      throw exception::NotAnSSHURI(host);

   if (parsed_host != nullptr)
   {
      host_result = parsed_host;
      free(parsed_host);
   }

   if (parsed_username != nullptr)
   {
      user_result = parsed_username;
      free(parsed_username);
   }

   if (parsed_port != nullptr)
   {
      port_result = parsed_port;
      free(parsed_port);
   }

   return std::make_tuple(host_result, user_result, port_result);
}

void SSHChannel::allocate(ssh_session session) {
   auto channel = ssh_channel_new(session);

   if (channel == nullptr)
      throw exception::SSHAllocationFailure();

   this->_channel = std::shared_ptr<ssh_channel_struct>(channel, ssh_channel_free);
}

void SSHChannel::open(void) {
   if (ssh_channel_open_session(**this) != SSH_OK)
      throw exception::SSHException(ssh_get_error(ssh_channel_get_session(**this)));
}

bool SSHChannel::is_open(void) {
   return static_cast<bool>(ssh_channel_is_open(**this));
}

void SSHChannel::close(void) {
   if (this->is_closed())
      return;

   if (!this->is_eof())
      this->eof();

   ssh_channel_close(**this);
}

bool SSHChannel::is_closed(void) {
   return static_cast<bool>(ssh_channel_is_closed(**this));
}

void SSHChannel::eof(void) {
   if (ssh_channel_send_eof(**this) != SSH_OK)
      throw exception::SSHException(ssh_get_error(ssh_channel_get_session(**this)));
}

bool SSHChannel::is_eof(void) {
   return static_cast<bool>(ssh_channel_is_eof(**this));
}

void SSHChannel::exec(const std::string &command) {
   if (ssh_channel_request_exec(**this, command.c_str()) != SSH_OK)
      throw exception::SSHException(ssh_get_error(ssh_channel_get_session(**this)));
}

std::pair<std::vector<std::uint8_t>,std::vector<std::uint8_t>> SSHChannel::read(void) {
   int bytes_read = 0;
   std::vector<std::uint8_t> stdout_vec, stderr_vec, buffer;

   buffer.resize(0x1000);

   do
   {
      if (bytes_read > 0)
         stdout_vec.insert(stdout_vec.end(), buffer.data(), buffer.data()+bytes_read);

      bytes_read = ssh_channel_read(**this, buffer.data(), buffer.size(), 0);
   } while (bytes_read > 0);

   if (bytes_read < 0)
      throw exception::SSHException(ssh_get_error(ssh_channel_get_session(**this)));

   bytes_read = 0;

   do
   {
      if (bytes_read > 0)
         stderr_vec.insert(stderr_vec.end(), buffer.data(), buffer.data()+bytes_read);

      bytes_read = ssh_channel_read(**this, buffer.data(), buffer.size(), 1);
   } while (bytes_read > 0);

   return std::make_pair(stdout_vec,stderr_vec);
}

void SSHChannel::write(const std::vector<std::uint8_t> &data) {
   auto result = ssh_channel_write(**this, data.data(), data.size());
   
   if (result == SSH_ERROR)
      throw exception::SSHException(ssh_get_error(ssh_channel_get_session(**this)));

   if (result != data.size())
      throw exception::SSHOutputTruncated();
}

void SSHChannel::write_stderr(const std::vector<std::uint8_t> &data) {
   auto result = ssh_channel_write_stderr(**this, data.data(), data.size());

   if (result == SSH_ERROR)
      throw exception::SSHException(ssh_get_error(ssh_channel_get_session(**this)));

   if (result != data.size())
      throw exception::SSHOutputTruncated();
}

int SSHChannel::exit_code(void) {
   return ssh_channel_get_exit_status(**this);
}

void SSHSession::allocate(void) {
   auto session_obj = ssh_new();

   if (session_obj == nullptr)
      throw exception::SSHAllocationFailure();
         
   this->_session = std::shared_ptr<ssh_session_struct>(session_obj, ssh_free);
}

void SSHSession::validate_host(void) {
   enum ssh_known_hosts_e known_state;
   ssh_key server_pubkey;
   std::uint8_t *hash_bytes;
   std::size_t hash_size;

   Logger::Debug("getting ssh server pubkey...");
   
   if (ssh_get_server_publickey(**this, &server_pubkey) < 0)
      throw exception::SSHException(ssh_get_error(**this));

   Logger::Raw(Logger::Level::Debug, "done.\n");
   Logger::Debug("getting sha1 hash of key...");

   auto result = ssh_get_publickey_hash(server_pubkey, SSH_PUBLICKEY_HASH_SHA1, &hash_bytes, &hash_size);
   ssh_key_free(server_pubkey);

   if (result < 0)
      throw exception::SSHHashError();

   Logger::Raw(Logger::Level::Debug, "{}\n", to_hex_string(hash_bytes, hash_size));

   Logger::Debug("checking state of server...");
   known_state = ssh_session_is_known_server(**this);

   switch(known_state)
   {
   case SSH_KNOWN_HOSTS_OK:
      Logger::Raw(Logger::Level::Debug, "OK.\n");
      break;

   case SSH_KNOWN_HOSTS_CHANGED:
      Logger::Raw(Logger::Level::Debug, "key changed.\n");
      Logger::FatalN("SSH server identity changed, key hash is now {}.", to_hex_string(hash_bytes, hash_size));
      Logger::FatalN("If this wasn't expected, this could be indicative of a man-in-the-middle attack.");
      ssh_clean_pubkey_hash(&hash_bytes);

      throw exception::SSHHostKeyChanged();

   case SSH_KNOWN_HOSTS_OTHER:
      Logger::Raw(Logger::Level::Debug, "key confusion.\n");
      Logger::FatalN("Key not found, but another type of key exists.");
      Logger::FatalN("An attacker might be changing the server key to confuse your client.");
      ssh_clean_pubkey_hash(&hash_bytes);

      throw exception::SSHHostKeyConfusion();

   case SSH_KNOWN_HOSTS_NOT_FOUND:
      Logger::Raw(Logger::Level::Debug, "creating hosts file...");

   case SSH_KNOWN_HOSTS_UNKNOWN:
   {
      Logger::Raw(Logger::Level::Debug, "unknown.\n");

      if (Logger::GetLevel() == Logger::Level::Silent)
      {
         ssh_clean_pubkey_hash(&hash_bytes);
         throw exception::SSHHostUnknown();
      }

      Logger::FatalN("The server is unknown. Do you trust the hostkey? [yes/no]");
      Logger::FatalN("SHA1 hash of server pubkey: {}", to_hex_string(hash_bytes, hash_size));

      std::string answer;

      do
      {
         if (answer.size() > 0)
            Logger::FatalN("Please answer yes or no.");

         std::cerr << ">>> " << std::flush;
         std::cin >> answer;
      } while (answer != "yes" && answer != "no");

      if (answer == "no")
      {
         Logger::DebugN("key is not trusted.");
         ssh_clean_pubkey_hash(&hash_bytes);
         throw exception::SSHHostUnknown();
      }

      Logger::Fatal("Saving host {}...", to_hex_string(hash_bytes, hash_size));
      ssh_clean_pubkey_hash(&hash_bytes);

      if (ssh_session_update_known_hosts(**this) < 0)
         throw exception::UpdateSSHKnownHostsFailed();

      Logger::Raw(Logger::Level::Fatal, "done.\n");
      
      break;
   }

   case SSH_KNOWN_HOSTS_ERROR:
      ssh_clean_pubkey_hash(&hash_bytes);
      throw exception::SSHException(ssh_get_error(**this));
   }
}

void SSHSession::authenticate_none() {
   Logger::Debug("attempting no authentication...");
   auto result = ssh_userauth_none(**this, nullptr);
   Logger::Raw(Logger::Level::Debug, "result={}...", result);

   switch(result)
   {
   case SSH_AUTH_SUCCESS:
      Logger::Raw(Logger::Level::Debug, "success!\n");
      return;
      
   case SSH_AUTH_DENIED:
      Logger::Raw(Logger::Level::Debug, "SSH_AUTH_DENIED.\n");
      throw exception::SSHAuthenticationFailure();

   case SSH_AUTH_PARTIAL:
      Logger::Raw(Logger::Level::Debug, "SSH_AUTH_PARTIAL.\n");
      throw exception::SSHPartialAuthentication();

   case SSH_AUTH_ERROR:
   default:
      throw exception::SSHException(ssh_get_error(**this));
   }
}

void SSHSession::authenticate_pubkey() {
   Logger::Debug("attempting automatic public key authentication...");
   auto result = ssh_userauth_publickey_auto(**this, nullptr, nullptr);
   Logger::Raw(Logger::Level::Debug, "result={}...", result);

   if (result == SSH_AUTH_SUCCESS)
   {
      Logger::Raw(Logger::Level::Debug, "success!\n");
      return;
   }

   Logger::Raw(Logger::Level::Debug, "general failure.\n");
   Logger::DebugN("attempting to authenticate with local keys...");
   
   for (auto &keypair : this->_keys)
   {
      auto &pubkey = keypair.public_key();
      result = ssh_userauth_try_publickey(**this, nullptr, *pubkey);

      if (result != SSH_AUTH_SUCCESS)
         continue;

      Logger::DebugN("found a pubkey that authenticates!");
      Logger::DebugN("loading private key...");
      auto &privkey = keypair.private_key();
      Logger::DebugN("private key loaded.");

      Logger::Debug("attempting authentication with private key...");
      result = ssh_userauth_publickey(**this, nullptr, *privkey);
      Logger::Raw(Logger::Level::Debug, "result={}...", result);

      if (result == SSH_AUTH_SUCCESS)
      {
         Logger::Raw(Logger::Level::Debug, "success!\n", result);
         return;
      }

      Logger::Raw(Logger::Level::Debug, "failure.\n");
   }

   switch (result)
   {
   case SSH_AUTH_DENIED:
      Logger::Raw(Logger::Level::Debug, "SSH_AUTH_DENIED.\n");
      throw exception::SSHAuthenticationFailure();

   case SSH_AUTH_PARTIAL:
      Logger::Raw(Logger::Level::Debug, "SSH_AUTH_PARTIAL.\n");
      throw exception::SSHPartialAuthentication();

   case SSH_AUTH_ERROR:
      Logger::Raw(Logger::Level::Debug, "SSH_AUTH_ERROR.\n");
      
   default:
      throw exception::SSHException(ssh_get_error(**this));
   }
}

void SSHSession::authenticate_interactive() {
   Logger::DebugN("starting keyboard interactive mode.");
   
   auto result = ssh_userauth_kbdint(**this, nullptr, nullptr);

   while (result == SSH_AUTH_INFO)
   {
      const char *name, *instruction;
      int prompts;

      name = ssh_userauth_kbdint_getname(**this);
      instruction = ssh_userauth_kbdint_getinstruction(**this);
      prompts = ssh_userauth_kbdint_getnprompts(**this);

      if (std::strlen(name) > 0)
         Logger::FatalN("{}", name);
      if (std::strlen(instruction) > 0)
         Logger::FatalN("{}", instruction);

      for (int i=0; i<prompts; ++i)
      {
         const char *prompt;
         char echo;

         prompt = ssh_userauth_kbdint_getprompt(**this, i, &echo);

         if (echo)
         {
            Logger::Fatal("{}", prompt);

            std::string answer;
            std::cin >> answer;

            if (ssh_userauth_kbdint_setanswer(**this, i, answer.c_str()) < 0)
               throw exception::SSHException(ssh_get_error(**this));
         }
         else
         {
            auto result = password_prompt(prompt);

            if (ssh_userauth_kbdint_setanswer(**this, i, result.c_str()) < 0)
               throw exception::SSHException(ssh_get_error(**this));
         }
      }

      result = ssh_userauth_kbdint(**this, nullptr, nullptr);
   }

   switch (result)
   {
   case SSH_AUTH_SUCCESS:
      Logger::Raw(Logger::Level::Debug, "SSH_AUTH_SUCCESS.\n");
      break;
      
   case SSH_AUTH_DENIED:
      Logger::Raw(Logger::Level::Debug, "SSH_AUTH_DENIED.\n");
      throw exception::SSHAuthenticationFailure();

   case SSH_AUTH_PARTIAL:
      Logger::Raw(Logger::Level::Debug, "SSH_AUTH_PARTIAL.\n");
      throw exception::SSHPartialAuthentication();

   case SSH_AUTH_ERROR:
   default:
      throw exception::SSHException(ssh_get_error(**this));
   }
}

void SSHSession::authenticate_password(void) {
   Logger::DebugN("performing password-based authentication.");
   
   auto password = password_prompt("Password: ");

   if (ssh_userauth_password(**this, nullptr, password.c_str()) == SSH_AUTH_ERROR)
      throw exception::SSHAuthenticationFailure();

   Logger::DebugN("password authentication succeeded.");
}

void SSHSession::set_host(const std::string &host) {
   this->set_option(SSH_OPTIONS_HOST, host.c_str());

   Logger::DebugN("parsing ssh config");
   
   if (ssh_options_parse_config(**this, nullptr) < 0)
      throw exception::SSHException(ssh_get_error(**this));
}

void SSHSession::parse_host(const std::string &host) {
   std::string parsed_host;
   std::optional<std::string> parsed_user, parsed_port;

   std::tie(parsed_host, parsed_user, parsed_port) = parse_host_string(host);

   this->set_host(parsed_host);

   if (parsed_user.has_value())
      this->set_username(*parsed_user);

   if (parsed_port.has_value())
      this->set_port(static_cast<std::uint16_t>(std::stoi(*parsed_port)));
}

void SSHSession::set_username(const std::string &username) {
   this->set_option(SSH_OPTIONS_USER, username.c_str());
}

void SSHSession::set_port(std::uint16_t port) {
   this->set_option(SSH_OPTIONS_PORT, &port);
}

bool SSHSession::is_connected(void) {
   return static_cast<bool>(ssh_is_connected(**this));
}

void SSHSession::connect(void) {
   if (ssh_connect(**this) != SSH_OK)
      throw exception::SSHException(ssh_get_error(**this));

   try
   {
      this->validate_host();
   }
   catch (exception::Exception &exc)
   {
      this->disconnect();
      throw exc;
   }
}

void SSHSession::disconnect(void) {
   for (auto &channel : this->_channels)
      channel.reset();
   
   ssh_disconnect(**this);
}

void SSHSession::authenticate(void) {
   try
   {
      Logger::DebugN("attempting no authentication initially...");
      this->authenticate_none();
      Logger::DebugN("success!");
      return;
   }
   catch (exception::SSHAuthenticationFailure &exc)
   {
      Logger::DebugN("failure.");
   }

   Logger::Debug("getting authentication methods...");
   auto methods = ssh_userauth_list(**this, nullptr);
   Logger::Raw(Logger::Level::Debug, "methods: {:#08x}.\n", methods);

   if (methods & SSH_AUTH_METHOD_NONE)
   {
      Logger::DebugN("methods & SSH_AUTH_METHOD_NONE");
      
      try
      {
         this->authenticate_none();
         return;
      }
      catch (exception::SSHAuthenticationFailure &exc)
      {
      }
      catch (exception::SSHPartialAuthentication &exc)
      {
      }
   }
   if (methods & SSH_AUTH_METHOD_PUBLICKEY)
   {
      Logger::DebugN("methods & SSH_AUTH_METHOD_PUBLICKEY");
      
      try
      {
         this->authenticate_pubkey();
         return;
      }
      catch (exception::SSHAuthenticationFailure &exc)
      {
      }
      catch (exception::SSHPartialAuthentication &exc)
      {
      }
   }
   if (methods & SSH_AUTH_METHOD_INTERACTIVE)
   {
      Logger::DebugN("methods & SSH_AUTH_METHOD_INTERACTIVE");
      
      try
      {
         this->authenticate_interactive();
         return;
      }
      catch (exception::SSHAuthenticationFailure &exc)
      {
      }
      catch (exception::SSHPartialAuthentication &exc)
      {
      }
   }
   if (methods & SSH_AUTH_METHOD_PASSWORD)
   {
      Logger::DebugN("methods & SSH_AUTH_METHOD_PASSWORD");
      
      try
      {
         this->authenticate_password();
         return;
      }
      catch (exception::SSHAuthenticationFailure &exc)
      {
      }
      catch (exception::SSHPartialAuthentication &exc)
      {
      }
   }

   throw exception::SSHAuthenticationFailure();
}

bool SSHSession::has_channel(std::size_t index) {
   return index < this->_channels.size() && this->_channels[index] != nullptr;
}

std::size_t SSHSession::create_channel(void) {
   auto id = this->_channels.size();
   
   this->_channels.push_back(std::shared_ptr<SSHChannel>(new SSHChannel(**this)));
   this->_channels.back()->open();
   
   return id;
}

SSHChannel &SSHSession::get_channel(std::size_t index) {
   if (!this->has_channel(index))
      throw exception::ChannelNotOpen(index);

   return *this->_channels[index];
}

void SSHSession::destroy_channel(std::size_t index) {
   if (!this->has_channel(index))
      throw exception::ChannelNotOpen(index);

   this->_channels[index].reset();
}

ExecResult SSHSession::exec(const std::string &command, std::optional<std::vector<std::uint8_t>> input) {
   auto channel_id = this->create_channel();
   auto &channel = this->get_channel(channel_id);
   channel.exec(command);

   if (input.has_value())
      channel.write(*input);

   channel.eof();
   auto fds = channel.read();
   auto exit_code = channel.exit_code();

   this->destroy_channel(channel_id);

   return std::make_tuple(exit_code, fds.first, fds.second);
}

ExecResult SSHSession::exec(const std::string &command, std::optional<std::string> input) {
   if (input.has_value())
   {
      std::vector<std::uint8_t> vec(input->begin(), input->end());
      return this->exec(command, vec);
   }
   else
      return this->exec(command, std::optional<std::vector<std::uint8_t>>(std::nullopt));
}
