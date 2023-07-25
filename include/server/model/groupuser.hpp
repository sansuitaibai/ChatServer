#ifndef GROUPUSER_H
#define GROUPUSE_H

#include <iostream>
#include <vector>
#include <string>
#include "user.hpp"

// 表groupuser 的ORM映射
class GroupUser : public User
{

public:
    void setRole(std::string role) { role_ = role; }
    std::string getRole() { return role_; }

private:
    std::string role_; // 组成员的角色
};

#endif