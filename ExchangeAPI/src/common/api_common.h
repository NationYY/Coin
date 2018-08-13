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