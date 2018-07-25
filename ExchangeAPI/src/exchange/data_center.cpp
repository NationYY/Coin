#include "stdafx.h"
#include "data_center.h"


CDataCenter::CDataCenter() : m_updateEntrustDepthTime(0)
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

void CDataCenter::UpdateBuyEntrustDepth(std::string strPrice, std::string strVolume, int serverTime)
{
	m_mapBuyEntrustDepth[strPrice] = strVolume;
	m_updateEntrustDepthTime = serverTime;
}

void CDataCenter::DelBuyEntrustDepth(std::string strPrice, int serverTime)
{
	m_mapBuyEntrustDepth.erase(strPrice);
	m_updateEntrustDepthTime = serverTime;
}

void CDataCenter::UpdateSellEntrustDepth(std::string strPrice, std::string strVolume, int serverTime)
{
	m_mapSellEntrustDepth[strPrice] = strVolume;
	m_updateEntrustDepthTime = serverTime;
}

void CDataCenter::DelSellEntrustDepth(std::string strPrice, int serverTime)
{
	m_mapSellEntrustDepth.erase(strPrice);
	m_updateEntrustDepthTime = serverTime;
}