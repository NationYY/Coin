#include "stdafx.h"
#include "bw_exchange.h"
#include "exchange/bw/bw_http_api.h"
#include "exchange/bw/bw_websocket_api.h"
#ifdef _OPEN_BW_
CBWExchange::CBWExchange(std::string strAPIKey, std::string strSecretKey)
{
	m_pWebSocketAPI = new CBWWebsocketAPI(strAPIKey, strSecretKey);
	m_pWebSocketAPI->SetExchange(this);
	m_pHttpAPI = new CBWHttpAPI(strAPIKey, strSecretKey);
	m_pHttpAPI->SetExchange(this);
	m_pHttpTradeAPI = new CBWHttpAPI(strAPIKey, strSecretKey);
	m_pHttpTradeAPI->SetExchange(this);
	m_listSupportMarket.push_back(eMarketType_ETH_BTC);
	m_listSupportMarket.push_back(eMarketType_ETH_USDT);
	m_listSupportMarket.push_back(eMarketType_BTC_USDT);
	m_listSupportMarket.push_back(eMarketType_BWB_USDT);
	m_listSupportMarket.push_back(eMarketType_BWB_BTC);
	m_listSupportMarket.push_back(eMarketType_BWB_ETH);
	m_listSupportMarket.push_back(eMarketType_BWB_QC);
	m_listSupportMarket.push_back(eMarketType_USDT_QC);
}


CBWExchange::~CBWExchange()
{
}

void CBWExchange::OnWebsocketResponse(const char* szExchangeName, Json::Value& retObj, const std::string& strRet)
{
	if(retObj.isArray() && retObj[0].isArray() && retObj[0][0].isString() && retObj[0][0].asString() == "AE")
	{
		if(retObj[0].size() > 5)
		{
			int updateTime = 0;
			std::string marketName = "";
			if(retObj[0][3].isString())
				updateTime = atoi(retObj[0][3].asString().c_str());
			if(retObj[0][2].isString())
				marketName = retObj[0][2].asString();
			m_dataCenter.ClearAllEntrustDepth(marketName);
			Json::Value info = retObj[0][4];
			if(info["asks"].isArray())
			{
				for(int i = 0; i < (int)info["asks"].size(); ++i)
				{
					std::string price = info["asks"][i][0].asString();
					std::string volume = info["asks"][i][1].asString();
					m_dataCenter.UpdateSellEntrustDepth(price, volume, updateTime, marketName);
				}
			}
			info = retObj[0][5];
			if(info["bids"].isArray())
			{
				for(int i = 0; i < (int)info["bids"].size(); ++i)
				{
					std::string price = info["bids"][i][0].asString();
					std::string volume = info["bids"][i][1].asString();
					m_dataCenter.UpdateBuyEntrustDepth(price, volume, updateTime, marketName);
				}
			}
		}
		if(m_webSocketCallbakMessage)
			m_webSocketCallbakMessage(eWebsocketAPIType_EntrustDepth, szExchangeName, retObj, strRet);
	}
	else if(retObj.isArray() && retObj[0].isString() && retObj[0].asString() == "E")
	{
		if(retObj.size() > 6 && retObj[4].isString() && retObj[5].isString() && retObj[6].isString())
		{
			int updateTime = 0;
			std::string marketName = "";
			if(retObj[2].isString())
				updateTime = atoi(retObj[2].asString().c_str());
			if(retObj[3].isString())
				marketName = retObj[3].asString();
			std::string type = retObj[4].asString();
			std::string price = retObj[5].asString();
			std::string volume = retObj[6].asString();
			if(type == "BID")
			{
				if(volume == "0")
					m_dataCenter.DelBuyEntrustDepth(price, updateTime, marketName);
				else
					m_dataCenter.UpdateBuyEntrustDepth(price, volume, updateTime, marketName);
			}
			else if(type == "ASK")
			{
				if(volume == "0")
					m_dataCenter.DelSellEntrustDepth(price, updateTime, marketName);
				else
					m_dataCenter.UpdateSellEntrustDepth(price, volume, updateTime, marketName);
			}
		}
		if(m_webSocketCallbakMessage)
			m_webSocketCallbakMessage(eWebsocketAPIType_EntrustDepth, szExchangeName, retObj, strRet);
	}
	else if(retObj.isArray() && retObj[0].isArray() && retObj[0][0].isString() && retObj[0][0].asString() == "T")
	{
		std::string marketName = "";
		if(retObj[0][3].isString())
			marketName = retObj[0][3].asString();
		double price = atof(retObj[0][5].asString().c_str());
		m_dataCenter.SetLatestExecutedOrderPrice(price, marketName);
		if(m_webSocketCallbakMessage)
			m_webSocketCallbakMessage(eWebsocketAPIType_LatestExecutedOrder, szExchangeName, retObj, strRet);
	}
	else if(retObj.isArray() && retObj[0].isString() && retObj[0].asString() == "T")
	{
		std::string marketName = "";
		if(retObj[3].isString())
			marketName = retObj[3].asString();
		double price = atof(retObj[5].asString().c_str());
		m_dataCenter.SetLatestExecutedOrderPrice(price, marketName);
		if(m_webSocketCallbakMessage)
			m_webSocketCallbakMessage(eWebsocketAPIType_LatestExecutedOrder, szExchangeName, retObj, strRet);
	}
}

