#ifndef ROBOT_MANUAL_H
#define ROBOT_MANUAL_H

#include <string>
#include <map>
#include "function.hpp"

using namespace std;
using namespace QQRobot;

namespace QQRobot
{
    class Manual : public Function
    {
    public:
        Manual()
        {
            // ��ʼ�������ֲ���Ϣ
            string statInfo;
            statInfo = "��ѯosu!�û�ͳ����Ϣ��\n";
            statInfo = statInfo
                + "!stat <�û���> *[ģʽ]\n"
                + "ģʽ��0 = osu!/std, 1 = Taiko, 2 = CTB, 3 = osu!mania/mania"
                + "���ò����ǿ�ѡ�ģ�Ĭ��Ϊ0�����������ֻ���ӦӢ������Ӣ������ĸ\n"
                + "���磬��ѯctbͳ����Ϣ��!stat WubWoofWolf *c";
            manInfoMap["stat"] = statInfo;
        }
        Manual(MessageSender *sender, Robot *robot) : Function(sender, robot)
        {
            Manual();
        }

        bool handleMessage(Message &fromMsg, Message &toMsg)
        {
            vector<string> strs = stringutil::split(fromMsg.getContent(), " ");
            if (strs.size() > 1)
            {
                string cmd = strs[1];
                toMsg.setContent(manInfoMap[cmd]);
                sender->sendMessage(toMsg);
                return true;
            }
            return false;
        }


    private:
        map<string, string> manInfoMap;

    };
}

#endif