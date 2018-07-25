#pragma once
enum eHttpAPIType
{
	eHttpAPIType_MarketList,	//交易对列表
	eHttpAPIType_Balance,		//余额
	eHttpAPIType_Ticker,		//获取交易对行情
	eHttpAPIType_Max,
};

enum eWebsocketAPIType
{
	eWebsocketAPIType_EntrustDepth,	//交易深度
	eWebsocketAPIType_Max,
};

enum eMarketType
{
	eMarketType_ETH_BTC,
	eMarketType_ETH_USDT,
	eMarketType_BTC_USDT,
	eMarketType_Max,
};

struct SBalanceInfo
{
	std::string name;
	double total;
	double freeze;
	double balance;
};