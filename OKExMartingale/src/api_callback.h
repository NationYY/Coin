#pragma once
#include "mfc_window/okex_martingale_dlg.h"
extern COKExMartingaleDlg* g_pDlg;
void local_websocket_callbak_open(const char* szExchangeName)
{
	LOCAL_INFO("连接成功");
	g_pDlg->m_tListenPong = 0;
	if(g_pDlg->m_bRun)
	{
		g_pDlg->m_bRun = false;
		g_pDlg->OnBnClickedButtonStart();
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

void local_http_callbak_message(eHttpAPIType apiType, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData)
{
	switch(apiType)
	{
	case eHttpAPIType_SpotTrade:
		{
			if(retObj.isObject() && retObj["result"].isBool() && retObj["result"].asBool())
			{
				g_pDlg->OnTradeSuccess(retObj["client_oid"].asString(), retObj["order_id"].asString());
			}
			else
				LOCAL_ERROR("http type=%d ret=%s", apiType, strRet.c_str());
		}
		break;
	case eHttpAPIType_SpotTradeInfo:
		{
			if(retObj.isObject() && retObj["order_id"].isString())
			{
				SSPotTradeInfo info;
				info.orderID = retObj["order_id"].asString();
				info.price = retObj["price"].asString();
				info.size = retObj["size"].asString();
				info.side = retObj["side"].asString();
				info.strTimeStamp = retObj["timestamp"].asString();
				info.timeStamp = CFuncCommon::ISO8601ToTime(info.strTimeStamp);
				info.filledSize = retObj["filled_size"].asString();
				info.filledNotional = retObj["filled_notional"].asString();
				info.status = retObj["status"].asString();
				g_pDlg->UpdateTradeInfo(info);
				CActionLog("trade", "http更新订单信息 order=%s filled_size=%s price=%s status=%s side=%s", info.orderID.c_str(), info.filledSize.c_str(), info.price.c_str(), info.status.c_str(), info.side.c_str());
			}
			else
				LOCAL_ERROR("http type=%d ret=%s", apiType, strRet.c_str());
		}
		break;
	default:
		break;
	}
}

time_t lastKlineTime = 0;
std::string lastKlineRetStr = "";
Json::Value lastKlineJson;
void local_websocket_callbak_message(eWebsocketAPIType apiType, const char* szExchangeName, Json::Value& retObj, const std::string& strRet)
{
	switch(apiType)
	{
	case eWebsocketAPIType_SpotKline:
		{
			char* szEnd = NULL;
			time_t curTime = CFuncCommon::ISO8601ToTime(retObj["data"][0]["candle"][0].asString());
			CActionLog("all_kline", "%s", strRet.c_str());
			if(curTime >= lastKlineTime)
			{
				if(curTime > lastKlineTime && lastKlineTime != 0)
				{
					CActionLog("market", "%s", lastKlineRetStr.c_str());
					SKlineData data;
					data.time = CFuncCommon::ISO8601ToTime(lastKlineJson["data"][0]["candle"][0].asString());
					data.openPrice = CFuncCommon::Round(stod(lastKlineJson["data"][0]["candle"][1].asString()) + DOUBLE_PRECISION, g_pDlg->m_nPriceDecimal);
					data.highPrice = CFuncCommon::Round(stod(lastKlineJson["data"][0]["candle"][2].asString()) + DOUBLE_PRECISION, g_pDlg->m_nPriceDecimal);
					data.lowPrice = CFuncCommon::Round(stod(lastKlineJson["data"][0]["candle"][3].asString()) + DOUBLE_PRECISION, g_pDlg->m_nPriceDecimal);
					data.closePrice = CFuncCommon::Round(stod(lastKlineJson["data"][0]["candle"][4].asString()) + DOUBLE_PRECISION, g_pDlg->m_nPriceDecimal);
					data.volume = lastKlineJson["data"][0]["candle"][5].asString();
					g_pDlg->AddKlineData(data);
				}
				lastKlineTime = curTime;
				lastKlineRetStr = strRet;
				lastKlineJson = retObj;
			}
		}
		break;
	case eWebsocketAPIType_SpotTicker:
		{
			char* szEnd = NULL;
			CActionLog("market", "%s", strRet.c_str());
			STickerData data;
			data.baseVolume24h = retObj["data"][0]["base_volume_24h"].asString();
			data.quoteVolume24h = retObj["data"][0]["quote_volume_24h"].asString();
			data.sell = CFuncCommon::Round(stod(retObj["data"][0]["best_ask"].asString()) + DOUBLE_PRECISION, g_pDlg->m_nPriceDecimal);
			data.buy = CFuncCommon::Round(stod(retObj["data"][0]["best_bid"].asString()) + DOUBLE_PRECISION, g_pDlg->m_nPriceDecimal);
			data.high = CFuncCommon::Round(stod(retObj["data"][0]["high_24h"].asString()) + DOUBLE_PRECISION, g_pDlg->m_nPriceDecimal);
			data.low = CFuncCommon::Round(stod(retObj["data"][0]["low_24h"].asString()) + DOUBLE_PRECISION, g_pDlg->m_nPriceDecimal);
			data.last = CFuncCommon::Round(stod(retObj["data"][0]["last"].asString()) + DOUBLE_PRECISION, g_pDlg->m_nPriceDecimal);
			data.time = CFuncCommon::ISO8601ToTime(retObj["data"][0]["timestamp"].asString());
			g_pDlg->OnRevTickerInfo(data);
		}
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
				SSPotTradeInfo info;
				info.orderID = retObj["data"][0]["order_id"].asString();
				info.price = retObj["data"][0]["price"].asString();
				info.size = retObj["data"][0]["size"].asString();
				info.side = retObj["data"][0]["side"].asString();
				info.strTimeStamp = retObj["data"][0]["timestamp"].asString();
				info.timeStamp = CFuncCommon::ISO8601ToTime(info.strTimeStamp);
				info.filledSize = retObj["data"][0]["filled_size"].asString();
				info.filledNotional = retObj["data"][0]["filled_notional"].asString();
				info.status = retObj["data"][0]["status"].asString();
				g_pDlg->UpdateTradeInfo(info);
			}
			else
				LOCAL_ERROR("ws type=%d ret=%s", apiType, strRet.c_str());
		}
		break;
	case eWebsocketAPIType_SpotAccountInfo:
		{
			if(retObj.isObject() && retObj["data"].isArray())
			{
				SSpotAccountInfo info;
				info.balance = stod(retObj["data"][0]["balance"].asString());
				info.hold = stod(retObj["data"][0]["hold"].asString());
				info.available = stod(retObj["data"][0]["available"].asString());
				info.currency = retObj["data"][0]["currency"].asString();
				g_pDlg->UpdateAccountInfo(info);
			}
			else
				LOCAL_ERROR("ws type=%d ret=%s", apiType, strRet.c_str());
		}
		break;
	default:
		break;
	}
}