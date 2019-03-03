#pragma once
#include "mfc_window/ManualOKExFuturesDlg.h"
#include "exchange/okex/okex_websocket_api.h"
#include "exchange/okex/okex_http_api.h"
extern CManualOKExFuturesDlg* g_pDlg;
extern CExchange* pExchange;
#define OKEX_WEB_SOCKET ((COkexWebsocketAPI*)pExchange->GetWebSocket())
#define OKEX_HTTP ((COkexHttpAPI*)pExchange->GetHttp())

void local_http_callbak_message(eHttpAPIType apiType, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData)
{
	/*
	switch(apiType)
	{
	case eHttpAPIType_FuturesAccountInfoByCurrency:
		{
			bool bRet = false;
			if(retObj.isObject())
			{
				SFuturesAccountInfo data;
				if(g_pDlg->m_bSwapFutures)
				{
					if(retObj["info"].isObject() && retObj["info"]["equity"].isString())
					{
						bRet = true;
						if(customData == OKEX_HTTP->m_futuresAccountInfoByCurrencyIndex)
							data.equity = retObj["info"]["equity"].asString();
					}
				}
				else
				{
					if(retObj["equity"].isString())
					{
						bRet = true;
						if(customData == OKEX_HTTP->m_futuresAccountInfoByCurrencyIndex)
							data.equity = retObj["equity"].asString();
					}
				}
				g_pDlg->UpdateAccountInfo(data);
			}
			if(!bRet)
				LOCAL_ERROR("http type=%d ret=%s", apiType, strRet.c_str());
		}
		break;
	case eHttpAPIType_FuturesServerTime:
		{
		}
		break;
	case eHttpAPIType_FuturesTrade:
		{
			if(retObj.isObject() && ((retObj["error_code"].isInt() && retObj["error_code"].asInt() == 0) || (retObj["error_code"].isString() && retObj["error_code"].asString() == "0")))
			{
				if(retObj["client_oid"].isString())
					g_pDlg->OnTradeSuccess(retObj["client_oid"].asString(), retObj["order_id"].asString());
			}
			else
			{
				g_pDlg->OnTradeFail(strCustomData);
				LOCAL_ERROR("http type=%d ret=%s client_oid=%s", apiType, strRet.c_str(), strCustomData.c_str());
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
				g_pDlg->UpdateTradeInfo(info);
				CActionLog("trade", "http更新订单信息 client_order=%s, order=%s, filledQTY=%s, price=%s, priceAvg=%s, status=%s, tradeType=%s", info.strClientOrderID.c_str(), info.orderID.c_str(), info.filledQTY.c_str(), retObj["price"].asString().c_str(), retObj["price_avg"].asString().c_str(), info.status.c_str(), tradeType.c_str());
			}
			else
				LOCAL_ERROR("http type=%d ret=%s", apiType, strRet.c_str());
		}
		break;
	case eHttpAPIType_FuturesCancelOrder:
		{
			if(retObj.isObject() && ((retObj["result"].isBool() && retObj["result"].asBool() == true) || (retObj["result"].isString() && retObj["result"].asString() == "true")))
			{
				CActionLog("trade", "撤消订单成功 order=%s", retObj["order_id"].asString().c_str());
			}
			else
				LOCAL_ERROR("http type=%d ret=%s", apiType, strRet.c_str());
		}
		break;
	default:
		break;
	}
	*/
}

void local_websocket_callbak_open(const char* szExchangeName)
{
	LOCAL_INFO("连接成功");
	g_pDlg->m_tListenPong = 0;
	if(g_pDlg->m_bRun)
	{
		g_pDlg->m_bRun = false;
		g_pDlg->OnBnClickedStart();
	}
}

void local_websocket_callbak_close(const char* szExchangeName)
{
	LOCAL_ERROR("断开连接");
	g_pDlg->m_tListenPong = 0;
}

void local_websocket_callbak_fail(const char* szExchangeName)
{
	LOCAL_ERROR("连接失败");
	g_pDlg->m_tListenPong = 0;
}


