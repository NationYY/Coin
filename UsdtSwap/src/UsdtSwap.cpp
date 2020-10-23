// InformationGathering.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>
#include "common/dump.h"
#include <clib/lib/util/config.h>
#include <clib/lib/file/file_util.h>
#include "log/local_log.h"
#include "log/local_action_log.h"
#include "exchange/okex/okex_exchange.h"
#include "exchange/binance/binance_exchange.h"
#include "common/func_common.h"
#include "api_callback.h"
#include <conio.h>
#include <log/log.h>
COkexExchange* pOkexExchange = NULL;
CBinanceExchange* pBinanceExchange = NULL;
clib::config init_config;
string okex_api_key = "";
string okex_secret_key = "";
string okex_passphrase = "";
string binance_api_key = "";
string binance_secret_key = "";
/*固定配置*/

std::string strCoinType = "BTC";
std::string strStandardCurrency = "USDT";


////////////
int nExitCode = 0;
boost::thread logicThread;
void LogicThread();
void Update15Sec();
void OKexPong();
void BinancePong();
void OnOkexWSConnectSuccess();
void OnOkexWSLoginSuccess();
void OnBinanceMarketWSConnectSuccess();
void OnBinanceAccountWSConnectSuccess();
void OnBinanceGotListenKey(std::string key);
void BinanceMarketSubscribe();
time_t tLastUpdate15Sec = 0;
time_t tListenOkexPong = 0;
time_t tListenBinancePing = 0;
std::string strBinanceListenKey = "";
#include "algorithm/hmac.h"
int _tmain(int argc, _TCHAR* argv[])
{
	std::string m_strSecretKey = "2b5eb11e18796d12d88f13dc27dbbd02c2cc51ff7059765ed9821957d82bb4d9";
	std::string confirmation = "symbol=BTCUSDT&side=BUY&type=LIMIT&timeInForce=GTC&quantity=1&price=0.1&recvWindow=5000&timestamp=1499827319559";
	unsigned char *out = NULL;
	unsigned int outSize = 0;
	HmacEncode("sha256", m_strSecretKey.c_str(), m_strSecretKey.length(), confirmation.c_str(), confirmation.length(), out, outSize);


	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);

	HANDLE stdIn = GetStdHandle(STD_INPUT_HANDLE);
	SetConsoleMode(stdIn, 0);
	logicThread = boost::thread(&LogicThread);

	if(!init_config.open("./config.ini"))
		cout<<"无法打开配置"<<std::endl;
	okex_api_key = init_config.get("okex", "api_key", "");
	okex_secret_key = init_config.get("okex", "secret_key", "");
	okex_passphrase = init_config.get("okex", "passphrase", "");

	binance_api_key = init_config.get("binance", "api_key", "");
	binance_secret_key = init_config.get("binance", "secret_key", "");

	char tempBuf[1024] = "";
	DWORD count = 0;
	DWORD len = 0;
	INPUT_RECORD irb[32];
	while(nExitCode <= 0 && ReadConsoleInput(stdIn, irb, 32, &len) && len)
	{
		for(DWORD i = 0; i < len; i++)
		{
			if(irb[i].EventType == KEY_EVENT && irb[i].Event.KeyEvent.bKeyDown)
			{
				if(irb[i].Event.KeyEvent.dwControlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED))
					continue; // 按住Ctrl键时，输入无效！

				wchar_t u = irb[i].Event.KeyEvent.uChar.UnicodeChar;
				if(u == 0 || u == '\n')
					continue;

				if(irb[i].Event.KeyEvent.wVirtualKeyCode != VK_BACK)
					tempBuf[count++] = irb[i].Event.KeyEvent.uChar.AsciiChar;
				else
				{
					// 处理回退键
					if(count > 0)
					{
						_putch(irb[i].Event.KeyEvent.uChar.UnicodeChar);
						_putch(0);
						tempBuf[--count] = 0;
					}
				}
				_putch(irb[i].Event.KeyEvent.uChar.UnicodeChar);

				// 当收到回车时处理命令
				if(irb[i].Event.KeyEvent.uChar.UnicodeChar == '\r')
				{
					_putch('\n');
					tempBuf[count - 1] = 0;
					count = 0;
					if(strcmp(tempBuf, "exit") == 0)
					{
						nExitCode = 1;
						logicThread.join();
					}
				}
			}
		}
	}
	system("pause");
	return 0;
}


