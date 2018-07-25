#pragma once
enum eHttpAPIType
{
	eHttpAPIType_MarketList,	//���׶��б�
	eHttpAPIType_Balance,		//���
	eHttpAPIType_Ticker,		//��ȡ���׶�����
	eHttpAPIType_Max,
};

enum eWebsocketAPIType
{
	eWebsocketAPIType_EntrustDepth,	//�������
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