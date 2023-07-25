#include "chatserver.hpp"
#include "chatservice.hpp"
#include <iostream>
#include<signal.h>

//处理服务器ctrl + c 结束后的，重置user的状态信息
void resetHandler(int )
{
    ChatService::instance()->reset();
    exit(0);
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "command invalid! example: ./ChatServer ip port" << std::endl;
        exit(-1);
        
    }

    // 解析出ip和port
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    signal(SIGINT, resetHandler);

    EventLoop loop; //创建一个epoll对象
    InetAddress addr(ip, port);
    ChatServer server(&loop, addr, "ChatServer");
    server.start(); //创建listenfd 加入到epoll_ctl进行监听
    loop.loop(); //epoll_wait 阻塞方式等待用户连接， 处理用户读写事件等
    return 0;
}