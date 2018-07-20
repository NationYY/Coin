#include "stdafx.h"
#include "exx_exchange.h"
#include "exchange/exx/exx_http_api.h"
#include "exchange/exx/exx_web_socket_api.h"

CExxExchange::CExxExchange(std::string strAPIKey, std::string strSecretKey, std::string strContentType)
{
	m_pWebSocketAPI = new CExxWebSocketAPI(strAPIKey, strSecretKey);
	m_pHttpAPI = new CExxHttpAPI(strAPIKey, strSecretKey, strContentType);
}


CExxExchange::~CExxExchange()
{
}

void CExxExchange::OnHttpResponse(eHttpAPIType type, Json::Value& retObj, const std::string& strRet)
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
	default:
		break;
	}
	CExchange::OnHttpResponse(type, retObj, strRet);
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
		m_dataCenter.SetBuyAndSellPrice(atof(strBuy.c_str()), atof(strSell.c_str()));
	}
}