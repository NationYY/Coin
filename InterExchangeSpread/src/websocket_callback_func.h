#pragma once
extern CInterExchangeSpreadDlg* g_pDlg;
void common_websocket_callbak_open(const char* szExchangeName)
{
	g_pDlg->AddLog("%s websocket success connect!", szExchangeName);
	if(g_pDlg->m_bIsRun)
		g_pDlg->OnWebsocketConnect(szExchangeName);
}

void common_websocket_callbak_close(const char* szExchangeName)
{
	g_pDlg->AddLog("%s websocket disconnect!", szExchangeName);
}

void common_websocket_callbak_fail(const char* szExchangeName)
{
	g_pDlg->AddLog("%s websocket fail connect!", szExchangeName);
}


