#include "http.hpp"

using namespace malexandria;

std::size_t HTTPRequest::ReadCallback(char *buffer, std::size_t unit_size, std::size_t mem_size, void *userdata)
{
   auto &obj = *reinterpret_cast<HTTPRequest *>(userdata);

   if (!obj.has_put_data())
      return 0;

   if (obj._put_data->second >= obj._put_data->first.size())
   {
      obj._put_data = std::nullopt;
      return 0;
   }

   auto resolved_size = unit_size * mem_size;
   auto next_offset = obj._put_data->second;
   auto put_size = obj._put_data->first.size();
   auto copy_size = (put_size - next_offset < resolved_size) ? put_size - next_offset : resolved_size;
   
   std::memcpy(reinterpret_cast<std::uint8_t *>(buffer),
               obj._put_data->first.data()+next_offset,
               copy_size);

   obj._put_data->second += copy_size;

   return copy_size;
}

std::size_t HTTPRequest::WriteCallback(void *buffer, std::size_t unit_size, std::size_t mem_size, void *userdata) {
   auto &obj = *reinterpret_cast<HTTPRequest *>(userdata);

   if (!obj.has_data())
      obj._get_data = std::vector<std::uint8_t>();

   auto resolved_size = unit_size * mem_size;
   auto cast_buffer = reinterpret_cast<std::uint8_t *>(buffer);

   obj._get_data->insert(obj._get_data->end(), cast_buffer, cast_buffer+resolved_size);

   return resolved_size;
}

std::size_t HTTPRequest::HeaderCallback(char *buffer, std::size_t unit_size, std::size_t mem_size, void *userdata) {
   auto &obj = *reinterpret_cast<HTTPRequest *>(userdata);
   auto resolved_size = unit_size * mem_size;
   std::string header_data(buffer, buffer+resolved_size);
   MLX_DEBUGN("unparsed header data: {}", header_data);
   
   auto split_point = header_data.find(':');
   auto header_key = header_data.substr(0, split_point);
   MLX_DEBUGN("header key: {}", header_key);
   
   std::string header_value;

   if (split_point == std::string::npos)
      return resolved_size;
   
   split_point += 1;

   while (split_point < header_data.size() && (header_data[split_point] == ' ' || header_data[split_point] == '\t'))
      ++split_point;

   auto linefeed = header_data.find("\r\n");
   header_value = header_data.substr(split_point, linefeed-split_point);
   MLX_DEBUGN("header value: {}", header_value);

   obj._recv_headers.push_back(std::make_pair(header_key, header_value));

   return resolved_size;
}

void HTTPRequest::set_url(const std::string &url) {
   this->_url = url;
   
   curl_easy_setopt(**this, CURLOPT_URL, this->_url.c_str());
}

CURLcode HTTPRequest::perform(void) {
   struct curl_slist *headers = nullptr;
   this->_get_data.reset();

   for (auto &entry : this->_req_headers)
      headers = curl_slist_append(headers, fmt::format("{}: {}", entry.first, entry.second).c_str());

   if (headers != nullptr)
      curl_easy_setopt(**this, CURLOPT_HTTPHEADER, headers);

   auto result = curl_easy_perform(**this);

   if (headers != nullptr)
      curl_slist_free_all(headers);

   return result;
}

std::vector<std::pair<std::string,std::string>> &HTTPRequest::request_headers(void) {
   return this->_req_headers;
}

std::map<std::string,std::string> HTTPRequest::request_header_map(void) {
   return std::map<std::string,std::string>(this->_req_headers.begin(), this->_req_headers.end());
}

void HTTPRequest::set_header(const std::string &key, const std::string &value) {
   this->_req_headers.push_back(std::make_pair(key,value));
}

void HTTPRequest::remove_header(const std::string &key) {
   for (auto iter=this->_req_headers.begin(); iter!=this->_req_headers.end(); ++iter)
   {
      if (iter->first == key)
      {
         this->_req_headers.erase(iter);
         return;
      }
   }
}

std::vector<std::pair<std::string,std::string>> &HTTPRequest::received_headers(void) {
   return this->_recv_headers;
}

std::map<std::string,std::string> HTTPRequest::received_headers_map(void) {
   return std::map<std::string,std::string>(this->_recv_headers.begin(), this->_recv_headers.end());
}

bool HTTPRequest::has_data(void) {
   return this->_get_data.has_value();
}

std::vector<std::uint8_t> &HTTPRequest::get_data(void) {
   if (!this->has_data())
      throw exception::HTTPDataNotReceived();

   return *this->_get_data;
}

bool HTTPRequest::has_put_data(void) {
   return this->_put_data.has_value();
}

std::vector<std::uint8_t> &HTTPRequest::put_data(void) {
   if (!this->has_put_data())
      throw exception::NoPUTData();

   return this->_put_data->first;
}

void HTTPRequest::set_put_data(const std::vector<std::uint8_t> &data) {
   this->_put_data = std::make_pair(data, 0);
   curl_easy_setopt(**this, CURLOPT_UPLOAD, 1L);

   curl_off_t size = data.size();
   curl_easy_setopt(**this, CURLOPT_INFILESIZE_LARGE, size);
}

bool HTTPRequest::has_post_data(void) {
   return this->_post_data.has_value();
}

std::string &HTTPRequest::post_data(void) {
   if (!this->has_post_data())
      throw exception::NoPOSTData();

   return *this->_post_data;
}

void HTTPRequest::set_post_data(const std::string &data) {
   this->_post_data = data;

   curl_easy_setopt(**this, CURLOPT_POSTFIELDSIZE, this->_post_data->size());
   curl_easy_setopt(**this, CURLOPT_POSTFIELDS, this->_post_data->c_str());
}

long HTTPRequest::response_code(void) {
   long result;

   curl_easy_getinfo(**this, CURLINFO_RESPONSE_CODE, &result);

   return result;
}
