#include "stdafx.h"
#include "coinex_web_socket_api.h"
#include <json/json.h>
#include <clib/lib/math/math_ex.h>

CCoinexWebSocketAPI::CCoinexWebSocketAPI(string api_key, string secret_key)
{
	SetKey(api_key, secret_key);
	SetUri("wss://socket.coinex.com/");
}


CCoinexWebSocketAPI::~CCoinexWebSocketAPI()
{
}

void CCoinexWebSocketAPI::GetServerTime()
{
	Request("{\"method\":\"server.time\", \"params\" : [], \"id\" : 12}");
}

void CCoinexWebSocketAPI::LoginIn()
{
	time_t tNow = time(NULL)*1000;
	char szBuff[256];
	_snprintf(szBuff, 256, "access_id=%s&tonce=%lld&secret_key=%s", m_api_key.c_str(), tNow, m_secret_key.c_str());
	clib::string out;
	clib::math::md5(szBuff, strlen(szBuff), out);
	_strupr((char*)out.c_str());
	Json::FastWriter writer;
	Json::Value root;
	root["method"] = "server.sign";
	Json::Value params;
	params["access_id"] = m_api_key;
	params["tonce"] = tNow;
	params["authorization"] = out.c_str();
	root["params"] = params;
	root["id"] = 22;
	std::string json_file = writer.write(root);
	
	Request(json_file.c_str());
}


void CCoinexWebSocketAPI::Ping()
{
	Request("{\"method\":\"server.ping\",\"params\":[],\"id\" : 11}");
}