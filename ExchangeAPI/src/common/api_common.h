#pragma once
enum eHttpAPIType
{
	eHttpAPIType_MarketList,					//���׶��б�
	eHttpAPIType_Balance,						//���
	eHttpAPIType_Ticker,						//��ȡ���׶�����
	eHttpAPIType_EntrustDepth,					//�������
	eHttpAPIType_Trade,							//�ҵ�
	eHttpAPIType_TradeOrderListState,			//�ҵ��б�״̬
	eHttpAPIType_TradeOrderState,				//�ҵ�״̬
	eHttpAPIType_CancelTrade,					//����
	eHttpAPIType_FuturesAccountInfoByCurrency,	//��һ���ֺ�Լ�˻���Ϣ
	eHttpAPIType_FuturesTrade,					//��Լ�µ�
	eHttpAPIType_FuturesServerTime,				//��Լ������ʱ��
	eHttpAPIType_FuturesTradeInfo,				//��ѯ��Լ����������Ϣ
	eHttpAPIType_FuturesCancelOrder,			//��Լ����
	eHttpAPIType_FuturesInstruments,			//��ȡ��Լ�Ҷ���Ϣ
	eHttpAPIType_SetFuturesLeverage,			//���ú�Լ�ܸ�
	eHttpAPIType_GetFuturesSomeKline,			//��ȡ����K��
	eHttpAPIType_SpotTrade,						//�ֻ��µ�
	eHttpAPIType_SpotTradeInfo,					//��ѯ�ֻ�����������Ϣ
	eHttpAPIType_SpotAccountInfoByCurrency,		//��һ�����ֻ��˻���Ϣ
	eHttpAPIType_SpotCancelOrder,				//�ֻ�����
	eHttpAPIType_SpotInstruments,				//��ȡ�ֻ��Ҷ���Ϣ
	eHttpAPIType_FuturesPositionInfo,			//��һ���ֲֳ���Ϣ
	eHttpAPIType_Max,
};

enum eWebsocketAPIType
{
	eWebsocketAPIType_Pong,			//����
	eWebsocketAPIType_EntrustDepth,	//�������
	eWebsocketAPIType_LatestExecutedOrder,//���³ɽ��Ķ���
	eWebsocketAPIType_FuturesKline,	//��ԼK��
	eWebsocketAPIType_FuturesTicker,//��Լ����
	eWebsocketAPIType_Login,		//��¼
	eWebsocketAPIType_FuturesOrderInfo,//��Լ���˽�����Ϣ
	eWebsocketAPIType_FuturesAccountInfo,//��Լ�����˻���Ϣ
	eWebsocketAPIType_SpotKline,	//�ֻ�K��
	eWebsocketAPIType_SpotTicker,	//�ֻ�����
	eWebsocketAPIType_SpotOrderInfo,	//�ֻ����˽�����Ϣ
	eWebsocketAPIType_SpotAccountInfo,	//�ֻ������˻���Ϣ
	eWebsocketAPIType_FuturesEntrustDepth,//�ڻ����
	eWebsocketAPIType_FuturesPositionInfo,//��Լ���˳ֲ���Ϣ
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
	eFuturesTradeType_OpenBull,	//����
	eFuturesTradeType_OpenBear,	//����
	eFuturesTradeType_CloseBull,//ƽ��
	eFuturesTradeType_CloseBear,//ƽ��
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