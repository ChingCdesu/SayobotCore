#pragma once
#ifndef SAYOBOT_COMMAND_HPP
#define SAYOBOT_COMMAND_HPP

#include <cqcppsdk/cqcppsdk.h>
#include <math.h>
#include <sys/stat.h>
#include <time.h>

#include <fstream>
#include <random>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "SayobotException.hpp"
#include "SayobotHttp.hpp"
#include "SayobotImage.hpp"
#include "SayobotMysql.hpp"
#include "zalgo.hpp"

#pragma region path
#define BACKGROUND_PATH "../png/stat/"
#define EDGE_PATH "../png/tk/"
#define FONT_PATH "../fonts/"
#define SKIN_PATH "../png/rank/"
#define COUNTRY_PATH "../png/country/"
#define GLOBAL_PATH "../png/world/s.png"
#define AVATAR_PATH "../png/avatars/"
#pragma endregion

#define SAYOBOT_SELF 1394932996
#define SAYOBOT_GROUP 693190897
#define COLA 1361262679
#define SAYOKO 1469708225
#define DEBUGGER 1415336930

#define BIG_POINTSIZE 54.0
#define MID_POINTSIZE 43.0
#define SMALL_POINTSIZE 29.0

using Message = cq::message::Message;
using MessageSegment = cq::message::MessageSegment;

namespace Sayobot {

    class Sudo {
    public:
        Sudo(const std::vector<std::string> list) {
            if (list.size() == 0) {
                this->result = "sudo command excepted";
                return;
            }

            const std::vector<std::string> sub_type = {"sql", "cmd", "update"};
            int index;
            for (index = 0; index < sub_type.size(); ++index) {
                if (!sub_type[index].compare(list[0])) break;
            }

            if (index == sub_type.size()) {
                char buffer[128];
                sprintf_s(
                    buffer, 128, "no sudo command named \"%s\"", list[0].c_str());
                throw Sayobot::BaseException(buffer, Sayobot::COMMAND_NOT_FOUND);
            }

            this->args.clear();
            for (auto it = list.begin() + 1; it != list.end(); ++it) {
                this->args.push_back(*it);
            }

            switch (index) {
            case 0:
                SqlOperation();
                break;
            case 1:
                CmdOperation();
                break;
            case 2:
                ManualUpdate();
                break;
            default:
                break;
            }
        }

        std::string GetResult() {
            return this->result;
        }

    private:
        std::string result;
        std::vector<std::string> args;

    private:
        void SqlOperation() {
            Sayobot::Database db;
            db.Connect();
            cq::utils::string_trim(this->args[0]);
            std::string type = this->args[0].substr(0, 6);
            std::transform(type.begin(), type.end(), type.begin(), ::tolower);

            if (!type.compare("select")) {
                json res = db.Select(this->args[0]);
                this->result = res.dump(2);
            } else {
                db.Execute(this->args[0]);
                db.Commit();
                this->result = "数据库更新成功";
            }
            db.Close();
            // cq::utils::string_trim(this->result);
        }

        void CmdOperation() {
            // this->result = "not supported yet";
            // return;
            cq::utils::string_trim(this->args[0]);
            this->result = ExecCommand(this->args[0]);
            this->result = cq::utils::string_convert_encoding(
                this->result, "gb2312", "utf-8", 2.0f);
        }

        void ManualUpdate() {
            const std::vector<char> mode_str = {'o', 't', 'c', 'm'};
            if (1 != this->args[this->args.size() - 1].size()) {
                throw Sayobot::InvaildArgumentException(
                    "sudo update", "invaild mode char", Sayobot::INVAILD_MODE);
            }
            int mode_num;
            for (mode_num = 0; mode_num < 4; ++mode_num) {
                if (mode_str[mode_num] == this->args[this->args.size() - 1][0]) break;
            }
            if (4 == mode_num) {
                throw Sayobot::InvaildArgumentException(
                    "sudo update", "invaild mode char", Sayobot::INVAILD_MODE);
            }
            this->args.pop_back();
            std::string qq_or_nick = "";
            for (auto it = this->args.begin(); it != this->args.end(); ++it) {
                qq_or_nick += *it;
                qq_or_nick += ' ';
            }

            Sayobot::Database db;
            db.Connect();
            char *buffer = new char[1024];
            sprintf_s(buffer,
                      1024,
                      "select userid from userconfig where nick=\'%s\' or qq=\'%s\'",
                      qq_or_nick.c_str(),
                      qq_or_nick.c_str());
            json data;
            try {
                data = db.Select(buffer)[0];
            } catch (Sayobot::QueryException &ex) {
                db.Close();
                if (1065 == ex.Code())
                    throw UserNotFound("这个人好像没有绑定",
                                       Sayobot::USER_OTHER_NOT_SET);
                else
                    throw ex;
            }

            std::string userid = data["userid"].get<std::string>();
            sprintf_s(buffer,
                      1024,
                      "%sget_user?u=%s&m=%d",
                      OSU_API_V1,
                      userid.c_str(),
                      mode_num);
            std::string url = buffer;
            data = json::parse(Sayobot::NetConnection::HttpsGet(url))[0];

            data["TotalHits"] =
                std::to_string(std::stoll(data["count300"].get<std::string>())
                               + std::stoll(data["count100"].get<std::string>())
                               + std::stoll(data["count50"].get<std::string>()));

            sprintf_s(buffer,
                      1024,
                      "insert into userstats values (%s,\'%s\',%s,%s,%s,%s"
                      ",%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,\'%c\',now())",
                      data["user_id"].get<std::string>().c_str(),
                      data["country"].get<std::string>().c_str(),
                      data["TotalHits"].get<std::string>().c_str(),
                      data["total_score"].get<std::string>().c_str(),
                      data["ranked_score"].get<std::string>().c_str(),
                      data["accuracy"].get<std::string>().c_str(),
                      data["pp_raw"].get<std::string>().c_str(),
                      data["pp_rank"].get<std::string>().c_str(),
                      data["pp_country_rank"].get<std::string>().c_str(),
                      data["count_rank_ssh"].get<std::string>().c_str(),
                      data["count_rank_ss"].get<std::string>().c_str(),
                      data["count_rank_sh"].get<std::string>().c_str(),
                      data["count_rank_s"].get<std::string>().c_str(),
                      data["count_rank_a"].get<std::string>().c_str(),
                      data["playcount"].get<std::string>().c_str(),
                      data["level"].get<std::string>().c_str(),
                      mode_str[mode_num]);

            db.Insert(buffer);
            this->result = "更新完成";
            db.Close();
            delete[] buffer;
        }

