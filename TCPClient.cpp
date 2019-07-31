#include "stdafx.h"
#include "IDEAI_TCPClient.h"


IDEAI_TCPClient::IDEAI_TCPClient(char* ptr, const std::string& server_ip, const int& port)
{
	try
	{
		////Get local macaddress
		//char* ptr = getMAC();

		// �Ҧ�asio�����ݭnio_service�ﹳ
		boost::asio::io_service io_service;
		// socket��H
		tcp::socket socket(io_service);
		// �]�w���I�A�o�̨ϥΤF�����s���A�i�H�ק�IP�a�}���ջ��{�s��
		tcp::endpoint ep(boost::asio::ip::address_v4::from_string(server_ip), port);
		// �s��Server
		boost::system::error_code ec;
		socket.connect(ep, ec);

		/*tcp::resolver resolver(io_service);
		tcp::resolver::query query("192.168.2.12", "2222");
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

		tcp::socket socket(io_service);
		boost::asio::connect(socket, endpoint_iterator);*/

		for (;;)
		{


			// �VServer�ݵo�emac address
			std::string message(ptr);
			boost::system::error_code ignored_error;
			boost::asio::write(socket, boost::asio::buffer(message), ignored_error);

			//�qServer�ݱ���message
			boost::array<char, 128> buf;
			boost::system::error_code error;
			size_t len = socket.read_some(boost::asio::buffer(buf), error);
			//std::cout.write(buf.data(), len);

			if (error == boost::asio::error::eof)
				break; // Connection closed cleanly by peer. // Handle disconnection.
			else if (error)
				throw boost::system::system_error(error); // Some other error.


		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

}


IDEAI_TCPClient::~IDEAI_TCPClient()
{
}


//char* getMAC() {
//	//structure contains information about a particular network adapter on the local computer.
//	PIP_ADAPTER_INFO AdapterInfo;
//	DWORD dwBufLen = sizeof(AdapterInfo);
//	char *mac_addr = (char*)malloc(17);
//
//	AdapterInfo = (IP_ADAPTER_INFO *)malloc(sizeof(IP_ADAPTER_INFO));
//	if (AdapterInfo == NULL) {
//		printf("Error allocating memory needed to call GetAdaptersinfo\n");
//
//	}
//
//	// Make an initial call to GetAdaptersInfo to get the necessary size into the dwBufLen     variable
//	if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == ERROR_BUFFER_OVERFLOW) {
//
//		AdapterInfo = (IP_ADAPTER_INFO *)malloc(dwBufLen);
//		if (AdapterInfo == NULL) {
//			printf("Error allocating memory needed to call GetAdaptersinfo\n");
//		}
//	}
//
//	if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == NO_ERROR) {
//		PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;// Contains pointer to current adapter info
//		do {
//			//put the entire resulting string to mac_addr
//			sprintf(mac_addr, "%02X:%02X:%02X:%02X:%02X:%02X",
//				pAdapterInfo->Address[0], pAdapterInfo->Address[1],
//				pAdapterInfo->Address[2], pAdapterInfo->Address[3],
//				pAdapterInfo->Address[4], pAdapterInfo->Address[5]);
//			//print IP address, mac_address
//			//printf("Address: %s, mac: %s\n", pAdapterInfo->IpAddressList.IpAddress.String, mac_addr);
//			return mac_addr;
//
//			printf("\n");
//			pAdapterInfo = pAdapterInfo->Next;
//		} while (pAdapterInfo);
//	}
//	free(AdapterInfo);
//}

