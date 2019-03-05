#pragma once
enum eHttpAPIType
{
	eHttpAPIType_MarketList,					//交易对列表
	eHttpAPIType_Balance,						//余额
	eHttpAPIType_Ticker,						//获取交易对行情
	eHttpAPIType_EntrustDepth,					//交易深度
	eHttpAPIType_Trade,							//挂单
	eHttpAPIType_TradeOrderListState,			//挂单列表状态
	eHttpAPIType_TradeOrderState,				//挂单状态
	eHttpAPIType_CancelTrade,					//撤单
	eHttpAPIType_FuturesAccountInfoByCurrency,	//单一币种合约账户信息
	eHttpAPIType_FuturesTrade,					//合约下单
	eHttpAPIType_FuturesServerTime,				//合约服务器时间
	eHttpAPIType_FuturesTradeInfo,				//查询合约单条订单信息
	eHttpAPIType_FuturesCancelOrder,			//合约撤单
	eHttpAPIType_FuturesInstruments,			//获取合约币对信息
	eHttpAPIType_SetFuturesLeverage,			//设置合约杠杆
	eHttpAPIType_GetFuturesSomeKline,			//获取部分K线
	eHttpAPIType_SpotTrade,						//现货下单
	eHttpAPIType_SpotTradeInfo,					//查询现货单条订单信息
	eHttpAPIType_SpotAccountInfoByCurrency,		//单一币种现货账户信息
	eHttpAPIType_SpotCancelOrder,				//现货撤单
	eHttpAPIType_SpotInstruments,				//获取现货币对信息
	eHttpAPIType_FuturesPositionInfo,			//单一币种持仓信息
	eHttpAPIType_Max,
};

enum eWebsocketAPIType
{
	eWebsocketAPIType_Pong,			//心跳
	eWebsocketAPIType_EntrustDepth,	//交易深度
	eWebsocketAPIType_LatestExecutedOrder,//最新成交的订单
	eWebsocketAPIType_FuturesKline,	//合约K线
	eWebsocketAPIType_FuturesTicker,//合约行情
	eWebsocketAPIType_Login,		//登录
	eWebsocketAPIType_FuturesOrderInfo,//合约个人交易信息
	eWebsocketAPIType_FuturesAccountInfo,//合约个人账户信息
	eWebsocketAPIType_SpotKline,	//现货K线
	eWebsocketAPIType_SpotTicker,	//现货行情
	eWebsocketAPIType_SpotOrderInfo,	//现货个人交易信息
	eWebsocketAPIType_SpotAccountInfo,	//现货个人账户信息
	eWebsocketAPIType_FuturesEntrustDepth,//期货深度
	eWebsocketAPIType_FuturesPositionInfo,//合约个人持仓信息
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
	eMarketType_BWB_QC,
	eMarketType_USDT_QC,
	eMarketType_Max,
};

enum eTradeType
{
	eTradeType_buy,
	eTradeType_sell,
};

enum eKlineType
{
	eKlineType_1min,
	eKlineType_3min,
	eKlineType_5min,
	eKlineType_15min,
	eKlineType_30min,
	eKlineType_1hour,
	eKlineType_2hour,
	eKlineType_4hour,
	eKlineType_6hour,
	eKlineType_12hour,
	eKlineType_day,
	eKlineType_3day,
	eKlineType_week
};

enum eCoinType
{
	eCoinType_btc,
	eCoinType_ltc,
	eCoinType_eth,
	eCoinType_etc,
	eCoinType_bch,
	eCoinType_eos,
	eCoinType_xrp,
	eCoinType_btg
};

enum eFuturesCycle
{
	eFuturesCycle_this_week,
	eFuturesCycle_next_week,
	eFuturesCycle_quarter
};

enum eFuturesTradeType
{
	eFuturesTradeType_OpenBull,	//开多
	eFuturesTradeType_OpenBear,	//开空
	eFuturesTradeType_CloseBull,//平多
	eFuturesTradeType_CloseBear,//平空
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