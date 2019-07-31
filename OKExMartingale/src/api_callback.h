#pragma once
#include "mfc_window/okex_martingale_dlg.h"
#include "exchange/okex/okex_websocket_api.h"
#include "exchange/okex/okex_http_api.h"
extern COKExMartingaleDlg* g_pDlg;
extern CExchange* pExchange;
#define OKEX_WEB_SOCKET ((COkexWebsocketAPI*)pExchange->GetWebSocket())
#define OKEX_HTTP ((COkexHttpAPI*)pExchange->GetHttp())

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
	g_pDlg->m_bLoginSuccess = false;
	g_pDlg->m_bNeedSubscribe = true;
}

void local_websocket_callbak_fail(const char* szExchangeName)
{
	LOCAL_ERROR("连接失败");
	g_pDlg->m_tListenPong = 0;
	g_pDlg->m_bLoginSuccess = false;
	g_pDlg->m_bNeedSubscribe = true;
}

void local_http_callbak_message(eHttpAPIType apiType, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData)
{
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
						{
							data.equity = retObj["info"]["equity"].asString();
							data.availBalance = retObj["info"]["total_avail_balance"].asString();
						}
					}
				}
				else
				{
					if(retObj["equity"].isString())
					{
						bRet = true;
						if(customData == OKEX_HTTP->m_futuresAccountInfoByCurrencyIndex)
						{
							data.equity = retObj["equity"].asString();
							data.availBalance = retObj["total_avail_balance"].asString();
						}
					}
				}
				g_pDlg->UpdateAccountInfo(data);
			}
			if(!bRet)
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
	case eWebsocketAPIType_FuturesEntrustDepth:
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
						SFuturesDepth depthInfo;
						depthInfo.price = data["bids"][i][0].asString();
						depthInfo.size = data["bids"][i][1].asString();
						depthInfo.brokenSize = data["bids"][i][2].asString();
						depthInfo.tradeNum = data["bids"][i][3].asString();
						g_pDlg->UpdateDepthInfo(true, depthInfo);
					}
				}
				if(data["asks"].isArray())
				{
					for(int i = 0; i < (int)data["asks"].size(); ++i)
					{
						SFuturesDepth depthInfo;
						depthInfo.price = data["asks"][i][0].asString();
						depthInfo.size = data["asks"][i][1].asString();
						depthInfo.brokenSize = data["asks"][i][2].asString();
						depthInfo.tradeNum = data["asks"][i][3].asString();
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
	case eWebsocketAPIType_FuturesKline:
		{
			time_t curTime = CFuncCommon::ISO8601ToTime(retObj["data"][0]["candle"][0].asString());
			if(g_pDlg->m_bFirstKLine)
			{
				g_pDlg->m_bFirstKLine = false;
				g_pDlg->ComplementedKLine(curTime, 90);
			}
			CActionLog("all_kline", "%s", strRet.c_str());
			if(curTime >= lastKlineTime)
			{
				if(curTime > lastKlineTime && lastKlineTime != 0)
				{
					CActionLog("market", "%s", lastKlineRetStr.c_str());
					SKlineData data;
					data.time = CFuncCommon::ISO8601ToTime(lastKlineJson["data"][0]["candle"][0].asString());
					data.openPrice = stod(lastKlineJson["data"][0]["candle"][1].asString());
					data.highPrice = stod(lastKlineJson["data"][0]["candle"][2].asString());
					data.lowPrice = stod(lastKlineJson["data"][0]["candle"][3].asString());
					data.closePrice = stod(lastKlineJson["data"][0]["candle"][4].asString());
					data.volume = stoi(lastKlineJson["data"][0]["candle"][5].asString());
					data.volumeByCurrency = stod(lastKlineJson["data"][0]["candle"][6].asString());
					g_pDlg->AddKlineData(data);
				}
				lastKlineTime = curTime;
				lastKlineRetStr = strRet;
				lastKlineJson = retObj;
			}
		}
		break;
	case eWebsocketAPIType_FuturesTicker:
		{
			CActionLog("market", "%s", strRet.c_str());
			if(retObj.isObject() && retObj["data"].isArray())
			{
				for(int i = 0; i < (int)retObj["data"].size(); ++i)
				{
					STickerData data;
					data.volume = stoi(retObj["data"][i]["volume_24h"].asString());
					data.sell = stod(retObj["data"][i]["best_ask"].asString());
					data.buy = stod(retObj["data"][i]["best_bid"].asString());
					data.high = stod(retObj["data"][i]["high_24h"].asString());
					data.low = stod(retObj["data"][i]["low_24h"].asString());
					data.last = stod(retObj["data"][i]["last"].asString());
					data.time = CFuncCommon::ISO8601ToTime(retObj["data"][i]["timestamp"].asString());
					g_pDlg->OnRevTickerInfo(data);
				}
			}
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
	case eWebsocketAPIType_FuturesOrderInfo:
		{
			if(retObj.isObject() && retObj["data"].isArray())
			{
				for(int i = 0; i<(int)retObj["data"].size(); ++i)
				{
					SFuturesTradeInfo info;
					info.strClientOrderID = retObj["data"][i]["client_oid"].asString();
					info.timeStamp = CFuncCommon::ISO8601ToTime(retObj["data"][i]["timestamp"].asString());
					info.filledQTY = retObj["data"][i]["filled_qty"].asString();
					info.orderID = retObj["data"][i]["order_id"].asString();
					info.price = stod(retObj["data"][i]["price"].asString());
					info.priceAvg = stod(retObj["data"][i]["price_avg"].asString());
					info.state = retObj["data"][i]["status"].asString();
					info.size = retObj["data"][i]["size"].asString();
					std::string tradeType = retObj["data"][i]["type"].asString();
					if(tradeType == "1")
						info.tradeType = eFuturesTradeType_OpenBull;
					else if(tradeType == "2")
						info.tradeType = eFuturesTradeType_OpenBear;
					else if(tradeType == "3")
						info.tradeType = eFuturesTradeType_CloseBull;
					else if(tradeType == "4")
						info.tradeType = eFuturesTradeType_CloseBear;
					g_pDlg->UpdateTradeInfo(info);
					CActionLog("trade", "ws更新订单信息 order=%s, filledQTY=%s, price=%s, priceAvg=%s, state=%s, tradeType=%s", info.orderID.c_str(), info.filledQTY.c_str(), retObj["data"][0]["price"].asString().c_str(), retObj["data"][0]["price_avg"].asString().c_str(), info.state.c_str(), tradeType.c_str());
				}
			}
			else
				LOCAL_ERROR("ws type=%d ret=%s", apiType, strRet.c_str());
		}
		break;
	case eWebsocketAPIType_FuturesPositionInfo:
		{
			if(retObj.isObject() && retObj["data"].isArray())
			{
				SFuturesPositionInfo info;
				if(retObj["table"].asString() == "swap/position")
				{
					for(int i = 0; i < (int)retObj["data"][0]["holding"].size(); ++i)
					{
						if(retObj["data"][0]["holding"][i]["side"].asString() == "short")
						{
							info.bearCount = retObj["data"][0]["holding"][i]["position"].asString();
							if(info.bearCount != "0")
							{
								info.bearFreeCount = retObj["data"][0]["holding"][i]["avail_position"].asString();
								double priceAvg = stod(retObj["data"][0]["holding"][i]["avg_cost"].asString());
								info.bearPriceAvg = CFuncCommon::Double2String(priceAvg, g_pDlg->m_nPriceDecimal);
								info.bearMargin = retObj["data"][0]["holding"][i]["margin"].asString();
								info.broken = retObj["data"][0]["holding"][i]["liquidation_price"].asString();
							}
						}
						else if(retObj["data"][0]["holding"][i]["side"].asString() == "long")
						{
							info.bullCount = retObj["data"][0]["holding"][i]["position"].asString();
							if(info.bullCount != "0")
							{
								info.bullFreeCount = retObj["data"][0]["holding"][i]["avail_position"].asString();
								double priceAvg = stod(retObj["data"][0]["holding"][i]["avg_cost"].asString());
								info.bullPriceAvg = CFuncCommon::Double2String(priceAvg, g_pDlg->m_nPriceDecimal);
								info.bullMargin = retObj["data"][0]["holding"][i]["margin"].asString();
								info.broken = retObj["data"][0]["holding"][i]["liquidation_price"].asString();
							}
						}
					}
				}
				else
				{
					info.bullCount = retObj["data"][0]["long_qty"].asString();
					if(info.bullCount != "0")
					{
						info.bullFreeCount = retObj["data"][0]["long_avail_qty"].asString();
						double priceAvg = stod(retObj["data"][0]["long_avg_cost"].asString());
						info.bullPriceAvg = CFuncCommon::Double2String(priceAvg, g_pDlg->m_nPriceDecimal);
						info.bullMargin = retObj["data"][0]["long_margin"].asString();
					}
					info.bearCount = retObj["data"][0]["short_qty"].asString();
					if(info.bearCount != "0")
					{
						info.bearFreeCount = retObj["data"][0]["short_avail_qty"].asString();
						double priceAvg = stod(retObj["data"][0]["short_avg_cost"].asString());
						info.bullPriceAvg = CFuncCommon::Double2String(priceAvg, g_pDlg->m_nPriceDecimal);
						info.bearPriceAvg = retObj["data"][0]["short_avg_cost"].asString();
						info.bearMargin = retObj["data"][0]["short_margin"].asString();
					}
					info.broken = retObj["data"][0]["liquidation_price"].asString();
				}
				g_pDlg->UpdatePositionInfo(info);
			}
		}
		break;
	default:
		break;
	}
}