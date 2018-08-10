#pragma once
#include "common/api_common.h"
class CDataCenter
{
public:
	CDataCenter();
	~CDataCenter();
	void ClearAllBalance(){
		m_mapBalanceInfo.clear();
	}
	void SetBalance(const char* szCoinName, double total, double freeze, double balance);
	void SetBuyAndSellPrice(double buyPrice, double sellPrice);
	void UpdateBuyEntrustDepth(std::string& strPrice, std::string& strVolume, int serverTime, std::string marketName = "");
	void DelBuyEntrustDepth(std::string& strPrice, int serverTime, std::string marketName = "");
	void UpdateSellEntrustDepth(std::string& strPrice, std::string& strVolume, int serverTime, std::string marketName = "");
	void DelSellEntrustDepth(std::string& strPrice, int serverTime, std::string marketName = "");
	void AddTradeOrders(std::string& orderID);
	void DelTradeOrders(std::string& orderID);
	void DeleteTradeOrder(std::string& orderID);
	void UpdateTradeOrder(std::string& orderID, __int64 date);
	void FinishTradeOrder(std::string& orderID, double price, double amount, __int64 date, std::string& type);
	void SetLatestExecutedOrderPrice(double price, std::string marketName = ""){
		m_mapLatestExecutedOrderPrice[marketName] = price;
	}
	void ClearAllEntrustDepth(std::string marketName = ""){
		m_mapEntrustDepth[marketName].mapBuyEntrustDepth.clear();
		m_mapEntrustDepth[marketName].mapSellEntrustDepth.clear();
	}
public:
	struct SEntrustDepthInfo
	{
		std::map<std::string, std::string> mapBuyEntrustDepth;
		std::map<std::string, std::string> mapSellEntrustDepth;
	};
	std::map<std::string, SBalanceInfo> m_mapBalanceInfo;
	std::map<std::string, SEntrustDepthInfo> m_mapEntrustDepth;
	struct SOrderInfo
	{
		time_t addTime;
		time_t lastCancelTime;
		int checkIndex;
		__int64 serverCreatDate;
		SOrderInfo()
		{
			memset(this, 0, sizeof(SOrderInfo));
		}
	};
	struct SFinishOrderInfo
	{
		std::string id;
		std::string type;
		double price;
		double amount;
		time_t time;
	};
	std::map<std::string, SOrderInfo> m_mapTradeOrderID;
	std::list<SFinishOrderInfo> m_listAllFinishOrder;
	bool m_bJustUpdateFinishOrder;
	double m_buyPrice;
	double m_sellPrice;
	time_t m_updateEntrustDepthTime;
	int m_orderCheckIndex;
	std::map<std::string, double> m_mapLatestExecutedOrderPrice;
};
