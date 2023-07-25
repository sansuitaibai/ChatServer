#ifndef USERMODEL_H
#define USERMODEL_H

#include"user.hpp"

//对应User表的数据操作类
class UserModel
{
public:
    /*
        user表的增加方法 , 注意传递的是引用，
        比如id是auto_increment的，因此只有insert后才知道对应id
    */
    bool insert(User& user);
    
    //根据用户传入的id返回id对应信息
    User query(int id);

    //更新用户的状态信息
    bool updateState(User user);

    //重置用户的状态信息
    void resetState();

private:





};


#endif