#include "stdafx.h"
#include "data_center.h"
#include <time.h>

CDataCenter::CDataCenter() : m_updateEntrustDepthTime(0), m_orderCheckIndex(1), m_bJustUpdateFinishOrder(false),
m_latestExecutedOrderPrice(0)
{
}


CDataCenter::~CDataCenter()
{
}

void CDataCenter::SetBalance(const char* szCoinName, double total, double freeze, double balance)
{
	SBalanceInfo info;
	info.name = szCoinName;
	info.total = total;
	info.freeze = freeze;
	info.balance = balance;
	m_mapBalanceInfo[info.name] = info;
}

void CDataCenter::SetBuyAndSellPrice(double buyPrice, double sellPrice)
{
	m_buyPrice = buyPrice;
	m_sellPrice = sellPrice;
}

void CDataCenter::UpdateBuyEntrustDepth(std::string& strPrice, std::string& strVolume, int serverTime)
{
	m_mapBuyEntrustDepth[strPrice] = strVolume;
	m_updateEntrustDepthTime = serverTime;
}

void CDataCenter::DelBuyEntrustDepth(std::string& strPrice, int serverTime)
{
	m_mapBuyEntrustDepth.erase(strPrice);
	m_updateEntrustDepthTime = serverTime;
}

void CDataCenter::UpdateSellEntrustDepth(std::string& strPrice, std::string& strVolume, int serverTime)
{
	m_mapSellEntrustDepth[strPrice] = strVolume;
	m_updateEntrustDepthTime = serverTime;
}

void CDataCenter::DelSellEntrustDepth(std::string& strPrice, int serverTime)
{
	m_mapSellEntrustDepth.erase(strPrice);
	m_updateEntrustDepthTime = serverTime;
}

void CDataCenter::AddTradeOrders(std::string& orderID)
{
	SOrderInfo info;
	info.addTime = time(NULL);
	m_mapTradeOrderID[orderID] = info;
}

void CDataCenter::DelTradeOrders(std::string& orderID)
{
	m_mapTradeOrderID.erase(orderID);
}

void CDataCenter::DeleteTradeOrder(std::string& orderID)
{
	m_mapTradeOrderID.erase(orderID);
}

void CDataCenter::FinishTradeOrder(std::string& orderID, double price, double amount, __int64 date, std::string& type)
{
	if(m_mapTradeOrderID.find(orderID) != m_mapTradeOrderID.end())
	{
		m_mapTradeOrderID.erase(orderID);
		SFinishOrderInfo info;
		info.id = orderID;
		info.price = price;
		info.amount = amount;
		info.time = date/1000;
		info.type = type;
		m_listAllFinishOrder.push_back(info);
		m_bJustUpdateFinishOrder = true;
	}
}

void CDataCenter::UpdateTradeOrder(std::string& orderID, __int64 date)
{
	std::map<std::string, SOrderInfo>::iterator it = m_mapTradeOrderID.find(orderID);
	if(it != m_mapTradeOrderID.end())
		it->second.serverCreatDate = date;
}