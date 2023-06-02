#ifndef __MALEXANDRIA_HTTP_HPP
#define __MALEXANDRIA_HTTP_HPP

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <curl/curl.h>
#include <fmt/core.h>

#include "exception.hpp"
#include "logger.hpp"

namespace malexandria
{
   class HTTPRequest
   {
      std::shared_ptr<CURL> _handle;
      std::string _url;
      std::optional<std::vector<std::uint8_t>> _get_data;
      std::optional<std::pair<std::vector<std::uint8_t>,std::size_t>> _put_data;
      std::optional<std::string> _post_data;
      std::vector<std::pair<std::string,std::string>> _req_headers, _recv_headers;
      
      void init(void) {
         this->_handle = std::shared_ptr<CURL>(curl_easy_init(), curl_easy_cleanup);
         
         curl_easy_setopt(**this, CURLOPT_READFUNCTION, HTTPRequest::ReadCallback);
         curl_easy_setopt(**this, CURLOPT_READDATA, this);
         curl_easy_setopt(**this, CURLOPT_WRITEFUNCTION, HTTPRequest::WriteCallback);
         curl_easy_setopt(**this, CURLOPT_WRITEDATA, this);
         curl_easy_setopt(**this, CURLOPT_HEADERFUNCTION, HTTPRequest::HeaderCallback);
         curl_easy_setopt(**this, CURLOPT_HEADERDATA, this);
      }

      static std::size_t ReadCallback(char *buffer, std::size_t unit_size, std::size_t mem_size, void *userdata);
      static std::size_t WriteCallback(void *buffer, std::size_t unit_size, std::size_t mem_size, void *userdata);
      static std::size_t HeaderCallback(char *buffer, std::size_t unit_size, std::size_t mem_size, void *userdata);

   public:
      HTTPRequest() { this->init(); }
      HTTPRequest(const std::string &url) {
         this->init();
         this->set_url(url);
      }
      HTTPRequest(const HTTPRequest &other)
         : _handle(other._handle),
           _url(other._url),
           _get_data(other._get_data),
           _put_data(other._put_data),
           _post_data(other._post_data),
           _req_headers(other._req_headers),
           _recv_headers(other._recv_headers)
      {}

      HTTPRequest &operator=(const HTTPRequest &other) {
         this->_handle = other._handle;
         this->_url = other._url;
         this->_get_data = other._get_data;
         this->_put_data = other._put_data;
         this->_post_data = other._post_data;
         this->_req_headers = other._req_headers;
         this->_recv_headers = other._recv_headers;

         return *this;
      }

      CURL *operator*(void) {
         return this->get_handle();
      }

      CURL *get_handle(void) {
         if (this->_handle == nullptr)
            throw exception::NullPointer();

         return this->_handle.get();
      }

      void set_url(const std::string &url);
      CURLcode perform(void);
      
      std::vector<std::pair<std::string,std::string>> &request_headers(void);
      std::map<std::string,std::string> request_header_map(void);
      void set_header(const std::string &key, const std::string &value);
      void remove_header(const std::string &key);
      
      std::vector<std::pair<std::string,std::string>> &received_headers(void);
      std::map<std::string,std::string> received_headers_map(void);

      bool has_data(void);
      std::vector<std::uint8_t> &get_data(void);

      bool has_put_data(void);
      std::vector<std::uint8_t> &put_data(void);
      void set_put_data(const std::vector<std::uint8_t> &data);

      bool has_post_data(void);
      std::string &post_data(void);
      void set_post_data(const std::string &data);

      long response_code(void);
   };      
}

#endif
