#pragma once
#include <clib/lib/net/sample_handle.h>
#include <clib/lib/net/session.h>

class server_net_handle : public clib::sample_handle
{
public:
	server_net_handle();
	bool init();

	virtual void connection(clib::psession  ps);
	virtual void close(clib::psession  ps);
	virtual void error(clib::psession  ps, s_int32 etype);
	virtual void send_empty(clib::psession c){}
	virtual bool receive(clib::psession c, clib::pnpackage ppack);
	clib::psession _session;
};

