#pragma once
enum eHttpAPIType
{
	eHttpAPIType_MarketList,			//交易对列表
	eHttpAPIType_Balance,				//余额
	eHttpAPIType_Ticker,				//获取交易对行情
	eHttpAPIType_EntrustDepth,			//交易深度
	eHttpAPIType_Trade,					//挂单
	eHttpAPIType_TradeOrderListState,	//挂单列表状态
	eHttpAPIType_TradeOrderState,		//挂单状态
	eHttpAPIType_CancelTrade,			//撤单
	eHttpAPIType_Max,
};

enum eWebsocketAPIType
{
	eWebsocketAPIType_EntrustDepth,	//交易深度
	eWebsocketAPIType_LatestExecutedOrder,//最新成交的订单
	eWebsocketAPIType_Max,
};

enum eMarketType
{
	eMarketType_ETH_BTC,
	eMarketType_ETH_USDT,
	eMarketType_BTC_USDT,
	eMarketType_BWB_USDT,
	eMarketType_BWB_BTC,
	eMarketType_BWB_ETH,
	eMarketType_Max,
};

enum eTradeType
{
	eTradeType_buy,
	eTradeType_sell,
};
struct SBalanceInfo
{
	SBalanceInfo()
	{
		name = "";
		total = freeze = balance = 0;
	}
	std::string name;
	double total;
	double freeze;
	double balance;
};