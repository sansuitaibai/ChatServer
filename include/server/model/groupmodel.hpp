#ifndef GROUPMODEL_H
#define GROUPMODEL_H
#include<iostream>
#include<string>
#include<vector>
#include"group.hpp"


//allgroup, groupuser表的操作
class GroupModel
{

public:
    //创建群组
    bool createGroup(Group &group);
    //加入群组
    void addGroup(int userid, int groupid, std::string role);
    //查询用户所在群组信息
    std::vector<Group> queryGroups(int userid);
    //根据指定的groupid查询群组用户id列表， 除了userid自己，主要用于用户群聊业务给群组其他成员发送消息
    std::vector<int> queryGroupUsers(int userid , int groupid);

private:


};






#endif