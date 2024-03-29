#include "stdafx.h"
#include "IDEAI_VideoTransfer.h"
bool Is_finish;

    async_tcp_client::async_tcp_client(boost::asio::io_service& io_service, const std::string& server, const std::string& path)
		: resolver_(io_service), socket_(io_service)
	{
		size_t pos = server.find(':');
		if (pos == std::string::npos)
		{
			return;
		}
		std::string port_string = server.substr(pos + 1);
		std::string server_ip_or_host = server.substr(0, pos);
		source_file.open(path, std::ios_base::binary | std::ios_base::ate);
		if (!source_file)
		{
			boost::mutex::scoped_lock lk(debug_mutex);
			std::cout << __LINE__ << "Failed to open " << path << std::endl;
			return;
		}
		size_t file_size = source_file.tellg();
		source_file.seekg(0);
		std::ostream request_stream(&request_);
		request_stream << path << "\n" << file_size << "\n\n";
		{
			boost::mutex::scoped_lock lk(debug_mutex);
			std::cout << "Request size: " << request_.size() << std::endl;
		}
		tcp::resolver::query query(server_ip_or_host, port_string);
		resolver_.async_resolve(query, boost::bind(&async_tcp_client::handle_resolve, this, boost::asio::placeholders::error, boost::asio::placeholders::iterator));
	};

	void async_tcp_client::handle_resolve(const boost::system::error_code & err, tcp::resolver::iterator endpoint_iterator)
	{
		if (!err)
		{
			tcp::endpoint endpoint = *endpoint_iterator;
			socket_.async_connect(endpoint, boost::bind(&async_tcp_client::handle_connect, this, boost::asio::placeholders::error, ++endpoint_iterator));
		}
		else
		{
			boost::mutex::scoped_lock lk(debug_mutex);
			std::cout << "Error: " << err.message() << '\n';
		}
	};

	void async_tcp_client::handle_connect(const boost::system::error_code &err, tcp::resolver::iterator endpoint_iterator)
	{
		if (!err)
		{
			boost::asio::async_write(socket_, request_, boost::bind(&async_tcp_client::handle_write_file, this, boost::asio::placeholders::error));
		}
		else if (endpoint_iterator != tcp::resolver::iterator())
		{
			socket_.close();
			tcp::endpoint endpoint = *endpoint_iterator;
			socket_.async_connect(endpoint, boost::bind(&async_tcp_client::handle_connect, this, boost::asio::placeholders::error, ++endpoint_iterator));
		}
		else
		{
			boost::mutex::scoped_lock lk(debug_mutex);
			std::cout << "Error: " << err.message() << '\n';
		};
	}

	void async_tcp_client::handle_write_file(const boost::system::error_code& err)
	{
		if (!err)
		{
			if (source_file)
				//if(source_file.eof() == false)
			{
				source_file.read(buf.c_array(), (std::streamsize)buf.size());
				if (source_file.gcount() <= 0)
				{
					boost::mutex::scoped_lock lk(debug_mutex);
					std::cout << "read file error" << std::endl;
					return;
				};
				{
					boost::mutex::scoped_lock lk(debug_mutex);
					std::cout << "Send " << source_file.gcount() << "bytes, total: " << source_file.tellg() << " bytes.\n";
				}
				boost::asio::async_write(socket_, boost::asio::buffer(buf.c_array(), source_file.gcount()), boost::bind(&async_tcp_client::handle_write_file, this, boost::asio::placeholders::error));
			}
			else
			{
				return;
			}
		}
		else
		{
			boost::mutex::scoped_lock lk(debug_mutex);
			std::cout << "Error: " << err.message() << "\n";
		}
	};




	async_tcp_connection::async_tcp_connection(boost::asio::io_service& io_service)
		: socket_(io_service), file_size(0)
	{
	}
	void async_tcp_connection::start()
	{
		if (debugmode)
		{
			boost::mutex::scoped_lock lk(debug_mutex);
			debug_global << __FUNCTION__ << std::endl;
		}
		//Start an asynchronous operation to read data into a streambuf
		async_read_until(socket_, request_buf, "\n\n", boost::bind(&async_tcp_connection::handle_read_request, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}
	boost::asio::ip::tcp::socket& async_tcp_connection::socket()
	{
		return socket_;
	}


	void async_tcp_connection::handle_read_request(const boost::system::error_code& err, std::size_t bytes_transferred)
	{
		if (err)
		{
			return handle_error(__FUNCTION__, err);
		}
		if (debugmode)
		{
			boost::mutex::scoped_lock lk(debug_mutex);
			debug_global << __FUNCTION__ << "(" << bytes_transferred << ")" << ", in_avail = " << request_buf.in_avail() << ", size = " << request_buf.size() << ", max_size = " << request_buf.max_size() << ".\n";
		}
		std::istream request_stream(&request_buf);
		std::string file_path;
		request_stream >> file_path;
		request_stream >> file_size;
		request_stream.read(buf.c_array(), 2);
		if (debugmode)
		{
			boost::mutex::scoped_lock lk(debug_mutex);
			debug_global << file_path << " size is " << file_size << ", tellg = " << request_stream.tellg() << std::endl;
		}
		size_t pos = file_path.find_last_of('\\');
		if (pos != std::string::npos)
		{
			file_path = file_path.substr(pos + 1);
		}
		//output_file.open(file_path, std::ios_base::binary);
		std::string path = "Input\\Movies\\";
		size_t pos1 = file_path.find('.');
		std::string str_container = file_path.substr(pos1);
		path = path + "CropVideo" + str_container;
		output_file.open(path, std::ios_base::binary);
		if (!output_file)
		{
			if (debugmode)
			{
				boost::mutex::scoped_lock lk(debug_mutex);
				debug_global << __LINE__ << "Failed to open: " << file_path << std::endl;
			}
			return;
		}
		do
		{
			request_stream.read(buf.c_array(), (std::streamsize)buf.size());
			if (debugmode)
			{
				boost::mutex::scoped_lock lk(debug_mutex);
				debug_global << __FUNCTION__ << " write " << request_stream.gcount() << " bytes.\n";
			}
			output_file.write(buf.c_array(), request_stream.gcount());
		} while (request_stream.gcount() > 0);
		async_read(socket_, boost::asio::buffer(buf.c_array(), buf.size()), boost::bind(&async_tcp_connection::handle_read_file_content, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}

	void async_tcp_connection::handle_read_file_content(const boost::system::error_code& err, std::size_t bytes_transferred)
	{
		if (bytes_transferred>0)
		{
			output_file.write(buf.c_array(), (std::streamsize)bytes_transferred);
			if (debugmode)
			{
				boost::mutex::scoped_lock lk(debug_mutex);
				debug_global << __FUNCTION__ << " recv " << output_file.tellp() << " bytes." << std::endl;
			}
			if (output_file.tellp() >= (std::streamsize)file_size)
			{
				Is_finish = true;
				return;
				
			}
		}
		if (err)
		{
			return handle_error(__FUNCTION__, err);
		}
		async_read(socket_, boost::asio::buffer(buf.c_array(), buf.size()), boost::bind(&async_tcp_connection::handle_read_file_content, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}

	void async_tcp_connection::handle_error(const std::string& function_name, const boost::system::error_code& err)
	{
		if (debugmode)
		{
			boost::mutex::scoped_lock lk(debug_mutex);
			debug_global << __FUNCTION__ << " in " << function_name << " due to " << err << " " << err.message() << std::endl;
		}
	}

	async_tcp_server::async_tcp_server(unsigned short port) :acceptor_(io_service_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port), true)
	{
		ptr_async_tcp_connection new_connection_(new async_tcp_connection(io_service_));
		acceptor_.async_accept(new_connection_->socket(), boost::bind(&async_tcp_server::handle_accept, this, new_connection_, boost::asio::placeholders::error));
		//io_service_.run();
		io_service_.poll();
	}
	void async_tcp_server::handle_accept(ptr_async_tcp_connection current_connection, const boost::system::error_code& e)
	{
		if (debugmode)
		{
			boost::mutex::scoped_lock lk(debug_mutex);
			debug_global << __FUNCTION__ << " " << e << ", " << e.message() << std::endl;
		}
		if (!e)
		{
			current_connection->start();
		}
	}

	async_tcp_server::~async_tcp_server()
	{
		io_service_.stop();
	}

	void async_tcp_server::AsynUpdate()
	{
		io_service_.poll();
	}



void send_data(std::string const& filename, std::string const& adr)
{
	try
	{
		boost::asio::io_service io_service;
		{
			boost::mutex::scoped_lock lk(debug_mutex);
			std::cout << "Adress is: " << adr << " and file is: " << filename << '\n';
		}

		if (debugmode)
		{
			boost::mutex::scoped_lock lk(debug_mutex);
			debug_global << adr << '\n';
		}

		async_tcp_client client(io_service, adr, filename);
		io_service.run();
	}
	catch (std::exception const& e)
	{
		std::cerr << "Exception in " << ": " << e.what() << "\n";
	};
};

void rec_data(std::string const& port)
{
	try
	{
		{
			boost::mutex::scoped_lock lk(debug_mutex);
			std::cout << "Receiving data...\n";
		}
		int num = std::stoi(port);
		async_tcp_server recv_file_tcp_server(num);
		if (debugmode)
		{
			boost::mutex::scoped_lock lk(debug_mutex);
			debug_global << "Received\n";
		}
		while (!Is_finish)
			recv_file_tcp_server.AsynUpdate();
	}
	catch (std::exception const& e)
	{
		std::cerr << "Exception in " << ": " << e.what() << "\n";
	};
};

void StartTransfer(std::string const& filename, std::string const& adr)
{
	boost::thread_group g;
	boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
	g.create_thread([&] { send_data(filename,adr); });
	boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
	g.join_all();
}

void StartListen(std::string const& port)
{
	//set poll flag 
	Is_finish = false;
	boost::thread_group g;
	boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
	g.create_thread([&] { rec_data(port); }); // get the receiver running
	boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
	g.join_all();

}


char* getMAC() {
	//structure contains information about a particular network adapter on the local computer.
	PIP_ADAPTER_INFO AdapterInfo;
	DWORD dwBufLen = sizeof(AdapterInfo);
	char *mac_addr = (char*)malloc(17);

	AdapterInfo = (IP_ADAPTER_INFO *)malloc(sizeof(IP_ADAPTER_INFO));
	if (AdapterInfo == NULL) {
		printf("Error allocating memory needed to call GetAdaptersinfo\n");

	}

	// Make an initial call to GetAdaptersInfo to get the necessary size into the dwBufLen     variable
	if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == ERROR_BUFFER_OVERFLOW) {

		AdapterInfo = (IP_ADAPTER_INFO *)malloc(dwBufLen);
		if (AdapterInfo == NULL) {
			printf("Error allocating memory needed to call GetAdaptersinfo\n");
		}
	}

	if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == NO_ERROR) {
		PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;// Contains pointer to current adapter info
		do {
			//put the entire resulting string to mac_addr
			sprintf(mac_addr, "%02X:%02X:%02X:%02X:%02X:%02X",
				pAdapterInfo->Address[0], pAdapterInfo->Address[1],
				pAdapterInfo->Address[2], pAdapterInfo->Address[3],
				pAdapterInfo->Address[4], pAdapterInfo->Address[5]);
			//print IP address, mac_address
			//printf("Address: %s, mac: %s\n", pAdapterInfo->IpAddressList.IpAddress.String, mac_addr);
			return mac_addr;

			printf("\n");
			pAdapterInfo = pAdapterInfo->Next;
		} while (pAdapterInfo);
	}
	free(AdapterInfo);
}