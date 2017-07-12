#ifndef ROBOT_H
#define ROBOT_H

#include <string.h>
#include "message.hpp"
#include "private_message.hpp"
#include "group_message.hpp"
#include "message_sender.hpp"
#include "stringutil.hpp"
#include "functions/function.hpp"
#include "functions/osu_query/osu_query.hpp"
#include "functions/blacklist.hpp"
#include "functions/manual.hpp"
#include "functions/interpreter.hpp"

using namespace QQRobot;

namespace QQRobot
{
    class Robot
    {
    public:
        string qq;
        string masterQQ;

        BlackList blacklist;
        OsuQuery osuQuery;
        Interpreter interpreter;
        Manual man;

        MessageSender *sender;

        Robot()
        {
            qq = "3381775672";
            masterQQ = "1013644379";
        }

        Robot(MessageSender *sender)
        {
            Robot();
            this->sender = sender;
        }

        CQ_EVENT_RET onPrivateMessage(PrivateMessage fromMsg)
        {
            string fromContent = fromMsg.getContent();

            PrivateMessage toMsg;

            // �����Ϣ�����������ˣ��ͰѸ���Ϣת��������
            if (fromMsg.from != masterQQ)
            {
                toMsg.to = masterQQ;
                toMsg.setContent("QQ" + fromMsg.from + "��Ϣ: " + fromContent);
                sender->sendPrivateMessage(toMsg);
                return EVENT_BLOCK;
            }

            Function *func = NULL;

            // ִ��Ⱥ����Ϣ��������﷨:!sendtogroup Ŀ��Ⱥ�� [ĳ��QQ��] �ݲ�֧�ֿո����Ϣ
            if (fromContent.find("!sendtogroup") == 0)
            {
                // ֻ�������д�����Ȩ��
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
            // ִ��˽����Ϣ��������﷨:!send ĳ��QQ�� �ݲ�֧�ֿո����Ϣ
            else if (fromContent.find("!send") == 0)
            {
                // ֻ�������д�����Ȩ��
                if (fromMsg.from != masterQQ)
                    return EVENT_IGNORE;

                vector<string> strs = stringutil::split(fromContent, " ");
                PrivateMessage toMsg;
                toMsg.to = strs[1];
                toMsg.setContent(strs[2]);
                sender->sendPrivateMessage(toMsg);
            }
            else if (fromContent.find("!blacklist") != string::npos)
                func = (Function*)&blacklist;

            if (func != NULL)
            {
                func->sender = sender;
                func->robot = this;
                bool handleBlock = func->handleMessage(fromMsg, toMsg);
                if (handleBlock)
                    return EVENT_BLOCK;
            }

            return EVENT_IGNORE;
        }

        CQ_EVENT_RET onGroupMessage(GroupMessage fromMsg)
        {
            string fromContent = fromMsg.getContent();

            GroupMessage toMsg;
            toMsg.type = fromMsg.type;

            // �ظ�
            toMsg.to = fromMsg.groupQQ;
            int index;

            // �Լ���AT
            bool atMe = fromMsg.getAtQQ() == this->qq;
            if (atMe)
            {
                //ҲAT�Է�
                toMsg.setAtQQ(fromMsg.from);

                if ((index = fromContent.find("����")) != string::npos)
                {
                    toMsg.setContent(Function::functionInfo());
                    sender->sendGroupMessage(toMsg);
                    return EVENT_BLOCK;
                }
            }

            if (checkIsInBlackList(fromMsg, toMsg))
                return EVENT_BLOCK;

            Function *func = NULL;

            if (fromContent.find("!man") != string::npos)
                func = (Function*)&man;
            else if (fromContent.find("!stat") != string::npos)
                func = (Function*)&osuQuery;
            else if(fromContent.find("eval:") != string::npos)
                func = (Function*)&interpreter;
            else if (fromContent.find("!blacklist") != string::npos)
                func = (Function*)&blacklist;
            else if(atMe)
            {
                // echo
                toMsg.setContent(fromMsg.atContent());
                sender->sendGroupMessage(toMsg);
                return EVENT_BLOCK;
            }

            if (func != NULL)
            {
                func->sender = sender;
                func->robot = this;
                bool handleBlock = func->handleMessage(fromMsg, toMsg);
                if (handleBlock)
                    return EVENT_BLOCK;
            }

            RET:
            return EVENT_IGNORE;
        }

        CQ_EVENT_RET onDiscussMessage(GroupMessage fromMsg)
        {
            fromMsg.type = 1;
            return onGroupMessage(fromMsg);
        }

    private:
        bool checkIsInBlackList(GroupMessage &fromMsg, GroupMessage &toMsg)
        {
            if (blacklist.exist(fromMsg.from) || blacklist.exist("all"))
            {
                toMsg.setAtQQ(fromMsg.from);
                toMsg.setContent("���Ѿ����ؽ�С������:C");
                sender->sendGroupMessage(toMsg);
                return true;
            }
            return false;
        }
    };
}

#endif