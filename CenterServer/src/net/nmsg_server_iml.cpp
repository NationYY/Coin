#include "stdafx.h"
#include "nmsg_server_iml.h"
#include "logic/account_manager.h"
#include "Resource.h"
#include "mfc_window/CenterServerDlg.h"
extern CCenterServerDlg* g_pDlg;
nmsg_server_iml::nmsg_server_iml()
{
}


nmsg_server_iml::~nmsg_server_iml()
{
}

bool nmsg_server_iml::cg_login(clib::psession __c, clib::pnpackage __p, const clib::string& key)
{
	time_t tDate;
	bool bRet = g_pDlg->m_accountManager.CheckAccount(key, tDate, __c);
	char szBuff[128] = {};
	sprintf(szBuff, "%llu", tDate);
	nmsg_server::gc_login_rst(__c, bRet ? 0 : 1, szBuff);
	return true;
}

bool nmsg_server_iml::cg_ping(clib::psession __c, clib::pnpackage __p)
{
	nmsg_server::gc_pong(__c);
	return true;
}