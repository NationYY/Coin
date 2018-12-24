#pragma once
#include <boost/thread/condition.hpp>
#include <boost/thread/thread.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <curl/curl.h>
#include "http_def.h"
#include <deque>
class CExchange;
typedef boost::function<void(eHttpAPIType, Json::Value&, const std::string&, int, std::string)> HTTP_CALLBACK_FUNCTION_TYPE;
class CHttpAPI
{
public:
	CHttpAPI();
	virtual ~CHttpAPI();
	void SetKey(std::string strAPIKey, std::string strSecretKey, std::string strPassphrase = "");
	void SetURL(std::string strURL);
	void SetExchange(CExchange* pExchange){
		m_pExchange = pExchange;
	}
	void SetContentType(std::string strContentType);
	void Run(int threadNums);
	void Close();
	void Update();
	void SetCallBakFunction(HTTP_CALLBACK_FUNCTION_TYPE func){
		m_callBackFunc = func;
	}
	void RequestAsync(SHttpReqInfo& info);
	void Request(SHttpReqInfo& reqInfo, SHttpResponse& resInfo);
	virtual void API_Balance() = 0;
	virtual void API_Ticker(eMarketType type) = 0;
	virtual void API_EntrustDepth(eMarketType type) = 0;
	virtual void API_Trade(eMarketType type, std::string strAmount, std::string strPrice, bool bBuy, int customData, std::string strCustomData) = 0;
	virtual void API_GetTradeOrderListState(eMarketType type, int page, bool bBuy) = 0;
	virtual void API_GetTradeOrderState(eMarketType type, std::string strID) = 0;
	virtual void API_CancelTrade(eMarketType type, std::string strID, std::string strCustomData) = 0;
private:
	void _ProcessHttp();
	void _Request(CURL* pCurl, SHttpReqInfo& reqInfo, SHttpResponse& resInfo);
	void _GetReq(CURL* pCurl, std::string& _strURL, const char* szMethod, const char* szGetParams, std::string& strResponse);
	void _PostReq(CURL* pCurl, std::string& _strURL, const char* szMethod, const char* szPostParams, std::string& strResponse);
protected:
	virtual void OnResponse(eHttpAPIType type, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData);
private:
	HTTP_CALLBACK_FUNCTION_TYPE m_callBackFunc;
	boost::thread_group m_workers;
	std::deque<SHttpReqInfo> m_queueReqInfo;
	boost::mutex m_reqMutex;
	std::deque<SHttpResponse> m_queueResponseInfo;
	boost::mutex m_responseMutex;
	boost::condition_variable m_condReqInfo;
	std::string m_strURL;
	std::string m_strContentType;
	int m_threadNum;
	CURL* m_pMainCurl;
	bool m_bClose;
	int m_nCloseThreadCnt;
protected:
	std::string m_strAPIKey;		//用户申请的apiKey
	std::string m_strSecretKey;		//请求参数签名的私钥
	std::string m_strPassphrase;
	CExchange* m_pExchange;
	
};

