

#ifndef __WEBSOCKET_PROTOCOL_H__
#define __WEBSOCKET_PROTOCOL_H__

#include <string>

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include "audio_service.h"


typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;

// Define the callback function type
typedef void (*ws_recv_callback_t)(const char *buffer, size_t size);

typedef struct websocket_data_t {
    std::string hostname;
    std::string port;
    std::string path; 
    std::string hello;
    std::string headers;
} websocket_data_t;


class WebsocketProtocol{
private:
    WebsocketProtocol(){};
    ~WebsocketProtocol(){};

     void on_message(websocketpp::connection_hdl hdl, client::message_ptr msg);
     bool verify_subject_alternative_name(const char *hostname, X509 *cert);
     bool verify_common_name(char const *hostname, X509 *cert);
     bool verify_certificate(const char *hostname, bool preverified, boost::asio::ssl::verify_context &ctx);
     context_ptr on_tls_init(const char *hostname, websocketpp::connection_hdl);
     void on_open( websocketpp::connection_hdl hdl);
     void on_close( websocketpp::connection_hdl hdl);
     int websocket_connect();
     void websocket_thread();

public:
    static WebsocketProtocol& GetInstance() {
        static WebsocketProtocol instance;
        return instance;
    }
    /**
     * 发送二进制数据
     * 
     * @param data 数据指针
     * @param size 数据大小
     * @return 错误码
     */
    int websocket_send_binary(const char *data, int size);

    /**
     * 发送文本数据
     * 
     * @param data 数据指针
     * @param size 数据大小
     * @return 错误码
     */
    int websocket_send_text(const char *data, int size) ;
    /**
     * 设置回调函数和数据
     * 
     * @param bin_cb 二进制数据接收回调
     * @param txt_cb 文本数据接收回调
     * @param ws_data WebSocket数据
     * @return 返回值
     */
    int websocket_set_callbacks(ws_recv_callback_t bin_cb, ws_recv_callback_t txt_cb, websocket_data_t *ws_data);

    void OnIncomingAudio(std::function<void(std::unique_ptr<AudioStreamPacket> packet, bool wait)> callback);
    void OnIncomingJson(std::function<void(const char *buffer, size_t size)> callback);
    void OnData(std::function<void(const char*, size_t, bool binary)> callback);


    bool SendAudio(std::unique_ptr<AudioStreamPacket> packet);

    /**
     * 启动WebSocket线程
     * 
     * @return 返回值
     */
    int websocket_start() ;
    void param_init();
    inline int getConnectState() const { return g_iHasConnected; }
    inline int getShakeState() const { return g_iHasShaked; }

public:
    std::function<void(std::unique_ptr<AudioStreamPacket> packet, bool wait)> on_incoming_audio_;
    std::function<void(const char *buffer, size_t size)> on_incoming_json_;
    std::function<void(const char*, size_t, bool binary)> on_data_;
    

    int version_ = 1;
    websocket_data_t ws_data;
    client* m_client;
    client* g_p_ws_client;
     websocketpp::connection_hdl g_hdl;
     websocket_data_t *g_ws_data;
     ws_recv_callback_t g_ws_recv_bin_cb;
     ws_recv_callback_t g_ws_recv_txt_cb;
     volatile int g_iHasShaked;
     volatile int g_iHasConnected;
};


#endif