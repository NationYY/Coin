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
string trade_type = "";
string trade_size = "";
string coin_type = "";
string money_type = "";
string futures_cycle = "";

bool is_swap_futures = false;
int nExitCode = 0;
boost::thread logicThread;
void LogicThread();
void Update15Sec();
void Pong();
void OnOkexWSConnectSuccess();
void OnOkexWSLoginSuccess();
void OnRevTickerInfo(STickerData &data);
void OnTradeSuccess(std::string& clientOrderID, std::string& serverOrderID);
void UpdateTradeInfo(SFuturesTradeInfo& info);
void TradeLogic();
void OnTradeFail(std::string& clientOrderID);
time_t tLastUpdate15Sec = 0;
time_t tListenPong = 0;
STickerData tick_data;
time_t tigger_time;
time_t success_time = 0;
int price_decimal = 0;
std::string open_off;
std::string close_rate;
std::string leverage;
double tigger_price = -1;
SFuturesTradeInfo trade_info;
time_t StringToDatetime(string str)
{
	char *cha = (char*)str.data();             // 将string转换成char*。
	tm tm_;                                    // 定义tm结构体。
	int year, month, day, hour, minute, second;// 定义时间的各个int临时变量。
	sscanf(cha, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);// 将string存储的日期时间，转换为int临时变量。
	tm_.tm_year = year - 1900;                 // 年，由于tm结构体存储的是从1900年开始的时间，所以tm_year为int临时变量减去1900。
	tm_.tm_mon = month - 1;                    // 月，由于tm结构体的月份存储范围为0-11，所以tm_mon为int临时变量减去1。
	tm_.tm_mday = day;                         // 日。
	tm_.tm_hour = hour;                        // 时。
	tm_.tm_min = minute;                       // 分。
	tm_.tm_sec = second;                       // 秒。
	tm_.tm_isdst = 0;                          // 非夏令时。
	time_t t_ = mktime(&tm_);                  // 将tm结构体转换成time_t格式。
	return t_;                                 // 返回值。 
}

int _tmain(int argc, _TCHAR* argv[])
{
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);

	HANDLE stdIn = GetStdHandle(STD_INPUT_HANDLE);
	SetConsoleMode(stdIn, 0);
	if(!init_config.open("./config.ini"))
		cout<<"无法打开配置"<<std::endl;
	apiKey = init_config.get("trade", "apiKey", "");
	secretKey = init_config.get("trade", "secretKey", "");
	passphrase = init_config.get("trade", "passphrase", "");

	trade_type = init_config.get("trade", "dir", "");
	trade_size = init_config.get("trade", "size", "");
	is_swap_futures = init_config.get_bool("trade", "swap", false);
	coin_type = init_config.get("trade", "coin_type", "");
	money_type = init_config.get("trade", "money_type", "");
	futures_cycle = init_config.get("trade", "futures_cycle", "");
	std::string _tigger_time  = init_config.get("trade", "tigger_time", "");
	tigger_time = StringToDatetime(_tigger_time);
	open_off = init_config.get("trade", "open_off", "");
	close_rate = init_config.get("trade", "close_rate", "");
	leverage = init_config.get("trade", "leverage", "");
	time_t tNow = time(NULL);
	if(tigger_time - tNow < 5)
	{
		LOCAL_INFO("时间不对");
		system("pause");
		return 0;
	}
	logicThread = boost::thread(&LogicThread);
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
	pExchange = new COkexExchange(apiKey, secretKey, passphrase);
	pExchange->SetHttpCallBackMessage(okex_http_callbak_message);
	pExchange->SetWebSocketCallBackOpen(okex_websocket_callbak_open);
	pExchange->SetWebSocketCallBackClose(okex_websocket_callbak_close);
	pExchange->SetWebSocketCallBackFail(okex_websocket_callbak_fail);
	pExchange->SetWebSocketCallBackMessage(okex_websocket_callbak_message);
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
		TradeLogic();
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
		pExchange = new COkexExchange(apiKey, secretKey, passphrase);
		pExchange->SetHttpCallBackMessage(okex_http_callbak_message);
		pExchange->SetWebSocketCallBackOpen(okex_websocket_callbak_open);
		pExchange->SetWebSocketCallBackClose(okex_websocket_callbak_close);
		pExchange->SetWebSocketCallBackFail(okex_websocket_callbak_fail);
		pExchange->SetWebSocketCallBackMessage(okex_websocket_callbak_message);
		pExchange->Run();
		return;
	}

	if(tLastUpdate15Sec == 0)
		tLastUpdate15Sec = tNow;
	if(tNow - tLastUpdate15Sec < 15)
		return;
	tLastUpdate15Sec = tNow;
	OKEX_WEB_SOCKET->Ping();
	tListenPong = time(NULL);
}

