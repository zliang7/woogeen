//
//Intel License
//

#include "asio_token.h"
#include <boost/asio.hpp>
#include <iostream>
#include <sstream>
#include <vector>

using boost::posix_time::milliseconds;
using std::stringstream;

using namespace std;
using boost::asio::ip::tcp;


std::vector<std::string>& splitex(const std::string& s,
                                char delim,
                                std::vector<std::string>& elems) {
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}

std::vector<std::string> splitex(const std::string& s, char delim) {
  std::vector<std::string> elems;
  splitex(s, delim, elems);
  return elems;
}

string getToken(const string& addr, const string& room_id) {
  try {
    boost::asio::io_service io_service;
    // Get a list of endpoints corresponding to the server name.
    boost::asio::ip::tcp::resolver resolver(io_service);
    vector<string> list = splitex(addr, '/');
    string server = list[2];
    list = splitex(server, ':');
    tcp::resolver::query query(list[0], list[1]);
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    // Try each endpoint until we successfully establish a connection.
    tcp::socket socket(io_service);
    boost::asio::connect(socket, endpoint_iterator);

    // Body
    string content;
    if(!room_id.empty()){
      content = "{\"role\":\"presenter\",\"username\":\"user\",\"room\":\"" + room_id + "\"}";
    }else{
      content = "{\"room\":\"\",\"role\":\"presenter\",\"username\":\"user\"}";
    }
    cout <<"the token to pass to server:"<<content<<endl;
    // Form the request. We specify the "Connection: close" header so that the
    // server will close the socket after transmitting the response. This will
    // allow us to treat all data up until the EOF as the content.
    boost::asio::streambuf request;
    std::ostream request_stream(&request);
    request_stream << "POST "
                   << "/createToken/"
                   << " HTTP/1.1\r\n";
    request_stream << "Host: " << list[0] + ":" + list[1] << "\r\n";
    request_stream << "Accept: application/json\r\n";
    request_stream << "Content-Type: application/json\r\n";
    request_stream << "Content-Length: " << content.length() << "\r\n";
    request_stream << "Connection: close\r\n\r\n";
    request_stream << content;

    // Send the request.
    boost::asio::write(socket, request);

    // Read the response status line. The response streambuf will automatically
    // grow to accommodate the entire line. The growth may be limited by passing
    // a maximum size to the streambuf constructor.
    boost::asio::streambuf response;
    boost::asio::read_until(socket, response, "\r\n");

    // Check that response is OK.
    std::istream response_stream(&response);
    std::string http_version;
    response_stream >> http_version;
    unsigned int status_code;
    response_stream >> status_code;
    std::string status_message;
    std::getline(response_stream, status_message);
    if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
      std::cout << "Invalid response\n";
      return "";
    }
    if (status_code != 200) {
      std::cout << "Response returned with status code " << status_code << "\n";
      return "";
    }

    // Read the response headers, which are terminated by a blank line.
    boost::asio::read_until(socket, response, "\r\n\r\n");

    // Process the response headers.
    std::string header;
    while (std::getline(response_stream, header) && header != "\r")
      std::cout << header << "\n";
    std::cout << "\n";

    std::ostringstream token_stream;

    // Write whatever content we already have to output.
    if (response.size() > 0) {
      token_stream << &response;
    }

    // Read until EOF, writing data to output as we go.
    boost::system::error_code error;
    while (boost::asio::read(socket, response,
                             boost::asio::transfer_at_least(1), error))
      token_stream << &response;
    if (error != boost::asio::error::eof)
      throw boost::system::system_error(error);
    return token_stream.str();
  } catch (std::exception& e) {
    std::cout << "Exception: " << e.what() << "\n";
    return "";
  }
}

