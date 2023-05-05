#ifndef __MALEXANDRIA_SSH_HPP
#define __MALEXANDRIA_SSH_HPP

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <tuple>
#include <utility>

#include <libssh/libssh.h>
#include <libssh/config_parser.h>

#include "logger.hpp"
#include "fs.hpp"
#include "platform.hpp"
#include "utility.hpp"

#if defined(MALEXANDRIA_WIN32)
#include <conio.h>
#define GETCH _getch
#else
#include <curses.h>
#define GETCH getch
#endif

namespace malexandria
{
   std::string password_prompt(std::string prompt, bool echo=false);
   int password_callback(const char *prompt, char *buf, std::size_t len, int echo, int verify, void *userdata);
   
   class SSHKey
   {
      std::shared_ptr<ssh_key_struct> _key;

   public:
      SSHKey() {}
      SSHKey(ssh_key key) { this->set_key(key); }
      SSHKey(const SSHKey &other) : _key(other._key) {}

      SSHKey &operator=(const SSHKey &other) {
         this->_key = other._key;

         return *this;
      }
      ssh_key operator*() {
         return this->get_key();
      }

      void allocate();
      bool has_key(void)
      {
         return this->_key != nullptr;
      }
      
      ssh_key get_key(void) {
         if (!this->has_key())
            throw exception::NullPointer();

         return this->_key.get();
      }
      void set_key(ssh_key key) {
         this->_key = std::shared_ptr<ssh_key_struct>(key, ssh_key_free);
      }
   };

   class SSHKeyPair
   {
      std::filesystem::path _priv_key_file, _pub_key_file;
      SSHKey _pub_key, _priv_key;

   public:
      SSHKeyPair() {}
      SSHKeyPair(const std::filesystem::path &private_key,
                 std::optional<std::filesystem::path> public_key=std::nullopt)
      {
         this->load(private_key, public_key);
      }
      SSHKeyPair(const SSHKeyPair &other) : _priv_key_file(other._priv_key_file),
                                            _pub_key_file(other._pub_key_file),
                                            _pub_key(other._pub_key),
                                            _priv_key(other._priv_key)
      {}

      SSHKeyPair &operator=(const SSHKeyPair &other)
      {
         this->_priv_key_file = other._priv_key_file;
         this->_pub_key_file = other._pub_key_file;
         this->_pub_key = other._pub_key;
         this->_priv_key = other._priv_key;

         return *this;
      }

      SSHKey &public_key(void);
      SSHKey &private_key(void);
      void load(const std::filesystem::path &private_key, std::optional<std::filesystem::path> public_key=std::nullopt);
   };

   using HostParseResult = std::tuple<std::string,std::optional<std::string>,std::optional<std::string>>;
   HostParseResult parse_host_string(const std::string &host);

   class SSHSession;

   class SSHChannel
   {
      friend SSHSession;
      
      std::shared_ptr<ssh_channel_struct> _channel;

      void allocate(ssh_session session);
      SSHChannel(ssh_session session) { this->allocate(session); }
      SSHChannel(const SSHChannel &other) : _channel(other._channel) {}
      SSHChannel &operator=(const SSHChannel &other) {
         this->_channel = other._channel;

         return *this;
      }
      
   public:
      ~SSHChannel() {
         if (this->_channel != nullptr && this->_channel.use_count() <= 1)
            this->close();
      }

      ssh_channel operator*() { return this->get_channel(); }
      ssh_channel get_channel(void) {
         if (this->_channel == nullptr)
            throw exception::NullPointer();

         return this->_channel.get();
      }

      void open();
      bool is_open();
      void close();
      bool is_closed();
      void eof();
      bool is_eof();
      
      void exec(const std::string &command);
      std::pair<std::vector<std::uint8_t>,std::vector<std::uint8_t>> read(void);
      void write(const std::vector<std::uint8_t> &data);
      void write(const std::string &data);
      void write_stderr(const std::vector<std::uint8_t> &data);
      void write_stderr(const std::string &error);
      int exit_code();
   };

   class SCPSession
   {
      friend SSHSession;
      
   public:
      enum class Mode : int
      {
         Read = SSH_SCP_READ,
         Write = SSH_SCP_WRITE,
         Recursive = SSH_SCP_RECURSIVE
      };

   private:
      std::shared_ptr<ssh_scp_struct> _scp;
      std::shared_ptr<ssh_session_struct> _session;