void Pong()
{
	tListenPong = 0;
}

void OnOkexWSConnectSuccess()
{
	if(OKEX_WEB_SOCKET)
	{
		OKEX_WEB_SOCKET->API_FuturesTickerData(true, is_swap_futures, coin_type, money_type, futures_cycle);
		OKEX_WEB_SOCKET->API_LoginFutures(apiKey, secretKey, time(NULL));
		LOCAL_INFO("登录");
		bool bFound = false;
		std::string instrumentID;
		if(is_swap_futures)
			instrumentID = coin_type + "-" + money_type + "-SWAP";
		else
			instrumentID = coin_type + "-" + money_type + "-" + futures_cycle;
		for(int i = 0; i < 3; ++i)
		{
			SHttpResponse resInfo;
			OKEX_HTTP->API_FuturesInstruments(false, is_swap_futures, &resInfo);
			bFound = false;
			if(resInfo.retObj.isArray())
			{
				for(int j = 0; j < (int)resInfo.retObj.size(); ++j)
				{
					if(resInfo.retObj[j]["instrument_id"].asString() == instrumentID)
					{
						std::string strTickSize = resInfo.retObj[j]["tick_size"].asString();
						int pos = strTickSize.find_first_of(".");
						if(pos != std::string::npos)
						{
							price_decimal = strTickSize.size() - pos - 1;
							bFound = true;
						}
						else
						{
							price_decimal = 0;
							bFound = true;
						}
						LOCAL_INFO("PriceDecimal=%d", price_decimal);
						if(bFound)
							break;
					}
				}
			}
			if(bFound)
				break;
		}
		SHttpResponse resInfo;
		OKEX_HTTP->API_FuturesSetLeverage(false, is_swap_futures, coin_type, money_type, leverage, &resInfo);
		if(is_swap_futures)
		{
			std::string strInstrumentID = coin_type + "-" + money_type + "-SWAP";
			if(!resInfo.retObj.isObject() || ((resInfo.retObj["instrument_id"].asString() != strInstrumentID) && (resInfo.retObj["code"].asInt() != 35017)))
			{
				LOCAL_INFO("设置杠杆失败");
			}
			else
			{
				LOCAL_INFO("设置杠杆成功");
			}
		}
		else
		{
			if(resInfo.retObj.isObject())
				LOCAL_INFO("not object");
			if(resInfo.retObj["result"].isString())
				LOCAL_INFO("result is string");
			if(!resInfo.retObj.isObject() || (resInfo.retObj["result"].asString() != "true"))
			{
				LOCAL_INFO("设置杠杆失败");
			}
			else
			{
				LOCAL_INFO("设置杠杆成功");
			}
		}
	}
}

void OnOkexWSLoginSuccess()
{
	OKEX_WEB_SOCKET->API_FuturesOrderInfo(true, is_swap_futures, coin_type, money_type, futures_cycle);
	LOCAL_INFO("登录成功");
}

void OnRevTickerInfo(STickerData &data)
{
	tick_data = data;
	tick_data.bValid = true;
	char szBuffer[128] = {};
	_snprintf(szBuffer, 128, "buy=%.2f sell=%.2f last=%.2f", tick_data.buy, tick_data.sell, tick_data.last);
	SetConsoleTitle(szBuffer);
}

