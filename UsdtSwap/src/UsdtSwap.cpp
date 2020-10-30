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
string okex_api_key = "";
string okex_secret_key = "";
string okex_passphrase = "";
string binance_api_key = "";
string binance_secret_key = "";
STickerData binance_tickdata;
STickerData okex_tickdata;
SFuturesTradePairInfo okex_trade_info;
SFuturesTradePairInfo binance_trade_info;

std::string okex_avail_balance = "0";
std::string binance_avail_balance = "0";
/**********************固定配置**********************/
std::string strCoinType = "BTC";			//目标币
std::string strStandardCurrency = "USDT";	//本币
int okex_price_decimal = 1;					//okex价格小数位数
int binance_price_decimal = 2;				//binance价格小数位数
int binance_open_decimal = 3;				//binance开仓小数位数
std::string okex_leverage = "10";				//杠杆
std::string binance_leverage = "10";				//杠杆
double open_ratio = 0.9;					//开仓仓位
int main_dir = 0;							//okex的开单方向 0:空 1:多
double target_profit_loss = 0.01;			//目标盈亏
double okex_each_size = 0.01;				//okex单张对应币数
double okex_shouxufei = 0.0004;
int okex_add_money = 1000;					//okex补充资金
/****************************************************/
int next_main_dir = -1;
int nExitCode = 0;
bool bStop = false;
boost::thread logicThread;
void LogicThread();
void Update15Sec();
void OKexPong();
void BinancePong();
void OnRecvBinanTickPrice(double price);
void OnRecvBinandepthPrice(double buy, double sell);
void OnOkexWSConnectSuccess();
void OnOkexWSLoginSuccess();
void OnBinanceMarketWSConnectSuccess();
void OnBinanceAccountWSConnectSuccess();
void OnBinanceGotListenKey(std::string key);
void BinanceMarketSubscribe();
void OkexSubscribe();
void OnRevOkexTickerInfo(STickerData &data);
void UpdateOkexTradeInfo(SFuturesTradeInfo& info);
void UpdateBinanceTradeInfo(SFuturesTradeInfo& info);
void TradeLogic();
void OkexTradeSuccess(std::string& clientOrderID, std::string& serverOrderID);
void BinanceTradeSuccess(std::string& clientOrderID, __int64 serverOrderID);
void UpdateOkexChiCang(int size);
time_t tLastUpdate15Sec = 0;
time_t tListenOkexPong = 0;
time_t tListenBinancePing = 0;
time_t tLastUpdateListenKeyTime = 0;
std::string strBinanceListenKey = "";
std::string emptyString = "";
time_t tOpenExeTime = 0;
double okex_first_balance = 0.0;
double binance_first_balance = 0.0;

double okex_new_balance = 0.0;
double okex_transfer_balance = 0.0;
double okex_target_profit_loss_price = 0.0;
double okex_now_balace = 0.0;

