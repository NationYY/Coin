#pragma once
#include <clib/lib/net/sample_factory.h>
class client_factory : public clib::sample_factory
{
public:
	client_factory();
	virtual ~client_factory();
public:
	virtual clib::pclient create_client(boost::asio::io_service& io_service,
		clib::pclient_manager pmanager,
		clib::phandle ph, clib::ifactory * pf);
};
typedef client_factory* pclient_factory;

