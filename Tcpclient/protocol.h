#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef unsigned int uint;

#define REGIST_OK "regist ok"
#define REGIST_FAILED "regist failed : name existed"

#define LOGIN_OK "login ok"
#define LOGIN_FAILED "login failed : name error or pwd error or relogin"

#define SEARCH_USR_NO "no such people"
#define SEARCH_USR_ONLINE "online"
#define SEARCH_USR_OFFLINE "offline"

#define UNKNOW_ERROR "unknow error"
#define EXISTED_FRIEND "friend exist"
#define ADD_FRIEND_OFFLINE "usr offline"
#define ADD_FRIEND_NOT_EXIST "usr not exist"

#define DELETE_FRIEND_OK "delete friend ok"

enum ENUM_MSG_TYPE
{
    ENUM_MSG_TYPE_MIN=0,
    ENUM_MSG_TYPE_REGIST_REQUEST, // 注册请求
    ENUM_MSG_TYPE_REGIST_RESPOND, // 注册回复

    ENUM_MSG_TYPE_LOGIN_REQUEST, // 登录请求
    ENUM_MSG_TYPE_LOGIN_RESPOND, // 登录回复

    ENUM_MSG_TYPE_ALL_ONLINE_REQUEST, // 返回所有在线用户请求
    ENUM_MSG_TYPE_ALL_ONLINE_RESPOND, // 返回所有在线用户回复

    ENUM_MSG_TYPE_SEARCH_USR_REQUEST, // 搜索用户请求
    ENUM_MSG_TYPE_SEARCH_USR_RESPOND, // 搜索用户回复

    ENUM_MSG_TYPE_ADD_FRIEND_REQUEST, // 添加用户请求
    ENUM_MSG_TYPE_ADD_FRIEND_RESPOND, // 添加用户回复

    ENUM_MSG_TYPE_ADD_FRIEND_AGREE, // 同意好友回复
    ENUM_MSG_TYPE_ADD_FRIEND_REFUSE,// 拒绝好友回复

    ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST, // 刷新好友请求
    ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND, // 刷新好友回复

    ENUM_MSG_TYPE_DEL_FRIEND_REQUEST, // 删除好友请求
    ENUM_MSG_TYPE_DEL_FRIEND_RESPOND, // 删除好友回复
//    ENUM_MSG_TYPE_REQUEST,
//    ENUM_MSG_TYPE_RESPOND,
//    ENUM_MSG_TYPE_REQUEST,
//    ENUM_MSG_TYPE_RESPOND,
//    ENUM_MSG_TYPE_REQUEST,
//    ENUM_MSG_TYPE_RESPOND,
//    ENUM_MSG_TYPE_REQUEST,
//    ENUM_MSG_TYPE_RESPOND,
    ENUM_MSG_TYPE_MAX=0x00ffffff
};

struct PDU
{
    uint uiPDULen;  // 总的协议数据单元大小
    uint uiMsgType; // 消息类型
    char caData[64];
    uint uiMsgLen;  // 实际消息长度
    int caMsg[];    // 实际消息

};

PDU *mkPDU(uint uiMsgLen);

#endif // PROTOCOL_H
