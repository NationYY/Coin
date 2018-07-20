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
public:
	std::map<std::string, SBalanceInfo> m_mapBalanceInfo;
	double m_buyPrice;
	double m_sellPrice;
};
