#include "usermodel.hpp"
#include "db.h"
#include <iostream>

/*
    user表的增加方法 , 注意传递的是引用，
    比如id是auto_increment的，因此只有insert后才知道对应id
*/
bool UserModel::insert(User &user)
{
    // 1. 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into user(name , password, state) values('%s','%s','%s')",
            user.getName().c_str(), user.getPwd().c_str(), user.getState().c_str());
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            // 获取插入成功的用户数据生成的主键id
            // 利用mysql.h中提供的方法
            auto id = mysql_insert_id(mysql.getConnection());
            user.setId(id);
            return true;
        }
    }
    return false;
}

// 根据用户传入的id返回id对应信息
User UserModel::query(int id)
{

    // 1. 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select * from user where id = %d", id);
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr)
            { // 查到数据
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);
                mysql_free_result(res);
                return user;
            }
            mysql_free_result(res);
        }
    }
    return User();
}

// 更新用户的状态信息
bool UserModel::updateState(User user)
{
    // 1. 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "update user set  state = '%s' where id = %d",
            user.getState().c_str(), user.getId());
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            std::cout <<"mysql.update" << std::endl;
            return true;
        }
    }
    return false;
}

// 重置用户的状态信息
void UserModel::resetState()
{
    // 1. 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "update user set  state = '%s' where state ='online'",
            "offline");
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}