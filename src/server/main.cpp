#include "chatserver.h"
#include "chatservice.h"
#include <signal.h>

void handler(int sig)
{
    ChatService::instance()->reset();
    exit(0);
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        cout << "usage: " << argv[0] << " portnumber" << endl;
        exit(-1);
    }
    int portnumber = atoi(argv[1]);
    if (portnumber == 0)
        ERR_EXIT("portnumber invalid");

    signal(SIGINT, handler);

    EventLoop loop;
    InetAddress addr("127.0.0.1", portnumber);
    ChatServer server(&loop, addr, "chatserver10086");

    server.start();
    loop.loop();
}