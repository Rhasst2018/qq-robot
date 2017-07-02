#include "group_message.hpp"
#include "message.hpp"
#include "message_sender.hpp"
#include "osu_query/osu_query.hpp"
#include "interpreters/js.hpp"
#include "interpreters/scheme.hpp"
#include "blacklist.hpp"
#include "stringutil.hpp"
#include <string.h>

using namespace Interpreters;

namespace QQRobot
{
    class Robot
    {
    public:
        MessageSender sender;

        Robot() { }
        Robot(MessageSender sender)
        {
            this->sender = sender;
        }

        void setQQ(string qq)
        {
            this->qq = qq;
        }

        CQ_EVENT_RET onPrivateMessage(Message fromMsg)
        {
            string fromContent = fromMsg.getContent();

            Message toMsg;

            // �����Ϣ�����������ˣ��ͰѸ���Ϣת��������
            if (fromMsg.from != masterQQ)
            {
                toMsg.to = masterQQ;
                toMsg.setContent("QQ" + fromMsg.from + "��Ϣ: " + fromContent);
                sender.sendPrivateMessage(toMsg);
                return EVENT_BLOCK;
            }

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
                sender.sendGroupMessage(toGpMsg);
            }
            // ִ��˽����Ϣ��������﷨:!send ĳ��QQ�� �ݲ�֧�ֿո����Ϣ
            else if (fromContent.find("!send") == 0)
            {
                // ֻ�������д�����Ȩ��
                if (fromMsg.from != masterQQ)
                    return EVENT_IGNORE;

                vector<string> strs = stringutil::split(fromContent, " ");
                GroupMessage toMsg;
                toMsg.to = strs[1];
                toMsg.setContent(strs[2]);
                sender.sendPrivateMessage(toMsg);
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
                    toMsg.setContent(functionInfo());
                    sender.sendGroupMessage(toMsg);
                    return EVENT_BLOCK;
                }
            }

            if (fromContent.find("!stat") == 0)
            {
                //��ѯ
                //wstring result = OsuQuery::query(fromContent);
                //toMsg.setContent(result);
                toMsg.setContent("��δ����");
                sender.sendGroupMessage(toMsg);
                return EVENT_BLOCK;
            }
            else if((index = fromContent.find("eval:")) != string::npos)
            {
                if (blacklist.exist(fromMsg.from) || blacklist.exist("all"))
                {
                    toMsg.setAtQQ(fromMsg.from);
                    toMsg.setContent("���Ѿ����ؽ�С������:C");
                    sender.sendGroupMessage(toMsg);
                    return EVENT_BLOCK;
                }

                string code = fromContent.substr(index + 5);
                string result;

                if (defaultEvalLanguage == "js")
                {
                    try
                    {
                        // ��Ϣ�е�ĳЩ�ַ������룬����'[��']'���ֱ�ת������&#91;��&#93;�������������������
                        // ����Ƿ�������
                        if (isBadCode(code)) {
                            toMsg.setAtQQ(fromMsg.from);
                            blacklist.addQQ(fromMsg.from);
                            toMsg.setContent("�㷢���Ƕ�����룬���Ѿ����ؽ�С�����ˣ�");
                            sender.sendGroupMessage(toMsg);
                            return EVENT_BLOCK;
                        }
                        code = toCode(code);
                        // ת����Utf8
                        code = stringutil::string_To_UTF8(code);
                        // ִ�д��룬��ȡ���
                        result = js.evalForUTF8(code);
                        // ת����string
                        result = stringutil::UTF8_To_string(result);
						result = result != "" ? result : " "; //�����֧�ַ��Ϳ�
                        
                    }
                    catch (exception &e)
                    {
                        string info = "�����쳣��:\n";
                        info += e.what();
                        result = info;
                    }
                }
                else if (defaultEvalLanguage == "scheme")
                {
                    result = scheme.eval(code);
                }

                toMsg.setContent(result);
                sender.sendGroupMessage(toMsg);
                return EVENT_BLOCK;
            }
            else if (fromContent.find("!blacklist") != string::npos)
            {
                if (fromMsg.from != masterQQ)
                {
                    toMsg.setAtQQ(fromMsg.from);
                    toMsg.setContent("��Ȩ��");
                    sender.sendGroupMessage(toMsg);
                    return EVENT_BLOCK;
                }
                vector<string> strs = stringutil::split(fromContent, " ");
                if (strs.size() < 2)
                    goto RET;
                string operatorStr = strs[1];

                if (operatorStr == "add" && strs.size() >= 3)
                {
                    string argStr = GroupMessage::tryGetQQFromAtContent(strs[2]);
                    blacklist.addQQ(argStr);
                    toMsg.setContent(argStr + " �ɹ��ؽ�С����");
                }
                else if (operatorStr == "del" && strs.size() >= 3)
                {
                    string argStr = GroupMessage::tryGetQQFromAtContent(strs[2]);
                    blacklist.delQQ(argStr);
                    toMsg.setContent(argStr + " �ɹ���С�����ͷ�");
                }
                else if (operatorStr == "list")
                {
                    toMsg.setContent(blacklist.empty() ? "��������" : "������:\n" + blacklist.toMutilLineStr(" "));
                }
                else if (operatorStr == "clear")
                {
                    blacklist.clear();
                    toMsg.setContent("�ɹ���պ�����");
                }
                sender.sendGroupMessage(toMsg);
                return EVENT_BLOCK;
            }
            else if ((index = fromContent.find("!set-eval-lang")) != string::npos)
            {
                defaultEvalLanguage = fromContent.substr(index + 14 + 1);
                toMsg.setContent("done");
                sender.sendGroupMessage(toMsg);
                return EVENT_BLOCK;
            }
            else if(atMe)
            {
                // echo
                toMsg.setContent(fromMsg.atContent());
                sender.sendGroupMessage(toMsg);
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
        string qq;
        string defaultEvalLanguage = "js";
        JS js;
		Scheme scheme;
        BlackList blacklist;
        string masterQQ = "1013644379";

        string toCode(string str)
        {
            str = stringutil::replace_all(str, "&#91;", "[");
            str = stringutil::replace_all(str, "&#93;", "]");
            return str;
        }

        bool isBadCode(string code)
        {
            // �򵥵Ķ����
            vector<string> badCodes;
            badCodes.push_back("while(1)");
            badCodes.push_back("while(!0)");
            badCodes.push_back("while(true)");
            badCodes.push_back("while(!false)");
            badCodes.push_back("for(;;)");

            for (vector<string>::iterator it = badCodes.begin(); it != badCodes.end(); it++)
                if (code.find(*it) != string::npos)
                    return true;
            return false;
        }

        string functionInfo()
        {
            string info = "�ҵĹ������£�\n";
            info += "  * �ҿ���ִ��JS���򣬷��ͣ�eval: <code>`������:eval: 1+2��ע��ֺ���Ӣ�ĵģ��ֺź��������հס�\n";
            info += "  * �����@�ң���Ҳ��@�㡣\n";
            info += " ��ע�⣺�벻Ҫд�ᵼ���Ҽң�ϵͳ���쳣�Ĵ��룬����С����Ŷ���ں���������ζ���һ�ܾ�ĳЩ����:D ��";
            return info;
        }


    };
}