        static std::string ExecCommand(const std::string &cmd) {
            std::string ret = "";
#ifdef WIN32
            const size_t MAX_READ_SIZE = 10240;
            SECURITY_ATTRIBUTES sa;
            ZeroMemory(&sa, sizeof(sa));
            sa.nLength = sizeof(sa);
            sa.bInheritHandle = TRUE;
            sa.lpSecurityDescriptor = NULL;

            HANDLE hReadPipe, hWritePipe;
            if (!::CreatePipe(&hReadPipe, &hWritePipe, &sa, MAX_READ_SIZE)) {
                throw Sayobot::BaseException("Cannot create pipe",
                                             Sayobot::BROKEN_PIPE);
            }

            STARTUPINFO si;
            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            si.wShowWindow = SW_HIDE;
            si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
            si.hStdError = hWritePipe;
            si.hStdOutput = hWritePipe;

            PROCESS_INFORMATION pi;
            ZeroMemory(&pi, sizeof(pi));

            if (CreateProcess(NULL,
                              (LPSTR)cmd.c_str(),
                              NULL,
                              NULL,
                              TRUE,
                              0,
                              NULL,
                              NULL,
                              &si,
                              &pi)) {
                if (WAIT_TIMEOUT != WaitForSingleObject(pi.hProcess, 10000)) {
                    DWORD dwReadLen = 0;
                    DWORD dwStdLen = 0;
                    if (PeekNamedPipe(hReadPipe, NULL, 0, NULL, &dwReadLen, NULL)
                        && dwReadLen > 0) {
                        char szPipeOut[MAX_READ_SIZE];
                        ZeroMemory(szPipeOut, MAX_READ_SIZE);
                        if (ReadFile(
                                hReadPipe, szPipeOut, dwReadLen, &dwStdLen, NULL)) {
                            ret += szPipeOut;
                        }
                    } else {
                        ret = "No result";
                    }
                } else {
                    ret = "cmd.exe time out";
                }
                if (pi.hProcess) CloseHandle(pi.hProcess);
                if (pi.hThread) CloseHandle(pi.hThread);
            } else {
                ret = "cannot execute command, error code: "
                      + std::to_string(GetLastError());
            }
            CloseHandle(hReadPipe);
            CloseHandle(hWritePipe);

            // FILE *fp = NULL;
            // fp = _popen(cmd.c_str(), "r");
            //
            // if (!fp)
            //{
            //    throw Sayobot::BaseException("Failed to create command pipe", 2);
            //}
            // char buffer[1024];
            // while (fgets(buffer, sizeof(buffer), fp) != NULL)
            //{
            //    ret += buffer;
            //}
            //_pclose(fp);
#else

            FILE *fp = NULL;
            fp = popen(cmd.c_str(), "r");

            if (!fp) {
                throw Sayobot::BaseException("Failed to create command pipe", 2);
            }
            char buffer[1024];
            while (fgets(buffer, sizeof(buffer), fp) != NULL) {
                ret += buffer;
            }
            pclose(fp);
#endif
            return ret;
        }
    };

    struct UserPanelData {
        osu_api::mode mode;
        osu_api::user_info uinfo;
        Sayobot::UserConfigData config;
        Sayobot::UserStatData stat;
        int compareDays;
    };

    class Command {
    public:
        /*
         * 指令类
         * 把指令头！去掉后剩下部分传入

         * 基本使用：
         * std::string result;
          try
          {
               result = Sayobot::Command(str, qq, group, e.target).GetResult();
               cq::send_message(e.target, result);
          }
          catch (Sayobot::BaseException &ex)
          {
               (异常处理...)
          }
        */
        Command(const std::string &Command, int64_t sender, int64_t group,
                cq::Target target) {
            this->sender = sender;
            this->group = group;
            this->target = target;

            std::string c = Command;
            cq::utils::string_trim(c);

            std::stringstream ss(c);
            this->args.clear();
            ss >> this->cmd;

            int cIndex = this->CommandIndex();
            if (cIndex != Command::Commands.size()) {
                while (!ss.eof()) {
                    std::string argBuffer;
                    ss >> argBuffer;
                    if (argBuffer[0] == '{') {
                        char c;
                        do {
                            if (argBuffer[argBuffer.size() - 1] == '}') {
                                break;
                            }
                            if (ss.eof()) {
                                throw Sayobot::InvaildArgumentException(
                                    "Parse Command",
                                    "阁下的命令还没写完呢 (记得加“}”)",
                                    Sayobot::INCOMPLETE_COMMAND);
                            }
                            ss.get(c);
                            argBuffer += c;
                        } while (c != '}');
                        argBuffer = argBuffer.substr(1, argBuffer.size() - 2);
                    }
                    if (!argBuffer.empty()) {
                        cq::message::unescape(argBuffer);
                        this->args.push_back(argBuffer);
                    }
                }

                this->execute(this, cIndex);
            } else {
                this->result = "";
            }
        }

        std::string GetResult() {
            return this->result;
        }

    private:
        const static std::vector<std::string> Commands;
        const static std::set<int64_t> sudoers;
        std::string cmd;
        std::vector<std::string> args;
        std::string result;
        int64_t sender;
        int64_t group;
        cq::Target target;

    private:
        static void execute(Sayobot::Command *ptr, int index) {
            ptr->result.clear();
            const std::vector<void (Sayobot::Command::*)()> funcs = {
                &Sayobot::Command::UserSet,
                &Sayobot::Command::UserUnset,
                &Sayobot::Command::Help,
                &Sayobot::Command::UpdateEdge,
                &Sayobot::Command::UpdateSign,
                &Sayobot::Command::UpdateBackground,
                &Sayobot::Command::UpdateOpacity,
                &Sayobot::Command::UpdateAvatar,
                &Sayobot::Command::Roll,
                &Sayobot::Command::Report,
                &Sayobot::Command::Reset,
                &Sayobot::Command::sudo,
                &Sayobot::Command::groupSleep,
                &Sayobot::Command::Recommend,
                &Sayobot::Command::zalgo};

            if (index >= 0 && index <= 3)
                ptr->UserStat(index);
            else {
                (ptr->*funcs[index - 4])();
            }
        }