void LogicThread()
{
	pOkexExchange = new COkexExchange(okex_api_key, okex_secret_key, okex_passphrase);
	pOkexExchange->SetHttpCallBackMessage(okex_http_callbak_message);
	pOkexExchange->SetMarketWebSocketCallBackOpen(okex_websocket_callbak_open);
	pOkexExchange->SetMarketWebSocketCallBackClose(okex_websocket_callbak_close);
	pOkexExchange->SetMarketWebSocketCallBackFail(okex_websocket_callbak_fail);
	pOkexExchange->SetMarketWebSocketCallBackMessage(okex_websocket_callbak_message);
	pOkexExchange->Run(false);

	pBinanceExchange = new CBinanceExchange(binance_api_key, binance_secret_key);
	pBinanceExchange->SetHttpCallBackMessage(binance_http_callbak_message);
	pBinanceExchange->SetMarketWebSocketCallBackOpen(binance_market_websocket_callbak_open);
	pBinanceExchange->SetMarketWebSocketCallBackClose(binance_market_websocket_callbak_close);
	pBinanceExchange->SetMarketWebSocketCallBackFail(binance_market_websocket_callbak_fail);
	pBinanceExchange->SetMarketWebSocketCallBackMessage(binance_market_websocket_callbak_message);

	pBinanceExchange->SetAccountWebSocketCallBackOpen(binance_account_websocket_callbak_open);
	pBinanceExchange->SetAccountWebSocketCallBackClose(binance_account_websocket_callbak_close);
	pBinanceExchange->SetAccountWebSocketCallBackFail(binance_account_websocket_callbak_fail);
	pBinanceExchange->SetAccountWebSocketCallBackMessage(binance_account_websocket_callbak_message);
	BinanceMarketSubscribe();
	pBinanceExchange->Run();

	clib::log::start_debug_file(false);

	clib::string log_path = "log/";
	bool bRet = clib::file_util::mkfiledir(log_path.c_str(), true);

	CLocalLogger& _localLogger = CLocalLogger::GetInstance();
	_localLogger.SetBatchMode(true);
	_localLogger.SetLogPath(log_path.c_str());
	_localLogger.Start();

	CLocalActionLog::GetInstancePt()->set_log_path(log_path.c_str());
	CLocalActionLog::GetInstancePt()->start();
	
	LOCAL_INFO("初始化完成");

	while(nExitCode <= 0)
	{
		CLocalLogger::GetInstancePt()->SwapFront2Middle();
		pOkexExchange->Update();
		pBinanceExchange->Update();
		Update15Sec();
		boost::this_thread::sleep(boost::posix_time::milliseconds(10));
	}
	LOCAL_INFO("退出了");
	CLocalLogger::GetInstancePt()->SwapFront2Middle();
	boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
	CLocalLogger::ReleaseInstance();
	CLocalActionLog::ReleaseInstance();
}

void Update15Sec()
{
	time_t tNow = time(NULL);
	if(tListenOkexPong && tNow - tListenOkexPong > 15)
	{
		tListenOkexPong = 0;
		delete pOkexExchange;
		pOkexExchange = new COkexExchange(okex_api_key, okex_secret_key, okex_passphrase);
		pOkexExchange->SetHttpCallBackMessage(okex_http_callbak_message);
		pOkexExchange->SetMarketWebSocketCallBackOpen(okex_websocket_callbak_open);
		pOkexExchange->SetMarketWebSocketCallBackClose(okex_websocket_callbak_close);
		pOkexExchange->SetMarketWebSocketCallBackFail(okex_websocket_callbak_fail);
		pOkexExchange->SetMarketWebSocketCallBackMessage(okex_websocket_callbak_message);
		pOkexExchange->Run();
		return;
	}
	if(tListenBinancePing && tNow - tListenBinancePing > 240)
	{
		tListenBinancePing = 0;
		delete pBinanceExchange;
		pBinanceExchange = new CBinanceExchange(binance_api_key, binance_secret_key);
		pBinanceExchange->SetHttpCallBackMessage(binance_http_callbak_message);
		pBinanceExchange->SetMarketWebSocketCallBackOpen(binance_market_websocket_callbak_open);
		pBinanceExchange->SetMarketWebSocketCallBackClose(binance_market_websocket_callbak_close);
		pBinanceExchange->SetMarketWebSocketCallBackFail(binance_market_websocket_callbak_fail);
		pBinanceExchange->SetMarketWebSocketCallBackMessage(binance_market_websocket_callbak_message);

		pBinanceExchange->SetAccountWebSocketCallBackOpen(binance_account_websocket_callbak_open);
		pBinanceExchange->SetAccountWebSocketCallBackClose(binance_account_websocket_callbak_close);
		pBinanceExchange->SetAccountWebSocketCallBackFail(binance_account_websocket_callbak_fail);
		pBinanceExchange->SetAccountWebSocketCallBackMessage(binance_account_websocket_callbak_message);
		BinanceMarketSubscribe();
		pBinanceExchange->Run();
	}

	if(tLastUpdate15Sec == 0)
		tLastUpdate15Sec = tNow;
	if(tNow - tLastUpdate15Sec < 15)
		return;
	tLastUpdate15Sec = tNow;
	OKEX_WEB_SOCKET->Ping();
	tListenOkexPong = time(NULL);
}

void OKexPong()
{
	tListenOkexPong = 0;
}

void BinancePong()
{
	LOCAL_INFO("binance ping");
	BINANCE_MARKET_WEB_SOCKET->Ping();
	tListenBinancePing = time(NULL);
}

void OnOkexWSConnectSuccess()
{

}

void OnOkexWSLoginSuccess()
{
	
}


void OnBinanceMarketWSConnectSuccess()
{
	BinanceMarketSubscribe();
	BINANCE_HTTP->API_ListenKey(true);
}

void OnBinanceAccountWSConnectSuccess()
{

}

void OnBinanceGotListenKey(std::string key)
{
	if(strBinanceListenKey != key)
	{
		strBinanceListenKey = key;
		BINANCE_ACCOUNT_WEB_SOCKET->SetListenKey(key);
		pBinanceExchange->RunAccountWebSocket();
	}
	
}

void BinanceMarketSubscribe()
{
	BINANCE_MARKET_WEB_SOCKET->API_FuturesTickerData(true, strCoinType, strStandardCurrency);
}