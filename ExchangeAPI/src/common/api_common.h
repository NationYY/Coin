#pragma once
enum eHttpAPIType
{
	eHttpAPIType_MarketList,	//���׶��б�
	eHttpAPIType_Balance,		//���
	eHttpAPIType_Ticker,		//��ȡ���׶�����
	eHttpAPIType_Max,
};

enum eMarketType
{
	
};

struct SBalanceInfo
{
	std::string name;
	double total;
	double freeze;
	double balance;
};