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
		m_pWebSocketAPI->PushRet(retObj, msg->get_payload().c_str());
	//if(callbak_message != 0)callbak_message(msg->get_payload().c_str());
}