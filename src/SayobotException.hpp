#pragma once
#ifndef SAYOBOT_EXCEPTION_HPP
#define SAYOBOT_EXCEPTION_HPP

#include <string>

namespace Sayobot
{
    /*
     * 基本异常
     */
    class BaseException
    {
    public:
        BaseException(const std::string& Msg, unsigned int code)
        {
            this->Msg = Msg;
            this->code = code;
        }
        BaseException(const char* Msg, unsigned int code)
        {
            this->Msg = Msg;
            this->code = code;
        }

        unsigned int Code()
        {
            return this->code;
        }

        std::string Message()
        {
            return this->Msg;
        }

        virtual std::string Type()
        {
            return "BaseException";
        }

        virtual std::string Display()
        {
            std::string ret;
            char* buffer = new char[512];
            sprintf_s(buffer,
                      512,
                      "[%s-%u] %s",
                      this->Type().c_str(),
                      this->code,
                      this->Msg.c_str());
            ret = buffer;
            delete[] buffer;
            return ret;
        }

    private:
        std::string Msg;
        unsigned int code;
    };

    /*
     * MySQL异常
     * （继承自基本异常）
     */
    class SqlException : public BaseException
    {
    public:
        SqlException(const std::string& Msg, unsigned int code)
            : BaseException(Msg, code)
        {
        }
        std::string Type()
        {
            return "SqlException";
        }
        std::string Display()
        {
            std::string ret;
            char* buffer = new char[512];
            sprintf_s(buffer,
                      512,
                      "[%s-%u] %s",
                      this->Type().c_str(),
                      this->Code(),
                      this->Message().c_str());
            ret = buffer;
            delete[] buffer;
            return ret;
        }
    };

    /*
     * MySQL Query异常
     * （继承自基本异常）
     */
    class QueryException : public BaseException
    {
    public:
        QueryException(const std::string& query, const std::string& Msg,
                       unsigned int code)
            : BaseException(Msg, code)
        {
            this->query = query;
        }
        QueryException(const char* query, const std::string& Msg, unsigned int code)
            : BaseException(Msg, code)
        {
            this->query = query;
        }
        const char* ErrorQuery()
        {
            return this->query.c_str();
        }
        std::string Type()
        {
            return "QueryException";
        }
        std::string Display()
        {
            std::string ret;
            char* buffer = new char[512];
            sprintf_s(buffer,
                      512,
                      "[%s-%u] %s\t Query: %s",
                      this->Type().c_str(),
                      this->Code(),
                      this->Message().c_str(),
                      this->ErrorQuery());
            ret = buffer;
            delete[] buffer;
            return ret;
        }

    private:
        std::string query;
    };

    /*
     * 网络功能异常
     * （继承自基本异常）
     */
    class NetException : public BaseException
    {
    public:
        NetException(const std::string& Msg, unsigned int code)
            : BaseException(Msg, code)
        {
        }
        std::string Type()
        {
            return "NetException";
        }
        std::string Display()
        {
            std::string ret;
            char* buffer = new char[512];
            sprintf_s(buffer,
                      512,
                      "[%s-%u] %s",
                      this->Type().c_str(),
                      this->Code(),
                      this->Message().c_str());
            ret = buffer;
            delete[] buffer;
            return ret;
        }
    };

    /*
     * 参数数量异常
     * （继承自基本异常）
     */
    class InvaildArgumentsCount : public BaseException
    {
    public:
        InvaildArgumentsCount(const std::string& command, const std::string& Msg,
                              unsigned int code)
            : BaseException(Msg, code)
        {
            this->command = command;
        }
        std::string Type()
        {
            return "InvaildArgumentsCount";
        }
        std::string Display()
        {
            std::string ret;
            char* buffer = new char[512];
            sprintf_s(buffer,
                      512,
                      "[%s-%u] %s\t Command: %s",
                      this->Type().c_str(),
                      this->Code(),
                      this->Message().c_str(),
                      this->command.c_str());
            ret = buffer;
            delete[] buffer;
            return ret;
        }

    private:
        std::string command;
    };

    /*
     * 用户被ban或不存在
     * （继承自基本异常）
     */
    class UserNotFound : public BaseException
    {
    public:
        UserNotFound(const std::string& Msg, unsigned int code)
            : BaseException(Msg, code)
        {
        }

