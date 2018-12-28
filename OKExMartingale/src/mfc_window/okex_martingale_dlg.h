
// OKExMartingaleDlg.h : 头文件
//

#pragma once
#include <clib/lib/util/config.h>
#include "afxwin.h"
enum eTimerType
{
	eTimerType_APIUpdate,
	eTimerType_EntrustDepth,
	eTimerType_Ping,
};

enum eBollTrend
{
	eBollTrend_Normal,			//空转
	eBollTrend_ShouKou,			//收口
	eBollTrend_ShouKouChannel,	//收口通道
	eBollTrend_ZhangKou,		//张口
};

struct SKlineData
{
	__int64 time;		//时间
	char szTime[20];
	double openPrice;	//开盘价
	double highPrice;	//最高价
	double lowPrice;	//最低价
	double closePrice;	//收盘价
	int volume;			//成交量(张)
	double volumeByCurrency;//成交量(币)
	SKlineData(){
		memset(this, 0, sizeof(SKlineData));
	}
};

struct STickerData
{
	bool bValid;
	__int64 time;		//时间
	int volume;			//成交量(张)
	double sell;		//卖一价格
	double buy;			//买一价格
	double high;		//24小时最高价格
	double low;			//24小时最低价格
	double last;		//最新成交价格
	STickerData(){
		memset(this, 0, sizeof(STickerData));
	}
};

struct SBollInfo
{
	__int64 time;
	char szTime[20];
	double mb;
	double up;
	double dn;
	SBollInfo(){
		memset(this, 0, sizeof(SBollInfo));
	}
	void Reset(){
		memset(this, 0, sizeof(SBollInfo));
	}
};

struct SFuturesAccountInfo
{
	double equity;	//账户权益
	double margin;	//保证金
	SFuturesAccountInfo(){
		memset(this, 0, sizeof(SFuturesAccountInfo));
	}
};


// COKExMartingaleDlg 对话框
class COKExMartingaleDlg : public CDialogEx
{
// 构造
public:
	COKExMartingaleDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_OKEXMARTINGALE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	void AddKlineData(SKlineData& data);
	void OnRevTickerInfo(STickerData &data);
	void Pong();
	void OnLoginSuccess();
	void SetHScroll();
private:
	void OnBollUpdate();
	void CheckBollTrend();
	void __CheckTrend_Normal();
	void __CheckTrend_ZhangKou();
	void __CheckTrend_ShouKou();
	void __CheckTrend_ShouKouChannel();
	void __SetBollState(eBollTrend state, int nParam = 0, double dParam = 0.0);

public:
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonTest();
	CListBox m_ctrlListLog;
private:
	clib::config m_config;
	std::string m_apiKey;
	std::string m_secretKey;
	std::string m_passphrase;
	std::set<std::string> m_setAllTestFile;
private:
	std::vector<SKlineData> m_vecKlineData;
	std::vector<SBollInfo> m_vecBollData;
	eBollTrend m_eBollState;
	eBollTrend m_eLastBollState;
	int m_nZhangKouConfirmBar;
	double m_nZhangKouMinValue;
	int m_nShouKouConfirmBar;
	int m_nShouKouChannelConfirmBar;
	bool m_bZhangKouUp;
	int m_nZhangKouTradeCheckBar;
	bool m_bTest;
	STickerData m_curTickData;
public:
	bool m_bRun;
	time_t m_tListenPong;
public:
	int m_nBollCycle;				//布林线周期
	int m_nPriceDecimal;			//价格小数点精度
	int m_nZhangKouCheckCycle;		//布林张口确认周期
	int m_nZhangKouTrendCheckCycle;	//布林张口趋势确认周期 必须是奇数
	int m_nShouKouCheckCycle;		//布林收口确认周期
	int m_nZhangKouDoubleConfirmCycle;	//布林张口二次确认周期
	int m_nShoukouDoubleConfirmCycle;	//布林收口二次确认周期
	std::string m_strKlineCycle;		//布林线周期
	std::string m_strCoinType;			//货币类型
	std::string m_strFuturesCycle;		//合约周期
	std::string m_strFuturesTradeSize;	//下单张数
	int m_nKlineCycle;					//布林线周期对应秒数
	std::string m_strLeverage;					//合约倍数
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
