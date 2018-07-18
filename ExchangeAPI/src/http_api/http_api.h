#pragma once
#include <boost/thread/condition.hpp>
#include <boost/thread/thread.hpp>
#include <curl/curl.h>
#include "http_def.h"
#include <deque>
class CHttpAPI
{
	
public:
	CHttpAPI();
	~CHttpAPI();
	void SetKey(string strAPIKey, string strSecretKey);
	void SetURL(string strURL);
	void SetContentType(string strContentType);
	void Run(int threadNums);
	void Update();
	void SetCallBackMessage(http_callbak_message callbakMessage);
	void PushReqInfo(SHttpReqInfo& info);
	virtual void API_Balance(){}
private:
	void _ProcessHttp();
	void _GetReq(CURL* pCurl, const char* szMethod, const char* szGetParams, std::string& strResponse);
	void _PostReq(CURL* pCurl, const char* szMethod, const char* szPostParams, std::string& strResponse);
private:
	http_callbak_message m_callbakMessage;
	boost::thread_group m_workers;
	std::deque<SHttpReqInfo> m_queueReqInfo;
	boost::mutex m_reqMutex;
	std::deque<SHttpResponse> m_queueResponseInfo;
	boost::mutex m_responseMutex;
	boost::condition_variable m_condReqInfo;
	string m_strURL;
	string m_strContentType;
	int m_threadNum;
protected:
	string m_strAPIKey;			//用户申请的apiKey
	string m_strSecretKey;		//请求参数签名的私钥
	
};

