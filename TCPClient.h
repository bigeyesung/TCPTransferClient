#pragma once
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <iostream>

//#include "stdafx.h"
//#include <iostream>
//#include <stdio.h>
//#include <Windows.h>
//#include <Iphlpapi.h>
//#include <Assert.h>
//#include "IDEAI_TCPClient.h"
//#pragma comment(lib, "iphlpapi.lib")

using boost::asio::ip::tcp;

class IDEAI_TCPClient
{
public:
	IDEAI_TCPClient(char* ptr, const std::string& server_ip, const int& port);
	~IDEAI_TCPClient();
};

//char* getMAC();
