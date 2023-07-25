#include "db.h"
#include <muduo/base/Logging.h> //日志功能


/*
    实现mysql连接模块
*/

// 初始化数据库连接
MySQL::MySQL()
{
    conn_ = mysql_init(NULL);
    if (conn_ == NULL)
        LOG_INFO << "mysql_init is fail!";
    ;
    alivetime_ = 0;
}
// 释放数据库连接资源
MySQL::~MySQL()
{
    if (conn_ != nullptr)
        mysql_close(conn_);
}
// 连接数据库
bool MySQL::connect(std::string ip, unsigned short port ,
                    std::string user , std::string password , std::string dbname )
{
    MYSQL *p = mysql_real_connect(conn_, ip.c_str(), user.c_str(),
                                  password.c_str(), dbname.c_str(), port, nullptr, 0);
    if (p == NULL)
    {
        LOG_INFO << "mysql_real_connect is fail!";
        return false;
    }
    // c/c++代码默认的字符编码为ASCII , 不能不设置，从MySql拉取下来的中文显示 ？
    mysql_query(conn_, "set name gbk");
    LOG_INFO << "connect mysql success!";
    return true;
}
// 更新操作 insert、delete、update
bool MySQL::update(std::string sql)
{
    if (mysql_query(conn_, sql.c_str()) != 0)
    {
        LOG_INFO << "更新失败:" << sql;
        return false;
    }
    return true;
}
// 查询操作 select
MYSQL_RES *MySQL::query(std::string sql)
{
    if (mysql_query(conn_, sql.c_str()) != 0)
    {
        LOG_INFO << "查询失败:" << sql;
        return nullptr;
    }
    return mysql_use_result(conn_);
}

// 返回连接的Mysql对象
MYSQL *MySQL::getConnection()
{
    return conn_;
}

bool MySQL::print(MYSQL_RES *res)
{
    if (res == nullptr)
    {
        LOG_INFO << "Connection::print";
        return false;
    }
    int num = mysql_num_fields(res);
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)) != NULL)
    {
        for (int i = 0; i < num; ++i)
        {
            std::cout << row[i] << "\t\t";
        }
        std::cout << std::endl;
    }
    return true;
}

void MySQL::free_result(MYSQL_RES *res)
{
    if (res == NULL)
        return;
    mysql_free_result(res);
}

// 刷新连接的最后进入连接队列中的时间
void MySQL::refreshAliveTime()
{
    alivetime_ = std::clock();
}

// 返回连接的空闲的时间
clock_t MySQL::getAliveTime() const
{
    return std::clock() - alivetime_;
}
