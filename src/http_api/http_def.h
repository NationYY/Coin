#pragma once
enum eHttpReqType
{
	eHttpReqType_Get = 1,
	eHttpReqType_Post,
	eHttpReqType_Max
};

enum eHttpParamType
{
	eHttpParamType_String,
	eHttpParamType_Int
};

enum eHttpConfirmationType
{
	eHttpConfirmationType_HeaderAuthorization,
	eHttpConfirmationType_Max,
};

struct SHttpParam
{
	SHttpParam(){}
	SHttpParam(eHttpParamType _type, std::string& _value) : type(type), value(_value)
	{
	}
	SHttpParam(eHttpParamType _type, const char* _value) : type(type), value(_value)
	{
	}
	eHttpParamType type;
	string value;
};
struct SHttpReqInfo
{
	eHttpAPIType apiType;
	eHttpReqType reqType;
	std::string strMethod;
	std::map<std::string, SHttpParam> mapParams;
	eHttpConfirmationType confirmationType;
	SHttpReqInfo()
	{
		strMethod = "";
		reqType = eHttpReqType_Max;
		apiType = eHttpAPIType_Max;
		confirmationType = eHttpConfirmationType_Max;
	}
};


struct SHttpResponse
{
	eHttpAPIType apiType;
	Json::Value retObj;
	std::string strRet;
	SHttpResponse()
	{
		strRet = "";
		apiType = eHttpAPIType_Max;
	}
};

typedef void(*http_callbak_message)(eHttpAPIType type, Json::Value& retObj, const std::string& strRet);