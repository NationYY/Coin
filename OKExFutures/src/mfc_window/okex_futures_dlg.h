
// okex_futures_dlg.h : 头文件
//

#pragma once

#include <clib/lib/util/config.h>
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

struct SBollInfo
{
	__int64 time;
	double mb;
	double up;
	double dn;
	SBollInfo(){
		memset(this, 0, sizeof(SBollInfo));
	}
};
// COKExFuturesDlg 对话框
class COKExFuturesDlg : public CDialogEx
{
// 构造
public:
	COKExFuturesDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_OKEXFUTURES_DIALOG };

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
	afx_msg void OnTimer(UINT_PTR nIDEvent);
private:
	clib::config m_config;
	std::string m_accessKey;
	std::string m_secretKey;
public:
	afx_msg void OnBnClickedButtonStart();
public:
	void AddKlineData(SKlineData& data);
	void Pong();
private:
	void Test();
	void OnBollUpdate();
	void CheckBollTrend();
	void __CheckTrend_Normal();
	void __CheckTrend_ZhangKou();
	void __CheckTrend_ShouKou();
	void __CheckTrend_ShouKouChannel();
	void __SetBollState(eBollTrend state, int nParam=0, double dParam=0.0);
public:
	bool m_bRun;
	time_t m_tListenPong;
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
private:
	int m_nBollCycle;				//布林线周期
	int m_nPriceDecimal;			//价格小数点精度
	int m_nZhangKouCheckCycle;		//布林张口确认周期
	int m_nZhangKouTrendCheckCycle;	//布林张口趋势确认周期 必须是奇数
	int m_nShouKouCheckCycle;		//布林收口确认周期
	int m_nZhangKouDoubleConfirmCycle;	//布林张口二次确认周期
	int m_nShoukouDoubleConfirmCycle;	//布林收口二次确认周期
public:
	afx_msg void OnBnClickedButtonTest();
	afx_msg void OnDestroy();
};
