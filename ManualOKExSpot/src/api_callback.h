#pragma once
#include "mfc_window/ManualOKExSpotDlg.h"
#include "exchange/okex/okex_websocket_api.h"
#include "exchange/okex/okex_http_api.h"
extern CManualOKExSpotDlg* g_pDlg;
extern CExchange* pExchange;
#define OKEX_WEB_SOCKET ((COkexWebsocketAPI*)pExchange->GetWebSocket())
#define OKEX_HTTP ((COkexHttpAPI*)pExchange->GetHttp())

void local_http_callbak_message(eHttpAPIType apiType, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData)
{
	
	switch(apiType)
	{
	case eHttpAPIType_SpotAccountInfoByCurrency:
		{
			bool bRet = false;
			if(retObj.isObject())
			{
				SSpotAccountInfo data;
				if(retObj["currency"].isString())
				{
					bRet = true;
					//if(customData == OKEX_HTTP->m_SpotAccountInfoByCurrencyIndex)
					{
						data.currency = retObj["currency"].asString();
						data.balance = retObj["balance"].asString();
						data.hold = retObj["hold"].asString();
						data.available = retObj["available"].asString();
						g_pDlg->UpdateAccountInfo(data);
					}
				}
			}
			if(!bRet)
				LOCAL_ERROR("http type=%d ret=%s", apiType, strRet.c_str());
		}
		break;
	case eHttpAPIType_SpotTrade:
		{
			//LOCAL_ERROR("eHttpAPIType_SpotTrade ret=%s", strRet.c_str());
			if(retObj.isObject() && (retObj["result"].isBool() && retObj["result"].asBool()))
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
	case eHttpAPIType_SpotTradeInfo:
		{
			if(retObj.isObject() && retObj["order_id"].isString())
			{
				SSpotTradeInfo info;
				info.strClientOrderID = "";
				info.timeStamp = CFuncCommon::ISO8601ToTime(retObj["timestamp"].asString());
				info.filledSize = CFuncCommon::ToString(stoi(retObj["filled_size"].asString()));
				info.orderID = retObj["order_id"].asString();
				info.price = stod(retObj["price"].asString());
				info.status = retObj["status"].asString();
				info.size = CFuncCommon::ToString(stoi(retObj["size"].asString()));
				info.tradeType = retObj["side"].asString();
				g_pDlg->UpdateTradeInfo(info);
				CActionLog("trade", "http更新订单信息 client_order=%s, order=%s, filledSize=%s, price=%s, priceAvg=%s, status=%s, tradeType=%s", info.strClientOrderID.c_str(), info.orderID.c_str(), info.filledSize.c_str(), retObj["price"].asString().c_str(), retObj["price_avg"].asString().c_str(), info.status.c_str(), info.tradeType.c_str());
			}
			else
				LOCAL_ERROR("http type=%d ret=%s", apiType, strRet.c_str());
		}
		break;
	case eHttpAPIType_SpotCancelOrder:
		{
			if(retObj.isObject() && ((retObj["result"].isBool() && retObj["result"].asBool() == true) || (retObj["result"].isString() && retObj["result"].asString() == "true")))
			{
				g_pDlg->OnCancelSuccess(retObj["order_id"].asString());
				CActionLog("trade", "撤消订单成功 order=%s", retObj["order_id"].asString().c_str());
			}
			else
				LOCAL_ERROR("http type=%d ret=%s", apiType, strRet.c_str());
		}
		break;
	default:
		break;
	}
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
	case eWebsocketAPIType_SpotTicker:
		{
			char* szEnd = NULL;
			//CActionLog("market", "%s", strRet.c_str());
			STickerData data;
			data.volume = stoi(retObj["data"][0]["base_volume_24h"].asString());
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
	case eWebsocketAPIType_SpotOrderInfo:
		{
			if(retObj.isObject() && retObj["data"].isArray())
			{
				SSpotTradeInfo info;
				info.strClientOrderID = "";
				info.timeStamp = CFuncCommon::ISO8601ToTime(retObj["data"][0]["timestamp"].asString());
				info.filledSize = retObj["data"][0]["filled_size"].asString();
				info.orderID = retObj["data"][0]["order_id"].asString();
				info.price = stod(retObj["data"][0]["price"].asString());
				info.status = retObj["data"][0]["status"].asString();
				info.size = retObj["data"][0]["size"].asString();
				info.tradeType = retObj["data"][0]["side"].asString();
				g_pDlg->UpdateTradeInfo(info);
				CActionLog("trade", "ws更新订单信息 order=%s, filledSize=%s, price=%s, priceAvg=%s, status=%s, tradeType=%s", info.orderID.c_str(), info.filledSize.c_str(), retObj["data"][0]["price"].asString().c_str(), retObj["data"][0]["price_avg"].asString().c_str(), info.status.c_str(), info.tradeType.c_str());
			}
		}
		break;
	case eWebsocketAPIType_SpotAccountInfo:
		{
			if(retObj.isObject() && retObj["data"].isArray())
			{
				SSpotAccountInfo info;
				info.balance = retObj["data"][0]["balance"].asString();
				info.currency = retObj["data"][0]["currency"].asString();
				info.hold = retObj["data"][0]["hold"].asString();
				info.available = retObj["data"][0]["available"].asString();
				g_pDlg->UpdateAccountInfo(info);
			}
			else
				LOCAL_ERROR("ws type=%d ret=%s", apiType, strRet.c_str());
		}
		break;
	case eWebsocketAPIType_EntrustDepth:
		{
			if(retObj.isObject() && retObj["data"].isArray() && retObj["action"].isString())
			{
				//LOCAL_ERROR("depth %s", strRet.c_str());
				if(retObj["action"].asString() == "partial")
				{
					g_pDlg->ClearDepth();
					g_pDlg->m_bWaitDepthBegin = false;
				}
				else
				{
					if(g_pDlg->m_bWaitDepthBegin)
						break;
				}

				Json::Value& data = retObj["data"][0];
				if(data["bids"].isArray())
				{
					for(int i = 0; i < (int)data["bids"].size(); ++i)
					{
						SSpotDepth depthInfo;
						depthInfo.price = data["bids"][i][0].asString();
						depthInfo.size = data["bids"][i][1].asString();
						double size = stod(depthInfo.size);
						depthInfo.showSize = CFuncCommon::Double2String(size+DOUBLE_PRECISION, g_pDlg->m_nSizeDecimal).c_str();
						depthInfo.tradeNum = data["bids"][i][2].asString();
						g_pDlg->UpdateDepthInfo(true, depthInfo);
					}
				}
				if(data["asks"].isArray())
				{
					for(int i = 0; i < (int)data["asks"].size(); ++i)
					{
						SSpotDepth depthInfo;
						depthInfo.price = data["asks"][i][0].asString();
						depthInfo.size = data["asks"][i][1].asString();
						double size = stod(depthInfo.size);
						depthInfo.showSize = CFuncCommon::Double2String(size + DOUBLE_PRECISION, g_pDlg->m_nSizeDecimal).c_str();
						depthInfo.tradeNum = data["asks"][i][2].asString();
						g_pDlg->UpdateDepthInfo(false, depthInfo);
					}
				}
				std::string src = "";
				if(!g_pDlg->CheckDepthInfo(data["checksum"].asInt(), src))
				{
					LOCAL_ERROR("check %s", src.c_str());
					LOCAL_ERROR("depth %s", strRet.c_str());
				}
				else
				{
					if(retObj["action"].asString() == "partial")
						LOCAL_ERROR("第一次crc校验成功");
				}
				//LOCAL_ERROR("num=%d", num);
			}
		}
		break;
	default:
		break;
	}
}

