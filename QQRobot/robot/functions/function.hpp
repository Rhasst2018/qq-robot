#ifndef ROBOT_FUNCTION_H
#define ROBOT_FUNCTION_H

#include <string>
#include "../message.hpp"
#include "../message_sender.hpp"

using namespace std;
using namespace QQRobot;

namespace QQRobot
{
    /*
    �����˹��ܳ���
    */
    class Function
    {
    public:
        MessageSender *sender;
        Robot *robot;

        Function() {}
        Function(MessageSender *sender, Robot *robot)
        {
            this->sender = sender;
            this->robot = robot;
        }

        /*
        ������Ϣ
        return: �Ƿ񲻺���
        */
        virtual bool handleMessage(Message &fromMsg, Message &toMsg) = 0;

        static string functionInfo()
        {
            string info = "�ҵĹ������£�\n";
            info += "  * �ҿ���ִ��JS���򣬷��ͣ�eval: <JS����>`�����磺eval: 1+2��ע��ֺ���Ӣ�ĵģ��ֺź��������հס�\n";
            info += "  * �����@�ң���Ҳ��@�㡣\n";
            info += "  * osu!��ѯ: 1. !stat\n";
            info += "  * �����÷���ѯ: !man <������>�����磺!man stat";
            return info;
        }
    };
}

#endif // ! FUNCTION_H
