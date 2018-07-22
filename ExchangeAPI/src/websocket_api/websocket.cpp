#include "stdafx.h"
#include "websocket.h"
#include "websocket_api.h"
#include "common/func_common.h"
void WebSocket::on_message(websocketpp::connection_hdl hdl, message_ptr msg)
{
	Json::Value retObj;
	Json::Reader reader;
	if(m_bUTF8)
	{
		char szRet[2048] = { 0 };
		CFuncCommon::EncodeConvert("utf-8", "gb2312", (char*)msg->get_payload().c_str(), msg->get_payload().length(), szRet, 2048);
		reader.parse(szRet, retObj);
		if(m_pWebSocketAPI)
			m_pWebSocketAPI->PushRet(2, retObj, szRet);
	}
	else
	{
		reader.parse(msg->get_payload().c_str(), retObj);
		if(m_pWebSocketAPI)
			m_pWebSocketAPI->PushRet(2, retObj, msg->get_payload().c_str());
	}
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

void WebSocket::start()
{
	websocketpp::lib::error_code ec;
	client::connection_ptr con = m_endpoint.get_connection(m_uri, ec);

	if(ec) {
		m_endpoint.get_alog().write(websocketpp::log::alevel::app, ec.message());
	}

	m_endpoint.set_open_handshake_timeout(30000);
	m_endpoint.set_close_handshake_timeout(30000);


	//con->set_proxy("http://humupdates.uchicago.edu:8443");

	m_endpoint.connect(con);

	// Start the ASIO io_service run loop
	m_endpoint.run();
	Json::Value retObj;
	if(m_pWebSocketAPI)
		m_pWebSocketAPI->PushRet(0, retObj, "");
}