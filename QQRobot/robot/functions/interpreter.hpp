#ifndef ROBOT_INTERPRETER_H
#define ROBOT_INTERPRETER_H

#include <string>
#include <typeinfo>
#include "../stringutil.hpp"
#include "function.hpp"
#include "interpreters/js.hpp"
#include "interpreters/scheme.hpp"

using namespace std;
using namespace QQRobot;
using namespace Interpreters;

namespace QQRobot
{
    class Robot;

    class Interpreter : public Function
    {

    public:
        Interpreter() {}
        Interpreter(MessageSender *sender, Robot *robot) : Function(sender, robot) {}

        bool handleMessage(Message &fromMsg, Message &toMsg)
        {
            string content = fromMsg.getContent();
            string code = content.substr(content.find("eval:") + 5);
            string result;

            if (evalLanguage == "js")
            {
                try
                {
                    // ��Ϣ�е�ĳЩ�ַ������룬����'[��']'���ֱ�ת������&#91;��&#93;�������������������
                    // ����Ƿ�������
                    if (isBadCode(code))
                    {
                        if(typeid(fromMsg) == typeid(GroupMessage))
                            ((GroupMessage&)toMsg).setAtQQ(fromMsg.from);
                        (robot->blacklist).addQQ(fromMsg.from);
                        toMsg.setContent("�㷢���Ƕ�����룬���Ѿ����ؽ�С�����ˣ�");
                        sender->sendMessage(toMsg);
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
            else if (evalLanguage == "scheme")
            {
                result = scheme.eval(code);
            }

            toMsg.setContent(result);
            sender->sendMessage(toMsg);
            return true;
        }
    private:
        string evalLanguage = "js";
        JS js;
        Scheme scheme;

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

        string toCode(string str)
        {
            str = stringutil::replace_all(str, "&#91;", "[");
            str = stringutil::replace_all(str, "&#93;", "]");
            str = stringutil::replace_all(str, "&amp;", "&");
            return str;
        }
    };
}

#endif