        int CommandIndex() {
            int ret = 0;
            for (auto it = Sayobot::Command::Commands.begin();
                 it != Sayobot::Command::Commands.end();
                 ++it) {
                if (0 == this->cmd.compare(*it)) break;
                ++ret;
            }
            return ret;
        }

        void UserStat(int mode) {
            const char mode_str[4] = {'o', 't', 'c', 'm'};
            Sayobot::Database db;
            db.Connect();
            int ret;
            UserPanelData panelData;
            panelData.compareDays = 0;
            panelData.config = {0};
            panelData.mode = (osu_api::mode)mode;
            panelData.stat = {0};
            panelData.uinfo = {0};

            if (0 != this->args.size()
                && this->args[this->args.size() - 1][0] == '#') {
                try {
                    panelData.compareDays =
                        std::stoul(this->args[this->args.size() - 1].substr(1));
                    this->args.pop_back();
                } catch (std::invalid_argument) {
                    throw Sayobot::InvaildArgumentException(
                        this->cmd, "天数必须是数字哦", Sayobot::INVAILD_DAY);
                }
            }
            std::string nick;
            // 组合nick
            for (auto it = this->args.begin(); it != this->args.end(); ++it) {
                nick += *it;
                nick += ' ';
            }
            nick = nick.substr(0, nick.size() - 1);

            if (0 == this->args.size()) {
                ret = db.GetUserConfig(this->sender, panelData.config);
                if (1065 == ret) {
                    throw UserNotFound("阁下还没绑定哦，用set把阁下的名字告诉我吧",
                                       Sayobot::USER_SELF_NOT_SET);
                } else if (0 != ret) {
                }

                ret = db.GetUserStatus(this->sender,
                                       panelData.compareDays,
                                       mode_str[mode],
                                       panelData.stat);

                if (1065 == ret && 0 != panelData.compareDays) {
                    throw Sayobot::BaseException(
                        "小夜没有查到阁下" + std::to_string(panelData.compareDays)
                            + "天前的信息",
                        Sayobot::NO_DAILY_DATA);
                } else if (0 != ret) {
                    panelData.stat.user_id = -1;
                }
            } else {
                const std::regex cqCodeRegex(
                    "^\\[ ?CQ ?: ?at ?, ?qq ?= ?(\\d+) ?\\] ?$",
                    std::regex_constants::ECMAScript | std::regex_constants::icase);
                // 判断CQ码
                if (std::regex_search(nick, cqCodeRegex)) {
                    auto it =
                        std::sregex_iterator(nick.begin(), nick.end(), cqCodeRegex);
                    std::string qq_str = (*it)[1].str();
                    int64_t qq = std::stoll(qq_str);

                    ret = db.GetUserConfig(qq, panelData.config);
                    if (1065 == ret) {
                        throw UserNotFound("小夜还不认识这个人哦，阁下把他介绍给我吧",
                                           Sayobot::USER_OTHER_NOT_SET);
                    } else if (0 != ret) {
                    }

                    ret = db.GetUserStatus(
                        qq, panelData.compareDays, mode_str[mode], panelData.stat);

                    if (1065 == ret && 0 != panelData.compareDays) {
                        throw Sayobot::BaseException(
                            "小夜没有查到阁下" + std::to_string(panelData.compareDays)
                                + "天前的信息",
                            Sayobot::NO_DAILY_DATA);
                    } else if (0 != ret) {
                        panelData.stat.user_id = -1;
                    }

                } else {
                    ret = db.GetUserConfig(nick, panelData.config);
                    if (1065 == ret) {
                        db.GetUserConfig(-1, panelData.config);
                    }
                    ret = db.GetUserStatus(
                        nick, panelData.compareDays, mode_str[mode], panelData.stat);

                    if (1065 == ret && 0 != panelData.compareDays) {
                        throw Sayobot::BaseException(
                            "小夜没有查到这个人"
                                + std::to_string(panelData.compareDays) + "天前的信息",
                            Sayobot::NO_DAILY_DATA);
                    } else if (0 != ret) {
                        panelData.stat.user_id = -1;
                    }
                }
            }
            if (panelData.config.user_id == -1) {
                panelData.config.username = nick;
            }
            osu_api::api_v1::GetUser(
                panelData.config.username, (osu_api::mode)mode, &panelData.uinfo);
            if (panelData.uinfo.playcount == 0) {
                if (this->sender != panelData.config.qq)
                    throw Sayobot::UserNotFound(
                        "阁下还没有玩过这个模式哦，赶紧去试试吧",
                        Sayobot::USER_SELF_NOT_PLAYED);
                else
                    throw Sayobot::UserNotFound("这个人还没有玩过这个模式哦",
                                                Sayobot::USER_OTHER_NOT_PLAYED);
            }

            MakePersonalCard(panelData);
        }

        void UserSet() {
            std::string nick;
            if (1 != this->args.size()) {
                for (auto it = this->args.begin(); it != this->args.end(); ++it) {
                    nick += *it;
                    nick += ' ';
                }
                nick = nick.substr(0, nick.size() - 1);
            } else {
                nick = this->args[0];
            }

            osu_api::user_info info;
            osu_api::api_v1::GetUser(nick, osu_api::mode::std, &info);
            Sayobot::Database db;
            db.Connect();
            db.NewUser(this->sender, info.username, info.user_id);
            db.Close();

            this->result =
                "阁下绑定成功啦，发送指令！o就可以查看阁下的资料卡。还有其"
                "他指令阁下可以通过help查看哦";
        }

        void UserUnset() {
            Sayobot::Database db;
            db.Connect();
            if (!db.UserExist(this->sender)) {
                throw Sayobot::UserNotFound("阁下还没绑定哦",
                                            Sayobot::USER_SELF_NOT_SET);
            }
            db.UserUnset(this->sender);
            db.Close();
            this->result =
                "啊咧咧，，阁下你叫什么名字呀，突然不记得了，快用 !set 告诉我吧";
        }

        void Help() {
            if (0 == this->args.size()) {
                this->result = MessageSegment::image("pic/Sayobot help.png");
            } else if (1 == this->args.size()) {
                const std::set<std::string> help_args = {
                    "框框", "背景", "个签", "大括号", "all"};
                if (0 == help_args.count(this->args[0])) {
                    throw Sayobot::InvaildArgumentException(
                        "help", "没有这个帮助的图片哦", Sayobot::NO_HELP_PIC);
                }
                char buffer[128];
                sprintf_s(buffer, 128, "pic/help-%s.png", this->args[0].c_str());
                this->result = MessageSegment::image(buffer);
            } else {
                throw Sayobot::InvaildArgumentsCount("help",
                                                     "help可以有0个或者1个参数哦阁下",
                                                     Sayobot::INVAILD_ARGUMENT_COUNT);
            }
        }

