#pragma once
#include "exchange/okex/okex_websocket_api.h"
#include "exchange/okex/okex_http_api.h"
#include "exchange/binance/binance_http_api.h"
#include "exchange/binance/binance_websocket_api.h"

#define DOUBLE_PRECISION 0.00000001
struct STickerData
{
	bool bValid;
	double sell;		//卖一价格
	double buy;			//买一价格
	double last;		//最新成交价格
	STickerData(){
		memset(this, 0, sizeof(STickerData));
	}
};


struct SFuturesTradeInfo
{
	std::string strClientOrderID;
	time_t timeStamp;	//委托时间
	std::string filledQTY;		//成交数量
	std::string orderID;//订单ID;
	double price;		//订单价格
	double priceAvg;	//成交均价
	std::string status;	//订单状态(-1.撤单成功；0:等待成交 1:部分成交 2:全部成交 ）
	eFuturesTradeType tradeType;
	time_t waitClientOrderIDTime;
	std::string size;
	time_t tLastUpdate;
	bool bModifyQTY;
	time_t tLastALLFillTime;
	bool isForceClose;//是否强平
	SFuturesTradeInfo()
	{
		Reset();
	}
	void Reset()
	{
		strClientOrderID = "";
		timeStamp = 0;
		filledQTY = "0";
		orderID = "";
		price = 0.0;
		priceAvg = 0.0;
		status = "";
		tradeType = eFuturesTradeType_OpenBull;
		waitClientOrderIDTime = 0;
		size = "0";
		tLastUpdate = 0;
		bModifyQTY = false;
		tLastALLFillTime = 0;
		isForceClose = false;
	}
	SFuturesTradeInfo& operator= (const SFuturesTradeInfo &t){
		if(this != &t){
			this->strClientOrderID = t.strClientOrderID;
			this->timeStamp = t.timeStamp;
			this->filledQTY = t.filledQTY;
			this->orderID = t.orderID;
			this->price = t.price;
			this->priceAvg = t.priceAvg;
			this->status = t.status;
			this->tradeType = t.tradeType;
			this->waitClientOrderIDTime = t.waitClientOrderIDTime;
			this->size = t.size;
			this->tLastUpdate = t.tLastUpdate;
			this->bModifyQTY = t.bModifyQTY;
			this->tLastALLFillTime = t.tLastALLFillTime;
			this->isForceClose = t.isForceClose;
			
		}
		return *this;
	}
};

struct SFuturesTradePairInfo
{
	SFuturesTradeInfo open;
	SFuturesTradeInfo close;
	SFuturesTradePairInfo()
	{
		Reset();
	}
	void Reset()
	{
		open.Reset();
		close.Reset();
	}
};

extern COkexExchange* pOkexExchange;
extern CBinanceExchange* pBinanceExchange;
extern void OKexPong();
extern void BinancePong();
extern void OnRecvBinanTickPrice(double price);
extern void OnRecvBinandepthPrice(double buy, double sell);
extern void OnOkexWSConnectSuccess();
extern void OnOkexWSLoginSuccess();
extern void OnRevOkexTickerInfo(STickerData &data);
extern void OnBinanceMarketWSConnectSuccess();
extern void OnBinanceAccountWSConnectSuccess();
extern void OnBinanceGotListenKey(std::string key);
extern void UpdateOkexTradeInfo(SFuturesTradeInfo& info);
extern void UpdateBinanceTradeInfo(SFuturesTradeInfo& info);
extern void OkexTradeSuccess(std::string& clientOrderID, std::string& serverOrderID);
extern void BinanceTradeSuccess(std::string& clientOrderID, __int64 serverOrderID);
#define OKEX_WEB_SOCKET ((COkexWebsocketAPI*)pOkexExchange->GetMarketWebSocket())
#define OKEX_HTTP ((COkexHttpAPI*)pOkexExchange->GetHttp())

#define BINANCE_MARKET_WEB_SOCKET ((CBinanceWebsocketAPI*)pBinanceExchange->GetMarketWebSocket())
#define BINANCE_ACCOUNT_WEB_SOCKET ((CBinanceWebsocketAPI*)pBinanceExchange->GetAccountWebSocket())

