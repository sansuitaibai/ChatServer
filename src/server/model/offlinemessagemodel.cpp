#include "offlinemessagemodel.hpp"
#include "db.h"
#include <iostream>

// 存储用户的离线消息
void OfflineMsgModel::insert(int userid, std::string msg)
{    // 1. 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into offlinemessage(userid, message) values(%d,'%s')",
            userid, msg.c_str());
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
   

}

// 删除用户的离线消息
void OfflineMsgModel::remove(int userid)
{
     // 1. 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "delete from  offlinemessage where userid = %d",
            userid);
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

// 查询用户的离线消息
std::vector<std::string> OfflineMsgModel::query(int userid)
{
    std::vector<std::string> ret;
         // 1. 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select message from  offlinemessage where userid = %d",
            userid);
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES* res = mysql.query(sql);
        MYSQL_ROW row;
        //获取所有的离线消息
        while((row = mysql_fetch_row(res)) != nullptr )
        {
            ret.push_back(row[0]);
        }
        mysql_free_result(res);
    }
    return ret;
}
