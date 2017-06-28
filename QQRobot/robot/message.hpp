#ifndef QQROBOT_MESSAGE_H
#define QQROBOT_MESSAGE_H

#include <string>
using namespace std;

namespace QQRobot
{
    typedef int64_t QQNumber;

    class Message
    {
    public:
        QQNumber fromQQ;	/* ����QQ�û� */
        QQNumber toQQ;		/* Ŀ��QQ�û� */

        void setContent(string content) { this->content = content; }
        string getContent() { return content; }
    protected:
        string content;		/* ��Ϣ���� */
    };
}

#endif