int finish_times = 0;
double binance_new_balance = 0.0;
double binance_now_balance = 0.0;
double binance_transfer_balance = 0.0;
int okex_chicang = 0;
bool is_okex_baocang = false;
double move_stop = target_profit_loss/4;
int move_stop_step = 0;
#include "algorithm/hmac.h"
int _tmain(int argc, _TCHAR* argv[])
{
	tOpenExeTime = time(NULL);
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);

	HANDLE stdIn = GetStdHandle(STD_INPUT_HANDLE);
	SetConsoleMode(stdIn, 0);
	logicThread = boost::thread(&LogicThread);
	clib::config init_config;
	if(!init_config.open("./config.ini"))
		cout<<"无法打开配置"<<std::endl;
	okex_api_key = init_config.get("okex", "api_key", "");
	okex_secret_key = init_config.get("okex", "secret_key", "");
	okex_passphrase = init_config.get("okex", "passphrase", "");
	okex_price_decimal = init_config.get_int("okex", "price_decimal", 1);
	okex_leverage = init_config.get("okex", "leverage", "10");
	std::string _main_dir = init_config.get("okex", "main_dir", "空");
	if(_main_dir == "空")
		main_dir = 0;
	else
		main_dir = 1;
	std::string _okex_each_size = init_config.get("okex", "each_size", "0.01");
	okex_each_size = stod(_okex_each_size);
	okex_add_money = init_config.get_int("okex", "add_money", 1000);

	
	binance_api_key = init_config.get("binance", "api_key", "");
	binance_secret_key = init_config.get("binance", "secret_key", "");
	binance_price_decimal = init_config.get_int("binance", "price_decimal", 2);
	binance_open_decimal = init_config.get_int("binance", "open_decimal", 3);
	binance_leverage = init_config.get("binance", "leverage", "10");
	strCoinType = init_config.get("trade", "coin_type", "BTC");
	strStandardCurrency = init_config.get("trade", "local_currency", "USDT");
	
	std::string _open_ratio = init_config.get("trade", "open_ratio", "0.9");
	open_ratio = stod(_open_ratio);
	std::string _target_profit_loss = init_config.get("trade", "target_profit_loss", "0.01");
	target_profit_loss = stod(_target_profit_loss);
	move_stop = target_profit_loss/4;
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
					else if(strcmp(tempBuf, "stop") == 0)
					{
						LOCAL_INFO("结束后暂停");
						bStop = true;
					}
					else if(strcmp(tempBuf, "go") == 0)
					{
						LOCAL_INFO("结束后继续");
						bStop = false;
					}
					else if(strcmp(tempBuf, "空") == 0)
					{
						LOCAL_INFO("下轮Okex开空");
						next_main_dir = 0;
					}
					else if(strcmp(tempBuf, "多") == 0)
					{
						LOCAL_INFO("下轮Okex开多");
						next_main_dir = 1;
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
	pOkexExchange->Run();

	pBinanceExchange = new CBinanceExchange(binance_api_key, binance_secret_key);
	pBinanceExchange->SetHttpCallBackMessage(binance_http_callbak_message);
	pBinanceExchange->SetMarketWebSocketCallBackOpen(binance_market_websocket_callbak_open);
	pBinanceExchange->SetMarketWebSocketCallBackClose(binance_market_websocket_callbak_close);
	pBinanceExchange->SetMarketWebSocketCallBackFail(binance_market_websocket_callbak_fail);
	pBinanceExchange->SetMarketWebSocketCallBackMessage(binance_websocket_callbak_message);

	pBinanceExchange->SetAccountWebSocketCallBackOpen(binance_account_websocket_callbak_open);
	pBinanceExchange->SetAccountWebSocketCallBackClose(binance_account_websocket_callbak_close);
	pBinanceExchange->SetAccountWebSocketCallBackFail(binance_account_websocket_callbak_fail);
	pBinanceExchange->SetAccountWebSocketCallBackMessage(binance_websocket_callbak_message);
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
		TradeLogic();
		if(okex_tickdata.bValid && binance_tickdata.bValid)
		{
			char szBuffer[128] = {};
			if(okex_first_balance > DOUBLE_PRECISION && binance_first_balance > DOUBLE_PRECISION)
				_snprintf(szBuffer, 128, "[okex_last]=%s [binance_last]=%s [begin]=%.1f [now]=%.1f [trigger]=%s [finisht]=%d", CFuncCommon::Double2String(okex_tickdata.last + DOUBLE_PRECISION, okex_price_decimal).c_str(), CFuncCommon::Double2String(binance_tickdata.last + DOUBLE_PRECISION, binance_price_decimal).c_str(), okex_first_balance + binance_first_balance, okex_new_balance + binance_new_balance + binance_transfer_balance + okex_transfer_balance, CFuncCommon::Double2String(okex_target_profit_loss_price + DOUBLE_PRECISION, okex_price_decimal).c_str()
				, finish_times);
			else
				_snprintf(szBuffer, 128, "[okex_last]=%s [binance_last]=%s", CFuncCommon::Double2String(okex_tickdata.last + DOUBLE_PRECISION, okex_price_decimal).c_str(), CFuncCommon::Double2String(binance_tickdata.last + DOUBLE_PRECISION, binance_price_decimal).c_str());
			SetConsoleTitle(szBuffer);
		}
		boost::this_thread::sleep(boost::posix_time::milliseconds(10));
	}
	LOCAL_INFO("退出了");
	CLocalLogger::GetInstancePt()->SwapFront2Middle();
	boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
	CLocalLogger::ReleaseInstance();
	CLocalActionLog::ReleaseInstance();
}

void UpdateOkexChiCang(int size)
{
	okex_chicang = size;
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
		pBinanceExchange->SetMarketWebSocketCallBackMessage(binance_websocket_callbak_message);

		pBinanceExchange->SetAccountWebSocketCallBackOpen(binance_account_websocket_callbak_open);
		pBinanceExchange->SetAccountWebSocketCallBackClose(binance_account_websocket_callbak_close);
		pBinanceExchange->SetAccountWebSocketCallBackFail(binance_account_websocket_callbak_fail);
		pBinanceExchange->SetAccountWebSocketCallBackMessage(binance_websocket_callbak_message);
		BinanceMarketSubscribe();
		pBinanceExchange->Run();
		return;
	}
	if(tLastUpdateListenKeyTime && tNow - tLastUpdateListenKeyTime > 60*10)
	{
		BINANCE_HTTP->API_ListenKeyLive(true);
		BINANCE_MARKET_WEB_SOCKET->Ping();
		tLastUpdateListenKeyTime = tNow;
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
	SHttpResponse resInfo;
	OKEX_HTTP->API_FuturesSetLeverage(false, true, strCoinType, strStandardCurrency, okex_leverage, &resInfo);
	std::string strInstrumentID = strCoinType + "-" + strStandardCurrency + "-SWAP";
	if(!resInfo.retObj.isObject() || ((resInfo.retObj["instrument_id"].asString() != strInstrumentID) && (resInfo.retObj["code"].asInt() != 35017)))
	{
		LOCAL_INFO("Okex设置杠杆失败 %s", resInfo.strRet.c_str());
	}
	else
	{
		LOCAL_INFO("Okex设置杠杆成功");
	}
	OKEX_WEB_SOCKET->API_LoginFutures(okex_api_key, okex_secret_key, time(NULL));
}

void OnOkexWSLoginSuccess()
{
	OkexSubscribe();
}


void OnBinanceMarketWSConnectSuccess()
{
	BinanceMarketSubscribe();
	BINANCE_HTTP->API_ListenKey(true);
	SHttpResponse resInfo;
	int nLeverage = atoi(binance_leverage.c_str());
	BINANCE_HTTP->API_FuturesSetLeverage(false, strCoinType, strStandardCurrency, nLeverage, &resInfo);
	if(resInfo.retObj.isObject() && resInfo.retObj["leverage"].isInt() && resInfo.retObj["leverage"].asInt() == nLeverage)
	{
		LOCAL_INFO("Binance设置杠杆成功");
	}
	else
	{
		LOCAL_INFO("Binance设置杠杆失败");
	}
}

