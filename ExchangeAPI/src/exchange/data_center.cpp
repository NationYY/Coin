#include "stdafx.h"
#include "data_center.h"


CDataCenter::CDataCenter()
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

void CDataCenter::UpdateBuyEntrustDepth(std::string strPrice, std::string strVolume)
{
	m_mapBuyEntrustDepth[strPrice] = strVolume;
}

void CDataCenter::DelBuyEntrustDepth(std::string strPrice)
{
	m_mapBuyEntrustDepth.erase(strPrice);
}

void CDataCenter::UpdateSellEntrustDepth(std::string strPrice, std::string strVolume)
{
	m_mapSellEntrustDepth[strPrice] = strVolume;
}

void CDataCenter::DelSellEntrustDepth(std::string strPrice)
{
	m_mapSellEntrustDepth.erase(strPrice);
}