#ifndef USER_H
#define USER_H
#include<iostream>
#include<string>

//属于User表的映射类
class User
{

public:
    User(int id = -1, std::string name = "", std::string pwd="", std::string state = "offline")
    {
        id_ = id;
        name_ = name;
        password_ = pwd;
        state_ = state;
    }

    //设置
    void setId(int id){ id_ = id;}
    void setName(std::string name){ name_ = name;}
    void setPwd(std::string pwd){ password_ = pwd;}
    void setState(std::string state){ state_ = state;}


    //获取
    int getId(){ return id_;}
    std::string getName(){ return name_ ;}
    std::string  getPwd(){ return password_ ;}
    std::string  getState(){ return state_ ;}

protected:
    int id_;
    std::string name_;
    std::string password_;
    std::string state_;
};




#endif 