/*
    muduo网络库提供给用户两个主要的类
    TcpServer: 用于编写服务器程序的
    TcpClient: 用于编写客户端程序

epoll + 线程池
好处： 能够将网络I/O代码和业务代码区别开
        用户只需要关注连接和断开 读写事件

*/
#include<muduo/net/TcpServer.h>
#include<muduo/net/EventLoop.h>
#include<iostream>
#include<functional>
#include<string>
using namespace muduo;
using namespace muduo::net;

/*
    基于网路库开发服务器程序步骤
    1. 组合TcpServer对象
    2. 创建TcpServerLoop 事件循环对象指针， 类似于epoll
    3. 明确TcpServer构造函数需要什么参数， 构建自己ChatServer的构造函数， TcpServer没有默认构造函数
    4. 在当前的服务器类的构造函数中，注册处理连接的回调函数， 以及处理读写事件的回调函数
    5. 为服务器设置合适的线程数量， muduo库会自己分配I/O线程和worker线程
*/

class ChatServer
{
public: 
    ChatServer(EventLoop* loop,
               const InetAddress& listenAddr,
               const string& nameArg)
            :server_(loop, listenAddr,nameArg),loop_(loop)
    {
        //给服务器注册用户连接的函数
        server_.setConnectionCallback(std::bind(&ChatServer::onConnection, this,std::placeholders::_1));
        //给服务器注册用户读写事件发送时的处理函数
        server_.setMessageCallback(std::bind(&ChatServer::onMessage, this, 
                                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        //设置服务器的线程数量
        server_.setThreadNum(4);
    }
    void start()
    {
        server_.start();//启动服务器
    }
private:
    //用于专门处理用户连接的创建和断开 epoll listenfd accept
    void onConnection(const TcpConnectionPtr& conn)
    {
     if(conn->connected())
     {
        std::cout << conn->peerAddress().toIpPort() << " -> " << 
            conn->localAddress().toIpPort() << " state:online" << std::endl;
     }
     else 
     {
        std::cout << conn->peerAddress().toIpPort() << " -> " << 
            conn->localAddress().toIpPort() << " state:offline" << std::endl;
        conn->shutdown(); //调用对应socketfd 进行shutdown
        //loop_->quit(); //相当于退出了epoll
     } 
    }

    //专门用来处理用户读写事件的
    void onMessage(const TcpConnectionPtr& conn, //连接
                   Buffer* buffer,  //缓冲区
                   Timestamp time) //数据接受的时间信息
    {
        std::string buf = buffer->retrieveAllAsString();
        std::cout << "recv data: " << buf << " time: " << time.toFormattedString() << std::endl;
        conn->send(buf); //回射给客户端
    }


private:
    TcpServer server_; 
    EventLoop* loop_; // 管理连接事件的epoll对象

};

int main()
{
    EventLoop loop; //创建一个epoll对象
    InetAddress addr("127.0.0.1", 8888);
    ChatServer server(&loop, addr, "ChatServer");
    server.start(); //创建listenfd 加入到epoll_ctl进行监听
    loop.loop(); //epoll_wait 阻塞方式等待用户连接， 处理用户读写事件等

    return 0;
}