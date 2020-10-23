#pragma once
#include "exchange/okex/okex_websocket_api.h"
#include "exchange/okex/okex_http_api.h"
#define DOUBLE_PRECISION 0.00000001
struct STickerData
{
	bool bValid;
	__int64 time;		//时间
	int volume;			//成交量(张)
	double sell;		//卖一价格
	double buy;			//买一价格
	double high;		//24小时最高价格
	double low;			//24小时最低价格
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
		}
		return *this;
	}
};
extern COkexExchange* pExchange;
extern void Pong();
extern void OnOkexWSConnectSuccess();
extern void OnOkexWSLoginSuccess();
extern void OnRevTickerInfo(STickerData &data);
extern void OnTradeSuccess(std::string& clientOrderID, std::string& serverOrderID);
extern void UpdateTradeInfo(SFuturesTradeInfo& info);
extern void OnTradeFail(std::string& clientOrderID);
#define OKEX_WEB_SOCKET ((COkexWebsocketAPI*)pExchange->GetMarketWebSocket())
#define OKEX_HTTP ((COkexHttpAPI*)pExchange->GetHttp())



void okex_websocket_callbak_open(const char* szExchangeName)
{
	LOCAL_INFO("连接成功");
	OnOkexWSConnectSuccess();
}

void okex_websocket_callbak_close(const char* szExchangeName)
{
	LOCAL_ERROR("断开连接");
}

void okex_websocket_callbak_fail(const char* szExchangeName)
{
	LOCAL_ERROR("连接失败");
}

void okex_http_callbak_message(eHttpAPIType apiType, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData)
{
	switch(apiType)
	{
	case eHttpAPIType_FuturesTrade:
		{
			if(retObj.isObject() && ((retObj["error_code"].isInt() && retObj["error_code"].asInt() == 0) || (retObj["error_code"].isString() && retObj["error_code"].asString() == "0")))
			{
				if(retObj["client_oid"].isString())
					OnTradeSuccess(retObj["client_oid"].asString(), retObj["order_id"].asString());
			}
			else
			{
				LOCAL_ERROR("http type=%d ret=%s strCustomData=%s", apiType, strRet.c_str(), strCustomData.c_str());
				OnTradeFail(strCustomData);
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
				UpdateTradeInfo(info);
				LOCAL_INFO("http更新订单信息 client_order=%s, order=%s, filledQTY=%s, price=%s, priceAvg=%s, status=%s, tradeType=%s", info.strClientOrderID.c_str(), info.orderID.c_str(), info.filledQTY.c_str(), retObj["price"].asString().c_str(), retObj["price_avg"].asString().c_str(), info.status.c_str(), tradeType.c_str());
			}
			else
				LOCAL_ERROR("http type=%d ret=%s", apiType, strRet.c_str());
		}
		break;
	}

}

void okex_websocket_callbak_message(eWebsocketAPIType apiType, const char* szExchangeName, Json::Value& retObj, const std::string& strRet)
{
	switch(apiType)
	{
	case eWebsocketAPIType_FuturesTicker:
		{
			char* szEnd = NULL;
			STickerData data;
			data.volume = stoi(retObj["data"][0]["volume_24h"].asString());
			data.sell = stod(retObj["data"][0]["best_ask"].asString());
			data.buy = stod(retObj["data"][0]["best_bid"].asString());
			data.high = stod(retObj["data"][0]["high_24h"].asString());
			data.low = stod(retObj["data"][0]["low_24h"].asString());
			data.last = stod(retObj["data"][0]["last"].asString());
			data.time = CFuncCommon::ISO8601ToTime(retObj["data"][0]["timestamp"].asString());
			OnRevTickerInfo(data);
		}
		break;
	case eWebsocketAPIType_Pong:
		{
			 Pong();
		}
		break;
	case eWebsocketAPIType_Login:
		{
			OnOkexWSLoginSuccess();
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
				LOCAL_INFO("ws更新订单信息 order=%s, filledQTY=%s, price=%s, priceAvg=%s, status=%s, tradeType=%s", info.orderID.c_str(), info.filledQTY.c_str(), retObj["data"][0]["price"].asString().c_str(), retObj["data"][0]["price_avg"].asString().c_str(), info.status.c_str(), tradeType.c_str());
				UpdateTradeInfo(info);
			}
		}
		break;
	}
}