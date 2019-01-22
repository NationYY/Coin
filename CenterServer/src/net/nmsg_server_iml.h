#pragma once
#include "net/nmsg_server.h"
class nmsg_server_iml : public nmsg_server
{
public:
	nmsg_server_iml();
	~nmsg_server_iml();
	virtual bool cg_login(clib::psession __c, clib::pnpackage __p, const clib::string& key);
	virtual bool cg_ping(clib::psession __c, clib::pnpackage __p);
};

