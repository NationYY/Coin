#pragma once
enum eHttpAPIType
{
	eHttpAPIType_MarketList,			//���׶��б�
	eHttpAPIType_Balance,				//���
	eHttpAPIType_Ticker,				//��ȡ���׶�����
	eHttpAPIType_EntrustDepth,			//�������
	eHttpAPIType_Trade,					//�ҵ�
	eHttpAPIType_TradeOrderListState,	//�ҵ��б�״̬
	eHttpAPIType_TradeOrderState,		//�ҵ�״̬
	eHttpAPIType_CancelTrade,			//����
	eHttpAPIType_Max,
};

enum eWebsocketAPIType
{
	eWebsocketAPIType_EntrustDepth,	//�������
	eWebsocketAPIType_LatestExecutedOrder,//���³ɽ��Ķ���
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