// chatservice.cpp
#include "chatservice.hpp"
#include "public.hpp"
#include <functional>
#include <vector>
#include <string>
#include <iostream>
#include <muduo/base/Logging.h> //日志
using namespace muduo;
using namespace std::placeholders;

// 业务类的构造方法, 注册消息类型对应回调处理方法
ChatService::ChatService()
{
    msgHandlerMap_.insert({EnMsgType::LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    msgHandlerMap_.insert({EnMsgType::REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    msgHandlerMap_.insert({EnMsgType::ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    msgHandlerMap_.insert({EnMsgType::ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});
    msgHandlerMap_.insert({EnMsgType::CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    msgHandlerMap_.insert({EnMsgType::ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
    msgHandlerMap_.insert({EnMsgType::GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});
    msgHandlerMap_.insert({EnMsgType::LOGINOUT_MSG, std::bind(&ChatService::loginout, this, _1, _2, _3)});

    // 连接redis服
    if (_redis.connect())
    {
        //std::cout << "redis_.connect() is success" << std::endl;
        // 给redis设置回调函数，用户处理redis发现有消息时，调用相应的回调函数进行处理
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage, this, _1, _2));
    }
}

// 服务器异常后，业务重置方法
void ChatService::reset()
{
    // 把online状态的用户，设置为offline
    userModel_.resetState();
}

// 返回单例实例对象指针
ChatService *ChatService::instance()
{
    static ChatService service;
    return &service;
}

// 获取消息对应的处理器
MsgHandler ChatService::getHandler(int msgid)
{
    // 记录错误日志，msgid没有对应的事件处理器
    auto it = msgHandlerMap_.find(msgid);
    if (it == msgHandlerMap_.end())
    {

        // std::string errstr = std::string("msgid: ") + msgid + " can not hangdler!";
        // 返回一个空的处理器
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp)
        {
            LOG_ERROR << "msgid: " << msgid << " can not handler!";
        };
    }
    else
    {
        return it->second;
    }
}

void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp)
{
    int userid = js["id"].get<int>();
    {
        std::lock_guard<std::mutex> lock(connMutex_);
        auto it = userConnMap_.find(userid);
        if (it != userConnMap_.end())
        {
            userConnMap_.erase(it);
        }
    }
    // 用户注销，相应的需要在redis中取消订阅通道
    _redis.unsubscribe(userid);

    // 更新用户的状态信息
    User user(userid, "", "", "offline");
    userModel_.updateState(user);
}

// 处理登录业务 id pwd
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp)
{

    // LOG_INFO << "do login service!!!" ;
    int id = js["id"].get<int>();
    std::string pwd = js["password"];
    // 根据id值返回id对应user对象信息
    User user = userModel_.query(id);
    if (user.getId() == id && user.getPwd() == pwd)
    {
        if (user.getState() == "online")
        { // 该用户已经登录，不允许重复登录
            json response;
            response["msgid"] = EnMsgType::LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "this account is using , input another!";
            conn->send(response.dump()); // 发送响应消息给对方败
        }
        else
        {

            // 登录成功，记录该用户的连接信息(注意线程安全)
            { // 加锁失败直接进入阻塞状态，等待锁的释放，不进行设置条件变量让线程进入等待状态
                std::lock_guard<std::mutex> lock(connMutex_);
                userConnMap_.insert({id, conn});
            }
            // id用户登录成功后，将自己的id进行订阅channel(id)
            _redis.subscribe(id);
            // 登录成功， 需要将用户的登录状态offline -> online
            user.setState("online");
            userModel_.updateState(user);
            json response;
            response["msgid"] = EnMsgType::LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();

            // 查询该用户是否有离线消息
            std::vector<std::string> msgret = offlineMsgModel_.query(id);
            if (!msgret.empty())
            {
                response["offlinemsg"] = msgret;
                // 读取用户的离线消息后，将用户的离线消息从offlineMessage表中删除
                offlineMsgModel_.remove(id);
            }

            // 查询用户的好友的信息
            std::vector<User> friendret = friendModel_.query(id);
            if (!friendret.empty())
            {
                std::vector<std::string> vec2;
                for (User &user : friendret)
                {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }

            // 查询自己拥有组的详细信息，组成员，组的信息
            std::vector<Group> groupret = groupModel_.queryGroups(id);
            if (!groupret.empty())
            {
                std::vector<std::string> vec3;
                for (Group &group : groupret)
                {
                    json js;
                    js["id"] = group.getId();
                    js["groupname"] = group.getName();
                    js["groupdesc"] = group.getDesc();

                    auto groupuser = group.getUsers();
                    std::vector<std::string> userlist;
                    for (GroupUser &user : groupuser)
                    {
                        json jsuser;
                        jsuser["id"] = user.getId();
                        jsuser["name"] = user.getName();
                        jsuser["state"] = user.getState();
                        jsuser["role"] = user.getRole();
                        userlist.push_back(jsuser.dump());
                    }
                    js["users"] = userlist;
                    vec3.push_back(js.dump());
                }
                response["groups"] = vec3;
            }

            conn->send(response.dump()); // 发送响应消息给对方
        }
    }
    else
    {
        // 登录失败

        json response;
        response["msgid"] = EnMsgType::LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "id or password is invalid！";
        conn->send(response.dump()); // 发送响应消息给对方
    }
}

// 处理注册, name password
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // LOG_INFO << "do reg service!!!" ;
    std::string name = js["name"];
    std::string pwd = js["password"];
    User user;
    user.setName(name);
    user.setPwd(pwd);
    bool state = userModel_.insert(user);
    if (state)
    { // 注册成功
        json response;
        response["msgid"] = EnMsgType::REG_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();
        response["name"] = name;
        conn->send(response.dump()); // 发送响应消息给对方
    }
    else
    { // 注册失败
        json response;
        response["msgid"] = EnMsgType::REG_MSG_ACK;
        response["errno"] = 1;
        conn->send(response.dump()); // 发送响应消息给对方
    }
}

// 客户端异常退出的业务处理
void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    // 在userConnMap_进行删除连接
    // 加锁失败直接进入阻塞状态，等待锁的释放，不进行设置条件变量让线程进入等待状态
    User user;
    {
        std::lock_guard<std::mutex> lock(connMutex_);
        for (auto it = userConnMap_.begin(); it != userConnMap_.end(); ++it)
        {
            if (it->second == conn)
            {
                // 从map表删除用户的连接信息
                user.setId(it->first);
                userConnMap_.erase(it);
                break;
            }
        }
    }

    // 用户注销，取消对应订阅通道
    _redis.unsubscribe(user.getId());
    // 更新用户的状态信息
    if (user.getId() != -1)
    {
        user.setState("offline");
        userModel_.updateState(user);
    }
}

// 一对一聊天服务
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    /*
        消息类型：ONE_CHAT_MSG

        msgid: :

        id : 发送者的id

        name: 发送者的姓名信息

        to: 接受者的id （在线直接找到对应tcp连接发送给对方，不在线存储离线消息，等待下一次用户上线）

        msg: 对应的消息

    */

    int toid = js["toid"].get<int>();
    {
        std::lock_guard<std::mutex> lock(connMutex_);
        auto it = userConnMap_.find(toid);
        if (it != userConnMap_.end())
        { // 用户在线，转发消息 : 不能放在外面，因为可能存在当用户正在发送的时候，连接被移除了

            it->second->send(js.dump());
            return;
        }
    }

    // 去数据库查询该用户是在online
    User user = userModel_.query(toid);
    
    if (user.getState() == "online")
    {
        _redis.publish(toid, js.dump());
    }
    else
    {
        // 用户不在线，存储离线消息
        offlineMsgModel_.insert(toid, js.dump());
    }
    

    //offlineMsgModel_.insert(toid, js.dump());
}

// 添加好友的业务 msgid id friendid
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    // 存储好友的信息
    friendModel_.insert(userid, friendid);
}

// 创建群组业务
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    std::string name = js["groupname"];
    string desc = js["groupdesc"];

    // 存储创建群组信息
    Group group(-1, name, desc);
    if (groupModel_.createGroup(group))
    {
        // 存储创建者的信息
        groupModel_.addGroup(userid, group.getId(), "creator");

        // 这里可以客户端发送一个成功的消息
    }
}

// 加入群组业务
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    groupModel_.addGroup(userid, groupid, "normal");
    // 可以给客户端发送一个成功的消息
}

// 群组聊天业务
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    std::vector<int> useridVec = groupModel_.queryGroupUsers(userid, groupid);
    std::lock_guard<std::mutex> lock(connMutex_);
    for (int id : useridVec)
    {
        auto it = userConnMap_.find(id);
        if (it != userConnMap_.end())
        {
            // 转发给在线用户群消息
            auto conn = it->second;
            conn->send(js.dump());
        }
        else
        {
            // 去数据库查询该用户是在online
            User user = userModel_.query(id);
            if (user.getState() == "online")
            {
                _redis.publish(id, js.dump());
            }
            else
            {
                // 用户不在线，存储离线消息
                offlineMsgModel_.insert(id, js.dump());
            }
        }
    }
}

// 从redis消息队列中获取订阅的消息
// redis的消息处理线程发现有消息时，调用相应的ChatService的回调函数进行处理
void ChatService::handleRedisSubscribeMessage(int userid, string msg)
{
    lock_guard<mutex> lock(connMutex_);
    auto it = userConnMap_.find(userid);
    if (it != userConnMap_.end())
    {
        it->second->send(msg);
        return;
    }

    // 存储该用户的离线消息
    offlineMsgModel_.insert(userid, msg);
}