void local_websocket_callbak_message(eWebsocketAPIType apiType, const char* szExchangeName, Json::Value& retObj, const std::string& strRet)
{
	switch(apiType)
	{
	case eWebsocketAPIType_FuturesTicker:
		{
			char* szEnd = NULL;
			CActionLog("market", "%s", strRet.c_str());
			STickerData data;
			data.volume = stoi(retObj["data"][0]["volume_24h"].asString());
			data.sell = stod(retObj["data"][0]["best_ask"].asString());
			data.buy = stod(retObj["data"][0]["best_bid"].asString());
			data.high = stod(retObj["data"][0]["high_24h"].asString());
			data.low = stod(retObj["data"][0]["low_24h"].asString());
			data.last = stod(retObj["data"][0]["last"].asString());
			data.time = CFuncCommon::ISO8601ToTime(retObj["data"][0]["timestamp"].asString());
			g_pDlg->OnRevTickerInfo(data);
		}
		break;
	case eWebsocketAPIType_Pong:
		{
			g_pDlg->Pong();
		}
		break;
	case eWebsocketAPIType_Login:
		{
			g_pDlg->OnLoginSuccess();
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
				g_pDlg->UpdateTradeInfo(info);
				CActionLog("trade", "ws更新订单信息 order=%s, filledQTY=%s, price=%s, priceAvg=%s, status=%s, tradeType=%s", info.orderID.c_str(), info.filledQTY.c_str(), retObj["data"][0]["price"].asString().c_str(), retObj["data"][0]["price_avg"].asString().c_str(), info.status.c_str(), tradeType.c_str());
			}
		}
		break;
	case eWebsocketAPIType_FuturesAccountInfo:
		{
			if(retObj.isObject() && retObj["data"].isArray())
			{
				SFuturesAccountInfo info;
				if(g_pDlg->m_bSwapFutures)
					info.equity = retObj["data"][0]["equity"].asString();
				else
					info.equity = retObj["data"][0][g_pDlg->m_strCoinType]["equity"].asString();
				g_pDlg->UpdateAccountInfo(info);
			}
			else
				LOCAL_ERROR("ws type=%d ret=%s", apiType, strRet.c_str());
		}
		break;
	case eWebsocketAPIType_FuturesEntrustDepth:
		{
			if(retObj.isObject() && retObj["data"].isArray() && retObj["action"].isString())
			{
				//LOCAL_ERROR("depth %s", strRet.c_str());
				if(retObj["action"].asString() == "partial")
					g_pDlg->ClearDepth();

				Json::Value& data = retObj["data"][0];
				if(data["bids"].isArray())
				{
					for(int i = 0; i < (int)data["bids"].size(); ++i)
					{
						SFuturesDepth depthInfo;
						if(g_pDlg->m_bSwapFutures)
							depthInfo.price = data["bids"][i][0].asString();
						else
							depthInfo.price = CFuncCommon::Double2String(data["bids"][i][0].asDouble(), g_pDlg->m_nPriceDecimal);

						
						depthInfo.size = data["bids"][i][1].asString();
						depthInfo.brokenSize = data["bids"][i][2].asInt();
						depthInfo.tradeNum = data["bids"][i][3].asInt();
						g_pDlg->UpdateDepthInfo(true, depthInfo);
					}
				}
				if(data["asks"].isArray())
				{
					for(int i = 0; i < (int)data["asks"].size(); ++i)
					{
						SFuturesDepth depthInfo;
						if(g_pDlg->m_bSwapFutures)
							depthInfo.price = data["asks"][i][0].asString();
						else
							depthInfo.price = CFuncCommon::Double2String(data["asks"][i][0].asDouble(), g_pDlg->m_nPriceDecimal);
						depthInfo.size = data["asks"][i][1].asString();
						depthInfo.brokenSize = data["asks"][i][2].asInt();
						depthInfo.tradeNum = data["asks"][i][3].asInt();
						g_pDlg->UpdateDepthInfo(false, depthInfo);
					}
				}
				//LOCAL_ERROR("num=%d", num);
			}
		}
		break;
	default:
		break;
	}
}

