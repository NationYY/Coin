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