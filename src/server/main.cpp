#include "chatserver.h"
#include "chatservice.h"
#include <signal.h>

void handler(int sig)
{
    ChatService::instance()->reset();
    exit(0);
}

int main()
{
    signal(SIGINT, handler);

    EventLoop loop;
    InetAddress addr("127.0.0.1", 9999);
    ChatServer server(&loop, addr, "chatserver10086");

    server.start();
    loop.loop();
}