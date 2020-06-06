#include <cqcppsdk/cqcppsdk.h>
#include <time.h>

#include <chrono>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>

#include "SayobotCommand.hpp"
#include "SayobotException.hpp"
#include "SayobotSharedMemory.hpp"

#define SAYOBOT_VERSION "Sayobot v1.1.5"
#define SAYOBOT_FOLDER "A:\\Sayobot\\CQ"
#ifndef WIN32
#define SetCurrentDirectory(msg) \
    printf("Function \"SetCurrentDirectory\" is not avaliable in your operate system!\n")
#define WinExec(cmd, option) \
    printf("Function \"WinExec\" is not avaliable in your operate system!\n")
#endif

using namespace cq;
using namespace std;
using Message = cq::message::Message;
using MessageSegment = cq::message::MessageSegment;

using hrc = std::chrono::high_resolution_clock;
using ms = std::chrono::milliseconds;

const std::set<int64_t> offial_list = {2658655094, 2062433139, 2010741172, 2171946401};
const std::set<int64_t> user_black_list = {};
const std::set<int64_t> group_black_list = {450590043};
const std::set<int64_t> non_listen_group_list = {};

CQ_INIT {
    // 插件启用
    cq::on_enable([] {
        try {
            logging::info("启用", "插件已启用");
            Magick::InitializeMagick(SAYOBOT_FOLDER);
            logging::info("初始化", "Magick++初始化完毕");
            SetCurrentDirectory(SAYOBOT_FOLDER);
            try {
                Sayobot::SharedMemory<Sayobot::MessageListener> memory(
                    "SayobotSharedMemory");
                Sayobot::MessageListener *listener =
                    (Sayobot::MessageListener *)memory.GetPtr();

                auto group_list = get_group_list();
                for (auto group : group_list) {
                    if (non_listen_group_list.count(group.group_id) == 0)
                        listener->AddGroup(group.group_id);
                }
                memory.Disconnect();
                logging::info("初始化", "共享内存初始化完毕");
#ifdef CQ_STD_MODE
                ofstream logFile("../log/v2/command_execute.log");
                logFile << SAYOBOT_VERSION << std::endl;
                logFile.close();
#endif
                logging::info("初始化", "插件初始化完毕");
            } catch (ApiError &ex) {
                logging::warning("获取group失败", "请检查权限是否打开");
            } catch (Sayobot::BaseException &ex) {
                logging::error(ex.Type(), "无法连接到共享内存块");
            } catch (std::exception &ex) {
                logging::error("初始化失败", ex.what());
            }
        } catch (...) {
            logging::error("Fatal", "启用插件时，发生错误");
        }
    });
    // 收到私聊消息
    cq::on_private_message([](const PrivateMessageEvent &e) {
        try {
            time_t t = time(NULL);
            tm *t_now = localtime(&t);
            char t_str[128];
            strftime(t_str, 128, "%Y-%m-%d %H:%M:%S", t_now);

            std::ofstream MessageLog("../log/v2/private.log", ios::app);
            MessageLog << "[ " << t_str << " ]"
                       << " \t qq: " << e.user_id << " \t 消息：" << e.message
                       << std::endl;
            MessageLog.close();
            try {
                if (0 == offial_list.count(e.user_id)
                    && 0 == user_black_list.count(e.user_id)) {
                    hrc::time_point begin = hrc::now();
                    std::string msg = e.message;
                    if (msg[0] == '!') {
                        msg = msg.substr(1);
                    } else if (msg.find(u8"！") == 0) {
                        msg = msg.substr(3);
                    }
                    Sayobot::Command command(msg, e.user_id, 0, e.target);
                    ms diff_t = std::chrono::duration_cast<ms>(hrc::now() - begin);
                    std::string res = command.GetResult();

                    std::regex beatmapLinkRegExp(
                        "^https://osu.ppy.sh/beatmapsets/(\\d+)#?.*$",
                        std::regex_constants::ECMAScript
                            | std::regex_constants::icase);
                    if (!res.empty()) {
                        cq::send_message(e.target, res);
                        logging::info("处理私聊指令成功",
                                      "指令原文：" + e.message + "  消耗时间："
                                          + std::to_string(diff_t.count()) + " ms");
                    } else if (e.message.find("好无聊啊") != std::string::npos) {
                        cq::send_message(e.target,
                                         "Welcome to OSU! 点击下载osu客户端 "
                                         "https://txy1.sayobot.cn/osu.zip  "
                                         "点击在线游玩 http://game.osu.sh");
                    } else if (std::regex_search(e.message, beatmapLinkRegExp)) {
                        auto it = std::sregex_iterator(
                            e.message.begin(), e.message.end(), beatmapLinkRegExp);
                        std::string sid = (*it)[1].str();

                        cq::send_message(
                            e.target,
                            "点击链接下载此图https://osu.sayobot.cn/?search=" + sid);
                    }
                    /*else
                    {
                        cq::send_message(e.target, "阁下，没有这个指令哦");
                        cq::send_message(
                            e.target, MessageSegment::image("pic/Sayobot help.png"));
                    }*/
                }
            } catch (ApiError &ex) {
                logging::error("处理私聊指令失败",
                               "CQ API错误：" + to_string(ex.code));
                if (ex.code == -30) {
                    send_private_message(
                        DEBUGGER, to_string(e.user_id) + "可能是公众号或不存在");
                } else if (ex.code == -11) {
                    cq::send_message(e.target, "你的图被小夜吃了，嘻嘻~");
                } else {
                    // cq::send_message(e.target,
                    //             "好像出现了严重错误！已经告诉麻麻了，阁下请等一会哦");
                    send_private_message(DEBUGGER,
                                         std::string("CQ API错误: ")
                                             + to_string(ex.code) + '\n' + ex.what());
                }
                std::ofstream logfile("../log/v2/command_execute.log", ios::app);
                logfile << "[ CQ API Error: " << ex.code << " ]"
                        << "\t [ " << e.user_id << " ] \t 消息：" << e.message
                        << std::endl;
                logfile.close();
                Sayobot::SharedMemory<Sayobot::MessageListener> memory(
                    "SayobotSharedMemory");
                Sayobot::MessageListener *listener =
                    (Sayobot::MessageListener *)memory.GetPtr();
                if (listener->PushError(ex.code)) {
                    cq::send_private_message(
                        DEBUGGER, "已经连续出现该错误5次: " + std::to_string(ex.code));
                }
                memory.Disconnect();
            } catch (Sayobot::BaseException &ex) {
                logging::warning("处理私聊指令失败", ex.Display());
                cq::send_message(e.target, ex.Message());
                std::ofstream logfile("../log/v2/command_execute.log", ios::app);
                logfile << ex.Display() << "\t [ " << e.user_id << " ] \t 消息"
                        << e.message << std::endl;
                logfile.close();
                Sayobot::SharedMemory<Sayobot::MessageListener> memory(
                    "SayobotSharedMemory");
                Sayobot::MessageListener *listener =
                    (Sayobot::MessageListener *)memory.GetPtr();
                if (listener->PushError(ex.Code())) {
                    cq::send_private_message(
                        DEBUGGER,
                        "已经连续出现该错误5次: " + std::to_string(ex.Code()));
                }
                memory.Disconnect();
            } catch (Magick::Exception &ex) {
                cq::send_message(e.target,
                                 "诶，小夜忘了怎么画这张图了，我去问一下麻麻");
                send_private_message(DEBUGGER,
                                     std::string("出现Magick++ 错误：") + ex.what());
                std::ofstream logfile("../log/v2/command_execute.log", ios::app);
                logfile << ex.what() << " \t [ " << e.user_id
                        << " ] \t 消息: " << e.message << std::endl;
                logfile.close();
            } catch (std::exception &ex) {
                logging::error("处理私聊指令失败",
                               std::string("未知错误：") + ex.what());
                // cq::send_message(e.target,
                //             "好像出现了严重错误！已经告诉麻麻了，阁下请等一会哦");
                std::ofstream logfile("../log/v2/command_execute.log", ios::app);
                logfile << ex.what() << " \t [ " << e.user_id
                        << " ] \t 消息: " << e.message << std::endl;
                logfile.close();
                send_private_message(DEBUGGER,
                                     std::string("出现系统级错误: ") + ex.what());
            }
            e.block();
        } catch (...) {
            logging::error("私聊Error", "Msg: " + e.message);
        }
    });
    // 收到消息（包括私聊和群消息）
    cq::on_message([](const MessageEvent &e) {
        if (0 == e.message_id % 10000) {
            WinExec("cmd.exe /c del /q data\\image\\*.cqimg", SW_HIDE);
            WinExec("cmd.exe /c del /q data\\image\\*.jpg", SW_HIDE);
            logging::info("清除缓存", "缓存清除完毕");
        }
    });
    // 收到群消息
    cq::on_group_message([](const GroupMessageEvent &e) {
        try {
            const std::regex atRegex(
                "\\[ ?CQ ?: ?at ?, ?qq ?= ?1394932996 ?\\] ?",
                std::regex_constants::ECMAScript | std::regex_constants::icase);

            std::regex beatmapLinkRegExp(
                "^https://osu.ppy.sh/beatmapsets/(\\d+)#?.*$",
                std::regex_constants::ECMAScript | std::regex_constants::icase);

            const std::vector<std::string> atMessage = {
                MessageSegment::image("pic/？？？.png"),
                "喵？",
                "你把我@出来了，你要实现我一百个愿望"};

            if (0 == group_black_list.count(e.group_id)) {
                if (e.message[0] == '!' || e.message.find(u8"！") == 0) {
                    try {
                        hrc::time_point begin = hrc::now();
                        Sayobot::Command command(e.message[0] < 0
                                                     ? e.message.substr(3)
                                                     : e.message.substr(1),
                                                 e.user_id,
                                                 e.group_id,
                                                 e.target);
                        ms diff_t = std::chrono::duration_cast<ms>(hrc::now() - begin);
                        std::string res = command.GetResult();
                        if (!res.empty()) {
                            cq::send_message(e.target, res);
                            logging::info("处理群聊指令成功",
                                          "指令原文：" + e.message + "  消耗时间："
                                              + std::to_string(diff_t.count())
                                              + " ms");
                        } else {
                        }
                    } catch (ApiError &ex) {
                        logging::error("处理群聊指令失败",
                                       "CQ API错误：" + to_string(ex.code));
                        if (ex.code == -30) {
                            send_private_message(DEBUGGER,
                                                 to_string(e.group_id) + "可能不存在");
                        } else if (ex.code == -36) {
                            send_private_message(
                                DEBUGGER,
                                to_string(e.group_id) + "这个群不允许私聊哦，是"
                                    + to_string(e.user_id) + "发的，和他说一下吧");
                        } else if (ex.code == -11) {
                            cq::send_message(e.target, "你的图被小夜吃了 嘻嘻~");
                        } else if (ex.code == -34) {
                            char buffer[128];
                            sprintf_s(buffer, 128, "小夜在群%lld被禁言惹", e.group_id);
                            send_private_message(SAYOKO, buffer);
                        } else {
                            send_private_message(
                                DEBUGGER,
                                std::string("CQ API错误: ") + to_string(ex.code));
                        }
                        std::ofstream logfile("../log/v2/command_execute.log",
                                              ios::app);
                        logfile << "[ CQ API Error: " << ex.code << " ]"
                                << "\t [ " << e.user_id << " - " << e.group_id
                                << " ] \t 消息：" << e.message << std::endl;
                        logfile.close();

                        Sayobot::SharedMemory<Sayobot::MessageListener> memory(
                            "SayobotSharedMemory");
                        Sayobot::MessageListener *listener =
                            (Sayobot::MessageListener *)memory.GetPtr();
                        if (listener->PushError(ex.code)) {
                            cq::send_private_message(
                                DEBUGGER,
                                "已经连续出现该错误5次: " + std::to_string(ex.code));
                        }
                        memory.Disconnect();
                    } catch (Sayobot::BaseException &ex) {
                        logging::warning("处理群聊指令失败", ex.Display());
                        cq::send_message(e.target, ex.Message());
                        std::ofstream logfile("../log/v2/command_execute.log",
                                              ios::app);
                        logfile << ex.Display() << " \t[ " << e.user_id << " - "
                                << e.group_id << " ] \t 消息: " << e.message
                                << std::endl;
                        logfile.close();
                        Sayobot::SharedMemory<Sayobot::MessageListener> memory(
                            "SayobotSharedMemory");
                        Sayobot::MessageListener *listener =
                            (Sayobot::MessageListener *)memory.GetPtr();
                        if (listener->PushError(ex.Code())) {
                            cq::send_private_message(
                                DEBUGGER,
                                "已经连续出现该错误5次: " + std::to_string(ex.Code()));
                        }
                        memory.Disconnect();
                    } catch (Magick::Exception &ex) {
                        cq::send_message(e.target,
                                         "诶，小夜忘了怎么画这张图了，我去问一下麻麻");
                        send_private_message(
                            DEBUGGER, std::string("出现Magick++ 错误：") + ex.what());
                        std::ofstream logfile("../log/v2/command_execute.log",
                                              ios::app);
                        logfile << ex.what() << " \t" << e.user_id << " - "
                                << e.group_id << " \t 消息: " << e.message
                                << std::endl;
                        logfile.close();
                    } catch (std::exception &ex) {
                        logging::error("处理群聊指令失败",
                                       std::string("未知错误：") + ex.what());
                        // cq::send_message(e.target,
                        //             "好像出现了严重错误！已经告诉麻麻了，阁下请等一会哦");
                        std::ofstream logfile("../log/v2/command_execute.log",
                                              ios::app);
                        logfile << ex.what() << " \t [" << e.user_id << " - "
                                << e.group_id << " ] \t 消息: " << e.message
                                << std::endl;
                        logfile.close();
                        send_private_message(
                            DEBUGGER, std::string("出现系统级错误: ") + ex.what());
                    }
                } else if (std::regex_match(e.message, atRegex)) {
                    std::default_random_engine random;
                    std::uniform_int_distribution<int> dist(0, 2);
                    cq::send_message(e.target, atMessage[dist(random)]);
                } else if (e.message.find("好无聊啊") != std::string::npos) {
                    cq::send_message(e.target,
                                     "Welcome to OSU! 点击下载osu客户端 "
                                     "https://txy1.sayobot.cn/osu.zip  "
                                     "点击在线游玩 http://game.osu.sh");
                } else if (std::regex_search(e.message, beatmapLinkRegExp)) {
                    auto it = std::sregex_iterator(
                        e.message.begin(), e.message.end(), beatmapLinkRegExp);
                    std::string sid = (*it)[1].str();

                    cq::send_message(
                        e.target,
                        "点击链接下载此图https://osu.sayobot.cn/?search=" + sid);
                } else if (non_listen_group_list.count(e.group_id) != 0) {
                    try {
                        Sayobot::SharedMemory<Sayobot::MessageListener> memory(
                            "SayobotSharedMemory");
                        Sayobot::MessageListener *listener =
                            (Sayobot::MessageListener *)memory.GetPtr();

                        if (listener->PushMessage(e.group_id,
                                                  e.user_id,
                                                  e.message.substr(0, 511).c_str())) {
                            ifstream repeatFile("./repeat.txt");
                            std::vector<std::string> repeatMessage;
                            repeatMessage.clear();
                            std::string temp;
                            while (std::getline(repeatFile, temp)) {
                                if (!temp.empty()) repeatMessage.push_back(temp);
                            }
                            repeatFile.close();
                            std::default_random_engine random(time(NULL));
                            std::uniform_int_distribution<int> dist(
                                0, repeatMessage.size() - 1);
                            cq::send_message(e.target, repeatMessage[dist(random)]);
                            cq::set_group_ban(e.group_id, e.user_id, 600);
                        }
                        memory.Disconnect();
                    } catch (ApiError &ex) {
                        logging::warning("无权限",
                                         to_string(e.group_id) + "没有禁言权限");
                    } catch (Sayobot::BaseException &ex) {
                        logging::error(ex.Type(), "无法链接到共享内存块");
                    } catch (std::exception &ex) {
                        logging::error("共享内存出现系统级错误", ex.what());
                    }
                }
            }

            e.block(); // 阻止当前事件传递到下一个插件
        } catch (...) {
            logging::error("群聊Error", "Msg: " + e.message);
        }
    });

    cq::on_group_member_decrease([](const GroupMemberDecreaseEvent &e) {
        if (e.sub_type == GroupMemberDecreaseEvent::SubType::KICK_ME) {
            char buffer[512];
            sprintf_s(buffer, 512, "小夜在群%lld被%lldT了", e.group_id, e.operator_id);
            send_group_message(SAYOBOT_GROUP, buffer);
        }

        if (SAYOBOT_GROUP == e.group_id
            && e.sub_type == GroupMemberDecreaseEvent::SubType::LEAVE) {
            char buffer[512];
            sprintf_s(buffer, 512, "%lld因为精神失常离开了本群", e.user_id);
            send_group_message(SAYOBOT_GROUP, buffer);
        }
    });

    cq::on_group_member_increase([](const GroupMemberIncreaseEvent &e) {
        if (0 == group_black_list.count(e.group_id)) {
            try {
                if (e.target.user_id != SAYOBOT_SELF) {
                    char buffer[512];
                    sprintf_s(buffer,
                              512,
                              "你好呀~ %s 认真阅读一下群公告哦~",
                              std::string(MessageSegment::at(e.user_id)).c_str());
                    send_group_message(e.group_id, buffer);
                }
            } catch (ApiError &ex) {
                char buffer[512];
                sprintf_s(
                    buffer,
                    512,
                    "CQ API error: %d\ngroup_id: %lld\nuser_id: %lld\nmessage: %s",
                    ex.code,
                    e.group_id,
                    e.user_id,
                    "群用户数量上升");
                send_private_message(DEBUGGER, buffer);
            } catch (...) {
                char buffer[512];
                sprintf_s(
                    buffer,
                    512,
                    "Uncatched error!\ngroup_id: %lld\nuser_id: %lld\nmessage: %s",
                    e.group_id,
                    e.user_id,
                    "群用户数量上升");
                send_private_message(DEBUGGER, buffer);
            }
        }
    });

    cq::on_group_request([](const GroupRequestEvent &e) {
        if (0 == group_black_list.count(e.group_id)) {
            if (e.sub_type == cq::GroupRequestEvent::SubType::INVITE) {
                char msg[512];
                sprintf_s(msg,
                          512,
                          "麻麻，%lld邀请小夜加入群%lld，备注：%s",
                          e.user_id,
                          e.group_id,
                          e.comment.c_str());
                send_private_message(SAYOKO, msg);
            } else if (e.sub_type == cq::GroupRequestEvent::SubType::ADD) {
                if (e.group_id == SAYOBOT_GROUP) {
                    Sayobot::Database db;
                    db.Connect();
                    Sayobot::UserConfigData d;
                    int ret = db.GetUserConfig(e.user_id, d);
                    if (ret == 0) {
                        cq::set_group_request(
                            e.flag, e.sub_type, cq::RequestEvent::Operation::APPROVE);
                    }
                }
            }
        }
    });

    cq::on_friend_request([](const FriendRequestEvent &e) {
        if (0 == user_black_list.count(e.user_id)) {
            char msg[512];
            sprintf_s(msg,
                      512,
                      "麻麻，%lld让小夜加他好友，备注：%s",
                      e.user_id,
                      e.comment.c_str());
            send_private_message(SAYOKO, msg);
        }
    });
}
