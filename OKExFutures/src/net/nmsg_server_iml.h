#pragma once
#include "net/nmsg_server.h"
class nmsg_server_iml : public nmsg_server
{
public:
	nmsg_server_iml();
	~nmsg_server_iml();
	virtual bool gc_login_rst(clib::psession __c, clib::pnpackage __p, s_int32 rst, const clib::string& time);
	virtual bool gc_pong(clib::psession __c, clib::pnpackage __p);
	virtual bool gc_account_invalid(clib::psession __c, clib::pnpackage __p);
};