#define BINANCE_HTTP ((CBinanceHttpAPI*)pBinanceExchange->GetHttp())
#define BINANCE_TRADE_HTTP ((CBinanceHttpAPI*)pBinanceExchange->GetTradeHttp())

void okex_websocket_callbak_open(const char* szExchangeName)
{
	LOCAL_INFO("Okex websocket连接成功");
	OnOkexWSConnectSuccess();
}

void okex_websocket_callbak_close(const char* szExchangeName)
{
	LOCAL_ERROR("Okex websocket断开连接");
}

void okex_websocket_callbak_fail(const char* szExchangeName)
{
	LOCAL_ERROR("Okex websocket连接失败");
}

void okex_http_callbak_message(eHttpAPIType apiType, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData)
{
	switch(apiType)
	{
	case eHttpAPIType_FuturesTransfer:
		{
			if(retObj.isObject() && ((retObj["result"].isBool() && retObj["result"].asBool() == true)))
			{
				LOCAL_INFO("okex 划转资金成功");
			}
			else
			{
				LOCAL_ERROR("okex 划转资金失败 %s", strRet.c_str());
			}
		}
		break;
	case eHttpAPIType_FuturesTrade:
		{
			if(retObj.isObject() && ((retObj["error_code"].isInt() && retObj["error_code"].asInt() == 0) || (retObj["error_code"].isString() && retObj["error_code"].asString() == "0")))
			{
				if(retObj["client_oid"].isString())
					OkexTradeSuccess(retObj["client_oid"].asString(), retObj["order_id"].asString());
			}
			else
			{
				LOCAL_ERROR("okex 下单失败 %s", strRet.c_str());
			}
		}
		break;
	case eHttpAPIType_FuturesTradeInfo:
		{
			if(retObj.isObject() && retObj["order_id"].isString())
			{
				SFuturesTradeInfo info;
				info.strClientOrderID = "";
				info.timeStamp = CFuncCommon::ISO8601ToTime(retObj["timestamp"].asString());
				info.filledQTY = CFuncCommon::ToString(stoi(retObj["filled_qty"].asString()));
				info.orderID = retObj["order_id"].asString();
				info.price = stod(retObj["price"].asString());
				info.priceAvg = stod(retObj["price_avg"].asString());
				info.status = retObj["status"].asString();
				info.size = CFuncCommon::ToString(stoi(retObj["size"].asString()));
				std::string tradeType = retObj["type"].asString();
				if(tradeType == "1")
					info.tradeType = eFuturesTradeType_OpenBull;
				else if(tradeType == "2")
					info.tradeType = eFuturesTradeType_OpenBear;
				else if(tradeType == "3")
					info.tradeType = eFuturesTradeType_CloseBull;
				else if(tradeType == "4")
					info.tradeType = eFuturesTradeType_CloseBear;
				UpdateOkexTradeInfo(info);
				//LOCAL_INFO("http更新Okex订单信息 client_order=%s, order=%s, filledQTY=%s, price=%s, priceAvg=%s, status=%s, tradeType=%s", info.strClientOrderID.c_str(), info.orderID.c_str(), info.filledQTY.c_str(), retObj["price"].asString().c_str(), retObj["price_avg"].asString().c_str(), info.status.c_str(), tradeType.c_str());
			}
			else
				LOCAL_ERROR("okex 查询订单失败 %s", strRet.c_str());
		}
		break;
	default:
		LOCAL_ERROR("unkown okex_http_ret type=%d ret=%s", apiType, strRet.c_str());
		break;
	}
}


