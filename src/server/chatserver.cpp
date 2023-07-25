#include "chatserver.hpp"
#include <iostream>
#include <string>
#include <functional>
#include"json.hpp"
#include"chatservice.hpp"
using namespace std::placeholders;
using json = nlohmann::json;
// 初始化聊天服务器对象
ChatServer::ChatServer(EventLoop *loop,               // 关注的事件，epoll
                       const InetAddress &listenAddr, // server的ip地址和端口号
                       const string &nameArg)         // server的名称
    : server_(loop, listenAddr, nameArg), loop_(loop)
{
    // 注册连接断开回调
    server_.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));

    // 注册读写事件发生的处理函数
    server_.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));

    // 设置线程数量
    server_.setThreadNum(4);
}

// 开启服务器
void ChatServer::start()
{
    server_.start();
}

// 回调连接，断开的函数
void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
    //登录失败情况
    if (!conn->connected())
    {
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown(); // 释放socket连接资源
    }
}

// 回调连接读写事件的处理
void ChatServer::onMessage(const TcpConnectionPtr &conn,
                           Buffer *buff,
                           Timestamp time)
{
    //取得事件的message
    std::string buf = buff->retrieveAllAsString();
    json js = json::parse(buf); //将数据反序列化
    //为了达到网络模块和业务模块的解耦合，这里需要使用虚函数和函数对象去封装一个业务类ChatService
    //在这个业务类中注册各种消息处理函数
    //并且每个处理函数的参数相同： TcpConnectionPtr , Json , Timestamp
   auto msgHandler =  ChatService::instance()->getHandler(js["msgid"].get<int>());
   //回调消息绑定的好的事件处理器来执行对应业务
   msgHandler(conn,js,time);

}