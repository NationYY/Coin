#pragma once
#include <clib/lib/net/sample_factory.h>
class server_factory : public clib::sample_factory
{
public:
	server_factory();
	virtual ~server_factory();
public:
	virtual clib::pclient create_client(boost::asio::io_service& io_service,
		clib::pclient_manager pmanager,
		clib::phandle ph, clib::ifactory * pf);
};
typedef server_factory* pserver_factory;

