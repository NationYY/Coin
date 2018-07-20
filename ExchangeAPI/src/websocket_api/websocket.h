

#ifndef __WEBSOCKET_H__
#define __WEBSOCKET_H__


#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/config/asio.hpp>

#include <websocketpp/client.hpp>

#include <iostream>


typedef void (*websocketpp_callbak_open)();
typedef void (*websocketpp_callbak_close)();
typedef void (*websocketpp_callbak_message)(Json::Value& retObj, const std::string& strRet);


//typedef websocketpp::client<websocketpp::config::asio_client> client;
typedef websocketpp::client<websocketpp::config::asio_tls> client;


// pull out the type of messages sent by our config
typedef websocketpp::config::asio_tls_client::message_type::ptr message_ptr;
typedef websocketpp::lib::shared_ptr<boost::asio::ssl::context> context_ptr;
typedef client::connection_ptr connection_ptr;

struct SWebSocketResponse
{
	int type; //0:close 1:open 2:api
	Json::Value retObj;
	std::string strRet;
	SWebSocketResponse()
	{
		strRet = "";
		type = 0;
	}
};

#define MAX_RETRY_COUNT		10000

enum CONNECTION_STATE
{
	CONNECTION_STATE_UNKONWN,
	CONNECTION_STATE_CONNECTING,
	CONNECTION_STATE_DISCONNECT,
};

class WebSocket
{
private:
    client m_endpoint;
	websocketpp::connection_hdl m_hdl;
	std::string m_uri;
	CONNECTION_STATE m_con_state;
	class CWebSocketAPI* m_pWebSocketAPI;
public:
	bool m_manual_close;//是否为主动关闭连接，如果不是用户主动关闭，当接到断开联接回调时则自动执行重新连接机制。
    typedef WebSocket type;

    WebSocket() :  
	m_manual_close(false),
	m_con_state(CONNECTION_STATE_UNKONWN),
	m_pWebSocketAPI(NULL)
	{
		
        m_endpoint.set_access_channels(websocketpp::log::alevel::all);
        m_endpoint.set_error_channels(websocketpp::log::elevel::all);

        // Initialize ASIO
        m_endpoint.init_asio();

        //Register our handlers
        //m_endpoint.set_socket_init_handler(bind(&type::on_socket_init,this,::_1));
		m_endpoint.set_tls_init_handler(websocketpp::lib::bind(&type::on_tls_init, this, websocketpp::lib::placeholders::_1));
		m_endpoint.set_message_handler(websocketpp::lib::bind(&type::on_message, this, websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));
		m_endpoint.set_open_handler(websocketpp::lib::bind(&type::on_open, this, websocketpp::lib::placeholders::_1));
		m_endpoint.set_close_handler(websocketpp::lib::bind(&type::on_close, this, websocketpp::lib::placeholders::_1));
		m_endpoint.set_fail_handler(websocketpp::lib::bind(&type::on_fail, this, websocketpp::lib::placeholders::_1));
		
    }

    ~WebSocket()
	{
	}

	void start();

    void on_socket_init(websocketpp::connection_hdl) 
	{

    }

    context_ptr on_tls_init(websocketpp::connection_hdl)
	{
        context_ptr ctx = websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv1);

        try {
            ctx->set_options(boost::asio::ssl::context::default_workarounds |
                             boost::asio::ssl::context::no_sslv2 |
                             boost::asio::ssl::context::no_sslv3 |
                             boost::asio::ssl::context::single_dh_use);
        } catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }
        return ctx;
    }

    void on_fail(websocketpp::connection_hdl hdl)
	{
		
        client::connection_ptr con = m_endpoint.get_con_from_hdl(hdl);
        
        std::cout << "Fail handler" << std::endl;
        std::cout << con->get_state() << std::endl;
        std::cout << con->get_local_close_code() << std::endl;
        std::cout << con->get_local_close_reason() << std::endl;
        std::cout << con->get_remote_close_code() << std::endl;
        std::cout << con->get_remote_close_reason() << std::endl;
        std::cout << con->get_ec() << " - " << con->get_ec().message() << std::endl;
		
    }

    void on_close_handshake_timeout(websocketpp::connection_hdl hdl)
	{
	}
	
    void on_open(websocketpp::connection_hdl hdl);
    void on_message(websocketpp::connection_hdl hdl, message_ptr msg);
    void on_close(websocketpp::connection_hdl hdl);

    void doclose() 
	{
		m_manual_close = true;
		m_endpoint.close(m_hdl,websocketpp::close::status::going_away,"");
    }

	void run(std::string &uri)
	{
		try {
			m_uri = uri;
			start();
		} catch (const std::exception & e) {
			std::cout << e.what() << std::endl;
		} catch (websocketpp::lib::error_code e) {
			std::cout << e.message() << std::endl;
		} catch (...) {
			std::cout << "other exception" << std::endl;
		}
	}

	void request(std::string requestInfo)
	{
		m_endpoint.send(m_hdl, requestInfo, websocketpp::frame::opcode::text);

		//m_endpoint.send(hdl, "{'event':'addChannel','channel':'ok_btcusd_ticker'}", websocketpp::frame::opcode::text);
		// m_endpoint.send(hdl, "{'event':'addChannel','channel':'ok_btcusd_depth'}", websocketpp::frame::opcode::text);
	}
	void set_websoket_api(class CWebSocketAPI* pAPI){
		m_pWebSocketAPI = pAPI;
	}
};


#endif /* __WEBSOCKET_H__ */