        void MakePersonalCard(const UserPanelData &data) {
            const std::vector<std::string> mode_str = {"/mode-osu-med.png",
                                                       "/mode-taiko-med.png",
                                                       "/mode-fruits-med.png",
                                                       "/mode-mania-med.png"};
            const std::vector<std::string> rank_str = {"/ranking-X-small.png",
                                                       "/ranking-XH-small.png",
                                                       "/ranking-S-small.png",
                                                       "/ranking-SH-small.png",
                                                       "/ranking-A-small.png"};
            char stemp[512];
            int64_t itemp;
            float ftemp;
            double dtemp;
            Sayobot::Image image;
            image.Create(1080, 1920);
#pragma region drawing
            // 绘制背景
            sprintf(stemp, BACKGROUND_PATH "%s", data.config.background.c_str());
            image.DrawPic(stemp, 0, 0);
            // 不透明贴图
            sprintf(stemp, "../png/fx%d.png", data.config.opacity);
            image.DrawPic(stemp, 0, 0);
            // 绘制个人信息框
            sprintf(stemp, EDGE_PATH "%s", data.config.edge.profile.c_str());
            image.DrawPic(stemp, 50, 20, 970, 600);
            // 绘制数据框
            for (int i = 0; i < 6; ++i) {
                sprintf(stemp, EDGE_PATH "%s", data.config.edge.data.c_str());
                image.DrawPic(stemp, 56 + 33.5 * i, 980 + 140 * i, 820, 140);
            }

            // 绘制签名框
            sprintf(stemp, EDGE_PATH "%s", data.config.edge.sign.c_str());
            image.DrawPic(stemp, 125, 570, 825, 150);
            // 绘制头像
            sprintf_s(stemp, 512, AVATAR_PATH "%d.png", data.uinfo.user_id);
            if (!fileExist(stemp)) {
                std::default_random_engine random(time(NULL));
                std::uniform_int_distribution<int> dist(100000, 900000);
                sprintf_s(stemp,
                          512,
                          "https://a.ppy.sh/%d?%d.png",
                          data.uinfo.user_id,
                          dist(random));
            }
            try {
                image.DrawPic(stemp, 165, 150, 350, 350);
            } catch (Magick::Exception &ex) {
                image.DrawPic(AVATAR_PATH "no-avatar.png", 165, 150, 350, 350);
            }
            // 绘制模式图标
            sprintf(stemp,
                    SKIN_PATH "%s%s",
                    data.config.skin.c_str(),
                    mode_str[(int)data.mode].c_str());
            image.DrawPic(stemp, 165, 150, 80, 80);
            // 绘制地球图标
            image.DrawPic(GLOBAL_PATH, 510, 150, 100, 100);
            // 绘制国旗
            sprintf(stemp,
                    COUNTRY_PATH "%s.png",
                    data.uinfo.country.size() ? data.uinfo.country.c_str() : "__");
            image.DrawPic(stemp, 560, 425, 80, 80);

            // 绘制rank图标
            for (int i = 0; i < 5; ++i) {
                sprintf(stemp,
                        SKIN_PATH "%s%s",
                        data.config.skin.c_str(),
                        rank_str[i].c_str());
                image.DrawPic(stemp, 165 + 120 * i, i % 2 ? 870 : 720, 82, 98);
            }
            Sayobot::TextStyle ts;
            // 绘制天数
            ts.color = data.config.color.time;
            ts.font_family = FONT_PATH + data.config.font.time;
            ts.pointsize = SMALL_POINTSIZE;
            if (data.compareDays != 0) {
                sprintf_s(stemp, 512, "compare with %u days ago", data.compareDays);
                image.Drawtext(stemp, ts, 30, 1840);
            }
            // 绘制时间
            time_t tt;
            time(&tt);
            strftime(
                stemp, 512, "%F %a %T by Sayobot with C++ & Magick++", localtime(&tt));
            image.Drawtext(stemp, ts, 30, 1880);
            // 绘制UID
            ts.color = data.config.color.profile;
            ts.font_family = FONT_PATH + data.config.font.profile;
            ts.pointsize = SMALL_POINTSIZE;
            sprintf(stemp, "UID: %d", data.uinfo.user_id);
            image.Drawtext(stemp, ts, 585, 365);
            // 绘制QQ号
            if (data.config.qq != -1) {
                sprintf(stemp, "QQ: %lld", data.config.qq);
            } else {
                sprintf(stemp, "QQ: unknown");
            }

            image.Drawtext(stemp, ts, 585, 395);
            // 绘制国家/地区排名
            itemp = data.uinfo.country_rank - data.stat.country_rank;
            ts.color = data.config.color.profile;
            ts.font_family = FONT_PATH + data.config.font.profile;
            ts.pointsize = SMALL_POINTSIZE;
            if (data.stat.user_id == -1) {
                sprintf_s(stemp, 512, "#%d", data.uinfo.country_rank);
            } else {
                sprintf_s(stemp,
                          512,
                          "#%d(%s%d)",
                          data.uinfo.country_rank,
                          itemp > 0 ? "↓" : "↑",
                          itemp < 0 ? -itemp : itemp);
            }
            image.Drawtext(stemp, ts, 660, 460);

            // 设置Data区块字体
            ts.color = data.config.color.data;
            ts.font_family = FONT_PATH + data.config.font.data;
            ts.pointsize = MID_POINTSIZE;
            // 绘制pp
            sprintf(stemp, "PPoint :     %.2f", data.uinfo.pp);
            image.Drawtext(stemp, ts, 140, 1210);
            // 绘制ranked score
            sprintf(stemp,
                    "Ranked Score : %s",
                    llToString(data.uinfo.ranked_score).c_str());
            image.Drawtext(stemp, ts, 106, 1070);
            // 绘制tth
            sprintf(stemp,
                    "Total Hits :    %s",
                    llToString(itemp + data.stat.total_hit).c_str());
            image.Drawtext(stemp, ts, 240, 1630);
            // 绘制pc
            sprintf_s(stemp, 512, "Playcount :    %d", data.uinfo.playcount);
            image.Drawtext(stemp, ts, 173, 1350);
            // 绘制lv
            sprintf(stemp, "Current Level :   %.2f", data.uinfo.level);
            image.Drawtext(stemp, ts, 274, 1770);
            // 绘制acc
            sprintf(stemp, "Hit Accuracy : %.2f%%", data.uinfo.accuracy);
            image.Drawtext(stemp, ts, 207, 1490);
            if (data.stat.user_id != -1) {
                // 绘制pp变化
                ftemp = data.uinfo.pp - data.stat.pp;
                ts.color = ftemp < 0 ? data.config.color.arrowdown
                                     : data.config.color.arrowup;
                sprintf_s(stemp,
                          512,
                          "%s%s",
                          ftemp < 0 ? "↓" : "↑",
                          DoubleRound2(ftemp < 0 ? -ftemp : ftemp).c_str());
                image.Drawtext(stemp, ts, 670, 1210);

                // 绘制ranked score变化
                itemp = data.uinfo.ranked_score - data.stat.ranked_score;
                ts.color = itemp < 0 ? data.config.color.arrowdown
                                     : data.config.color.arrowup;

                sprintf_s(stemp,
                          512,
                          "%s%s",
                          itemp < 0 ? "-" : "+",
                          llToString(itemp < 0 ? -itemp : itemp).c_str());
                image.Drawtext(stemp, ts, 626, 1070);
                // 绘制tth变化
                ts.color = itemp < 0 ? data.config.color.arrowdown
                                     : data.config.color.arrowup;
                itemp = data.uinfo.n300 + data.uinfo.n100 + data.uinfo.n50
                        - data.stat.total_hit;
                sprintf_s(stemp,
                          512,
                          "%s%s",
                          itemp < 0 ? "-" : "+",
                          llToString(itemp < 0 ? -itemp : itemp).c_str());
                image.Drawtext(stemp, ts, 780, 1630);

                // 绘制pc变化
                itemp = data.uinfo.playcount - data.stat.playcount;
                ts.color = itemp < 0 ? data.config.color.arrowdown
                                     : data.config.color.arrowup;
                sprintf_s(stemp,
                          512,
                          "%s%lld",
                          itemp < 0 ? "-" : "+",
                          itemp < 0 ? -itemp : itemp);
                image.Drawtext(stemp, ts, 713, 1350);

                // 绘制acc变化
                dtemp = data.uinfo.accuracy - data.stat.accuracy;
                ts.color = dtemp < 0 ? data.config.color.arrowdown
                                     : data.config.color.arrowup;

                sprintf(stemp,
                        "%s%.2f%%",
                        dtemp < 0 ? "↓" : "↑",
                        dtemp < 0 ? -dtemp : dtemp);
                image.Drawtext(stemp, ts, 747, 1490);
                // 绘制lv变化
                dtemp = data.uinfo.level - data.stat.level;
                ts.color = dtemp < 0 ? data.config.color.arrowdown
                                     : data.config.color.arrowup;

                sprintf(stemp,
                        "%s%.2f",
                        dtemp < 0 ? "-" : "+",
                        dtemp < 0 ? -dtemp : dtemp);
                image.Drawtext(stemp, ts, 814, 1770);
                // 绘制rank差值
                ts.color = data.config.color.name;
                ts.font_family = FONT_PATH + data.config.font.name;
                ts.pointsize = BIG_POINTSIZE;
                itemp = data.uinfo.count_ssh + data.uinfo.count_ss - data.stat.xh
                        - data.stat.x;
                sprintf(stemp,
                        "(%s%d)",
                        itemp < 0 ? "↓" : "↑",
                        itemp < 0 ? 0 - itemp : itemp);
                image.Drawtext(stemp, ts, 253, 860);

                itemp = data.uinfo.count_sh + data.uinfo.count_s - data.stat.sh
                        - data.stat.s;
                sprintf(stemp,
                        "(%s%d)",
                        itemp < 0 ? "↓" : "↑",
                        itemp < 0 ? 0 - itemp : itemp);
                image.Drawtext(stemp, ts, 494, 860);

                itemp = data.uinfo.count_a - data.stat.a;
                sprintf(stemp,
                        "(%s%d)",
                        itemp < 0 ? "↓" : "↑",
                        itemp < 0 ? 0 - itemp : itemp);
                image.Drawtext(stemp, ts, 735, 860);
            }

            // 绘制rank数量
            const std::vector<int> count_rank = {data.uinfo.count_ss,
                                                 data.uinfo.count_ssh,
                                                 data.uinfo.count_s,
                                                 data.uinfo.count_sh,
                                                 data.uinfo.count_a};
            ts.pointsize = BIG_POINTSIZE;
            ts.color = data.config.color.name;
            for (int i = 0; i < 5; ++i) {
                image.Drawtext(std::to_string(count_rank[i]),
                               ts,
                               253 + 120 * i,
                               i % 2 ? 940 : 790);
            }
            // 绘制全球排名差值
            if (data.stat.user_id != -1) {
                itemp = data.uinfo.global_rank - data.stat.global_rank;
                sprintf(stemp,
                        "(%s%d)",
                        itemp > 0 ? "↓" : "↑",
                        itemp < 0 ? -itemp : itemp);
                ts.color = itemp > 0 ? data.config.color.arrowdown
                                     : data.config.color.arrowup;
                ts.font_family = FONT_PATH + data.config.font.arrow;
                image.Drawtext(stemp, ts, 660, 270);
            }

            // 绘制全球排名
            ts.color = data.config.color.name;
            ts.font_family = FONT_PATH + data.config.font.name;
            image.Drawtext(std::to_string(data.uinfo.global_rank), ts, 600, 220);
            // 绘制名字
            image.Drawtext(data.uinfo.username, ts, 555, 325);
            // 绘制签名
            ts.color = data.config.color.sign;
            ts.font_family = FONT_PATH + data.config.font.sign;
            ts.gravity = MagickCore::GravityType::NorthGravity;
            ts.align = MagickCore::AlignType::CenterAlign;
            image.Drawtext(data.config.sign, ts, 540, 660);
#pragma endregion
            std::string hash = image.GetRandomHash();
            std::string filename = "data/image/" + hash + ".jpg";
            image.Save(filename);
            this->result = MessageSegment::image(hash + ".jpg");
        }

