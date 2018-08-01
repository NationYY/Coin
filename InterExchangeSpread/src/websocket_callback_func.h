#pragma once
extern CInterExchangeSpreadDlg* g_pDlg;
void coinex_websocket_callbak_open()
{
	g_pDlg->AddLog("coinex websocket success connect!");
}

void coinex_websocket_callbak_close()
{
	g_pDlg->AddLog("coinex websocket disconnect!");
}

void coinex_websocket_callbak_fail()
{
	g_pDlg->AddLog("coinex websocket fail connect!");
}