        std::string Display()
        {
            std::string ret;
            char* buffer = new char[512];
            sprintf_s(buffer,
                      512,
                      "[%s-%u] %s",
                      this->Type().c_str(),
                      this->Code(),
                      this->Message().c_str());
            ret = buffer;
            delete[] buffer;
            return ret;
        }
    };

    class InvaildArgumentException : public BaseException
    {
    public:
        InvaildArgumentException(const std::string& command, const std::string& Msg,
                                 unsigned int code)
            : BaseException(Msg, code)
        {
        }
        std::string Type()
        {
            return "InvaildArgumentException";
        }
        std::string Display()
        {
            std::string ret;
            char* buffer = new char[512];
            sprintf_s(buffer,
                      512,
                      "[%s-%u] %s",
                      this->Type().c_str(),
                      this->Code(),
                      this->Message().c_str());
            ret = buffer;
            delete[] buffer;
            return ret;
        }
    };

    class FileNotFound : public BaseException
    {
    public:
        FileNotFound(const std::string& Msg, unsigned int code)
            : BaseException(Msg, code)
        {
        }

        std::string Type()
        {
            return "FileNotFound";
        }

        std::string Display()
        {
            std::string ret;
            char* buffer = new char[512];
            sprintf_s(buffer,
                      512,
                      "[%s-%u] %s",
                      this->Type().c_str(),
                      this->Code(),
                      this->Message().c_str());
            ret = buffer;
            delete[] buffer;
            return ret;
        }
    };

    class SharedMemoryException : public BaseException
    {
    public:
        SharedMemoryException(const std::string& Msg, unsigned int code)
            : BaseException(Msg, code)
        {
        }

        std::string Type()
        {
            return "SharedMemoryException";
        }

        std::string Display()
        {
            std::string ret;
            char* buffer = new char[512];
            sprintf_s(buffer,
                      512,
                      "[%s-%u] %s",
                      this->Type().c_str(),
                      this->Code(),
                      this->Message().c_str());
            ret = buffer;
            delete[] buffer;
            return ret;
        }
    };
} // namespace Sayobot

namespace Sayobot
{
    // Mysql 1000-1494 2000-2054
    // WinHttp 12000-12175
    // HttpStatusCode 100-101 200-206 300-307 400-417 500-505

    enum // System相关
    {
        BROKEN_PIPE = 4000, // 管道连接失败
        INTERNET_INVAILD_URL = 12005, // 非法的URL
        INTERNET_NAME_NOT_RESOLVED = 12007, // 域名解析失败
        SHARED_MEMORY_CANNOT_GET_BLOCK = 4001, // 无法连接到共享内存
        SHARED_MEMORY_CANNOT_GET_SPACE = 4002 // 无法获取共享内存空间
    };

    enum // 指令相关
    {
        INCOMPLETE_COMMAND = 5000, // 未完成的指令
        NO_HELP_PIC = 5001, // 不存在的help图片
        INVAILD_ARGUMENT_COUNT = 5002, // 非法的参数数量
        EDGE_NOT_FOUND = 5003, // 不存在的边框
        BACKGROUND_NOT_FOUND = 5004, // 不存在的背景图片
        INVAILD_OPACITY = 5005, // 非法的透明度
        ROLL_OUT_OF_RANGE = 5006, // roll点超过最大值
        INVAILD_MODE = 5007, // 非法的mode字符串或数字
        INVAILD_DAY = 5008, // 非法的日期
        NO_DAILY_DATA = 5009, // 没有以往的数据
        COMMAND_NOT_FOUND = 5010,
    };

    enum // 用户相关
    {
        USER_SELF_NOT_SET = 6000, // 用户未绑定
        USER_OTHER_NOT_SET = 6001, // 特定用户未绑定
        USER_BANNED = 6002, // 用户被ban或不存在
        USER_SELF_NOT_PLAYED = 6003, // 用户未玩过的模式
        USER_OTHER_NOT_PLAYED = 6004, // 特定用户未玩过的模式
        USER_SELF_HAVE_SETTED = 6005, // qq号已绑定
        USER_OTHER_HAVE_SETTED = 6006 // uid已绑定
    };
} // namespace Sayobot
#endif