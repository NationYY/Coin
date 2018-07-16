#pragma once
enum eHttpReqType
{
	eHttpReqType_Get = 1,
	eHttpReqType_Post,
	eHttpReqType_Max
};

struct SHttpReqInfo
{
	eHttpReqType type;
	std::string strMethod;
	std::string strPostParams;
	SHttpReqInfo()
	{
		strMethod = "";
		strPostParams = "";
		type = eHttpReqType_Max;
	}
};


struct SHttpResponse
{
	std::string strResponse;
	SHttpResponse()
	{
		strResponse = "";
	}
};

typedef void(*http_callbak_message)(const char *message);