void OnBinanceAccountWSConnectSuccess()
{

}

void OnBinanceGotListenKey(std::string key)
{
	if(strBinanceListenKey != key)
	{
		tLastUpdateListenKeyTime = time(NULL);
		LOCAL_INFO("Binance listenKey=%s", key.c_str());
		strBinanceListenKey = key;
		BINANCE_ACCOUNT_WEB_SOCKET->SetListenKey(key);
		pBinanceExchange->RunAccountWebSocket();
	}
	
}

void BinanceMarketSubscribe()
{
	BINANCE_MARKET_WEB_SOCKET->API_FuturesTickerData(true, strCoinType, strStandardCurrency);
	BINANCE_MARKET_WEB_SOCKET->API_FuturesEntrustDepth(true, strCoinType, strStandardCurrency);
}

void OkexSubscribe()
{
	OKEX_WEB_SOCKET->API_FuturesTickerData(true, true, strCoinType, strStandardCurrency, emptyString);
	OKEX_WEB_SOCKET->API_FuturesOrderInfo(true, true, strCoinType, strStandardCurrency, emptyString);
	OKEX_WEB_SOCKET->API_FuturesPositionInfo(true, true, strCoinType, strStandardCurrency, emptyString);
}


void OnRecvBinanTickPrice(double price)
{
	binance_tickdata.bValid = true;
	binance_tickdata.last = price;
}

void OnRecvBinandepthPrice(double buy, double sell)
{
	binance_tickdata.buy = buy;
	binance_tickdata.sell = sell;
}

void OnRevOkexTickerInfo(STickerData &data)
{
	okex_tickdata = data;
	okex_tickdata.bValid = true;
}

void OkexTradeSuccess(std::string& clientOrderID, std::string& serverOrderID)
{
	if(okex_trade_info.open.strClientOrderID == clientOrderID)
	{
		okex_trade_info.open.orderID = serverOrderID;
		OKEX_HTTP->API_FuturesOrderInfo(true, true, strCoinType, strStandardCurrency, emptyString, serverOrderID);
	}
	if(okex_trade_info.close.strClientOrderID == clientOrderID)
	{
		okex_trade_info.close.orderID = serverOrderID;
		OKEX_HTTP->API_FuturesOrderInfo(true, true, strCoinType, strStandardCurrency, emptyString, serverOrderID);
	}
}

void BinanceTradeSuccess(std::string& clientOrderID, __int64 serverOrderID)
{
	std::string server_order = CFuncCommon::ToString(serverOrderID);
	if(binance_trade_info.open.strClientOrderID == clientOrderID)
	{
		binance_trade_info.open.orderID = server_order;
		BINANCE_HTTP->API_FuturesOrderInfo(true, strCoinType, strStandardCurrency, serverOrderID);
	}
	if(binance_trade_info.close.strClientOrderID == clientOrderID)
	{
		binance_trade_info.close.orderID = server_order;
		BINANCE_HTTP->API_FuturesOrderInfo(true, strCoinType, strStandardCurrency, serverOrderID);
	}
}

void UpdateOkexTradeInfo(SFuturesTradeInfo& info)
{
	if(info.tradeType == okex_trade_info.open.tradeType && okex_trade_info.open.orderID == info.orderID)
	{
		if((okex_trade_info.open.status == "1" || okex_trade_info.open.status == "2") && info.status == "0")
			return;
		okex_trade_info.open.timeStamp = info.timeStamp;
		okex_trade_info.open.filledQTY = info.filledQTY;
		okex_trade_info.open.price = info.price;
		okex_trade_info.open.priceAvg = info.priceAvg;
		okex_trade_info.open.status = info.status;
		okex_trade_info.open.tradeType = info.tradeType;
		okex_trade_info.open.size = info.size;
		okex_trade_info.open.tLastUpdate = time(NULL);
	}
	if(info.tradeType == okex_trade_info.close.tradeType && okex_trade_info.close.orderID == info.orderID)
	{
		if((okex_trade_info.close.status == "1" || okex_trade_info.close.status == "2") && info.status == "0")
			return;
		okex_trade_info.close.timeStamp = info.timeStamp;
		okex_trade_info.close.filledQTY = info.filledQTY;
		okex_trade_info.close.price = info.price;
		okex_trade_info.close.priceAvg = info.priceAvg;
		okex_trade_info.close.status = info.status;
		okex_trade_info.close.tradeType = info.tradeType;
		okex_trade_info.close.size = info.size;
		okex_trade_info.close.tLastUpdate = time(NULL);
	}
}

