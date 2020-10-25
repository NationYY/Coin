#include "stdafx.h"
#include "http_api.h"
#include "clib/lib/clib.h"
#include "clib/lib/memory_manager/memory_allocator.h"
#include "clib/lib/math/math_ex.h"
#include "algorithm/hmac.h"
#include "common/func_common.h"
#include <time.h>
#include <websocketpp/base64/base64.hpp>
#include "log/local_log.h"
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	std::string * tmp = (std::string *) userp;

	(*tmp).append((char*)contents, realsize);

	return realsize;
}


CHttpAPI::CHttpAPI() :m_pMainCurl(NULL), m_bClose(false), m_nCloseThreadCnt(0)
{
}


CHttpAPI::~CHttpAPI()
{
	if(m_pMainCurl)
	{
		curl_easy_cleanup(m_pMainCurl);
		m_pMainCurl = NULL;
	}
}

void CHttpAPI::SetKey(std::string strAPIKey, std::string strSecretKey, std::string strPassphrase)
{
	m_strAPIKey = strAPIKey;
	m_strSecretKey = strSecretKey;
	m_strPassphrase = strPassphrase;
}

void CHttpAPI::SetURL(std::string strURL)
{
	m_strURL = strURL;
}

void CHttpAPI::SetContentType(std::string strContentType)
{
	m_strContentType = strContentType;
}

void CHttpAPI::Run(int threadNums)
{
	m_pMainCurl = curl_easy_init();
	curl_easy_setopt(m_pMainCurl, CURLOPT_SSL_VERIFYPEER, false); // if want to use https  
	curl_easy_setopt(m_pMainCurl, CURLOPT_SSL_VERIFYHOST, false); // set peer and host verify false  
	//curl_easy_setopt(m_pMainCurl, CURLOPT_VERBOSE, 1);
	curl_easy_setopt(m_pMainCurl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(m_pMainCurl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(m_pMainCurl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(m_pMainCurl, CURLOPT_CONNECTTIMEOUT, 5);
	curl_easy_setopt(m_pMainCurl, CURLOPT_TIMEOUT, 5);
	curl_slist* pHeaders = NULL;
	pHeaders = curl_slist_append(pHeaders, "User-Agent:Mozilla / 5.0£®Windows NT 6.1; WOW64£©AppleWebKit / 537.36£®KHTML£¨»ÁGecko£©Chrome / 39.0.2171.71 Safari / 537.36");
	if(m_strContentType != "")
	{
		char szBuffer[256];
		_snprintf(szBuffer, 256, "Content-type:%s", m_strContentType.c_str());
		pHeaders = curl_slist_append(pHeaders, szBuffer);
	}
	curl_easy_setopt(m_pMainCurl, CURLOPT_HTTPHEADER, pHeaders);
	curl_easy_setopt(m_pMainCurl, CURLOPT_NOPROGRESS, 1);
	m_threadNum = threadNums;
	//curl_global_init(CURL_GLOBAL_ALL);
	for(int i = 0; i < threadNums; ++i)
	{
		m_workers.create_thread(boost::bind(&CHttpAPI::_ProcessHttp, this));
	}
}

void CHttpAPI::Close()
{
	m_bClose = true;
	m_condReqInfo.notify_all();
	while(m_nCloseThreadCnt != m_threadNum)
	{

	}
}

void AssembleParams(bool bJson, std::string &out, std::map<std::string, SHttpParam>& mapParams)
{
	if(bJson)
	{
		out = "{";
		std::map<std::string, SHttpParam>::iterator itB = mapParams.begin();
		std::map<std::string, SHttpParam>::iterator itE = mapParams.end();
		while(itB != itE)
		{
			if(out != "{")
				out.append(",");
			out.append("\"").append(itB->first).append("\"").append(":");
			if(itB->second.type == eHttpParamType_String)
				out.append("\"").append(itB->second.value).append("\"");
			else if(itB->second.type == eHttpParamType_Int)
				out.append(itB->second.value);
			itB++;
		}
		out.append("}");
	}
	else
	{
		out = "";
		std::map<std::string, SHttpParam>::iterator itB = mapParams.begin();
		std::map<std::string, SHttpParam>::iterator itE = mapParams.end();
		while(itB != itE)
		{
			if(out != "")
				out.append("&");
			out.append(itB->first).append("=").append(itB->second.value);
			itB++;
		}
	}
}

void CHttpAPI::_ProcessHttp()
{
	CURL* pCurl = curl_easy_init();
	curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, false); // if want to use https  
	curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYHOST, false); // set peer and host verify false  
	//curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1);
	curl_easy_setopt(pCurl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(pCurl, CURLOPT_CONNECTTIMEOUT, 10);
	curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 10);
	curl_easy_setopt(pCurl, CURLOPT_NOPROGRESS, 1);

	curl_slist* pHeaders = NULL;
	pHeaders = curl_slist_append(pHeaders, "User-Agent:Mozilla / 5.0£®Windows NT 6.1; WOW64£©AppleWebKit / 537.36£®KHTML£¨»ÁGecko£©Chrome / 39.0.2171.71 Safari / 537.36");
	if(m_strContentType != "")
	{
		char szBuffer[256];
		_snprintf(szBuffer, 256, "Content-type:%s", m_strContentType.c_str());
		pHeaders = curl_slist_append(pHeaders, szBuffer);
	}
	curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, pHeaders);
	while(!m_bClose)
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
		SHttpResponse resInfo;
		_Request(pCurl, reqInfo, &resInfo);
		if(resInfo.apiType != eHttpAPIType_Max)
		{
			boost::mutex::scoped_lock sl(m_responseMutex);
			m_queueResponseInfo.push_back(resInfo);
		}
	}
	m_nCloseThreadCnt++;
	curl_easy_cleanup(pCurl);
}
void CHttpAPI::_GetReq(CURL* pCurl, std::string& _strURL, const char* szMethod, const char* szGetParams, std::string& strResponse)
{
	std::string strURL = m_strURL;
	if(_strURL != "")
		strURL = _strURL;
	strURL.append("/").append(szMethod);
	if(szGetParams && strlen(szGetParams) > 0)
		strURL.append("?").append(szGetParams);
	curl_easy_setopt(pCurl, CURLOPT_URL, strURL.c_str());
	curl_easy_setopt(pCurl, CURLOPT_POST, 0); // post req 
	curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, (void*)&strResponse);
	curl_easy_setopt(pCurl, CURLOPT_CUSTOMREQUEST, "GET");
	CURLcode res = curl_easy_perform(pCurl);
}

