
// OKExFuturesDlg.h : 头文件
//

#pragma once

#include <clib/lib/util/config.h>
enum eTimerType
{
	eTimerType_APIUpdate,
	eTimerType_EntrustDepth,
	eTimerType_Ping,
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
	int m_nBollCycle;
	int m_nPriceDecimal;
public:
	afx_msg void OnBnClickedButtonStart();
public:
	void AddKlineData(SKlineData& data);
private:
	void Test();
	void OnBollUpdate();
private:
	std::vector<SKlineData> m_vecKlineData;
	std::vector<SBollInfo> m_vecBollData;
};
