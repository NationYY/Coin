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
	eHttpConfirmationType_Coinex,
	eHttpConfirmationType_Exx,
	eHttpConfirmationType_Zbg,
	eHttpConfirmationType_OKEx,
	eHttpConfirmationType_Binance,
	eHttpConfirmationType_Max,
};

struct SHttpParam
{
	SHttpParam(){}
	SHttpParam(eHttpParamType _type, std::string& _value) : type(_type), value(_value)
	{
	}
	SHttpParam(eHttpParamType _type, const char* _value) : type(_type), value(_value)
	{
	}
	eHttpParamType type;
	std::string value;
};
struct SHttpReqInfo
{
	std::string strURL;
	eHttpAPIType apiType;
	eHttpReqType reqType;
	std::string strMethod;
	bool bUTF8;
	std::map<std::string, SHttpParam> mapParams;
	eHttpConfirmationType confirmationType;
	int customData;
	std::string strCustomData;
	bool bSignature;
	SHttpReqInfo()
	{
		strMethod = strURL = strCustomData = "";
		reqType = eHttpReqType_Max;
		apiType = eHttpAPIType_Max;
		confirmationType = eHttpConfirmationType_Max;
		bUTF8 = false;
		bSignature = false;
		customData = 0;
	}
};


struct SHttpResponse
{
	eHttpAPIType apiType;
	Json::Value retObj;
	std::string strRet;
	int customData;
	std::string strCustomData;
	SHttpResponse()
	{
		strRet = strCustomData = "";
		apiType = eHttpAPIType_Max;
		customData = 0;
	}
};

typedef void(*http_callbak_message)(eHttpAPIType type, Json::Value& retObj, const std::string& strRet, int customData, std::string strCustomData);