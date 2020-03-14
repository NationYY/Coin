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
#include "common/func_common.h"
#include "api_callback.h"
#include <conio.h>
#include <log/log.h>
COkexExchange* pExchange = NULL;
clib::config init_config;
string apiKey = "";
string secretKey = "";
string passphrase = "";
int nExitCode = 0;
boost::thread logicThread;
void LogicThread();
void Update15Sec();
void Pong();
void Listen();
time_t tLastUpdate15Sec = 0;
time_t tListenPong = 0;
std::map<std::string, time_t> mapLastKlineTime;
std::map<std::string, __int64> mapLastKlineRecordTime;
int _tmain(int argc, _TCHAR* argv[])
{
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);

	HANDLE stdIn = GetStdHandle(STD_INPUT_HANDLE);
	SetConsoleMode(stdIn, 0);
	logicThread = boost::thread(&LogicThread);

	if(!init_config.open("./config.ini"))
		cout<<"无法打开配置"<<std::endl;
	apiKey = init_config.get("spot", "apiKey", "");
	secretKey = init_config.get("spot", "secretKey", "");
	passphrase = init_config.get("spot", "passphrase", "");


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
	pExchange = new COkexExchange(apiKey, secretKey, passphrase, true);
	pExchange->SetHttpCallBackMessage(local_http_callbak_message);
	pExchange->SetWebSocketCallBackOpen(local_websocket_callbak_open);
	pExchange->SetWebSocketCallBackClose(local_websocket_callbak_close);
	pExchange->SetWebSocketCallBackFail(local_websocket_callbak_fail);
	pExchange->SetWebSocketCallBackMessage(local_websocket_callbak_message);
	pExchange->Run();

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
		pExchange->Update();
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
	if(tListenPong && tNow - tListenPong > 15)
	{
		tListenPong = 0;
		delete pExchange;
		pExchange = new COkexExchange(apiKey, secretKey, passphrase, true);
		pExchange->SetHttpCallBackMessage(local_http_callbak_message);
		pExchange->SetWebSocketCallBackOpen(local_websocket_callbak_open);
		pExchange->SetWebSocketCallBackClose(local_websocket_callbak_close);
		pExchange->SetWebSocketCallBackFail(local_websocket_callbak_fail);
		pExchange->SetWebSocketCallBackMessage(local_websocket_callbak_message);
		pExchange->Run();
		return;
	}

	if(tLastUpdate15Sec == 0)
		tLastUpdate15Sec = tNow;
	if(tNow - tLastUpdate15Sec < 15)
		return;
	tLastUpdate15Sec = tNow;
	if(OKEX_WEB_SOCKET->Ping())
		tListenPong = time(NULL);
}

void Pong()
{
	tListenPong = 0;
}

void Listen()
{
	std::string strCnt = init_config.get("spot", "instrument_cnt", "");
	LOCAL_INFO("监听币对数:%s", strCnt.c_str());
	if(strCnt != "")
	{
		int cnt = atoi(strCnt.c_str());
		for(int i = 0; i < cnt; ++i)
		{

			char szKey[128] = { 0 };
			_snprintf_c(szKey, 128, "instrument_coin%d", i + 1);
			std::string strCoinType = init_config.get("spot", szKey, "");
			_snprintf_c(szKey, 128, "instrument_money%d", i + 1);
			std::string strMoneyType = init_config.get("spot", szKey, "");
			if(strCoinType != "" && strMoneyType != "")
			{
				OKEX_WEB_SOCKET->API_SpotTradeData(true, strCoinType, strMoneyType);
				std::string strKlineType = "candle60s";
				OKEX_WEB_SOCKET->API_SpotKlineData(true, strKlineType, strCoinType, strMoneyType);
				mapLastKlineTime[strCoinType + "-" + strMoneyType] = 0;
				mapLastKlineRecordTime[strCoinType + "-" + strMoneyType] = 0;
			}
		}
	}
}