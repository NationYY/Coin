#pragma once
#include <boost/thread/condition.hpp>
#include <boost/thread/thread.hpp>
#include <curl/curl.h>
#include "http_def.h"
class CHttpAPI
{
	
public:
	CHttpAPI();
	~CHttpAPI();
	void SetKey(string strAPIKey, string strSecretKey);
	void SetURL(string strURL);
	void Run(int threadNums);
	void Update();
private:
	void _ProcessHttp();
	void _GetReq(CURL* pCurl, const char* szMethod, std::string& strResponse);
	void _PostReq(CURL* pCurl, const char* szMethod, const char* szPostParams, std::string& strResponse);
private:
	boost::thread_group m_workers;
	std::deque<SHttpReqInfo> m_queueReqInfo;
	boost::mutex m_reqMutex;
	std::deque<SHttpResponse> m_queueResponseInfo;
	boost::mutex m_responseMutex;
	boost::condition_variable m_condReqInfo;
	string m_strAPIKey;			//用户申请的apiKey
	string m_strSecretKey;		//请求参数签名的私钥
	string m_strURL;
	int m_threadNum;
};

