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
    _server.setThreadNum(4);
}

void ChatServer::start()
{
    _server.start();
}

void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
    if (!conn->connected())
    {
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
}

void ChatServer::onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time)
{
    while (buffer->readableBytes() >= 4)
    {
        uint32_t length = buffer->peekInt32();
        if (length == 0 || length > 1024 * 1024)
        {
            cout << "msg length error" << endl;
            conn->forceClose();
            return;
        }
        size_t frameLength = 4 + static_cast<size_t>(length);
        if (buffer->readableBytes() < frameLength)
            break;
        string body(buffer->peek() + 4, length);
        buffer->retrieve(frameLength);

        json js = js.parse(body);

        ChatService *service = ChatService::instance();
        auto handler = service->getHandler(js["msgid"]/*.get<int>()*/);
        handler(conn, js, time);
    }
}