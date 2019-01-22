#include "stdafx.h"
#include "client_factory.h"
#include <clib/lib/net/client.h>
client_factory::client_factory() {

}

client_factory::~client_factory() {
}

clib::pclient client_factory::create_client(boost::asio::io_service& io_service,
	clib::pclient_manager pmanager,
	clib::phandle ph, clib::ifactory * pf)
{
	clib::pclient pc = new clib::client(io_service, pmanager, ph, pf);
	pc->set_time_out(600000);
	pc->set_max_package_size(64 * 1024);
	pc->set_limit_max_package_size(false);
	return pc;
}