void okex_websocket_callbak_message(eWebsocketAPIType apiType, const char* szExchangeName, Json::Value& retObj, const std::string& strRet)
{
	switch(apiType)
	{
	case eWebsocketAPIType_Pong:
		{
			OKexPong();
		}
		break;
	case eWebsocketAPIType_FuturesTicker:
		{
			char* szEnd = NULL;
			STickerData data;
			data.sell = stod(retObj["data"][0]["best_ask"].asString());
			data.buy = stod(retObj["data"][0]["best_bid"].asString());
			data.last = stod(retObj["data"][0]["last"].asString());
			OnRevOkexTickerInfo(data);
		}
		break;
	case eWebsocketAPIType_FuturesOrderInfo:
		{
			if(retObj.isObject() && retObj["data"].isArray())
			{
				SFuturesTradeInfo info;
				info.strClientOrderID = "";
				info.timeStamp = CFuncCommon::ISO8601ToTime(retObj["data"][0]["timestamp"].asString());
				info.filledQTY = retObj["data"][0]["filled_qty"].asString();
				info.orderID = retObj["data"][0]["order_id"].asString();
				info.price = stod(retObj["data"][0]["price"].asString());
				info.priceAvg = stod(retObj["data"][0]["price_avg"].asString());
				info.status = retObj["data"][0]["status"].asString();
				info.size = retObj["data"][0]["size"].asString();
				std::string tradeType = retObj["data"][0]["type"].asString();
				if(tradeType == "1")
					info.tradeType = eFuturesTradeType_OpenBull;
				else if(tradeType == "2")
					info.tradeType = eFuturesTradeType_OpenBear;
				else if(tradeType == "3")
					info.tradeType = eFuturesTradeType_CloseBull;
				else if(tradeType == "4")
					info.tradeType = eFuturesTradeType_CloseBear;
				//LOCAL_INFO("ws更新Okex订单信息 order=%s, filledQTY=%s, price=%s, priceAvg=%s, status=%s, tradeType=%s", info.orderID.c_str(), info.filledQTY.c_str(), retObj["data"][0]["price"].asString().c_str(), retObj["data"][0]["price_avg"].asString().c_str(), info.status.c_str(), tradeType.c_str());
				UpdateOkexTradeInfo(info);
			}
		}
	break;
	case eWebsocketAPIType_Login:
		{
			OnOkexWSLoginSuccess();
		}
		break;
	default:
		LOCAL_ERROR("not handle okex_websocket_ret type=%d ret=%s", apiType, strRet.c_str());
	}
}


void binance_market_websocket_callbak_open(const char* szExchangeName)
{
	LOCAL_INFO("Binance 行情websocket连接成功");
	OnBinanceMarketWSConnectSuccess();
}

void binance_market_websocket_callbak_close(const char* szExchangeName)
{
	LOCAL_ERROR("Binance 行情websocket断开连接");
}

void binance_market_websocket_callbak_fail(const char* szExchangeName)
{
	LOCAL_ERROR("Binance 行情websocket连接失败");
}

void binance_account_websocket_callbak_open(const char* szExchangeName)
{
	LOCAL_INFO("Binance 账户websocket连接成功");
	OnBinanceAccountWSConnectSuccess();
}

void binance_account_websocket_callbak_close(const char* szExchangeName)
{
	LOCAL_ERROR("Binance 账户websocket断开连接");
}

void binance_account_websocket_callbak_fail(const char* szExchangeName)
{
	LOCAL_ERROR("Binance 账户websocket连接失败");
}