        void UpdateEdge() {
            const std::vector<std::string> fields = {
                "ProfileEdge", "DataEdge", "SignEdge"};
            const std::vector<std::string> fontColors = {
                "ProfileFontColor", "DataFontColor", "SignFontColor"};

            if (2 != this->args.size())
                throw Sayobot::InvaildArgumentsCount("更新框框",
                                                     "更新框框有2个参数哦阁下",
                                                     Sayobot::INVAILD_ARGUMENT_COUNT);
            int index;
            try {
                index = std::stoi(this->args[0]);
                if (index > 2 || index < 0) throw std::exception();
            } catch (...) {
                throw Sayobot::InvaildArgumentException(
                    "更新框框",
                    "更新框框第1个参数是0-2的数字哦阁下",
                    Sayobot::EDGE_NOT_FOUND);
            }
            std::string filename = this->args[1] + (0 == index ? "0.png" : "1.png");
            std::string filepath = EDGE_PATH + filename;
            if (!fileExist(filepath))
                throw Sayobot::FileNotFound("没有这个框框哦阁下",
                                            Sayobot::EDGE_NOT_FOUND);
            // 获取字体颜色
            std::string color;
            std::ifstream colorFile(EDGE_PATH + this->args[1] + "0.col");
            if (colorFile.is_open()) {
                colorFile >> color;
                color = "#" + color;
            } else {
                color = "default";
            }
            int ret;
            Sayobot::Database db;
            db.Connect();
            if (!db.UserExist(this->sender)) {
                throw Sayobot::UserNotFound(
                    "阁下还没绑定哦，用set把阁下的名字告诉我吧",
                    Sayobot::USER_SELF_NOT_SET);
            }
            if (color.compare("default")) {
                ret = db.UserChangeConfig(this->sender, fontColors[index], color);
            } else {
                ret = db.UserResetConfig(this->sender, fontColors[index]);
            }
            ret = db.UserChangeConfig(this->sender, fields[index], filename);
            db.Close();

            if (ret == 0) {
                if (0 == this->group) {
                    this->result = "更新成功";
                } else {
                    this->result = "更新成功，阁下查看私聊哦";
                }
            } else {
                this->result = "更新失败惹";
            }
        }

