#include <iostream>
#include <vector>
#include <string>
#include "groupmodel.hpp"
#include "db.h"

// 创建群组, 引用传递
bool GroupModel::createGroup(Group &group)
{
    // 组装sql
    char sql[1024] = {0};
    sprintf(sql, "insert into allgroup(groupname, groupdesc) values('%s','$s')",
            group.getName().c_str(), group.getDesc().c_str());
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

// 加入群组
void GroupModel::addGroup(int userid, int groupid, std::string role)
{
    // 组装sql
    char sql[1024] = {0};
    sprintf(sql, "insert into groupuser values(%d,%d,'%s')", groupid, userid, role.c_str());
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

// 查询用户所在群组信息, 好友列表，群组消息
std::vector<Group> GroupModel::queryGroups(int userid)
{
    // 组装sql
    /*
        1. 先根据userid在groupuser表中查询该用户所属的群组信息
        2. 根据群组信息，查询该组的所有用户的userid，并且和user表进行联合查询， 查出用户的详细信息
    */
    char sql[1024] = {0};
    sprintf(sql, "select a.id, a.groupname, a.groupdesc from allgroup a inner join \
            groupuser b on a.id = b.groupid where b.userid = %d",
            userid);
    std::vector<Group> groupvec;
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res;
        res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                groupvec.emplace_back(group);
            }
            mysql_free_result(res);
        }
    }
    // 查询各组所拥有的成员列表
    for (auto &it : groupvec)
    {
        sprintf(sql, "select a.id , a.name, a.state , b.grouprole from user a\
            inner join groupuser b on b.userid=a.id where b.groupid = %d",
                it.getId());
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                GroupUser user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                user.setRole(row[3]);
                it.getUsers().emplace_back(user);
            }
            mysql_free_result(res);
        }
    }

    // 返回组信息和组成员信息
    return groupvec;
}

// 根据指定的groupid查询群组用户id列表， 除了userid自己，主要用于用户群聊业务给群组其他成员发送消息
std::vector<int> GroupModel::queryGroupUsers(int userid, int groupid)
{

    // 组装sql
    char sql[1024] = {0};
    sprintf(sql, "select userid from groupuser  where  groupid = %d and userid != %d",
            groupid, userid);
    std::vector<Group> groupvec;
    MySQL mysql;
    std::vector<int> ret;
    if (mysql.connect())
    {
        MYSQL_RES *res;
        res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                ret.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
        }
    }
    return ret;
}