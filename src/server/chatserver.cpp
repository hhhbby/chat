#include "chatserver.h"
#include "chatservice.h"
#include <functional>
using std::bind;
using std::placeholders::_1, std::placeholders::_2, std::placeholders::_3;

ChatServer::ChatServer(EventLoop *loop, const InetAddress &listenAddr, const string &nameArg)
                        :_server(loop, listenAddr, nameArg), _loop(loop)
{
    _server.setConnectionCallback(bind(&ChatServer::onConnection, this, _1));

    auto f = [this](const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time) {
        onMessage(conn, buffer, time);
    };
    _server.setMessageCallback(f);
}

void ChatServer::start()
{
    _server.start();
}

void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
    if (!conn->connected())
    {
        conn->shutdown();
    }
}

void ChatServer::onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time)
{
    string buf = buffer->retrieveAllAsString();
    json js = js.parse(buf);

    ChatService *service = ChatService::instance();
    auto handler = service->getHandler(js["msgid"]/*.get<int>()*/);
    handler(conn, js, time);
}