void binance_http_callbak_message(eHttpAPIType apiType, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData)
{
	switch(apiType)
	{
	case eHttpAPIType_ListenKey:
		{
			if(retObj.isObject())
			{
				if(retObj["listenKey"].isString())
					OnBinanceGotListenKey(retObj["listenKey"].asString());
				else
					LOCAL_INFO("Binance listenKey live %s", strRet.c_str());
			}
			else
			{
				LOCAL_ERROR("error listen key: %s", strRet.c_str());
			}
		}
		break;
	case eHttpAPIType_FuturesTransfer:
		{
			if(retObj.isObject() && retObj["tranId"].isIntegral())
			{
				LOCAL_INFO("binance 划转资金成功");
			}
			else
			{
				LOCAL_ERROR("binance 划转资金失败 %s", strRet.c_str());
			}
		}
		break;
	case eHttpAPIType_FuturesTrade:
		{
			if(retObj.isObject() && (retObj["code"].isString()))
			{
				LOCAL_ERROR("binance 下单失败 %s", strRet.c_str());
			}
			else
			{
				if(retObj["clientOrderId"].isString())
				{
					if(retObj["orderId"].isInt())
						BinanceTradeSuccess(retObj["clientOrderId"].asString(), retObj["orderId"].asInt());
					else if(retObj["orderId"].isInt64())
						BinanceTradeSuccess(retObj["clientOrderId"].asString(), retObj["orderId"].asInt64());
					else
						LOCAL_ERROR("binance 下单单号返回类型不对 %s", strRet.c_str());
				}
			}
		}
		break;
	case eHttpAPIType_FuturesTradeInfo:
		{
			if(retObj.isObject() && (retObj["code"].isString()))
				LOCAL_ERROR("binance 查询订单失败 %s", strRet.c_str());
			else
			{
				SFuturesTradeInfo info;
				info.strClientOrderID = retObj["clientOrderId"].asString();
				info.timeStamp = retObj["time"].asInt64();

				std::string filledQTY = retObj["executedQty"].asString();
				std::string origQty = retObj["origQty"].asString();
				info.filledQTY = filledQTY;
				std::string server_order = CFuncCommon::ToString(retObj["orderId"].asInt64());
				info.orderID = server_order;
				info.price = stod(retObj["price"].asString());
				info.priceAvg = stod(retObj["avgPrice"].asString());
				info.size = origQty;
				double _filledQTY = stod(filledQTY);
				if(_filledQTY <= DOUBLE_PRECISION)
					info.status = "0";
				else if(_filledQTY > DOUBLE_PRECISION && filledQTY != origQty)
					info.status = "1";
				else if(_filledQTY > DOUBLE_PRECISION && filledQTY == origQty)
					info.status = "2";
				UpdateBinanceTradeInfo(info);
				LOCAL_INFO("http更新Binance订单信息 client_order=%s, order=%s, filledQTY=%s, price=%s, priceAvg=%s, status=%s", info.strClientOrderID.c_str(), info.orderID.c_str(), info.filledQTY.c_str(), retObj["price"].asString().c_str(), retObj["price_avg"].asString().c_str(), info.status.c_str());
			}
		}
		break;
	default:
		LOCAL_ERROR("unkown binance_http_ret type=%d ret=%s", apiType, strRet.c_str());
		break;
	}

}

void binance_websocket_callbak_message(eWebsocketAPIType apiType, const char* szExchangeName, Json::Value& retObj, const std::string& strRet)
{
	switch(apiType)
	{
	case eWebsocketAPIType_Pong:
		{
			BinancePong();
		}
		break;
	case eWebsocketAPIType_FuturesTicker:
		{
			if(retObj.isObject() && retObj["p"].isString())
			{
				OnRecvBinanTickPrice(stod(retObj["p"].asString()));			
			}
		}
		break;
	case eWebsocketAPIType_FuturesEntrustDepth:
		{
			if(retObj.isObject() && retObj["b"].isArray() && retObj["a"].isArray())
			{
				double buy = stod(retObj["b"][4][0].asString());
				double sell = stod(retObj["a"][0][0].asString());
				OnRecvBinandepthPrice(buy, sell);
			}
		}
		break;
	case eWebsocketAPIType_FuturesOrderInfo:
		{
			SFuturesTradeInfo info;
			info.timeStamp = retObj["T"].asInt64();
			info.strClientOrderID = retObj["o"]["c"].asString();
			std::string filledQTY = retObj["o"]["z"].asString();
			std::string origQty = retObj["o"]["q"].asString();

			info.filledQTY = filledQTY;
			std::string server_order = CFuncCommon::ToString(retObj["o"]["i"].asInt64());
			info.orderID = server_order;
			info.price = stod(retObj["o"]["p"].asString());
			info.priceAvg = stod(retObj["o"]["ap"].asString());
			info.size = origQty;
			std::string nowType = retObj["o"]["X"].asString();
			if(nowType == "NEW")
				info.status = "0";
			else if(nowType == "PARTIALLY_FILLED")
				info.status = "1";
			else if(nowType == "FILLED")
				info.status = "2";
			else if(nowType == "CANCELED")
				info.status = "-1";
			else if(nowType == "NEW_INSURANCE" || nowType == "NEW_ADL")
				info.isForceClose = true;
			UpdateBinanceTradeInfo(info);
			LOCAL_INFO("ws更新Binance订单信息 client_order=%s, order=%s, filledQTY=%s, price=%s, priceAvg=%s, status=%s", info.strClientOrderID.c_str(), info.orderID.c_str(), info.filledQTY.c_str(), retObj["price"].asString().c_str(), retObj["price_avg"].asString().c_str(), info.status.c_str());
		}
		break;
	default:
		LOCAL_ERROR("not handle binance_websocket_ret type=%d ret=%s", apiType, strRet.c_str());
		break;
	}
}