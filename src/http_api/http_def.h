#pragma once
enum eHttpReqType
{
	eHttpReqType_Get = 1,
	eHttpReqType_Post,
};

struct SHttpReqInfo
{
	eHttpReqType type;
	std::string strMethod;
	std::string strPostParams;
};


struct SHttpResponse
{
	std::string strResponse;
};