#include "json.hpp"
#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <unordered_map>
#include <functional>
using namespace std;
using json = nlohmann::json;

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <atomic>

#include "group.hpp"
#include "user.hpp"
#include "public.hpp"

// 记录当前系统登录的用户信息
User g_currentUser;
// 记录当前登录用户的好友列表信息
vector<User> g_currentUserFriendList;
// 记录当前登录用户的群组列表信息
vector<Group> g_currentUserGroupList;

// 控制主菜单页面程序
bool isMainMenuRunning = false;

// 用于读写线程之间的通信
sem_t rwsem;

// 记录登录状态
atomic_bool g_isLoginSuccess{false};

// 接收线程
void readTaskHandler(int clientfd);

// 获取系统时间（聊天信息需要添加时间信息）
string getCurrentTime();

// 主聊天页面程序
void mainMenu(int);

// 显示当前登录成功用户的基本信息
void showCurrentUserData();

void doRegResponse(json &responsejs);

void deLoginResponse(json &responsejs);

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        std::cerr << "command invalid! example: ./ChatClient ip port" << std::endl;
        exit(-1);
    }

    // 解析出ip和port
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    // 创建客户端的socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1)
    {
        std::cerr << "socketfd create error" << std::endl;
        exit(-1);
    }

    // 填写client需要的信息
    sockaddr_in server;
    memset(&server, 0, sizeof(sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    // client 和服务器进行连接
    if (connect(clientfd, (sockaddr *)&server, sizeof(sockaddr_in)) == -1)
    {
        std::cerr << "connect server error" << std::endl;
        close(clientfd);
        exit(-1);
    }

    // 初始化读写线程通信的信号量
    sem_init(&rwsem, 0, 0);

    // 连接服务器成功， 启动接受的子线程
    // 放在这里是为了之启动一次该线程， 防止用户loginout后，又出现启动子线程
    std::thread readTask(readTaskHandler, clientfd);
    readTask.detach();

    // 主线程用于接受用户的输入，发送数据
    for (;;)
    { // 显示首页面菜单，登录，注册和退出
        std::cout << "===========！请输入对应业务的序号！================" << std::endl;
        std::cout << "1. login" << std::endl;
        std::cout << "2. register" << std::endl;
        std::cout << "3. quit" << std::endl;
        std::cout << "===========================" << std::endl;
        std::cout << "choice: ";
        int choice = 0;
        std::cin >> choice;
        cin.get(); // 读取掉缓冲区残留的回车
        switch (choice)
        {
        case 1: // login业务
        {
            int id = 0;
            char pwd[50] = {0};
            std::cout << "userid: ";
            std::cin >> id;
            std::cin.get();
            std::cout << "userpassword: ";
            std::cin.getline(pwd, 50);

            json js;
            js["msgid"] = EnMsgType::LOGIN_MSG;
            js["id"] = id;
            js["password"] = pwd;
            std::string sendmsg = js.dump();
            g_isLoginSuccess = false;
            int len = send(clientfd, sendmsg.c_str(), strlen(sendmsg.c_str()) + 1, 0);
            if (len == -1)
            {
                std::cerr << "send login msg error: " << sendmsg << std::endl;
            }
            sem_wait(&rwsem);
            if (g_isLoginSuccess)
            {
                // 登录成功，进入聊天主菜单页面
                isMainMenuRunning = true;
                mainMenu(clientfd);
            }
        }
        break;
        case 2: // register 业务
        {
            char name[50] = {0};
            char pwd[50] = {0};
            std::cout << "username:";
            std::cin.getline(name, 50);
            std::cout << "userpassword:";
            std::cin.getline(pwd, 50);
            json js;
            js["msgid"] = EnMsgType::REG_MSG;
            js["name"] = name;
            js["password"] = pwd;
            std::string sendmsg(js.dump());
            int len = send(clientfd, sendmsg.c_str(), strlen(sendmsg.c_str()) + 1, 0);
            if (len == -1)
            {
                std::cerr << "send reg msg error: " << sendmsg << std::endl;
            }
            sem_wait(&rwsem); // 等待信号量， 子线程处理完成会通知main线程
        }
        break;
        case 3: // quit 业务
        {
            close(clientfd);
            sem_destroy(&rwsem);
            exit(0);
        }
        break;
        default:
        {
            std::cerr << "invlid input!" << std::endl;
        }
        break;
        }
    }
}

// 显示当前登录成功用户的基本信息
void showCurrentUserData()
{
    std::cout << "======================login user======================" << std::endl;
    std::cout << "current login user => id:" << g_currentUser.getId() << " name:" << g_currentUser.getName() << std::endl;
    cout << "----------------------friend list---------------------" << std::endl;
    if (!g_currentUserFriendList.empty())
    {
        for (User &user : g_currentUserFriendList)
        {
            std::cout << user.getId() << " " << user.getName() << std::endl;
        }
    }
    std::cout << "----------------------group list----------------------" << std::endl;
    if (!g_currentUserGroupList.empty())
    {
        for (Group &group : g_currentUserGroupList)
        {
            std::cout << group.getId() << " " << group.getName() << " " << group.getDesc() << std::endl;
            for (GroupUser &user : group.getUsers())
            {
                std::cout << user.getId() << " " << user.getName() << " " << user.getState() << " " << user.getRole() << std::endl;
            }
        }
    }
    std::cout << "======================================================" << std::endl;
}

// 接收线程
void readTaskHandler(int clientfd)
{
    for (;;)
    {
        char buffer[1024] = {0};
        int len = recv(clientfd, buffer, 1024, 0);
        if (len == -1 || len == 0)
        {
            close(clientfd);
            exit(-1);
        }
        // 接收ChatServer转发的数据，反序列化生成json数据对象
        json js = json::parse(buffer);
        int msgtype = js["msgid"].get<int>();
        if (EnMsgType::ONE_CHAT_MSG == msgtype)
        {
            cout << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                 << " said: " << js["msg"].get<string>() << endl;
            continue;
        }

        if (EnMsgType::GROUP_CHAT_MSG == msgtype)
        {
            cout << "群消息[" << js["groupid"] << "]:" << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                 << " said: " << js["msg"].get<string>() << endl;
            continue;
        }

        if (msgtype == EnMsgType::LOGIN_MSG_ACK)
        {
            deLoginResponse(js); // 处理登录的业务逻辑
            sem_post(&rwsem);
            continue;
        }

        if (msgtype == EnMsgType::REG_MSG_ACK)
        {
            doRegResponse(js);
            sem_post(&rwsem);
            continue;
        }
    }
}

void doRegResponse(json &responsejs)
{
    if (responsejs["errno"].get<int>() != 0)
    {
        std::cerr << responsejs["name"] << " is already exist, register error!" << std::endl;
    }
    else
    { // 注册成功
        std::cout << "name register success, userid: " << responsejs["id"]
                  << " name: " << responsejs["name"] << " , do not forget it!" << std::endl;
    }
}

void deLoginResponse(json &responsejs)
{
    if (responsejs["errno"].get<int>() != 0)
    { // 登录失败
        std::cerr << responsejs["errmsg"] << std::endl;
        g_isLoginSuccess = false;
        return;
    }
    else
    {
        // 登录成功
        g_currentUser.setId(responsejs["id"].get<int>());
        g_currentUser.setName(responsejs["name"]);

        // 记录当前用户的好友列表信息
        if (responsejs.contains("friends"))
        {
            g_currentUserFriendList.clear();

            std::vector<std::string> vec = responsejs["friends"];
            for (std::string &str : vec)
            {
                json js = json::parse(str);
                User user;
                user.setId(js["id"].get<int>());
                user.setName(js["name"]);
                user.setState(js["state"]);
                g_currentUserFriendList.push_back(user);
            }
        }

        // 记录当前用户的群组信息
        if (responsejs.contains("groups"))
        {
            g_currentUserGroupList.clear();

            std::vector<std::string> vecl = responsejs["groups"];
            for (std::string groupstr : vecl)
            {
                json grpjs = json::parse(groupstr);
                Group group;
                group.setId(grpjs["id"].get<int>());
                group.setName(grpjs["groupname"]);
                group.setDesc(grpjs["groupdesc"]);

                std::vector<std::string> vec2 = grpjs["users"];
                for (std::string &userstr : vec2)
                {
                    GroupUser user;
                    json js = json::parse(userstr);
                    user.setId(js["id"].get<int>());
                    user.setName(js["name"]);
                    user.setState(js["state"]);
                    user.setRole(js["role"]);
                    group.getUsers().push_back(user);
                }
                g_currentUserGroupList.push_back(group);
            }
        }
        // 显示登录用户的基本信息
        showCurrentUserData();

        // 显示当前用户的离线消息
        if (responsejs.contains("offlinemsg"))
        {
            std::vector<std::string> vec = responsejs["offlinemsg"];
            for (std::string &str : vec)
            {
                json js = json::parse(str);
                // time + [id] + name + " said: " + xxx
                if (EnMsgType::ONE_CHAT_MSG == js["msgid"].get<int>())
                {
                    cout << js["time"].get<string>() << " [" << js["id"] << "]:" << js["name"].get<string>()
                         << " said: " << js["msg"].get<string>() << endl;
                }
                else
                {
                    cout << "群消息[" << js["groupid"] << "]:" << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                         << " said: " << js["msg"].get<string>() << endl;
                }
            }
        }
    }
    g_isLoginSuccess = true;
}

// "help" command handler
void help(int fd = 0, string str = "");
// "chat" command handler
void chat(int, string);
// "addfriend" command handler
void addfriend(int, string);
// "creategroup" command handler
void creategroup(int, string);
// "addgroup" command handler
void addgroup(int, string);
// "groupchat" command handler
void groupchat(int, string);
// "loginout" command handler
void loginout(int, string);

// 系统支持的客户端命令列表
unordered_map<string, string> commandMap = {
    {"help", "显示所有支持的命令，格式help"},
    {"chat", "一对一聊天，格式chat:friendid:message"},
    {"addfriend", "添加好友，格式addfriend:friendid"},
    {"creategroup", "创建群组，格式creategroup:groupname:groupdesc"},
    {"addgroup", "加入群组，格式addgroup:groupid"},
    {"groupchat", "群聊，格式groupchat:groupid:message"},
    {"loginout", "注销，格式loginout"}};

// 注册系统支持的客户端命令处理
unordered_map<string, function<void(int, string)>> commandHandlerMap = {
    {"help", help},
    {"chat", chat},
    {"addfriend", addfriend},
    {"creategroup", creategroup},
    {"addgroup", addgroup},
    {"groupchat", groupchat},
    {"loginout", loginout}};

// 主聊天页面程序
void mainMenu(int clientfd)
{
    help();

    char buffer[1024] = {0};
    while (isMainMenuRunning)
    {
        cin.getline(buffer, 1024);
        string commandbuf(buffer);
        string command; // 存储命令
        int idx = commandbuf.find(":");
        if (-1 == idx)
        {
            command = commandbuf;
        }
        else
        {
            command = commandbuf.substr(0, idx);
        }
        auto it = commandHandlerMap.find(command);
        if (it == commandHandlerMap.end())
        {
            cerr << "invalid input command!" << endl;
            continue;
        }

        // 调用相应命令的事件处理回调，mainMenu对修改封闭，添加新功能不需要修改该函数
        it->second(clientfd, commandbuf.substr(idx + 1, commandbuf.size() - idx)); // 调用命令处理方法
    }
}

// "help" command handler
void help(int, string)
{
    cout << "show command list >>> " << endl;
    for (auto &p : commandMap)
    {
        cout << p.first << " : " << p.second << endl;
    }
    cout << endl;
}
// "addfriend" command handler
void addfriend(int clientfd, string str)
{
    int friendid = atoi(str.c_str());
    json js;
    js["msgid"] = EnMsgType::ADD_FRIEND_MSG;
    js["id"] = g_currentUser.getId();
    js["friendid"] = friendid;
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send addfriend msg error -> " << buffer << endl;
    }
}
// "chat" command handler
void chat(int clientfd, string str)
{
    int idx = str.find(":"); // friendid:message
    if (-1 == idx)
    {
        cerr << "chat command invalid!" << endl;
        return;
    }

    int friendid = atoi(str.substr(0, idx).c_str());
    string message = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = EnMsgType::ONE_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["toid"] = friendid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    string buffer = js.dump();
    // std::cout << "js: " << js <<" buffer: "<<
    // buffer <<" buffer.c_str(): " << buffer.c_str() << std::endl;

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send chat msg error -> " << buffer << endl;
    }
}
// "creategroup" command handler  groupname:groupdesc
void creategroup(int clientfd, string str)
{
    int idx = str.find(":");
    if (-1 == idx)
    {
        cerr << "creategroup command invalid!" << endl;
        return;
    }

    string groupname = str.substr(0, idx);
    string groupdesc = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = EnMsgType::CREATE_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send creategroup msg error -> " << buffer << endl;
    }
}
// "addgroup" command handler
void addgroup(int clientfd, string str)
{
    int groupid = atoi(str.c_str());
    json js;
    js["msgid"] = EnMsgType::ADD_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupid"] = groupid;
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send addgroup msg error -> " << buffer << endl;
    }
}
// "groupchat" command handler   groupid:message
void groupchat(int clientfd, string str)
{
    int idx = str.find(":");
    if (-1 == idx)
    {
        cerr << "groupchat command invalid!" << endl;
        return;
    }

    int groupid = atoi(str.substr(0, idx).c_str());
    string message = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = EnMsgType::GROUP_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["groupid"] = groupid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send groupchat msg error -> " << buffer << endl;
    }
}
// "loginout" command handler
void loginout(int clientfd, string)
{
    json js;
    js["msgid"] = EnMsgType::LOGINOUT_MSG;
    js["id"] = g_currentUser.getId();
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "send loginout msg error -> " << buffer << endl;
    }
    else
    {
        isMainMenuRunning = false;
    }
}

// 获取系统时间（聊天信息需要添加时间信息）
std::string getCurrentTime()
{
    auto nowtime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *ptm = localtime(&nowtime);
    char date[60] = {0};
    sprintf(date, "%d-%02d-%02d %02d:%02d%02d",
            (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
            (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);

    return std::string(date);
}