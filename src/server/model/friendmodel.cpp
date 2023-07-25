
#include "friendmodel.hpp"
#include "db.h"

// 添加好友关系, 好友列表一般在客户端进行存储
void FriendModel::insert(int userid, int friendid)
{
    // 1. 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into friend(userid, friendid) values(%d,%d)",
            userid, friendid);
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

// 返回用户好友列表
std::vector<User> FriendModel::query(int userid)
{
    std::vector<User> ret;
    // 1. 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, R"(select a.id,a.name ,a.state from user a inner join 
        friend b on a.id = b.friendid where b.userid = %d)", userid);
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        MYSQL_ROW row;
        // 获取所有的离线消息
        while ((row = mysql_fetch_row(res)) != nullptr)
        {
            User user;
            user.setId(atoi(row[0]));
            user.setName(row[1]);
            user.setState(row[2]);
            ret.push_back(user);
        }
        mysql_free_result(res);
    }
    return ret;
}