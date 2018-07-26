#include "stdafx.h"
#include "exx_exchange.h"
#include "exchange/exx/exx_http_api.h"
#include "exchange/exx/exx_websocket_api.h"

CExxExchange::CExxExchange(std::string strAPIKey, std::string strSecretKey)
{
	m_pWebSocketAPI = new CExxWebSocketAPI(strAPIKey, strSecretKey);
	m_pHttpAPI = new CExxHttpAPI(strAPIKey, strSecretKey, "");
	m_pHttpTradeAPI = new CExxHttpAPI(strAPIKey, strSecretKey, "");
	m_listSupportMarket.push_back(eMarketType_ETH_BTC);
	m_listSupportMarket.push_back(eMarketType_ETH_USDT);
	m_listSupportMarket.push_back(eMarketType_BTC_USDT);
}


CExxExchange::~CExxExchange()
{
}

void CExxExchange::OnWebsocketResponse(Json::Value& retObj, const std::string& strRet)
{
	if(retObj.isArray() && retObj[0].isArray() && retObj[0][0].isString() && retObj[0][0].asString() == "AE")
	{
		if(retObj[0].size() > 5)
		{
			int updateTime = 0;
			if(retObj[0][3].isString())
				updateTime = atoi(retObj[0][3].asString().c_str());
			Json::Value info = retObj[0][4];
			if(info["asks"].isArray())
			{
				for(int i = 0; i < info["asks"].size(); ++i)
				{
					std::string price = info["asks"][i][0].asString();
					std::string volume = info["asks"][i][1].asString();
					m_dataCenter.UpdateSellEntrustDepth(price, volume, updateTime);
				}
			}
			info = retObj[0][5];
			if(info["bids"].isArray())
			{
				for(int i = 0; i < info["bids"].size(); ++i)
				{
					std::string price = info["bids"][i][0].asString();
					std::string volume = info["bids"][i][1].asString();
					m_dataCenter.UpdateBuyEntrustDepth(price, volume, updateTime);
				}
			}
		}
		if(m_webSocketCallbakMessage)
			m_webSocketCallbakMessage(eWebsocketAPIType_EntrustDepth, retObj, strRet);
	}
	else if(retObj.isArray() && retObj[0].isString() && retObj[0].asString() == "E")
	{
		if(retObj.size() > 6 && retObj[4].isString() && retObj[5].isString() && retObj[6].isString())
		{
			int updateTime = 0;
			if(retObj[2].isString())
				updateTime = atoi(retObj[2].asString().c_str());
			std::string type = retObj[4].asString();
			std::string price = retObj[5].asString();
			std::string volume = retObj[6].asString();
			if(type == "BID")
			{
				if(volume == "0.00000000")
					m_dataCenter.DelBuyEntrustDepth(price, updateTime);
				else
					m_dataCenter.UpdateBuyEntrustDepth(price, volume, updateTime);
			}
			else if(type == "ASK")
			{
				if(volume == "0.00000000")
					m_dataCenter.DelSellEntrustDepth(price, updateTime);
				else
					m_dataCenter.UpdateSellEntrustDepth(price, volume, updateTime);
			}
		}
		if(m_webSocketCallbakMessage)
			m_webSocketCallbakMessage(eWebsocketAPIType_EntrustDepth, retObj, strRet);
	}
}

void CExxExchange::OnHttpResponse(eHttpAPIType type, Json::Value& retObj, const std::string& strRet, int customData)
{
	switch(type)
	{
	case eHttpAPIType_MarketList:
		break;
	case eHttpAPIType_Balance:
		Parse_Balance(retObj, strRet);
		break;
	case eHttpAPIType_Ticker:
		Parse_Ticker(retObj, strRet);
		break;
	case eHttpAPIType_EntrustDepth:
		Parse_EntrustDepth(retObj, strRet);
		break;
	case eHttpAPIType_Trade:
		Parse_Trade(retObj, strRet);
		break;
	case eHttpAPIType_TradeOrderListState:
		Parse_TradeOrderListState(retObj, strRet);
		break;
	case eHttpAPIType_TradeOrderState:
		Parse_TradeOrderState(retObj, strRet);
		break;
	default:
		break;
	}
	CExchange::OnHttpResponse(type, retObj, strRet, customData);
}