        void UpdateSign() {
            if (1 != this->args.size())
                throw Sayobot::InvaildArgumentsCount("更新个签",
                                                     "更新个签有1个参数哦阁下",
                                                     Sayobot::INVAILD_ARGUMENT_COUNT);

            Sayobot::Database db;
            db.Connect();
            if (!db.UserExist(this->sender)) {
                throw Sayobot::UserNotFound(
                    "阁下还没绑定哦，用set把阁下的名字告诉我吧",
                    Sayobot::USER_SELF_NOT_SET);
            }
            cq::utils::string_replace(this->args[0], "\'", "\\\'");
            int ret = db.UserChangeConfig(this->sender, "Sign", this->args[0]);
            db.Close();
            if (0 == ret) {
                if (0 == this->group) {
                    this->result = "更新成功";
                } else {
                    this->result = "更新成功，阁下查看私聊哦";
                }
            } else {
                this->result = "更新失败惹";
            }
        }

        void UpdateBackground() {
            if (1 != this->args.size())
                throw Sayobot::InvaildArgumentsCount("更换背景",
                                                     "更换背景有1个参数哦阁下",
                                                     Sayobot::INVAILD_ARGUMENT_COUNT);
            std::string filename = this->args[0] + ".png";
            if (!fileExist(BACKGROUND_PATH + filename))
                throw Sayobot::FileNotFound("没有这个背景哦阁下",
                                            Sayobot::BACKGROUND_NOT_FOUND);
            Sayobot::Database db;
            db.Connect();
            if (!db.UserExist(this->sender)) {
                throw Sayobot::UserNotFound(
                    "阁下还没绑定哦，用set把阁下的名字告诉我吧",
                    Sayobot::USER_SELF_NOT_SET);
            }
            db.UserChangeConfig(this->sender, "Background", filename);
            db.Close();
            if (0 == this->group) {
                this->result = "更新成功";
            } else {
                this->result = "更新成功，阁下查看私聊哦";
            }
        }

        void UpdateOpacity() {
            if (1 != this->args.size())
                throw Sayobot::InvaildArgumentsCount("更改透明度",
                                                     "更改透明度有1个参数哦阁下",
                                                     Sayobot::INVAILD_ARGUMENT_COUNT);
            int opacity;
            try {
                opacity = std::stoi(this->args[0]);
                if (opacity < 0 || opacity > 100) throw std::exception();
                if (opacity % 5) throw std::exception();
            } catch (...) {
                throw Sayobot::InvaildArgumentException(
                    "更改透明度",
                    "更改透明度第1个参数是0-100以内可被5整除的数字哦阁下",
                    Sayobot::INVAILD_OPACITY);
            }

            Sayobot::Database db;
            db.Connect();
            if (!db.UserExist(this->sender)) {
                throw Sayobot::UserNotFound(
                    "阁下还没绑定哦，用set把阁下的名字告诉我吧",
                    Sayobot::USER_SELF_NOT_SET);
            }
            db.UserChangeConfig(this->sender, "Opacity", this->args[0]);
            db.Close();
            if (0 == this->group) {
                this->result = "更新成功";
            } else {
                this->result = "更新成功，阁下查看私聊哦";
            }
        }

        void UpdateAvatar() {
            std::default_random_engine random(time(NULL));
            std::uniform_int_distribution<int> dist(100000, 900000);
            char temp[512];
            Sayobot::Database db;
            db.Connect();
            UserConfigData config = {0};
            int ret = db.GetUserConfig(this->sender, config);
            db.Close();

            if (1065 == ret) {
                throw Sayobot::UserNotFound(
                    "阁下还没绑定哦，用set把阁下的名字告诉我吧",
                    Sayobot::USER_SELF_NOT_SET);
            }
            sprintf_s(
                temp, 512, "https://a.ppy.sh/%d?%d.png", config.user_id, dist(random));
            std::string path = AVATAR_PATH + std::to_string(config.user_id) + ".png";
            try {
                Sayobot::NetConnection::DownloadPic(temp, path);
                this->result =
                    "更新头像完成，如果阁下的头像还是没有更新，等一会再试试吧";
            } catch (Magick::Exception) {
                this->result = "阁下好像没有游戏头像呢qwq";
            }
        }

