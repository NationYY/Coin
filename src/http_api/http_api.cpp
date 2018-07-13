#include "stdafx.h"
#include "http_api.h"


CHttpAPI::CHttpAPI()
{
}


CHttpAPI::~CHttpAPI()
{
}

void CHttpAPI::SetKey(string strAPIKey, string strSecretKey)
{
	m_strAPIKey = strAPIKey;
	m_strSecretKey = strSecretKey;
}

void SetURL(string strURL)
{
	m_strURL = strURL;
}

void CHttpAPI::Run(int threadNums)
{
	m_threadNum = threadNums;
	curl_global_init(CURL_GLOBAL_ALL);
	for(int i = 0; i < threadNums; ++i)
	{
		m_workers.create_thread(boost::bind(&CHttpAPI::_ProcessHttp, this));
	}
}

void CHttpAPI::_ProcessHttp()
{
	CURL* pCurl = curl_easy_init();
	curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, false); // if want to use https  
	curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYHOST, false); // set peer and host verify false  
	curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1);
	curl_easy_setopt(pCurl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(pCurl, CURLOPT_CONNECTTIMEOUT, 3);
	curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 3);
	while(!m_queueReqInfo.empty())
	{
		SHttpReqInfo info;
		{
			boost::mutex::scoped_lock sl(m_reqMutex);
			if(m_queueReqInfo.empty())
			{
				m_condReqInfo.wait(sl);
				continue;
			}
			info = m_queueReqInfo.front();
			m_queueReqInfo.pop_front();
		}
		std::string strResponse = "";
		if(info.type == eHttpReqType_Get)
		{
			
			_GetReq(pCurl, info.strMethod.c_str(), strResponse);
		}
		else if(info.type == eHttpReqType_Post)
		{
			_PostReq(pCurl, info.strMethod.c_str(), info.strPostParams.c_str(), strResponse);
		}
		if(strResponse != "")
		{
			boost::mutex::scoped_lock sl(m_responseMutex);
			SHttpResponse info;
			info.strResponse = strResponse;
			m_queueResponseInfo.push_back(info);
		}
	}
	curl_easy_cleanup(pCurl);
}
void CHttpAPI::_GetReq(CURL* pCurl, const char* szMethod, std::string& strResponse)
{
	std::string strURL = m_strURL;
	strURL.append("/").append(szMethod);
	curl_easy_setopt(pCurl, CURLOPT_URL, m_strURL.c_str());
	curl_easy_setopt(pCurl, CURLOPT_POST, 0); // post req 
	curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, (void*)&strResponse);
	CURLcode res = curl_easy_perform(pCurl);
}

void CHttpAPI::_PostReq(CURL* pCurl, const char* szMethod, const char* szPostParams, std::string& strResponse)
{
	std::string strURL = m_strURL;
	strURL.append("/").append(szMethod);
	curl_easy_setopt(pCurl, CURLOPT_URL, m_strURL.c_str());
	curl_easy_setopt(pCurl, CURLOPT_POST, 1); // post req
	curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, szPostParams); // params  
	curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, (void*)&strResponse);
	CURLcode res = curl_easy_perform(pCurl);
}

void CHttpAPI::Update()
{
	if(m_queueResponseInfo.size())
}