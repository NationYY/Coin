#pragma once
#include "mfc_window/okex_martingale_dlg.h"
extern COKExMartingaleDlg* g_pDlg;
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