        void Roll() {
            long long ret = 0;
            long long min = 0;
            long long max = 0;
            size_t pos;
            std::random_device rd;
            std::default_random_engine random(rd());
            this->result = "";
            switch (this->args.size()) {
            case 0: {
                std::uniform_int_distribution<int> dist(1, 100);
                this->result = std::to_string(dist(random));
                break;
            }
            case 1: {
                try {
                    max = std::stoll(this->args[0], 0, 0);
                } catch (std::out_of_range) {
                    this->result = "数字太大了啦，小夜算不过来惹TAT";
                    break;
                } catch (...) {
                    max = 100;
                }
                if (this->args[0] != std::to_string(max)) {
                    pos = this->args[0].find(u8"还是");
                    if (pos != std::string::npos && pos > 0
                        && pos + 6 < this->args[0].length()) {
                        if (rd() % 2) {
                            this->result = this->args[0].substr(0, pos);
                        } else {
                            this->result = this->args[0].substr(pos + 6);
                        }
                    } else if (this->args[0].size() > 4
                               && (pos = this->args[0].find(u8"不"))
                                      != std::string::npos
                               && this->args[0].find(this->args[0].substr(pos + 3, 1))
                                      < pos) {
                        if (rd() % 2)
                            this->result = "小夜觉得" + this->args[0].substr(pos + 3);
                        else
                            this->result =
                                "小夜觉得不" + this->args[0].substr(pos + 3);
                    } else if (this->args[0][1] == '~'
                               && this->args[0].length() >= 3
                               && args[0][0] < args[0][2]) {
                        char buf[4];
                        std::uniform_int_distribution<short> dist(args[0][0],args[0][2]);
                        buf[0] = dist(random);
                        buf[1] = 0;
                        this->result = buf;
                    }
                    break;
                }

                if (max > 1) {
                    std::uniform_int_distribution<long long> dist(1, max);
                    this->result = std::to_string(dist(random));
                    break;
                }
                if (max < 0) {
                    std::uniform_int_distribution<long long> dist(0, abs(max));
                    this->result = std::to_string(dist(random) + max);
                    break;
                }
                this->result = "说实话咱也看不懂阁下到底想做什么i";
                break;
            }
            case 2: {
                try {
                    min = std::stoll(this->args[0], 0, 0);
                    max = std::stoll(this->args[1], 0, 0);
                } catch (...) {
                    min = 0;
                    max = 0;
                }
                if (min < max && this->args[0] == std::to_string(min)
                    && this->args[1] == std::to_string(max)) {
                    std::uniform_int_distribution<int64_t> dist(min, max);
                    this->result = std::to_string(dist(random));
                } else {
                    if (args[0] != args[1]) {
                        std::uniform_int_distribution<int> dist(0,
                                                                this->args.size() - 1);
                        this->result = "当然是" + this->args[dist(random)] + "啦";
                    } else {
                        this->result = "这到底是为什么呢";
                    }
                }
                break;
            }
            default:
                break;
            }
            if (!this->result.length()) {
                this->result = "???这个东西好像并不能roll";
            }
            if ((pos = this->result.find(u8"我")) != std::string::npos) {
                this->result.replace(pos, 3, u8"你");
            }
            if ((pos = this->result.find(u8"？")) != std::string::npos) {
                this->result.replace(pos, 3, u8"！");
            }
            if ((pos = this->result.find(u8"吗")) != std::string::npos) {
                this->result.erase(pos, 3);
            }
            if ((pos = this->result.find(u8"呢")) != std::string::npos) {
                this->result.erase(pos, 3);
            }
            if ((pos = this->result.find(u8"嘛")) != std::string::npos) {
                this->result.erase(pos, 3);
            }
            return;
        }

        void Recommend() {
            int mode = 1;
            const std::vector<char> mode_char = {'o', 't', 'c', 'm'};
            int64_t qq = this->sender;
            std::string nick = "";
            Sayobot::Database db;
            json pp_diff_json;
            UserStatData now_stat;
            double diff_pp, now_pp;
            double min_diff, max_diff;

            if (0 != this->args.size()) {
                if (1 == this->args[this->args.size() - 1].length()) {
                    char c_mode = this->args[this->args.size() - 1][0];
                    for (; mode < 5; ++mode) {
                        if (c_mode == mode_char[mode - 1]) break;
                    }
                    if (mode > 4)
                        mode = 1;
                    else
                        this->args.pop_back();
                }

                for (auto it = this->args.begin(); it != this->args.end(); ++it)
                    nick += (*it);

                if (this->args.size() != 0) qq = 0;
                const std::regex cqCodeRegex(
                    "^\\[CQ:at,qq=(\\d+)\\]$",
                    std::regex_constants::ECMAScript | std::regex_constants::icase);

                if (std::regex_match(nick, cqCodeRegex)) {
                    auto it =
                        std::sregex_iterator(nick.begin(), nick.end(), cqCodeRegex);
                    qq = std::stoll((*it)[1].str());
                }
            }

            db.Connect();
            try {
                char query[512];
                sprintf_s(query,
                          512,
                          "select * from user_pp_diff where %s=\'%s\' and mode=\'%c\'",
                          qq ? "qq" : "nick",
                          qq ? std::to_string(qq).c_str() : nick.c_str(),
                          mode_char[mode - 1]);
                try {
                    pp_diff_json = db.Select(query)[0];
                } catch (Sayobot::QueryException &ex) {
                    pp_diff_json["diff_pp"] = "0.0";
                }
                if (qq) {
                    db.GetUserLatestedStatus(qq, mode_char[mode - 1], now_stat);
                } else {
                    db.GetUserLatestedStatus(nick, mode_char[mode - 1], now_stat);
                }
            } catch (Sayobot::QueryException &ex) {
                if (ex.Code() == 1065) {
                    if (qq == this->sender)
                        throw BaseException(
                            "阁下还没绑定哦，用set把阁下的名字告诉我吧",
                            Sayobot::USER_SELF_NOT_SET);
                    else
                        throw BaseException("小夜还不认识这个人哦，阁下把他介绍给我吧",
                                            Sayobot::USER_OTHER_NOT_SET);
                } else
                    throw ex;
            }
            diff_pp = std::stof(pp_diff_json["diff_pp"].get<std::string>());
            now_pp = now_stat.pp;
            min_diff = pow(now_pp, 0.4) * 0.195 * 0.95;
            max_diff = pow(now_pp, 0.4) * 0.195 + pow(diff_pp, 0.4) * 0.15;
            json post_data = json::object({});
            post_data["cmd"] = "beatmaplist";
            post_data["type"] = "search";
            post_data["limit"] = 100;
            post_data["class"] = 1;
            post_data["mode"] = mode == 1 ? mode : 15;
            post_data["stars"] = json::array({min_diff, max_diff});
            // post_data["random"] = true;

            char buffer[1024];
            // sprintf_s(
            //    buffer, 1024, "给阁下推荐的难度：%.2lf~%.2lf", min_diff, max_diff);
            // this->result = buffer;
            json receive_data =
                json::parse(NetConnection::HttpsPost(SAYO_API, post_data));
            std::random_device rd;
            std::default_random_engine random(rd());
            json beatmaplist = receive_data["data"];
            std::uniform_int_distribution<int> u(0, beatmaplist.size() - 1);
            unsigned sid = beatmaplist[u(random)]["sid"].get<unsigned>();
            std::vector<osu_api::beatmap_info> binfo(0);
            osu_api::api_v1::GetBeatmapset(sid, binfo);
            for (auto it : binfo) {
                if (it.stars > min_diff && it.stars < max_diff) {
                    sprintf_s(buffer,
                              1024,
                              "%s - %s\n"
                              "stars: %.2f aim: %.2f speed: %.2f\n"
                              "creator: %s\n\n"
                              "下载链接:\n"
                              "https://osu.ppy.sh/beatmapsets/%d\n"
                              "https://osu.sayobot.cn/?search=%d\n\n"
                              "（图片还在制作中...）",
                              it.title.c_str(),
                              it.artist.c_str(),
                              it.stars,
                              it.aim_stars,
                              it.speed_stars,
                              it.creator.c_str(),
                              it.beatmapset_id,
                              it.beatmapset_id);
                    this->result = buffer;
                }
            }
            // std::cout << receive_data.dump(2) << std::endl;
        }

