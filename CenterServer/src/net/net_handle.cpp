#include "stdafx.h"
#include "net_handle.h"
#include "resource.h"
#include "nmsg_server_iml.h"
client_net_handle::client_net_handle()
{

}
bool client_net_handle::init()
{
	_add(new nmsg_server_iml());
	return true;
}
// 连接账号服务器成功
void client_net_handle::connection(clib::psession ps)
{
	if(ps != NULL)
		ps->close_delay(true);
}

void client_net_handle::close(clib::psession ps)
{
	
}

void client_net_handle::error(clib::psession ps, s_int32 etype)
{
	
}

bool client_net_handle::receive(clib::psession c, clib::pnpackage ppack)
{
	return clib::sample_handle::receive(c, ppack);
}
