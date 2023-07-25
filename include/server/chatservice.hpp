#ifndef CHATSERVICE_H
#define CHATSERVICE_H
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <unordered_map>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <condition_variable>
#include "json.hpp"
#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "redis.hpp"

using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;

// 处理消息回调对象类型
using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp)>;

// 业务的实现
// 业务类，只需要存在一个单例即可，因此这里使用单例模式
// 聊天服务器业务类
class ChatService
{
public:
    // 返回单例实例对象指针
    static ChatService *instance();

    // 处理客户端退出的
    void loginout(const TcpConnectionPtr &conn, json &js, Timestamp);

    // 处理登录业务
    void login(const TcpConnectionPtr &conn, json &js, Timestamp);
    // 处理注册
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp);

    // 客户端异常退出的业务处理
    void clientCloseException(const TcpConnectionPtr &conn);

    // 一对一聊天服务
    void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 服务器异常后，业务重置方法
    void reset();

    // 添加好友的业务
    void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 创建群组业务
    void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 加入群组业务
    void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 群组聊天业务
    void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 获取消息对应的处理器
    MsgHandler getHandler(int msgid);

    //redis的消息处理线程发现有消息时，调用相应的ChatService的回调函数进行处理
    void handleRedisSubscribeMessage(int userid, string msg);

private:
    ChatService();

private:
    // 处理函数注册表, 消息类型type:消息对象处理对象
    std::unordered_map<int, MsgHandler> msgHandlerMap_;

    // 数据操作类对象
    UserModel userModel_;

    // offlineMessage表的操作类
    OfflineMsgModel offlineMsgModel_;

    // friend表的操作类
    FriendModel friendModel_;

    // groupmodel的对象
    GroupModel groupModel_;

    // 定义互斥锁，保证userConnMap的线程安全
    std::mutex connMutex_;

    // 存储在线用户的通信连接
    std::unordered_map<int, TcpConnectionPtr> userConnMap_;

    // redis操作对象
    Redis _redis;
};

#endif