void CBWExchange::OnHttpResponse(eHttpAPIType type, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData)
{
	switch(type)
	{
	case eHttpAPIType_Balance:
		Parse_Balance(retObj, strRet);
		break;
	case eHttpAPIType_TradeOrderState:
		Parse_TradeOrderState(retObj, strRet);
		break;
	case eHttpAPIType_Trade:
		Parse_Trade(retObj, strRet, strCustomData);
		break;
	case eHttpAPIType_CancelTrade:
		Parse_CancelTrade(retObj, strRet, strCustomData);
		break;
	}
	CExchange::OnHttpResponse(type, retObj, strRet, customData, strCustomData);
}


void CBWExchange::Parse_Balance(Json::Value& retObj, const std::string& strRet)
{
	m_dataCenter.ClearAllBalance();
	Json::Value& fundsList = retObj["datas"]["list"];
	for(int i = 0; i<(int)fundsList.size(); ++i)
	{
		switch(fundsList[i]["currencyTypeId"].asInt())
		{
		case USDT_ID:
			{
				std::string strFreeze = fundsList[i]["freeze"].asString();
				std::string strBalance = fundsList[i]["amount"].asString();
				double freeze = atof(strFreeze.c_str());
				double balance = atof(strBalance.c_str());
				m_dataCenter.SetBalance("usdt", freeze+balance, freeze, balance);
			}
			break;
		case BTC_ID:
			{
				std::string strFreeze = fundsList[i]["freeze"].asString();
				std::string strBalance = fundsList[i]["amount"].asString();
				double freeze = atof(strFreeze.c_str());
				double balance = atof(strBalance.c_str());
				m_dataCenter.SetBalance("btc", freeze+balance, freeze, balance);
			}
			break;
		case BWB_ID:
			{
				std::string strFreeze = fundsList[i]["freeze"].asString();
				std::string strBalance = fundsList[i]["amount"].asString();
				double freeze = atof(strFreeze.c_str());
				double balance = atof(strBalance.c_str());
				m_dataCenter.SetBalance("bwb", freeze+balance, freeze, balance);
			}
			break;
		case ETH_ID:
			{
				std::string strFreeze = fundsList[i]["freeze"].asString();
				std::string strBalance = fundsList[i]["amount"].asString();
				double freeze = atof(strFreeze.c_str());
				double balance = atof(strBalance.c_str());
				m_dataCenter.SetBalance("eth", freeze+balance, freeze, balance);
			}
			break;
		case QC_ID:
			{
				std::string strFreeze = fundsList[i]["freeze"].asString();
				std::string strBalance = fundsList[i]["amount"].asString();
				double freeze = atof(strFreeze.c_str());
				double balance = atof(strBalance.c_str());
				m_dataCenter.SetBalance("qc", freeze + balance, freeze, balance);
			}
			break;
		default:
			break;
		}
	}
}

void CBWExchange::Parse_TradeOrderState(Json::Value& retObj, const std::string& strRet)
{
	if(retObj["datas"].isObject() && retObj["datas"]["status"].isInt())
	{
		std::string id = retObj["datas"]["entrustId"].asString();
		__int64 date = retObj["datas"]["createTime"].asInt64();
		double price = atof(retObj["datas"]["price"].asString().c_str());
		double amount = atof(retObj["datas"]["amount"].asString().c_str());
		int type = retObj["datas"]["type"].asInt();
		eTradeType eType = eTradeType_buy;
		if(type == 0)
			eType = eTradeType_sell;
		else if(type == 1)
			eType = eTradeType_buy;
		switch(retObj["datas"]["status"].asInt())
		{
		case 0:
			m_dataCenter.UpdateTradeOrder(id, date);
			break;
		case 1:
			m_dataCenter.DeleteTradeOrder(id);
			break;
		case 2:
			m_dataCenter.FinishTradeOrder(id, price, amount, date, eType);
			break;
		case -1:
			m_dataCenter.DeleteTradeOrder(id);
			break;
		case 3:
			break;
		default:
			break;
		}
	}
}

void CBWExchange::Parse_Trade(Json::Value& retObj, const std::string& strRet, std::string strCustomData)
{
	if(retObj["resMsg"]["code"].isString() && retObj["resMsg"]["code"].asString() == "1" && retObj["datas"]["entrustId"].isString())
	{
		eTradeType type = eTradeType_buy;
		if(strCustomData == "buy")
			type = eTradeType_buy;
		else if(strCustomData == "sell")
			type = eTradeType_sell;
		m_dataCenter.AddTradeOrders(retObj["datas"]["entrustId"].asString(), type);
	}
}

void CBWExchange::Parse_CancelTrade(Json::Value& retObj, const std::string& strRet, std::string strCustomData)
{
	if(retObj["resMsg"]["code"].isString() && retObj["resMsg"]["code"].asString() == "1")
		m_dataCenter.DelTradeOrders(strCustomData);
}
#endif