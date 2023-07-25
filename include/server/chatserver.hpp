#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include<iostream>
#include<functional>
#include<string>
using namespace muduo;
using namespace muduo::net;

//聊天服务器的类
class ChatServer
{
public:
    // 初始化聊天服务器对象
    ChatServer(EventLoop *loop,               // 关注的事件，epoll
               const InetAddress &listenAddr, // server的ip地址和端口号
               const string &nameArg);        // server的名称

    // 开启服务器
    void start();

private:
    // 回调连接，断开的函数
    void onConnection(const TcpConnectionPtr &conn);

    // 回调连接读写事件的处理
    void onMessage(const TcpConnectionPtr &conn,
                   Buffer *buff,
                   Timestamp time);

private:
    TcpServer server_; //组合muduo库的，使用网络服务器功能的对象
    EventLoop *loop_; //指向事件循环对象指针
};

#endif
