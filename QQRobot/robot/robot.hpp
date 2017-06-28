#include "group_message.hpp"
#include "message_sender.hpp"
#include "osu_query/osu_query.hpp"
#include "interpreters/js/js.hpp"
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

        CQ_EVENT_RET onGroupMessage(GroupMessage msg)
        {
            CQ_EVENT_RET returnResult = EVENT_IGNORE;

            string fromContent = msg.atContent();

            // �ظ�
            msg.toGroupQQ = msg.fromGroupQQ;
            int index;

            // �Լ���AT
            if (msg.getAtQQ() == this->qq)
            {
                //ҲAT�Է�
                msg.setAtQQ(msg.fromQQ);

                if ((index = fromContent.find("����")) != string::npos)
                {
                    string info = "�ҵĹ������£�\n";
                    info += "  * �ҿ���ִ��JS���򣬷��ͣ�eval: <code>`������:eval: 1+2��ע��ֺ���Ӣ�ĵģ��ֺź��������հס�\n";
                    info += "  * �����@�ң���Ҳ��@�㡣\n";
                    info += " ��ע�⣺�벻Ҫд�ᵼ���Ҽң�ϵͳ���쳣�Ĵ��룬����С����Ŷ���ں���������ζ���һ�ܾ�ĳЩ����:D ��";
                    msg.setContent(info);
                }
                else
                {
                    // echo
                    msg.setContent(fromContent);
                }

                returnResult = EVENT_BLOCK;
                
            }

            if (fromContent.find("!stat") == 0)
            {
                //��ѯ
                //wstring result = OsuQuery::query(fromContent);
                //msg.setContent(result);
                msg.setContent("��δ����");
                returnResult = EVENT_BLOCK;
            }
            else if((index = fromContent.find("eval:")) != string::npos)
            {
                if (blacklist.exist(to_string(msg.fromQQ)))
                {

                    msg.setAtQQ(msg.fromQQ);
                    msg.setContent("���Ѿ����ؽ�С������:C");
                    returnResult = EVENT_BLOCK;
                    goto RETMSG;
                }

                if (defaultEvalLanguage == "js")
                {
                    try
                    {
                        string code = fromContent.substr(index + 5);
                        // ��Ϣ�е�ĳЩ�ַ������룬����'[��']'���ֱ�ת������&#91;��&#93;�������������������
                        code = toCode(code);
                        code = stringutil::string_To_UTF8(code);
                        string result = js.evalForUTF8(code);
                        result = stringutil::UTF8_To_string(result);
                        msg.setContent(result != "" ? result : " "); //�����֧�ַ��Ϳ�
                    }
                    catch (exception &e)
                    {
                        string info = "�����쳣��:\n";
                        info += e.what();
                        msg.setContent(info);
                    }
                    returnResult = EVENT_BLOCK;
                }
            }
            else if ((index = fromContent.find("!blacklist") == 0))
            {
                if (msg.fromQQ != 1013644379)
                {
                    msg.setAtQQ(msg.fromQQ);
                    msg.setContent("��Ȩ��");
                    returnResult = EVENT_BLOCK;
                    goto RETMSG;
                }
                vector<string> strs = stringutil::split(fromContent, " ");
                if (strs.size() < 2)
                    goto RET;
                string objStr = strs[1];

                if (objStr == "add" && strs.size() >= 3)
                {
                    string argStr = strs[2];
                    blacklist.addQQ(argStr);
                    msg.setContent(argStr + " �ɹ��ؽ�С����");
                }
                if (objStr == "del" && strs.size() >= 3)
                {
                    string argStr = strs[2];
                    blacklist.delQQ(argStr);
                    msg.setContent(argStr + " �ɹ���С�����ͷ�");
                }
                if (objStr == "list")
                {
                    msg.setContent(blacklist.empty() ? "��������" : "������:\n" + blacklist.toMutilLineStr("  "));
                }
                if (objStr == "clear")
                {
                    blacklist.clear();
                    msg.setContent("�ɹ���պ�����");
                }
                returnResult = EVENT_BLOCK;
            }

            RETMSG:
            if(returnResult == EVENT_BLOCK)
                sender.sendMessageToGroup(msg);

            RET:
            return returnResult;
        }

    private:
        string qq;
        string defaultEvalLanguage = "js";
        JS js;
        BlackList blacklist;

        string toCode(string str)
        {
            str = stringutil::replace_all(str, "&#91;", "[");
            str = stringutil::replace_all(str, "&#93;", "]");
            return str;
        }

	};
}