void UpdateBinanceTradeInfo(SFuturesTradeInfo& info)
{
	if(info.strClientOrderID.find("autoclose-") != string::npos && info.status == "2")
	{
		binance_trade_info.open.isForceClose = true;
		return;
	}
	if(binance_trade_info.open.orderID == info.orderID)
	{
		if((binance_trade_info.open.status == "1" || binance_trade_info.open.status == "2") && info.status == "0")
			return;
		binance_trade_info.open.timeStamp = info.timeStamp;
		binance_trade_info.open.filledQTY = info.filledQTY;
		binance_trade_info.open.price = info.price;
		binance_trade_info.open.priceAvg = info.priceAvg;
		binance_trade_info.open.status = info.status;
		binance_trade_info.open.size = info.size;
		binance_trade_info.open.tLastUpdate = time(NULL);
	}
	if(binance_trade_info.close.orderID == info.orderID)
	{
		if((binance_trade_info.close.status == "1" || binance_trade_info.close.status == "2") && info.status == "0")
			return;
		binance_trade_info.close.timeStamp = info.timeStamp;
		binance_trade_info.close.filledQTY = info.filledQTY;
		binance_trade_info.close.price = info.price;
		binance_trade_info.close.priceAvg = info.priceAvg;
		binance_trade_info.close.status = info.status;
		binance_trade_info.close.size = info.size;
		binance_trade_info.close.tLastUpdate = time(NULL);
	}
}

enum eStepType{
	eStepType_0,//比较余额,确定下单数量
	eStepType_1,//OKex下单
	eStepType_2,//等待OKex订单成交状态
	eStepType_3,//Binance下单
	eStepType_4,//等待Binance订单成交状态
	eStepType_5,//等待okex触发价格,同时检测
	eStepType_6,//等待okex平仓完成
	eStepType_7,//Binance平仓
	eStepType_8,//等待Binance平仓完成
	eStepType_end,//暂时卡住
};
eStepType step = eStepType_0;
double trade_balace = 0.0;
time_t open_okex_time = 0;
time_t open_binance_time = 0;
time_t finish_time = 0;
string okex_fillSize = "";
double okex_price_avg = 0.0;

string binance_fillSize = "";
double binance_price_avg = 0.0;

double okex_cost = 0.0;
double binance_cost = 0.0;