int step = 0;
void TradeLogic()
{	
	if(step == 0)
	{
		time_t tNow = time(NULL);
		if(tNow > tigger_time)
		{
			LOCAL_INFO("时间触发");
			step = 1;
		}
	}
	else if(step == 1)
	{
		std::string strClientOrderID = CFuncCommon::GenUUID();
		eFuturesTradeType tradeType;
		double openPrice = stod(open_off);
		if(trade_type == "空")
		{
			if(tigger_price > 0)
			{
				LOCAL_INFO("触发价格%.2f", tigger_price);
				tradeType = eFuturesTradeType_OpenBear;
				openPrice = (1 - openPrice)*tigger_price;
			}
			else
			{
				LOCAL_INFO("触发价格%.2f", tick_data.buy);
				tradeType = eFuturesTradeType_OpenBear;
				openPrice = (1 - openPrice)*tick_data.buy;
				tigger_price = tick_data.buy;
			}

		}
		else
		{
			if(tigger_price > 0)
			{
				LOCAL_INFO("触发价格%.2f", tigger_price);
				tradeType = eFuturesTradeType_OpenBull;
				openPrice = (1 + openPrice)*tigger_price;
			}
			else
			{
				LOCAL_INFO("触发价格%.2f", tick_data.sell);
				tradeType = eFuturesTradeType_OpenBull;
				openPrice = (1 + openPrice)*tick_data.sell;
				tigger_price = tick_data.sell;
			}
			
		}
		std::string price = CFuncCommon::Double2String(openPrice + DOUBLE_PRECISION, price_decimal).c_str();
		OKEX_HTTP->API_FuturesTrade(true, is_swap_futures, tradeType, coin_type, money_type, futures_cycle, price, trade_size, leverage, strClientOrderID);
		step = 2;
		trade_info.strClientOrderID = strClientOrderID;
		trade_info.waitClientOrderIDTime = time(NULL);
		trade_info.tradeType = tradeType;

	}
	else if(step == 2)
	{
		time_t tNow = time(NULL);
		if(success_time > 0 && tNow - success_time > 5)
		{
			if(trade_info.status == "0")
			{
				OKEX_HTTP->API_FuturesCancelOrder(true, is_swap_futures, coin_type, money_type, futures_cycle, trade_info.orderID);
				LOCAL_INFO("全部撤单!");
				step = 3;
			}
			else if(trade_info.status == "1" || trade_info.status == "2")
			{
				if(trade_info.status == "1")
				{
					OKEX_HTTP->API_FuturesCancelOrder(true, is_swap_futures, coin_type, money_type, futures_cycle, trade_info.orderID);
					LOCAL_INFO("部分撤单!");
				}
				std::string strClientOrderID = CFuncCommon::GenUUID();
				eFuturesTradeType tradeType;
				double closePrice = stod(close_rate);
				if(trade_type == "空")
				{
					tradeType = eFuturesTradeType_CloseBear;
					closePrice = (1 - closePrice)*tigger_price;
				}
				else
				{
					tradeType = eFuturesTradeType_CloseBull;
					closePrice = (1 + closePrice)*tigger_price;
				}
				std::string price = CFuncCommon::Double2String(closePrice + DOUBLE_PRECISION, price_decimal).c_str();
				OKEX_HTTP->API_FuturesTrade(true, is_swap_futures, tradeType, coin_type, money_type, futures_cycle, price, trade_info.filledQTY, leverage, strClientOrderID);
				step = 3;
			}
		}
		
	}
}


void OnTradeSuccess(std::string& clientOrderID, std::string& serverOrderID)
{
	if(trade_info.strClientOrderID == clientOrderID)
	{
		trade_info.orderID = serverOrderID;
		trade_info.waitClientOrderIDTime = 0;
		success_time = time(NULL);
		std::string rprice = CFuncCommon::Double2String(tick_data.last + DOUBLE_PRECISION, price_decimal).c_str();
		LOCAL_INFO("http下单成功 client_order=%s, order=%s last=%s", trade_info.strClientOrderID.c_str(), trade_info.orderID.c_str(), rprice.c_str());
		OKEX_HTTP->API_FuturesOrderInfo(true, is_swap_futures, coin_type, money_type, futures_cycle, serverOrderID);
	}
}

void UpdateTradeInfo(SFuturesTradeInfo& info)
{
	if(info.tradeType == trade_info.tradeType && trade_info.orderID == info.orderID)
	{
		if((trade_info.status == "1" || trade_info.status == "2") && info.status == "0")
			return;
		trade_info.timeStamp = info.timeStamp;
		trade_info.filledQTY = info.filledQTY;
		trade_info.price = info.price;
		trade_info.priceAvg = info.priceAvg;
		trade_info.status = info.status;
		trade_info.tradeType = info.tradeType;
		trade_info.size = info.size;
		trade_info.tLastUpdate = time(NULL);
	}
}

void OnTradeFail(std::string& clientOrderID)
{
	//if(trade_info.strClientOrderID == clientOrderID)
	{
		LOCAL_INFO("http下单失败");
		step =1;
	}
}