#pragma once
enum eHttpAPIType
{
	eHttpAPIType_MarketList,	//交易对列表
	eHttpAPIType_Balance,		//余额
	eHttpAPIType_Ticker,		//获取交易对行情
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