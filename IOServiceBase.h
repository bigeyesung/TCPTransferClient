#pragma once
#include <boost/asio.hpp>

typedef boost::asio::io_service			IOService;
typedef std::shared_ptr<IOService>		IOServiceRef;


class IOServiceBase
{
public:
	IOServiceBase();
	virtual ~IOServiceBase() {}

protected:
	boost::asio::io_service&	getIOService();
private:
	IOServiceRef m_ioService;
};

