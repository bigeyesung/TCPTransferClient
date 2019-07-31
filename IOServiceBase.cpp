#include "stdafx.h"
#include "IOServiceBase.h"



IOServiceBase::IOServiceBase()
{
	m_ioService = std::shared_ptr<boost::asio::io_service>(new boost::asio::io_service());

}

boost::asio::io_service & IOServiceBase::getIOService()
{
	return *m_ioService;
}
