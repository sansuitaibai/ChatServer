#ifndef DB_H
#define DB_H
#include <iostream>
#include <mysql/mysql.h>
#include <string>
#include <ctime>
#include "public.hpp"

// 数据库的配置信息
static std::string server = "127.0.0.1";
static std::string user = "root";
static std::string password = "123456";
static std::string dbname = "chat";

// //打印日志
// #define LOG(str) \
// 	std::cout << __FILE__ <<":" << __LINE__ <<" " << \
// 	__TIMESTAMP__ <<" : " << str << std::endl;

/*
    实现mysql连接模块
*/
class MySQL
{
public:
    // 初始化数据库连接
    MySQL();
    // 释放数据库连接资源
    ~MySQL();

    // 连接数据库
    bool connect(std::string ip = server, unsigned short port = 3306,
                        std::string user = user, std::string password = password,
                        std::string dbname = dbname);

    // 更新操作 insert、delete、update
    bool update(std::string sql);

    // 查询操作 select
    MYSQL_RES *query(std::string sql);

    // 返回连接的Mysql对象
    MYSQL *getConnection();

    bool print(MYSQL_RES *res);

    void free_result(MYSQL_RES *res);

    // 刷新连接的最后进入连接队列中的时间
    void refreshAliveTime();

    // 返回连接的空闲的时间
    clock_t getAliveTime() const;

private:
    MYSQL *conn_;       // 表示和Mysql Server的一条连接
    clock_t alivetime_; // 记录连接进入，重新进入连接队列中每一次的时间
};

#endif
