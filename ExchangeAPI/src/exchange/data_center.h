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
	void UpdateBuyEntrustDepth(std::string strPrice, std::string strVolume, int serverTime);
	void DelBuyEntrustDepth(std::string strPrice, int serverTime);
	void UpdateSellEntrustDepth(std::string strPrice, std::string strVolume, int serverTime);
	void DelSellEntrustDepth(std::string strPrice, int serverTime);
	void AddTradeOrders(std::string orderID);
	void DeleteTradeOrder(std::string orderID);
	void UpdateTradeOrder(std::string orderID, __int64 date);
	void FinishTradeOrder(std::string orderID, double price, double amount, __int64 date, std::string type);
public:
	std::map<std::string, SBalanceInfo> m_mapBalanceInfo;
	std::map<std::string, std::string> m_mapBuyEntrustDepth;
	std::map<std::string, std::string> m_mapSellEntrustDepth;
	struct SOrderInfo
	{
		time_t addTime;
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
};