bool first_trigger = false;
void TradeLogic()
{
	if(step == eStepType_0)
	{
		time_t tNow = time(NULL);
		if(tNow - tOpenExeTime < 20)
			return;
		if(!okex_tickdata.bValid)
			return;
		if(!binance_tickdata.bValid)
			return;
		open_okex_time = 0;
		finish_time = 0;
		open_binance_time = 0;
		trade_balace = 0;
		okex_fillSize = "";
		okex_price_avg = 0.0;
		okex_target_profit_loss_price = 0.0;
		okex_cost = 0.0;
		binance_fillSize = "";
		binance_price_avg = 0.0;
		okex_chicang = 0;
		is_okex_baocang = false;
		move_stop_step = 0;
		okex_trade_info.Reset();
		binance_trade_info.Reset();
		first_trigger = false;
		//获取两边的余额
		double binance_balance, okex_balance;
		{
			SHttpResponse resInfo;
			BINANCE_HTTP->API_FuturesAccountInfo(false, &resInfo);
			if(resInfo.retObj.isArray() && resInfo.retObj[0]["asset"].isString() && resInfo.retObj[0]["asset"].asString() == strStandardCurrency)
			{
				binance_avail_balance = resInfo.retObj[0]["availableBalance"].asString();
				binance_balance = stod(binance_avail_balance);
			}
			else
				return;
		}
		{
			SHttpResponse resInfo;
			OKEX_HTTP->API_FuturesAccountInfoByCurrency(false, true, strCoinType, strStandardCurrency, &resInfo);
			if(resInfo.retObj["info"].isObject() && resInfo.retObj["info"]["equity"].isString())
			{

				std::string szEquity = resInfo.retObj["info"]["equity"].asString();
				std::string szMargin = resInfo.retObj["info"]["margin"].asString();
				double equity = stod(szEquity);
				double margin = stod(szMargin);
				okex_balance = equity - margin;
				okex_avail_balance = CFuncCommon::Double2String(equity - margin, 0);

				if(okex_balance < okex_add_money && okex_add_money-okex_balance > 1)
				{
					int add = int(okex_add_money - okex_balance);
					std::string amount = CFuncCommon::ToString(add);
					SHttpResponse _resInfo;
					OKEX_HTTP->API_ZiJinTransferToSwapFutures(false, strCoinType, strStandardCurrency, amount, &_resInfo);
					if(_resInfo.retObj.isObject() && ((_resInfo.retObj["result"].isBool() && _resInfo.retObj["result"].asBool() == true)))
					{
						okex_balance += add;
						okex_avail_balance = CFuncCommon::Double2String(okex_balance, 0);
						if(okex_first_balance > DOUBLE_PRECISION)
							okex_first_balance += add;
					}
					Sleep(2000);
				}
			}
			else
				return;
		}
		LOCAL_INFO("[step0] balance okex:%s  binance:%s", okex_avail_balance.c_str(), binance_avail_balance.c_str());
		trade_balace = min(binance_balance, okex_balance);
		okex_new_balance = okex_balance;
		binance_new_balance = binance_balance;
		if(okex_first_balance < DOUBLE_PRECISION)
			okex_first_balance = okex_new_balance;
		if(binance_first_balance < DOUBLE_PRECISION)
			binance_first_balance = binance_new_balance;
		//多余资金划转
		if(binance_balance > trade_balace && binance_balance - trade_balace > 1)
		{
			std::string amount = CFuncCommon::ToString(int(binance_balance - trade_balace));
			BINANCE_TRADE_HTTP->API_SwapFuturesTransferToSpot(true, strStandardCurrency, amount, NULL);
			binance_new_balance -= int(binance_balance - trade_balace);
			LOCAL_INFO("[step0] binance划转资金%s", amount.c_str());
			binance_transfer_balance += int(binance_balance - trade_balace);
		}
		if(okex_balance > trade_balace && okex_balance - trade_balace > 1)
		{
			std::string amount = CFuncCommon::ToString(int(okex_balance - trade_balace));
			OKEX_HTTP->API_SwapFuturesTransferToZiJin(true, strCoinType, strStandardCurrency, amount, NULL);
			okex_new_balance -= int(okex_balance - trade_balace);
			LOCAL_INFO("[step0] okex划转资金%s", amount.c_str());
			okex_transfer_balance += int(okex_balance - trade_balace);
		}
		Sleep(2000);
		{
			SHttpResponse resInfo;
			BINANCE_HTTP->API_FuturesAccountInfo(false, &resInfo);
			if(resInfo.retObj.isArray() && resInfo.retObj[0]["asset"].isString() && resInfo.retObj[0]["asset"].asString() == strStandardCurrency)
			{
				std::string temp = resInfo.retObj[0]["availableBalance"].asString();
				binance_now_balance = stod(temp);
				LOCAL_INFO("[step0] balance最新余额%.2f", binance_now_balance);
			}
			else
				return;
		}
		{
			SHttpResponse resInfo;
			OKEX_HTTP->API_FuturesAccountInfoByCurrency(false, true, strCoinType, strStandardCurrency, &resInfo);
			if(resInfo.retObj["info"].isObject() && resInfo.retObj["info"]["equity"].isString())
			{
				std::string szEquity = resInfo.retObj["info"]["equity"].asString();
				std::string szMargin = resInfo.retObj["info"]["margin"].asString();
				double equity = stod(szEquity);
				double margin = stod(szMargin);
				okex_now_balace = equity - margin;
				LOCAL_INFO("[step0] okex最新余额%.2f 账户权益%s 已用保证金%s", okex_now_balace, szEquity.c_str(), szMargin.c_str());
			}
			else
				return;
		}
		step = eStepType_1;
	}
	else if(step == eStepType_1)
	{
		//Okex先下单
		int nLeverage = atoi(okex_leverage.c_str());
		double buyCnt = trade_balace / okex_tickdata.last;
		buyCnt *= nLeverage;
		buyCnt /= okex_each_size;
		int size = int(buyCnt*open_ratio);
		eFuturesTradeType type;
		double price = 0.0;
		if(main_dir == 0)
		{
			type = eFuturesTradeType_OpenBear;
			price = okex_tickdata.buy*0.995;
		}
		else
		{
			type = eFuturesTradeType_OpenBull;
			price = okex_tickdata.sell*1.005;
		}
		if(size <=0 )
			return;
		LOCAL_INFO("[step1] okex开%s单price=%s, size=%s", ((type == eFuturesTradeType_OpenBear)?"空":"多"), CFuncCommon::Double2String(price+DOUBLE_PRECISION, okex_price_decimal).c_str(), CFuncCommon::ToString(size));
		std::string strClientOrderID = CFuncCommon::GenUUID();
		std::string strPrice = CFuncCommon::Double2String(price+DOUBLE_PRECISION, okex_price_decimal).c_str();
		std::string strSize = CFuncCommon::ToString(size);
		OKEX_HTTP->API_FuturesTrade(true, true, type, strCoinType, strStandardCurrency, emptyString, strPrice, strSize, okex_leverage, strClientOrderID, NULL);
		okex_trade_info.open.strClientOrderID = strClientOrderID;
		okex_trade_info.open.tradeType = type;
		okex_trade_info.open.status = "0";
		open_okex_time = time(NULL);
		step = eStepType_2;
	}
	else if(step == eStepType_2)
	{
		if(open_okex_time == 0)
			return;
		//20秒等待时间
		time_t tNow = time(NULL);
		if(okex_trade_info.open.orderID == "")
		{
			if(tNow - open_okex_time > 20)
				step = eStepType_1;
			return;
		}
		if(okex_trade_info.open.status == "0")
		{
			if(tNow - open_okex_time > 20)
			{
				 SHttpResponse resInfo;
				 OKEX_HTTP->API_FuturesCancelOrder(false, true, strCoinType, strStandardCurrency, emptyString, okex_trade_info.open.orderID, &resInfo);
				 if(resInfo.retObj.isObject() && ((resInfo.retObj["result"].isBool() && resInfo.retObj["result"].asBool() == true) || (resInfo.retObj["result"].isString() && resInfo.retObj["result"].asString() == "true")))
					 LOCAL_INFO("okex 撤消订单成功 order=%s", resInfo.retObj["order_id"].asString().c_str());
				 else
					 LOCAL_ERROR("okex 撤消订单失败 %s", resInfo.strRet.c_str());
				 step = eStepType_1;
			}
			return;
		}
		
		if(okex_trade_info.open.status == "1")
		{
			if(tNow - open_okex_time > 20)
			{
				SHttpResponse resInfo;
				OKEX_HTTP->API_FuturesCancelOrder(false, true, strCoinType, strStandardCurrency, emptyString, okex_trade_info.open.orderID, &resInfo);
				if(resInfo.retObj.isObject() && ((resInfo.retObj["result"].isBool() && resInfo.retObj["result"].asBool() == true) || (resInfo.retObj["result"].isString() && resInfo.retObj["result"].asString() == "true")))
					LOCAL_INFO("okex 撤消订单成功 order=%s", resInfo.retObj["order_id"].asString().c_str());
				else
					LOCAL_ERROR("okex 撤消订单失败 %s", resInfo.strRet.c_str());
				okex_fillSize = okex_trade_info.open.filledQTY;
				okex_price_avg = okex_trade_info.open.priceAvg;
			}
			else
				return;
		}
		if(okex_trade_info.open.status == "2")
		{
			okex_fillSize = okex_trade_info.open.filledQTY;
			okex_price_avg = okex_trade_info.open.priceAvg;
		}
		LOCAL_INFO("[step2] okex最终成交%s张,平均价格%s", okex_fillSize.c_str(),  CFuncCommon::Double2String(okex_price_avg+DOUBLE_PRECISION, okex_price_decimal).c_str());
		int fillSize = atoi(okex_fillSize.c_str());
		okex_chicang = fillSize;
		okex_cost = fillSize*okex_each_size*okex_price_avg;
		if(main_dir == 0)
			okex_target_profit_loss_price = okex_price_avg*(1+target_profit_loss);
		else
			okex_target_profit_loss_price = okex_price_avg*(1-target_profit_loss);
		step = eStepType_3;
	}
	else if(step == eStepType_3)
	{
		eFuturesTradeType type;
		double price = 0.0;
		double open_cnt = 0.0;
		double okex_le = stod(okex_leverage);
		double binance_le = stod(binance_leverage);
		if(main_dir == 0)
		{
			type = eFuturesTradeType_OpenBull;
			open_cnt = (okex_cost/okex_le*binance_le)/binance_tickdata.sell;
			price = binance_tickdata.sell*1.0005;
		}
		else
		{
			type = eFuturesTradeType_OpenBear;
			open_cnt = (okex_cost/okex_le*binance_le)/binance_tickdata.buy;
			price = binance_tickdata.buy*0.9995;
		}
		LOCAL_INFO("[step3] binance开%s单price=%s, size=%s", ((type == eFuturesTradeType_OpenBear)?"空":"多"), CFuncCommon::Double2String(price + DOUBLE_PRECISION, binance_price_decimal).c_str(), CFuncCommon::Double2String(open_cnt + DOUBLE_PRECISION, binance_open_decimal).c_str());
		std::string strClientOrderID = CFuncCommon::GenUUID();
		std::string strPrice = CFuncCommon::Double2String(price + DOUBLE_PRECISION, binance_price_decimal).c_str();
		std::string strOpenCnt = CFuncCommon::Double2String(open_cnt + DOUBLE_PRECISION, binance_open_decimal).c_str();
		BINANCE_HTTP->API_FuturesTrade(true, type, strCoinType, strStandardCurrency, strPrice, strOpenCnt, strClientOrderID, NULL);
		binance_trade_info.open.strClientOrderID = strClientOrderID;
		binance_trade_info.open.tradeType = type;
		binance_trade_info.open.status = "0";
		step = eStepType_4;
		open_binance_time = time(NULL);
	}
	else if(step == eStepType_4)
	{
		if(open_binance_time == 0)
			return;
		time_t tNow = time(NULL);
		if(binance_trade_info.open.orderID == "")
		{
			if(tNow - open_binance_time > 20)
				step = eStepType_3;
			return;
		}
		if(binance_trade_info.open.status == "0")
		{
			if(tNow - open_binance_time > 20)
			{
				SHttpResponse resInfo;
				BINANCE_HTTP->API_FuturesCancelOrder(false, strCoinType, strStandardCurrency, atoll(binance_trade_info.open.orderID.c_str()), &resInfo);
				if(resInfo.retObj.isObject() && (resInfo.retObj["code"].isString()))
					LOCAL_ERROR("binance 撤消订单失败 %s", resInfo.strRet.c_str());
				else
					LOCAL_INFO("binance 撤消订单成功");
				step = eStepType_3;
			}
			return;
		}

		if(binance_trade_info.open.status == "1")
		{
			if(tNow - open_binance_time > 20)
			{
				SHttpResponse resInfo;
				BINANCE_HTTP->API_FuturesCancelOrder(false, strCoinType, strStandardCurrency, atoll(binance_trade_info.open.orderID.c_str()), &resInfo);
				if(resInfo.retObj.isObject() && (resInfo.retObj["code"].isString()))
					LOCAL_ERROR("binance 撤消订单失败 %s", resInfo.strRet.c_str());
				else
					LOCAL_INFO("binance 撤消订单成功");
				binance_fillSize = binance_trade_info.open.filledQTY;
				binance_price_avg = binance_trade_info.open.priceAvg;
			}
			else
				return;
		}
		if(binance_trade_info.open.status == "2")
		{
			binance_fillSize = binance_trade_info.open.filledQTY;
			binance_price_avg = binance_trade_info.open.priceAvg;
		}
		binance_cost = stod(binance_fillSize)*binance_price_avg;
		LOCAL_INFO("[step4] binance最终成交%s,平均价格%s", binance_fillSize.c_str(), CFuncCommon::Double2String(binance_price_avg + DOUBLE_PRECISION, binance_price_decimal).c_str());
		step = eStepType_5;
	}
	else if(step == eStepType_5)
	{
		if(main_dir == 0)
		{
			if(okex_tickdata.last > okex_price_avg && ((okex_tickdata.last-okex_price_avg)/okex_price_avg) >= target_profit_loss)
			{
				if(!first_trigger)
				{
					first_trigger = true;
					LOCAL_INFO("[step5] okex首次触发收益价格");
				}
				int nowStop = int((((okex_tickdata.last-okex_price_avg)/okex_price_avg)-target_profit_loss)/move_stop);
				if(nowStop > move_stop_step)
				{
					move_stop_step = nowStop;
					LOCAL_INFO("[step5] okex移动止盈进入step%d", nowStop);
				}
				else if(move_stop_step > 0 && nowStop < move_stop_step)
				{
					eFuturesTradeType type = eFuturesTradeType_CloseBear;
					double price = okex_tickdata.sell*1.01;
					std::string strClientOrderID = CFuncCommon::GenUUID();
					std::string strPrice = CFuncCommon::Double2String(price + DOUBLE_PRECISION, okex_price_decimal).c_str();
					std::string strSize = okex_fillSize;
					OKEX_HTTP->API_FuturesTrade(true, true, type, strCoinType, strStandardCurrency, emptyString, strPrice, strSize, okex_leverage, strClientOrderID, NULL);
					okex_trade_info.close.strClientOrderID = strClientOrderID;
					okex_trade_info.close.tradeType = type;
					okex_trade_info.close.status = "0";
					step = eStepType_6;
					LOCAL_INFO("[step5] okex触发收益价格平仓 price=%s, size=%s", strPrice.c_str(), strSize.c_str());
				}
			}
		}
		else
		{
			if(okex_tickdata.last < okex_price_avg && ((okex_price_avg-okex_tickdata.last) / okex_price_avg) >= target_profit_loss)
			{
				if(!first_trigger)
				{
					first_trigger = true;
					LOCAL_INFO("[step5] okex首次触发收益价格");
				}
				int nowStop = int((((okex_price_avg-okex_tickdata.last) / okex_price_avg)-target_profit_loss)/move_stop);
				if(nowStop > move_stop_step)
				{
					move_stop_step = nowStop;
					LOCAL_INFO("[step5] okex移动止盈进入step%d", nowStop);
				}
				else if(move_stop_step > 0 && nowStop < move_stop_step)
				{
					eFuturesTradeType type = eFuturesTradeType_CloseBull;
					double price = okex_tickdata.buy*0.99;
					std::string strClientOrderID = CFuncCommon::GenUUID();
					std::string strPrice = CFuncCommon::Double2String(price + DOUBLE_PRECISION, okex_price_decimal).c_str();
					std::string strSize = okex_fillSize;
					OKEX_HTTP->API_FuturesTrade(true, true, type, strCoinType, strStandardCurrency, emptyString, strPrice, strSize, okex_leverage, strClientOrderID, NULL);
					okex_trade_info.close.strClientOrderID = strClientOrderID;
					okex_trade_info.close.tradeType = type;
					okex_trade_info.close.status = "0";
					step = eStepType_6;
					LOCAL_INFO("[step5] okex触发收益价格平仓 price=%s, size=%s", strPrice.c_str(), strSize.c_str());
				}
			}
		}
		if(binance_trade_info.open.isForceClose)
		{
			LOCAL_INFO("[step5] binance爆仓");

			SHttpResponse resInfo;
			BINANCE_HTTP->API_FuturesAccountInfo(false, &resInfo);
			if(resInfo.retObj.isArray() && resInfo.retObj[0]["asset"].isString() && resInfo.retObj[0]["asset"].asString() == strStandardCurrency)
			{
				string _binance_balance = resInfo.retObj[0]["availableBalance"].asString();
				double now_binance_balance = stod(_binance_balance);
				double lose = trade_balace - now_binance_balance;
				int fillSize = atoi(okex_fillSize.c_str());
				double coin_cnt = fillSize*okex_each_size;
				lose /= coin_cnt;
				eFuturesTradeType type;
				double price = 0.0;
				std::string strClientOrderID = CFuncCommon::GenUUID();
				if(main_dir == 0)
				{
					type = eFuturesTradeType_CloseBear;
					price = okex_trade_info.open.priceAvg - lose;
				}
				else
				{
					type = eFuturesTradeType_CloseBull;
					price = okex_trade_info.open.priceAvg + lose;
				}
				std::string strPrice = CFuncCommon::Double2String(price + DOUBLE_PRECISION, okex_price_decimal).c_str();
				std::string strSize = okex_fillSize;
				OKEX_HTTP->API_FuturesTrade(true, true, type, strCoinType, strStandardCurrency, emptyString, strPrice, strSize, okex_leverage, strClientOrderID, NULL);
				okex_trade_info.close.strClientOrderID = strClientOrderID;
				okex_trade_info.close.tradeType = type;
				okex_trade_info.close.status = "0";
				step = eStepType_8;
				
				LOCAL_INFO("[step5] binance爆仓 okex平仓下单price=%s, size=%s, last=%s", strPrice.c_str(), strSize.c_str(), CFuncCommon::Double2String(okex_tickdata.last + DOUBLE_PRECISION, okex_price_decimal).c_str());
			}
		}
		if(okex_chicang == 0)
		{
			step = eStepType_6;
			is_okex_baocang = true;
			LOCAL_INFO("[step5] okex爆仓");
		}
	}
	else if(step == eStepType_6)
	{
		if(okex_trade_info.close.status == "2")
		{
			step = eStepType_7;
			LOCAL_INFO("[step6] okex平仓完成 price=%s, size=%s", CFuncCommon::Double2String(okex_trade_info.close.priceAvg + DOUBLE_PRECISION, okex_price_decimal).c_str(), okex_trade_info.close.filledQTY.c_str());
		}
		if(is_okex_baocang)
		{
			step = eStepType_7;
			LOCAL_INFO("[step6] okex爆仓跳过");
		}
	}
	else if(step == eStepType_7)
	{
		//计算亏损
		double lose = 0.0;
		SHttpResponse resInfo;
		OKEX_HTTP->API_FuturesAccountInfoByCurrency(false, true, strCoinType, strStandardCurrency, &resInfo);
		if(resInfo.retObj["info"].isObject() && resInfo.retObj["info"]["equity"].isString())
		{
			std::string szEquity = resInfo.retObj["info"]["equity"].asString();
			std::string szMargin = resInfo.retObj["info"]["margin"].asString();
			double equity = stod(szEquity);
			double margin = stod(szMargin);
			lose = okex_now_balace - (equity - margin);
			LOCAL_INFO("Okex 当前余额%.2f  账户权益%s 已用保证金%s", (equity - margin), szEquity.c_str(), szMargin.c_str());
			LOCAL_INFO("Okex 获取出亏损%.2f", lose);
		}
		if(lose > 0)
		{
			int closeSize = atoi(okex_trade_info.close.filledQTY.c_str());
			lose += ((okex_cost*okex_shouxufei) + (okex_trade_info.close.priceAvg*closeSize*okex_each_size)*okex_shouxufei);
			//币安也要计算出同样的损失
			double targetPrice = 0.0;
			eFuturesTradeType type;
			if(main_dir == 0)
			{
				type = eFuturesTradeType_OpenBear;
				targetPrice = ((lose / binance_cost)*binance_trade_info.open.priceAvg) + binance_trade_info.open.priceAvg;
			}
			else
			{
				type = eFuturesTradeType_OpenBull;
				targetPrice = binance_trade_info.open.priceAvg - (lose / binance_cost)*binance_trade_info.open.priceAvg;
			}
			if(targetPrice > 0)
			{
				double price = targetPrice;
				std::string strClientOrderID = CFuncCommon::GenUUID();
				std::string strPrice = CFuncCommon::Double2String(price + DOUBLE_PRECISION, binance_price_decimal).c_str();
				std::string strOpenCnt = binance_trade_info.open.filledQTY;
				BINANCE_HTTP->API_FuturesTrade(true, type, strCoinType, strStandardCurrency, strPrice, strOpenCnt, strClientOrderID, NULL);
				binance_trade_info.close.strClientOrderID = strClientOrderID;
				binance_trade_info.close.tradeType = type;
				binance_trade_info.close.status = "0";
				LOCAL_INFO("[step7] okex平仓亏损%.2f, binance平仓下单price=%s, size=%s, last=%s", lose, strPrice.c_str(), strOpenCnt.c_str(), CFuncCommon::Double2String(binance_tickdata.last + DOUBLE_PRECISION, binance_price_decimal).c_str());
				step = eStepType_8;
			}
		}
	}
	else if(step == eStepType_8)
	{
		if(binance_trade_info.close.status == "2" || (binance_trade_info.open.isForceClose && okex_trade_info.close.status == "2"))
		{
			if(finish_time == 0)
			{
				finish_times++;
				LOCAL_INFO("[step8] binance平仓完成");
				finish_time = time(NULL);
				double binance_balance, okex_balance;
				{
					SHttpResponse resInfo;
					BINANCE_HTTP->API_FuturesAccountInfo(false, &resInfo);
					if(resInfo.retObj.isArray() && resInfo.retObj[0]["asset"].isString() && resInfo.retObj[0]["asset"].asString() == strStandardCurrency)
					{
						binance_avail_balance = resInfo.retObj[0]["availableBalance"].asString();
						binance_balance = stod(binance_avail_balance);
					}
				}
				{
					SHttpResponse resInfo;
					OKEX_HTTP->API_FuturesAccountInfoByCurrency(false, true, strCoinType, strStandardCurrency, &resInfo);
					if(resInfo.retObj["info"].isObject() && resInfo.retObj["info"]["equity"].isString())
					{

						std::string szEquity = resInfo.retObj["info"]["equity"].asString();
						std::string szMargin = resInfo.retObj["info"]["margin"].asString();
						double equity = stod(szEquity);
						double margin = stod(szMargin);
						okex_balance = equity - margin;
						okex_avail_balance = CFuncCommon::Double2String(equity - margin, 0);
					}
				}
				okex_new_balance = okex_balance;
				binance_new_balance = binance_balance;
			}
			if(time(NULL) - finish_time > 20*60 && !bStop && !binance_trade_info.open.isForceClose)
			{
				if(next_main_dir != -1)
				{
					main_dir = next_main_dir;
					next_main_dir = -1;
				}
				else
				{
					if(okex_tickdata.last < okex_trade_info.close.priceAvg)
					{
						main_dir = 0;
					}
					else
					{
						main_dir = 1;
					}
				}
				LOCAL_INFO("[step8] 进入新一轮");
				step = eStepType_0;
			}
		}
	}
}