void CHttpAPI::_PostReq(CURL* pCurl, std::string& _strURL, const char* szMethod, const char* szPostParams, std::string& strResponse)
{
	std::string strURL = m_strURL;
	if(_strURL != "")
		strURL = _strURL;
	strURL.append("/").append(szMethod);
	curl_easy_setopt(pCurl, CURLOPT_URL, strURL.c_str());
	curl_easy_setopt(pCurl, CURLOPT_POST, 1); // post req
	curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, szPostParams);
	curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, (void*)&strResponse);
	curl_easy_setopt(pCurl, CURLOPT_CUSTOMREQUEST, "POST");
	CURLcode res = curl_easy_perform(pCurl);
}

void CHttpAPI::_DeleteReq(CURL* pCurl, std::string& _strURL, const char* szMethod, const char* szPostParams, std::string& strResponse)
{
	std::string strURL = m_strURL;
	if(_strURL != "")
		strURL = _strURL;
	strURL.append("/").append(szMethod);
	curl_easy_setopt(pCurl, CURLOPT_URL, strURL.c_str());
	curl_easy_setopt(pCurl, CURLOPT_POST, 1); // post req
	curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, szPostParams);
	curl_easy_setopt(pCurl, CURLOPT_CUSTOMREQUEST, "DELETE");
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
			OnResponse(responseInfo.apiType, responseInfo.retObj, responseInfo.strRet, responseInfo.customData, responseInfo.strCustomData);
	}
}



