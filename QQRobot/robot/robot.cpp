﻿/*
机器人，监听消息事件
author: hulang
*/
#include "robot.h"
#include "functions/function.hpp"
#include "functions/osu_query/osu_query.hpp"
#include "functions/blacklist.hpp"
#include "functions/manual.hpp"
#include "functions/weather_forecast.hpp"
#include "functions/interpreter.hpp"

using namespace QQRobot;

Robot::Robot()
{
    qq = "3381775672";
    masterQQ = "1013644379";

    man = new Manual();
    osuQuery = new OsuQuery();
    blacklist = new BlackList();
    weatherForecast = new WeatherForecast();
    interpreter = new Interpreter();
}

Robot::Robot(MessageSender *sender)
{
    Robot();
    this->sender = sender;
}

Robot::~Robot()
{
    delete man;
    delete osuQuery;
    delete blacklist;
    delete interpreter;
}

CQ_EVENT_RET Robot::onPrivateMessage(PrivateMessage &fromMsg)
{
    string fromContent = fromMsg.getContent();

    PrivateMessage toMsg;
    toMsg.to = fromMsg.from;

    // 如果消息不是来自主人，就把该消息转发给主人
    if (fromMsg.from != masterQQ)
    {
        toMsg.to = masterQQ;
        toMsg.setContent("QQ" + fromMsg.from + "消息: " + fromContent);
        sender->sendPrivateMessage(toMsg);
        return EVENT_BLOCK;
    }

    Function *func = NULL;

    // 执行群组消息代发命令，语法:!sendtogroup 目标群号 [某人QQ号] 暂不支持空格的消息
    if (fromContent.find("!sendtogroup") == 0)
    {
        // 只有主人有此命令权限
        if (fromMsg.from != masterQQ)
            return EVENT_IGNORE;

        vector<string> strs = stringutil::split(fromContent, " ");
        GroupMessage toGpMsg;
        toGpMsg.to = strs[1];
        if (strs.size() > 3)
        {
            toGpMsg.setAtQQ(strs[2]);
            toGpMsg.setContent(strs[3]);
        }
        else
        {
            toGpMsg.setContent(strs[2]);
        }
        sender->sendGroupMessage(toGpMsg);
    }
    // 执行私聊消息代发命令，语法:!send 某人QQ号 暂不支持空格的消息
    else if (fromContent.find("!send") == 0)
    {
        // 只有主人有此命令权限
        if (fromMsg.from != masterQQ)
            return EVENT_IGNORE;

        vector<string> strs = stringutil::split(fromContent, " ");
        PrivateMessage toMsg;
        toMsg.to = strs[1];
        toMsg.setContent(strs[2]);
        sender->sendPrivateMessage(toMsg);
    }
    else if (fromContent.find("!blacklist") != string::npos)
        func = (Function*)blacklist;

    if (func != NULL)
    {
        func->robot = this;
        bool handleBlock = func->handleMessage(fromMsg, toMsg);
        if (handleBlock)
            return EVENT_BLOCK;
    }

    return EVENT_IGNORE;
}

CQ_EVENT_RET Robot::onGroupMessage(GroupMessage &fromMsg)
{
    string fromContent = fromMsg.getContent();

    GroupMessage toMsg;
    toMsg.type = fromMsg.type;

    // 回复
    toMsg.to = fromMsg.groupQQ;
    int index;

    // 自己被AT
    bool atMe = fromMsg.getAtQQ() == this->qq;
    if (atMe)
    {
        //也AT对方
        toMsg.setAtQQ(fromMsg.from);

        if ((index = fromContent.find("功能")) != string::npos)
        {
            toMsg.setContent(Function::functionInfo());
            sender->sendGroupMessage(toMsg);
            return EVENT_BLOCK;
        }
    }

    Function *func = NULL;

    if (fromContent.find("!man") != string::npos)
        func = (Function*)man;
    else if (fromContent.find("!stat") != string::npos)
        func = (Function*)osuQuery;
    else if (fromContent.find("eval:") != string::npos)
        func = (Function*)interpreter;
    else if (fromContent.find("!blacklist") != string::npos)
        func = (Function*)blacklist;
    else if (fromContent.find("天气") != string::npos)
        func = (Function*)weatherForecast;
    else if (atMe)
    {
        // echo
        toMsg.setContent(fromMsg.atContent());
        sender->sendGroupMessage(toMsg);
        return EVENT_BLOCK;
    }

    if (func != NULL)
    {
        if (checkIsInBlackList(fromMsg, toMsg))
            return EVENT_BLOCK;
        func->robot = this;
        bool handleBlock = func->handleMessage(fromMsg, toMsg);
        if (handleBlock)
            return EVENT_BLOCK;
    }

    return EVENT_IGNORE;
}

CQ_EVENT_RET Robot::onDiscussMessage(GroupMessage &fromMsg)
{
    fromMsg.type = 1;
    return onGroupMessage(fromMsg);
}

bool Robot::checkIsInBlackList(GroupMessage &fromMsg, GroupMessage &toMsg)
{
    if (blacklist->exist(fromMsg.from) || blacklist->exist("all"))
    {
        toMsg.setAtQQ(fromMsg.from);
        toMsg.setContent("你已经被关进小黑屋了:C");
        sender->sendGroupMessage(toMsg);
        return true;
    }
    return false;
}