void CExxExchange::Parse_Balance(Json::Value& retObj, const std::string& strRet)
{
	m_dataCenter.ClearAllBalance();
	Json::Value& funds = retObj["funds"];
	if(funds.isObject())
	{
		Json::Value::Members members = retObj["funds"].getMemberNames();   // 获取所有key的值
		for(Json::Value::Members::iterator iterMember = members.begin(); iterMember != members.end(); iterMember++)   // 遍历每个key
		{
			std::string strKey = *iterMember;
			if(funds[strKey].isObject() && funds[strKey]["total"].isString())
			{
				std::string strTotal = funds[strKey]["total"].asString();
				double total = atof(strTotal.c_str());
				if(total > 0.0)
				{
					std::string strFreeze = funds[strKey]["freeze"].asString();
					std::string strBalance = funds[strKey]["balance"].asString();
					m_dataCenter.SetBalance(strKey.c_str(), total, atof(strFreeze.c_str()), atof(strBalance.c_str()));
					SBalanceInfo info;
					info.name = strKey;
					info.total = total;

					info.freeze = atof(strFreeze.c_str());

					info.balance = atof(strBalance.c_str());
				}
			}
		}
	}
}

void CExxExchange::Parse_Ticker(Json::Value& retObj, const std::string& strRet)
{
	if(retObj["ticker"].isObject())
	{
		Json::Value& ticker = retObj["ticker"];
		std::string strSell = ticker["sell"].asString();
		std::string strBuy = ticker["buy"].asString();
		int time = ticker["date"].asInt();
		
		m_dataCenter.SetBuyAndSellPrice(atof(strBuy.c_str()), atof(strSell.c_str()));
	}
}

void CExxExchange::Parse_EntrustDepth(Json::Value& retObj, const std::string& strRet)
{
	m_dataCenter.ClearAllBalance();
	int updateTime = 0;
	if(retObj["timestamp"].isInt())
		updateTime = retObj["timestamp"].asInt();
	if(retObj["asks"].isArray())
	{
		Json::Value& asks = retObj["asks"];
		for(int i = 0; i<asks.size(); ++i)
		{
			if(asks[i].isArray() && asks[i].size() > 1)
			{
				std::string price = asks[i][0].asString();
				std::string volume = asks[i][1].asString();
				m_dataCenter.UpdateSellEntrustDepth(price, volume, updateTime);
			}
		}
	}
	if(retObj["bids"].isArray())
	{
		Json::Value& bids = retObj["bids"];
		for(int i = 0; i<bids.size(); ++i)
		{
			if(bids[i].isArray() && bids[i].size() > 1)
			{
				std::string price = bids[i][0].asString();
				std::string volume = bids[i][1].asString();
				m_dataCenter.UpdateBuyEntrustDepth(price, volume, updateTime);
			}
		}
	}
}

void CExxExchange::Parse_Trade(Json::Value& retObj, const std::string& strRet)
{
	if(retObj["code"].isInt() && retObj["code"].asInt() == 100 && retObj["id"].isString())
		m_dataCenter.AddTradeOrders(retObj["id"].asString());
}

void CExxExchange::Parse_TradeOrderState(Json::Value& retObj, const std::string& strRet)
{
	if(retObj["status"].isInt())
	{
		std::string id = retObj["id"].asString();
		__int64 date = retObj["trade_date"].asInt64();
		double price = retObj["price"].asDouble();
		double amount = retObj["total_amount"].asDouble();
		std::string type = retObj["type"].asString();
		switch(retObj["status"].asInt())
		{
		case 0:
			m_dataCenter.UpdateTradeOrder(id, date);
			break;
		case 1:
			m_dataCenter.DeleteTradeOrder(id);
			break;
		case 2:
			m_dataCenter.FinishTradeOrder(id, price, amount, date, type);
			break;
		case 3:
			break;
		default:
			break;
		}
	}
}