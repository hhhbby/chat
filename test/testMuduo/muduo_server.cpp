#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>

using muduo::net::EventLoop;
using muduo::net::InetAddress;
using muduo::net::TcpConnectionPtr, muduo::net::Buffer, muduo::Timestamp;
using muduo::net::TcpServer;

#include <string>
#include <iostream>
#include <functional>
using std::bind;
using namespace std::placeholders;
using std::string, std::cout, std::endl;

class ChatServer
{
public:
    ChatServer(EventLoop *loop, const InetAddress &listenAddr, const string &nameArg)
        : _server(loop, listenAddr, nameArg), _loop(loop)
    {
        _server.setConnectionCallback(bind(&ChatServer::onConnection, this, _1));
        _server.setMessageCallback(bind(&ChatServer::onMessage, this, _1, _2, _3));
        // TODO 1 check subthreadpool num
        // 这里设置的sub Event的数量，也就是总共有5个线程池
        _server.setThreadNum(4);
    }

    void start()
    {
        _server.start();
    }

private:
    void onConnection(const TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {
            cout << "conn " << conn->peerAddress().toIpPort() << " --- " << conn->localAddress().toIpPort() << ", state : online" << endl;
        } else {
            cout << "conn " << conn->peerAddress().toIpPort() << " --- " << conn->localAddress().toIpPort() << ", state : offline" << endl;
            // TODO 2 check shutdonw mean and if invoked automatically
            // close write side
            conn->shutdown();
        }
    }

    void onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time)
    {
        string buf = buffer->retrieveAllAsString();
        cout << "recv data:" << buf << " time:" << time.toFormattedString() << endl;
        conn->send(buf);
    }

    TcpServer _server;
    EventLoop *_loop;
};

int main()
{
    EventLoop loop;
    InetAddress addr("127.0.0.1", 9000);
    ChatServer server(&loop, addr, "ChatServer");
    server.start();
    loop.loop();
}