#pragma once
#pragma comment(lib, "iphlpapi.lib")
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include <fstream>
#include <boost/enable_shared_from_this.hpp>
#include <stdio.h>
#include <Windows.h>
#include <Iphlpapi.h>
#include <Assert.h>


using boost::asio::ip::tcp;

static bool debugmode = true;
static boost::mutex debug_mutex;
static std::ostream debug_global(std::clog.rdbuf());



class async_tcp_client
{	

private:
	void handle_resolve(const boost::system::error_code & err, tcp::resolver::iterator endpoint_iterator);
	void handle_connect(const boost::system::error_code &err, tcp::resolver::iterator endpoint_iterator);
	void handle_write_file(const boost::system::error_code& err);

	tcp::resolver resolver_;
	tcp::socket socket_;
	boost::array<char, 1024> buf;
	boost::asio::streambuf request_;
	std::ifstream source_file;

public:
	async_tcp_client(boost::asio::io_service& io_service, const std::string& server, const std::string& path);
};

class async_tcp_connection : public boost::enable_shared_from_this<async_tcp_connection>
{
private:
	boost::asio::streambuf request_buf;
	std::ofstream output_file;
	boost::asio::ip::tcp::socket socket_;
	size_t file_size;
	boost::array<char, 40960> buf;
	void handle_read_request(const boost::system::error_code& err, std::size_t bytes_transferred);
	void handle_read_file_content(const boost::system::error_code& err, std::size_t bytes_transferred);
	void handle_error(const std::string& function_name, const boost::system::error_code& err);
	
public:
	async_tcp_connection(boost::asio::io_service& io_service);
	void start();
	boost::asio::ip::tcp::socket& socket();
};

class async_tcp_server : private boost::noncopyable
{
private:
	boost::asio::io_service io_service_;
	boost::asio::ip::tcp::acceptor acceptor_;

public:
	typedef boost::shared_ptr<async_tcp_connection> ptr_async_tcp_connection;
	async_tcp_server(unsigned short port);
	void handle_accept(ptr_async_tcp_connection current_connection, const boost::system::error_code& e);
	~async_tcp_server();
	void AsynUpdate();
};

void send_data(std::string const& filename, std::string const& adr);
void rec_data(std::string const& port);
void StartTransfer(std::string const& filename, std::string const& adr);
void StartListen(std::string const& port);
char* getMAC();