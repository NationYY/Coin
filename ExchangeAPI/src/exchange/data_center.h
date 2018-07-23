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
	void UpdateBuyEntrustDepth(std::string strPrice, std::string strVolume);
	void DelBuyEntrustDepth(std::string strPrice);
	void UpdateSellEntrustDepth(std::string strPrice, std::string strVolume);
	void DelSellEntrustDepth(std::string strPrice);
public:
	std::map<std::string, SBalanceInfo> m_mapBalanceInfo;
	std::map<std::string, std::string> m_mapBuyEntrustDepth;
	std::map<std::string, std::string> m_mapSellEntrustDepth;
	double m_buyPrice;
	double m_sellPrice;
};
