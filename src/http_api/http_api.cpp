#include "stdafx.h"
#include "http_api.h"
#include "clib/lib/clib.h"
#include "clib/lib/memory_manager/memory_allocator.h"
#include "clib/lib/math/math_ex.h"
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

void CHttpAPI::SetURL(string strURL)
{
	m_strURL = strURL;
}

void CHttpAPI::SetContentType(string strContentType)
{
	m_strContentType = strContentType;
}

void CHttpAPI::Run(int threadNums)
{
	m_threadNum = threadNums;
	//curl_global_init(CURL_GLOBAL_ALL);
	for(int i = 0; i < threadNums; ++i)
	{
		m_workers.create_thread(boost::bind(&CHttpAPI::_ProcessHttp, this));
	}
}

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	std::string * tmp = (std::string *) userp;

	(*tmp).append((char*)contents, realsize);

	return realsize;
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
	curl_slist* pHeaders = NULL;
	pHeaders = curl_slist_append(pHeaders, "User-Agent:Mozilla / 5.0£¨Windows NT 6.1; WOW64£©AppleWebKit / 537.36£¨KHTML£¬ÈçGecko£©Chrome / 39.0.2171.71 Safari / 537.36");
	if(m_strContentType != "")
	{
		char szBuffer[256];
		_snprintf(szBuffer, 256, "Content-type:%s", m_strContentType.c_str());
		pHeaders = curl_slist_append(pHeaders, szBuffer);
	}
	curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, pHeaders);
	while(true)
	{
		SHttpReqInfo reqInfo;
		{
			boost::mutex::scoped_lock sl(m_reqMutex);
			if(m_queueReqInfo.empty())
			{
				m_condReqInfo.wait(sl);
				continue;
			}
			reqInfo = m_queueReqInfo.front();
			m_queueReqInfo.pop_front();
		}
		switch(reqInfo.confirmationType)
		{
		case eHttpConfirmationType_HeaderAuthorization:
			{
				std::string confirmation = "";
				std::map<std::string, SHttpParam>::iterator itB = reqInfo.mapParams.begin();
				std::map<std::string, SHttpParam>::iterator itE = reqInfo.mapParams.end();
				while(itB != itE)
				{
					if(confirmation != "")
						confirmation.append("&");
					confirmation.append(itB->first).append("=").append(itB->second.value);
					itB++;
				}
				confirmation.append("&secret_key=").append(m_strSecretKey); 
				clib::string out;
				clib::math::md5(confirmation.c_str(), confirmation.length(), out);
				_strupr((char*)out.c_str());
				std::string strAuthorization = "authorization:";
				strAuthorization.append(out.c_str());
				curl_slist* pHeaders = NULL;
				pHeaders = curl_slist_append(pHeaders, strAuthorization.c_str());
				curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, pHeaders);
			}
			break;
		default:
			break;
		}
		std::string strResponse = "";
		if(reqInfo.reqType == eHttpReqType_Get)
		{
			std::string params = "";
			std::map<std::string, SHttpParam>::iterator itB = reqInfo.mapParams.begin();
			std::map<std::string, SHttpParam>::iterator itE = reqInfo.mapParams.end();
			while(itB != itE)
			{
				if(params != "")
					params.append("&");
				params.append(itB->first).append("=").append(itB->second.value);
				itB++;
			}
			_GetReq(pCurl, reqInfo.strMethod.c_str(), params.c_str(), strResponse);
		}
		else if(reqInfo.reqType == eHttpReqType_Post)
		{
			std::string params = "{";
			std::map<std::string, SHttpParam>::iterator itB = reqInfo.mapParams.begin();
			std::map<std::string, SHttpParam>::iterator itE = reqInfo.mapParams.end();
			while(itB != itE)
			{
				if(params != "")
					params.append(",");
				params.append("\"").append(itB->first).append("\"").append(":");
				if(itB->second.type == eHttpParamType_String)
					params.append("\"").append(itB->second.value).append("\"");
				else if(itB->second.type == eHttpParamType_Int)
					params.append(itB->second.value);
				itB++;
			}
			_PostReq(pCurl, reqInfo.strMethod.c_str(), params.c_str(), strResponse);
		}
		if(strResponse != "")
		{
			Json::Reader reader;
			SHttpResponse responseInfo;
			reader.parse(strResponse.c_str(), responseInfo.retObj);
			responseInfo.apiType = reqInfo.apiType;
			responseInfo.strRet = strResponse;
			boost::mutex::scoped_lock sl(m_responseMutex);
			m_queueResponseInfo.push_back(responseInfo);
		}
	}
	curl_easy_cleanup(pCurl);
}
void CHttpAPI::_GetReq(CURL* pCurl, const char* szMethod, const char* szGetParams, std::string& strResponse)
{
	std::string strURL = m_strURL;
	strURL.append("/").append(szMethod);
	if(szGetParams && strlen(szGetParams) > 0)
		strURL.append("?").append(szGetParams);
	curl_easy_setopt(pCurl, CURLOPT_URL, strURL.c_str());
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
	{
		SHttpResponse responseInfo;
		{
			boost::mutex::scoped_lock sl(m_responseMutex);
			if(m_queueResponseInfo.empty())
				return;
			responseInfo = m_queueResponseInfo.front();
			m_queueResponseInfo.pop_front();
		}
		if(responseInfo.apiType != eHttpAPIType_Max)
		{
			if(m_callbakMessage)
				m_callbakMessage(responseInfo.apiType, responseInfo.retObj, responseInfo.strRet);
		}
	}
}

void CHttpAPI::SetCallBackMessage(http_callbak_message callbakMessage)
{
	m_callbakMessage = callbakMessage;
}

void CHttpAPI::PushReqInfo(SHttpReqInfo& info)
{
	boost::mutex::scoped_lock sl(m_reqMutex);
	m_queueReqInfo.push_back(info);
	m_condReqInfo.notify_one();
}