void CHttpAPI::RequestAsync(SHttpReqInfo& info)
{
	boost::mutex::scoped_lock sl(m_reqMutex);
	m_queueReqInfo.push_back(info);
	m_condReqInfo.notify_one();
}

void CHttpAPI::Request(SHttpReqInfo& reqInfo, SHttpResponse* pResInfo)
{
	_Request(m_pMainCurl, reqInfo, pResInfo);
}

void CHttpAPI::_Request(CURL* pCurl, SHttpReqInfo& reqInfo, SHttpResponse* pResInfo)
{
	std::string strGetParams = "";
	std::string strPostParams = "";
	std::string strDeleteParams = "";
	switch(reqInfo.confirmationType)
	{
	case eHttpConfirmationType_Coinex:
		{
			AssembleParams(false, strGetParams, reqInfo.mapParams);
			std::string confirmation = strGetParams;
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
	case eHttpConfirmationType_Exx:
		{
			AssembleParams(false, strGetParams, reqInfo.mapParams);
			std::string confirmation = strGetParams;
			unsigned char *out = NULL;
			unsigned int outSize = 0;
			HmacEncode("sha512", m_strSecretKey.c_str(), m_strSecretKey.length(), confirmation.c_str(), confirmation.length(), out, outSize);
			strGetParams.append("&signature=");
			for(int i = 0; i < (int)outSize; i++)
			{
				char szBuffer[12] = { 0 };
				_snprintf(szBuffer, 12, "%02x", (unsigned int)out[i]);
				strGetParams.append(szBuffer);
			}
			free(out);
		}
		break;
	case eHttpConfirmationType_Zbg:
		{
			std::string strSignSrc = m_strAPIKey;
			char szBuffer[128];
			_snprintf(szBuffer, 128, "%lld", time(NULL) * 1000);
			strSignSrc.append(szBuffer);
			std::string params = "";
			if(reqInfo.reqType == eHttpReqType_Get)
			{
				std::map<std::string, SHttpParam>::iterator itB = reqInfo.mapParams.begin();
				std::map<std::string, SHttpParam>::iterator itE = reqInfo.mapParams.end();
				while(itB != itE)
				{
					params.append(itB->first).append(itB->second.value);
					itB++;
				}
				strSignSrc.append(params);
			}
			else if(reqInfo.reqType == eHttpReqType_Post)
			{
				AssembleParams(true, strPostParams, reqInfo.mapParams);
				if(strPostParams != "{}")
					strSignSrc.append(strPostParams);
			}
			strSignSrc.append(m_strSecretKey);
			clib::string out;
			clib::math::md5(strSignSrc.c_str(), strSignSrc.length(), out);
			std::string header;

			curl_slist* pHeaders = NULL;
			header = "Apiid:";
			header.append(m_strAPIKey);
			pHeaders = curl_slist_append(pHeaders, header.c_str());
			header = "Timestamp:";
			header.append(szBuffer);
			pHeaders = curl_slist_append(pHeaders, header.c_str());
			header = "Sign:";
			header.append(out.c_str());
			pHeaders = curl_slist_append(pHeaders, header.c_str());
			curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, pHeaders);
		}
		break;
	case eHttpConfirmationType_OKEx:
		{
			time_t tNow = time(NULL);
			std::string strTimestamp = CFuncCommon::LocaltimeToISO8601(tNow);
			std::string strSignSrc = strTimestamp;
			if(reqInfo.reqType == eHttpReqType_Get)
			{
				strSignSrc += "GET/";
				strSignSrc += reqInfo.strMethod;
				AssembleParams(false, strGetParams, reqInfo.mapParams);
				strSignSrc += strGetParams;
			}
			else if(reqInfo.reqType == eHttpReqType_Post)
			{
				strSignSrc += "POST/";
				strSignSrc += reqInfo.strMethod;
				AssembleParams(true, strPostParams, reqInfo.mapParams);
				strSignSrc += strPostParams;
			}
			unsigned char *out = NULL;
			unsigned int outSize = 0;
			HmacEncode("sha256", m_strSecretKey.c_str(), m_strSecretKey.length(), strSignSrc.c_str(), strSignSrc.length(), out, outSize);
			std::string sign = websocketpp::base64_encode(out, outSize);
			std::string header;
			curl_slist* pHeaders = NULL;
			header = "OK-ACCESS-KEY:";
			header.append(m_strAPIKey);
			pHeaders = curl_slist_append(pHeaders, header.c_str());
			header = "OK-ACCESS-SIGN:";
			header.append(sign);
			pHeaders = curl_slist_append(pHeaders, header.c_str());
			header = "OK-ACCESS-TIMESTAMP:";
			header.append(strTimestamp);
			pHeaders = curl_slist_append(pHeaders, header.c_str());
			header = "OK-ACCESS-PASSPHRASE:";
			header.append(m_strPassphrase);
			pHeaders = curl_slist_append(pHeaders, header.c_str());
			header = "Accept:application/json";
			pHeaders = curl_slist_append(pHeaders, header.c_str());
			header = "Content-type:application/json";
			pHeaders = curl_slist_append(pHeaders, header.c_str());
			curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, pHeaders);
			free(out);
		}
		break;
	case eHttpConfirmationType_Binance:
		{
			AssembleParams(false, strGetParams, reqInfo.mapParams);
			if(reqInfo.bSignature)
			{
				std::string confirmation = strGetParams;
				unsigned char *out = NULL;
				unsigned int outSize = 0;
				HmacEncode("sha256", m_strSecretKey.c_str(), m_strSecretKey.length(), confirmation.c_str(), confirmation.length(), out, outSize);
				strGetParams.append("&signature=");
				for(int i = 0; i < (int)outSize; i++)
				{
					char szBuffer[12] = { 0 };
					_snprintf(szBuffer, 12, "%02x", (unsigned int)out[i]);
					strGetParams.append(szBuffer);
				}
				free(out);
			}
			strPostParams = strDeleteParams = strGetParams;
			std::string header;
			curl_slist* pHeaders = NULL;
			header = "X-MBX-APIKEY:";
			header.append(m_strAPIKey);
			pHeaders = curl_slist_append(pHeaders, header.c_str());
			curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, pHeaders);
		}
		break;
	default:
		break;
	}
	std::string strResponse = "";
	if(reqInfo.reqType == eHttpReqType_Get)
	{
		if(strGetParams == "")
			AssembleParams(false, strGetParams, reqInfo.mapParams);
		_GetReq(pCurl, reqInfo.strURL, reqInfo.strMethod.c_str(), strGetParams.c_str(), strResponse);
	}
	else if(reqInfo.reqType == eHttpReqType_Post)
	{
		if(strPostParams == "")
			AssembleParams(false, strPostParams, reqInfo.mapParams);
		_PostReq(pCurl, reqInfo.strURL, reqInfo.strMethod.c_str(), strPostParams.c_str(), strResponse);
	}
	else if(reqInfo.reqType == eHttpReqType_Delete)
	{
		if(strDeleteParams == "")
			AssembleParams(false, strDeleteParams, reqInfo.mapParams);
		_DeleteReq(pCurl, reqInfo.strURL, reqInfo.strMethod.c_str(), strDeleteParams.c_str(), strResponse);
	}
	if(pResInfo)
	{
		pResInfo->apiType = reqInfo.apiType;
		pResInfo->customData = reqInfo.customData;
		pResInfo->strCustomData = reqInfo.strCustomData;
		if(reqInfo.bUTF8)
		{
			char szRet[4096*20] = {0};
			CFuncCommon::EncodeConvert("utf-8", "gb2312", (char*)strResponse.c_str(), strResponse.length(), szRet, 4096*20);
			Json::Reader reader;
			reader.parse(szRet, pResInfo->retObj);
			pResInfo->strRet = szRet;
		}
		else
		{
			Json::Reader reader;
			reader.parse(strResponse.c_str(), pResInfo->retObj);
			pResInfo->strRet = strResponse;
		}

	}
}

void CHttpAPI::OnResponse(eHttpAPIType type, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData)
{
	if(m_callBackFunc)
		m_callBackFunc(type, retObj, strRet, customData, strCustomData);
}