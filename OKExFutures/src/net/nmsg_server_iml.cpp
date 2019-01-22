#include "stdafx.h"
#include "Resource.h"
#include "mfc_window/okex_futures_dlg.h"
#include "nmsg_server_iml.h"
#include "common/func_common.h"
extern COKExFuturesDlg* g_pDlg;

nmsg_server_iml::nmsg_server_iml()
{
}


nmsg_server_iml::~nmsg_server_iml()
{
}

bool nmsg_server_iml::gc_login_rst(clib::psession __c, clib::pnpackage __p, s_int32 rst, const clib::string& time)
{
	g_pDlg->OnRecvLoginRst(rst, CFuncCommon::ToTime(time.c_str()));
	return true;
}

bool nmsg_server_iml::gc_account_invalid(clib::psession __c, clib::pnpackage __p)
{
	g_pDlg->OnRecvAccountInvalid();
	return true;
}

bool nmsg_server_iml::gc_pong(clib::psession __c, clib::pnpackage __p)
{
	g_pDlg->OnRecvServerPong();
	return true;
}