      void allocate(std::shared_ptr<ssh_session_struct> session, Mode mode, const std::string &location);
      SCPSession(std::shared_ptr<ssh_session_struct> session, Mode mode, const std::string &location) { this->allocate(session, mode, location); }
      SCPSession(const SCPSession &other) : _scp(other._scp) {}

      SCPSession &operator=(const SCPSession &other) {
         this->_scp = other._scp;

         return *this;
      }

   public:
      ~SCPSession() {
         if (this->_scp != nullptr && this->_scp.use_count() <= 1)
            this->close();
      }

      ssh_scp operator*() { return this->_scp.get(); }

      void init();
      void close();
      
      void push_directory(const std::string &dir, std::filesystem::perms mode);
      void push_file(const std::string &filename, std::uint32_t size, std::filesystem::perms mode);
      void push_file64(const std::string &filename, std::uint64_t size, std::filesystem::perms mode);
      void write(const std::vector<std::uint8_t> &data);

      int pull_request();
      std::uint32_t request_size();
      std::uint64_t request_size64();
      std::string request_filename();
      std::filesystem::perms request_permissions();
      void accept_request();
      std::vector<std::uint8_t> read(std::size_t size);
   };

   struct ExecResult
   {
      int exit_code;
      std::vector<std::uint8_t> output;
      std::vector<std::uint8_t> error;
   };
            
   class SSHSession
   {
   public:
      enum class Environment : std::uint8_t
      {
         Shell = 0,
         PowerShell,
         CMD,
         Unknown,
      };
      
      std::shared_ptr<ssh_session_struct> _session;
      std::vector<SSHKeyPair> _keys;
      std::vector<std::shared_ptr<SSHChannel>> _channels;
      std::optional<Environment> _env;

      void allocate();
      void validate_host();
      void authenticate_none();
      void authenticate_pubkey();
      void authenticate_interactive();
      void authenticate_password();

   public:
      SSHSession() { this->allocate(); }
      SSHSession(const std::string &host,
                 std::optional<std::string> username=std::nullopt,
                 std::optional<std::uint16_t> port=std::nullopt)
      {
         this->allocate();

         if (!username.has_value() && !port.has_value())
            this->parse_host(host);
         else
         {
            this->set_host(host);

            if (username.has_value())
               this->set_username(*username);

            if (port.has_value())
               this->set_port(*port);
         }
      }
      SSHSession(const SSHSession &other)
         : _session(other._session),
           _keys(other._keys),
           _channels(other._channels),
           _env(other._env)
      {}
      ~SSHSession() {
         if (this->is_connected())
            this->disconnect();
      }

      SSHSession &operator=(const SSHSession &other) {
         this->_session = other._session;
         this->_keys = other._keys;
         this->_channels = other._channels;
         this->_env = other._env;

         return *this;
      }
      ssh_session operator*() { return this->_session.get(); }
      
      template <typename T>
      void set_option(enum ssh_options_e option, T *value) {
         if (ssh_options_set(**this, option, value) < 0)
            throw exception::SSHException(ssh_get_error(**this));
      }
      void set_host(const std::string &host);
      void parse_host(const std::string &host);

      void set_username(const std::string &username);

      void set_port(std::uint16_t port);

      void add_key(SSHKeyPair &key)
      {
         this->_keys.push_back(key);
      }

      bool is_connected(void);
      void connect(void);
      void disconnect(void);
      void authenticate(void);

      bool has_channel(std::size_t index);
      std::size_t create_channel(void);
      SSHChannel &get_channel(std::size_t index);
      void destroy_channel(std::size_t index);

      ExecResult exec(const std::string &command, std::optional<std::vector<std::uint8_t>> input=std::nullopt);
      ExecResult exec(const std::string &command, std::string input);

      void upload(const std::filesystem::path &local_file, const std::filesystem::path &target);
      void upload(const std::vector<std::uint8_t> &vec, std::filesystem::perms mode, const std::filesystem::path &target);

      void download(const std::filesystem::path &remote_file, const std::filesystem::path &target);
      std::vector<std::uint8_t> download(const std::filesystem::path &remote_file);

      Environment get_environment(void);
      std::filesystem::path temp_file(void);
      void remove_file(const std::filesystem::path &filename);
      std::optional<std::filesystem::path> which(const std::string &program_name);
   };
}

#endif