        void Report() {
            // if (this->group)
            //{
            //    this->result = "请私聊小夜report哦";
            //    return;
            //}
            std::string msg;
            if (1 != this->args.size()) {
                for (auto it = this->args.begin(); it != this->args.end(); ++it) {
                    msg += *it;
                    msg += ' ';
                }
                msg = msg.substr(0, msg.size() - 1);
            } else {
                msg = this->args[0];
            }
            // send_private_message(SAYOKO, msg);
            std::ofstream report("../log/v2/report.txt", std::ios::app);
            report << this->sender << " - " << msg << std::endl;
            report.close();
            this->result = "阁下的意见已经报告给麻麻惹";
        }

        void Reset() {
            Sayobot::Database db;
            db.Connect();
            if (!db.UserExist(this->sender)) {
                throw Sayobot::UserNotFound(
                    "阁下还没绑定哦，用set把阁下的名字告诉我吧",
                    Sayobot::USER_SELF_NOT_SET);
            }
            const std::set<std::string> fields = {"Sign",
                                                  "background",
                                                  "profileEdge",
                                                  "dataEdge",
                                                  "SignEdge",
                                                  "profilefont",
                                                  "datafont",
                                                  "signfont",
                                                  "namefont",
                                                  "timefont",
                                                  "arrowfont",
                                                  "customdownfont",
                                                  "profilefontcolor",
                                                  "namefontcolor",
                                                  "bpdetailskin",
                                                  "arrowupfontcolor",
                                                  "arrowdownfontcolor",
                                                  "timecolor",
                                                  "opacity"};
            for (auto it = fields.begin(); it != fields.end(); ++it) {
                db.UserResetConfig(this->sender, *it);
            }
            db.Close();
            this->result = "已经恢复到初始状态了哦";
        }

        void sudo() {
            if (Command::sudoers.count(this->sender) == 0) {
                this->result = "";
                return;
            }

            Sayobot::Sudo p = Sayobot::Sudo(this->args);
            this->result = p.GetResult();
            cq::utils::string_trim(this->result);
            while (this->result[0] == '\n') this->result.erase(0, 1);
        }

        void groupSleep() {
            if (this->group != 0) {
                double hours = 6;
                if (this->args.size() != 0) {
                    std::string hours_str = this->args[0];
                    try {
                        hours = std::stod(hours_str);
                        if (hours > 10 || hours < 0) hours = 6;
                    } catch (...) {
                        hours = 6;
                    }
                }
                if (cq::can_send_record()) {
                    cq::send_message(this->target,
                                     cq::message::MessageSegment::record(
                                         "sayobot_records/oyasumi.mp3"));
                }
                cq::set_group_ban(this->group, this->sender, hours * 60 * 60);
            }
        }

        void UpdateFontColor() {
            const std::vector<std::string> fonttype = {
                "Time", "Name", "Profile", "Data", "Sign"};
        }

        void UpdateFont() {
            const std::vector<std::string> fonttype = {
                "Time", "Name", "Profile", "Data", "Sign"};
        }

        void zalgo() {
            if (this->args.size() != 0)
                this->result =
                    zalgo::zalgo(this->args[0], zalgo::zalgo_opt::normal, 2);
            else
                this->result = "";
        }

        static std::string llToString(int64_t n) {
            const char digit[4] = {' ', 'K', 'M', 'G'};
            long double v = n;
            int a = 0;
            while (v > 1000.0) {
                v /= 1000.0;
                ++a;
            }
            char *buf = new char[20];
            sprintf_s(buf, 20, "%.2Lf", v);
            std::string ret(buf);
            delete[] buf;
            return ret + digit[a];
        }

        static std::string DoubleRound2(double a) {
            char *buf = new char[20];
            sprintf_s(buf, 20, "%.2lf", a);
            std::string ret(buf);
            delete[] buf;
            return ret;
        }

        static bool fileExist(const std::string &path) {
            struct stat buffer;
            return (stat(path.c_str(), &buffer) == 0);
        }
    }; // namespace Sayobot

    const std::set<int64_t> Command::sudoers = {SAYOKO, DEBUGGER, COLA};
    const std::vector<std::string> Command::Commands = {"o",
                                                        "t",
                                                        "c",
                                                        "m",
                                                        "set",
                                                        "unset",
                                                        "help",
                                                        "更新框框",
                                                        "更新个签",
                                                        "更换背景",
                                                        "更改透明度",
                                                        "更新头像",
                                                        "roll",
                                                        "report",
                                                        "重置",
                                                        "sudo",
                                                        "sleep",
                                                        "recommend",
                                                        "zalgo"};
} // namespace Sayobot

#endif