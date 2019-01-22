#include "stdafx.h"
#include "net_handle.h"
#include "resource.h"
#include "mfc_window/okex_futures_dlg.h"
#include "nmsg_server_iml.h"
extern COKExFuturesDlg* g_pDlg;

server_net_handle::server_net_handle() : _session(NULL)
{

}
bool server_net_handle::init()
{
	_session = NULL;
	_add(new nmsg_server_iml());
	return true;
}
// 连接账号服务器成功
void server_net_handle::connection(clib::psession ps)
{
	if(ps != NULL && _session==NULL)
	{
		_session = ps;
		_session->add_ref();
		nmsg_server_iml::cg_login(_session, g_pDlg->GetAPIKey().c_str());
	}
}

void server_net_handle::close(clib::psession ps)
{
	if(_session)
	{
		_session->del_ref();
		_session = NULL;
	}
	g_pDlg->ConnectServer();
}

void server_net_handle::error(clib::psession ps, s_int32 etype)
{
	if(etype == clib::nerror_connection)
	{// 如果连接失败,则过一会儿再连接
		g_pDlg->RetryConnectServer();
	}
}
bool server_net_handle::receive(clib::psession c, clib::pnpackage ppack)
{
	return clib::sample_handle::receive(c, ppack);
}
