#ifndef PUBLIC_H
#define PUBLIC_H

/*
    server和client的公共文件
*/
//消息的类型
/*
    LOGIN_MSG: {"msgid":1, "id":13, "password":"123456"}
    REG_MSG: {"msgid":3, "name":"lisi", "password":"123456"}
    ONE_CHAT_MSG: {"msgid":5, "id":发送者的id , "name": 发送者的姓名, "to":接受者的id, "msg": 消息}
    ADD_FRIEND_MSG: {"msgid":6, "id": , "fiendid": }

*/

enum EnMsgType{ LOGIN_MSG = 1 //登录消息
            , LOGIN_MSG_ACK // 2
            , REG_MSG  //注册消息  3
            , REG_MSG_ACK //注册响应 4
            , ONE_CHAT_MSG //点对点聊天 5
            , ADD_FRIEND_MSG //添加好友 6

            , CREATE_GROUP_MSG //创建群组
            , ADD_GROUP_MSG //加入群组
            , GROUP_CHAT_MSG //群聊天
            , LOGINOUT_MSG //
            
        };




#endif 