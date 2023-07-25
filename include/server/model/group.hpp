#ifndef GROUP_H
#define GROUP_H
#include <iostream>
#include <vector>
#include <string>
#include "groupuser.hpp"

// allgroup表ORM映射
class Group
{
public:
    Group(int id = -1, std::string name = "", std::string desc = "")
    {
        id_ = id;
        name_ = name;
        desc_ = desc;
    }
    void setId(int id) { id_ = id; }
    void setName(std::string name) { name_ = name; }
    void setDesc(std::string desc) { desc_ = desc; }


    int getId() { return id_; }
    std::string getName() { return name_; };
    std::string getDesc() { return desc_; };
    //业务层获取成员时，groupmodel将获取的成员存储在group的users中
    std::vector<GroupUser> &getUsers() { return users_; } 

private:
    int id_;                       // group的id
    std::string name_;             // group的name
    std::string desc_;             // group的描述信息
    std::vector<GroupUser> users_; // 该组拥有成员
};

#endif