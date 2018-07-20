#pragma once
#include <boost/thread/condition.hpp>
#include <boost/thread/thread.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <curl/curl.h>
#include "http_def.h"
#include <deque>
typedef boost::function<void(eHttpAPIType, Json::Value&, const std::string&)> HTTP_CALLBACK_FUNCTION_TYPE;
class CHttpAPI
{
	
public:
	CHttpAPI();
	~CHttpAPI();
	void SetKey(std::string strAPIKey, std::string strSecretKey);
	void SetURL(std::string strURL);
	void SetContentType(std::string strContentType);
	void Run(int threadNums);
	void Update();
	void SetCallBakFunction(HTTP_CALLBACK_FUNCTION_TYPE func){
		m_callBackFunc = func;
	}
	void PushReqInfo(SHttpReqInfo& info);
	virtual void API_Balance(){}
	virtual void API_Ticker(const char* szType) {}
private:
	void _ProcessHttp();
	void _GetReq(CURL* pCurl, std::string& _strURL, const char* szMethod, const char* szGetParams, std::string& strResponse);
	void _PostReq(CURL* pCurl, std::string& _strURL, const char* szMethod, const char* szPostParams, std::string& strResponse);
protected:
	virtual void OnResponse(eHttpAPIType type, Json::Value& retObj, const std::string& strRet);
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
protected:
	std::string m_strAPIKey;			//用户申请的apiKey
	std::string m_strSecretKey;		//请求参数签名的私钥
	
};

