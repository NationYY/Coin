#include "stdafx.h"
#include "websocket.h"
#include "websocket_api.h"
void WebSocket::on_message(websocketpp::connection_hdl hdl, message_ptr msg)
{
	//std::cout << "Message: " << msg->get_payload().c_str() << std::endl;
	Json::Value retObj;
	Json::Reader reader;
	reader.parse(msg->get_payload().c_str(), retObj);
	if(m_pWebSocketAPI)
		m_pWebSocketAPI->PushRet(2, retObj, msg->get_payload().c_str());
	//if(callbak_message != 0)callbak_message(msg->get_payload().c_str());
}

void WebSocket::on_open(websocketpp::connection_hdl hdl)
{
	m_hdl = hdl;
	m_manual_close = false;
	Json::Value retObj;
	if(m_pWebSocketAPI)
		m_pWebSocketAPI->PushRet(1, retObj, "");
}

void WebSocket::on_close(websocketpp::connection_hdl hdl)
{
	Json::Value retObj;
	if(m_pWebSocketAPI)
		m_pWebSocketAPI->PushRet(0, retObj, "");
}