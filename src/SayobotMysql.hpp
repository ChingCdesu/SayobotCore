#pragma once
#ifndef SAYOBOT_MYSQL_HPP
#define SAYOBOT_MYSQL_HPP

#include <mysql.h>

#include <algorithm>
#include <json.hpp>

#include "SayobotException.hpp"
#ifdef _MSC_VER
#include "Utils/strptime.c"
#endif
#include "osu_api.hpp"

#define SQL_USER "sayobot"
#define SQL_HOST "localhost"
#define SQL_PWD "Sayobot233*233"
#define SQL_DATABASE "sayobotuserinfo"
#define SQL_PORT 32145

using json = nlohmann::json;

namespace Sayobot {

    struct UserConfigData {
        int32_t user_id;
        int64_t qq;
        std::string username;
        std::string sign;
        std::string background;
        struct {
            std::string profile;
            std::string data, sign;
        } edge;
        struct {
            std::string profile, data, sign, time, arrow, name;
        } font;
        std::string skin;
        struct {
            std::string profile, data, sign, time, arrowup, arrowdown, name;
        } color;
        int opacity;
    };
    struct UserStatData {
        int32_t user_id;
        int64_t qq;
        std::string username;
        int64_t total_score, ranked_score;
        int32_t total_hit;
        double accuracy;
        float pp, level;
        int32_t global_rank, country_rank;
        std::string country;
        int64_t playcount;
        int32_t xh, x, sh, s, a;
        time_t update_timestamp;
        osu_api::mode mode;
        unsigned days;
    };

    class Database {
    public:
        /*
         * 数据库对象
         */
        Database() {
            mysql_init(&this->conn);
        }

        /*
         * 连接到数据库
         * 在使用完毕后必须调用Close()函数
         */
        void Connect() {
            if (mysql_real_connect(&this->conn,
                                   SQL_HOST,
                                   SQL_USER,
                                   SQL_PWD,
                                   SQL_DATABASE,
                                   SQL_PORT,
                                   NULL,
                                   0)
                == NULL) {
                throw SqlException(mysql_error(&this->conn), mysql_errno(&this->conn));
            }
            if (int t = mysql_set_character_set(&conn, "utf8") != NULL) {
                mysql_close(&this->conn);
                throw SqlException(mysql_error(&this->conn), t);
            }
        }

        void Execute(const std::string& sql) {
            if (0 != mysql_query(&this->conn, sql.c_str())) {
                throw QueryException(
                    sql, mysql_error(&this->conn), mysql_errno(&this->conn));
            }
        }

        void Commit() {
            mysql_commit(&this->conn);
        }

#pragma region SqlNative
        /*
         * 执行Update语句
         * 可能抛出的错误: Sayobot::QueryException
         */
        void Update(const std::string& updateSql) {
            std::string query = updateSql;
            query.erase(0, query.find_first_not_of(" "));
            query.erase(query.find_last_not_of(" ") + 1);
            std::string sub = query.substr(0, 6);
            std::transform(sub.begin(), sub.end(), sub.begin(), ::tolower);
            if (sub.compare("update")) {
                throw QueryException(
                    updateSql, "This Query is not an Update Query!", 1047);
            }
            if (0 != mysql_query(&this->conn, updateSql.c_str())) {
                throw QueryException(
                    updateSql, mysql_error(&this->conn), mysql_errno(&this->conn));
            }
            mysql_commit(&this->conn);
        }

        /*
         * 执行Insert语句
         * 可能抛出的错误: Sayobot::QueryException
         */
        void Insert(const std::string& insertSql) {
            std::string query = insertSql;
            query.erase(0, query.find_first_not_of(" "));
            query.erase(query.find_last_not_of(" ") + 1);
            std::string sub = query.substr(0, 6);
            std::transform(sub.begin(), sub.end(), sub.begin(), ::tolower);
            if (sub.compare("insert")) {
                throw QueryException(
                    insertSql, "This Query is not an Insert Query!", 1047);
            }
            if (0 != mysql_query(&this->conn, insertSql.c_str())) {
                throw QueryException(
                    insertSql, mysql_error(&this->conn), mysql_errno(&this->conn));
            }
            mysql_commit(&this->conn);
        }

        /*
         * 执行Delete语句
         * 可能抛出的错误: Sayobot::QueryException
         */
        void Delete(const std::string& deleteSql) {
            std::string query = deleteSql;
            query.erase(0, query.find_first_not_of(" "));
            query.erase(query.find_last_not_of(" ") + 1);
            std::string sub = query.substr(0, 6);
            std::transform(sub.begin(), sub.end(), sub.begin(), ::tolower);
            if (sub.compare("delete")) {
                throw QueryException(
                    deleteSql, "This Query is not an Delete Query!", 1047);
            }
            if (0 != mysql_query(&this->conn, deleteSql.c_str())) {
                throw QueryException(
                    deleteSql, mysql_error(&this->conn), mysql_errno(&this->conn));
            }
            mysql_commit(&this->conn);
        }

        /*
         * 执行Select语句
         * 返回值: json_object数组
         * 每一个object的key为字段，value为值的字符串表示
         * 一个object为一行符合条件的数据
         * 可能抛出的错误: Sayobot::QueryException, Sayobot::SqlException
         */
        json Select(const std::string& selectSql) {
            std::string query = selectSql;
            query.erase(0, query.find_first_not_of(" "));
            query.erase(query.find_last_not_of(" ") + 1);
            std::string sub = query.substr(0, 6);
            std::transform(sub.begin(), sub.end(), sub.begin(), ::tolower);
            if (sub.compare("select")) {
                throw QueryException(
                    selectSql, "This Query is not an Selete Query!", 1047);
            }

            MYSQL_RES* result;
            MYSQL_ROW row;
            json ret = json::array();
            if (mysql_query(&this->conn, selectSql.c_str()) == 0) {
                result = mysql_store_result(&this->conn);
                if (NULL != result) {
                    unsigned int cFields = mysql_num_fields(result);
                    MYSQL_FIELD* field;
                    std::vector<std::string> fields(cFields);
                    for (int i = 0; i < cFields; ++i) {
                        field = mysql_fetch_field(result);
                        fields[i] = field->name;
                    }

                    while (row = mysql_fetch_row(result)) {
                        int i = 0;
                        json aResult;
                        for (int i = 0; i < cFields; ++i) {
                            std::string value = row[i];
                            aResult[fields[i]] = value;
                        }
                        ret.push_back(aResult);
                    }
                } else {
                    throw SqlException(mysql_error(&this->conn),
                                       mysql_errno(&this->conn));
                }
                mysql_free_result(result);
                if (0 == ret.size()) {
                    throw QueryException(selectSql, "Empty Set!", 1065);
                }
                return ret;
            } else {
                throw QueryException(
                    selectSql, mysql_error(&this->conn), mysql_errno(&this->conn));
            }
        }
#pragma endregion

        /*
         * 获取用户资料卡设置
         * 参数: qq (int64_t)
         * 返回值: json_object数组
         * 可能抛出的错误: Sayobot::QueryException, Sayobot::SqlException
         */
        int GetUserConfig(int64_t qq, UserConfigData& data) {
            char buffer[512];
            sprintf_s(buffer, 512, "select * from UserConfig where QQ=%lld", qq);
            json p;
            try {
                p = Select(buffer)[0];
            } catch (Sayobot::QueryException& ex) {
                return ex.Code();
            }
            data.user_id = std::stoi(p["UserID"].get<std::string>());
            data.qq = std::stoll(p["QQ"].get<std::string>());
            data.username = p["Nick"].get<std::string>();
            data.sign = p["Sign"].get<std::string>();
            data.background = p["Background"].get<std::string>();

            data.edge.profile = p["ProfileEdge"].get<std::string>();
            data.edge.data = p["DataEdge"].get<std::string>();
            data.edge.sign = p["SignEdge"].get<std::string>();

            data.font.profile = p["ProfileFont"].get<std::string>();
            data.font.data = p["DataFont"].get<std::string>();
            data.font.sign = p["SignFont"].get<std::string>();
            data.font.name = p["NameFont"].get<std::string>();
            data.font.time = p["TimeFont"].get<std::string>();
            data.font.arrow = p["ArrowFont"].get<std::string>();

            data.skin = p["BPDetailSkin"].get<std::string>();

            data.color.profile = p["ProfileFontColor"].get<std::string>();
            data.color.data = p["DataFontColor"].get<std::string>();
            data.color.sign = p["SignFontColor"].get<std::string>();
            data.color.time = p["TimeFontColor"].get<std::string>();
            data.color.arrowup = p["ArrowUpFontColor"].get<std::string>();
            data.color.arrowdown = p["ArrowDownFontColor"].get<std::string>();
            data.color.name = p["NameFontColor"].get<std::string>();

            data.opacity = std::stoi(p["Opacity"].get<std::string>());

            return 0;
        }

        /*
         * 获取用户资料卡设置
         * 参数: Username (std::string)
         * 返回值: json_object数组
         * 可能抛出的错误: Sayobot::QueryException, Sayobot::SqlException
         */
        int GetUserConfig(const std::string& Username, UserConfigData& data) {
            char buffer[512];
            sprintf_s(buffer,
                      512,
                      "select * from UserConfig where nick=\'%s\'",
                      Username.c_str());
            json p;
            try {
                p = Select(buffer)[0];
            } catch (Sayobot::QueryException& ex) {
                return ex.Code();
            }
            data.user_id = std::stoi(p["UserID"].get<std::string>());
            data.qq = std::stoll(p["QQ"].get<std::string>());
            data.username = p["Nick"].get<std::string>();
            data.sign = p["Sign"].get<std::string>();
            data.background = p["Background"].get<std::string>();

            data.edge.profile = p["ProfileEdge"].get<std::string>();
            data.edge.data = p["DataEdge"].get<std::string>();
            data.edge.sign = p["SignEdge"].get<std::string>();

            data.font.profile = p["ProfileFont"].get<std::string>();
            data.font.data = p["DataFont"].get<std::string>();
            data.font.sign = p["SignFont"].get<std::string>();
            data.font.name = p["NameFont"].get<std::string>();
            data.font.time = p["TimeFont"].get<std::string>();
            data.font.arrow = p["ArrowFont"].get<std::string>();

            data.skin = p["BPDetailSkin"].get<std::string>();

            data.color.profile = p["ProfileFontColor"].get<std::string>();
            data.color.data = p["DataFontColor"].get<std::string>();
            data.color.sign = p["SignFontColor"].get<std::string>();
            data.color.time = p["TimeFontColor"].get<std::string>();
            data.color.arrowup = p["ArrowUpFontColor"].get<std::string>();
            data.color.arrowdown = p["ArrowDownFontColor"].get<std::string>();
            data.color.name = p["NameFontColor"].get<std::string>();

            data.opacity = std::stoi(p["Opacity"].get<std::string>());

            return 0;
        }

        /*
         * 获取用户昨天自动更新到数据库中的成绩
         * 参数: qq (int64_t), mode (const char, [o,c,t,m] 其中之一)
         * 返回值: json_object数组
         * 可能抛出的错误: Sayobot::QueryException, Sayobot::SqlException
         */
        int GetUserLatestedStatus(int64_t qq, const char mode, UserStatData& data) {
            char buffer[512];
            sprintf_s(buffer,
                      512,
                      "select * from userstats_withinfo WHERE qq=%lld AND MODE = "
                      "\'%c\' order by lastupdate desc limit 1",
                      qq,
                      mode);
            json p;
            try {
                p = Select(buffer)[0];
            } catch (Sayobot::QueryException& ex) {
                return ex.Code();
            }
            tm time_temp;

            data.user_id = std::stoi(p["UserID"].get<std::string>());
            data.qq = std::stoll(p["QQ"].get<std::string>());
            data.username = p["Nick"].get<std::string>();
            data.total_hit = std::stoi(p["TTH"].get<std::string>());
            data.total_score = std::stoll(p["TTS"].get<std::string>());
            data.ranked_score = std::stoll(p["RS"].get<std::string>());
            data.accuracy = std::stod(p["ACC"].get<std::string>());
            data.pp = std::stof(p["PP"].get<std::string>());
            data.level = std::stof(p["Level"].get<std::string>());
            data.global_rank = std::stoi(p["GR"].get<std::string>());
            data.country_rank = std::stoi(p["CR"].get<std::string>());
            data.country = p["Country"].get<std::string>();
            data.playcount = std::stoi(p["PC"].get<std::string>());
            data.xh = std::stoi(p["SSSC"].get<std::string>());
            data.x = std::stoi(p["GSSC"].get<std::string>());
            data.sh = std::stoi(p["SSC"].get<std::string>());
            data.s = std::stoi(p["GSC"].get<std::string>());
            data.a = std::stoi(p["AC"].get<std::string>());
            strptime(p["LastUpdate"].get<std::string>().c_str(),
                     "%Y-%m-%d %H:%M:%S",
                     &time_temp);
            data.update_timestamp = mktime(&time_temp);
            const std::vector<char> mode_char = {'o', 't', 'c', 'm'};
            int m = 0;
            const char ch = p["Mode"].get<std::string>()[0];
            for (auto c : mode_char) {
                if (c == ch) break;
                ++m;
            }
            data.mode = (osu_api::mode)m;

            return 0;
        }

        /*
         * 获取用户昨天自动更新到数据库中的成绩
         * 参数: Username (std::string), mode (const char, [o,c,t,m] 其中之一)
         * 返回值: json_object数组
         * 可能抛出的错误: Sayobot::QueryException, Sayobot::SqlException
         */
        int GetUserLatestedStatus(const std::string& Username, const char mode,
                                  UserStatData& data) {
            char buffer[512];
            sprintf_s(buffer,
                      512,
                      "select * from userstats_withinfo WHERE nick=\'%s\' AND MODE = "
                      "\'%c\' order by lastupdate desc limit 1",
                      Username.c_str(),
                      mode);

            json p;
            try {
                p = Select(buffer)[0];
            } catch (Sayobot::QueryException& ex) {
                return ex.Code();
            }
            tm time_temp;

            data.user_id = std::stoi(p["UserID"].get<std::string>());
            data.qq = std::stoll(p["QQ"].get<std::string>());
            data.username = p["Nick"].get<std::string>();
            data.total_hit = std::stoi(p["TTH"].get<std::string>());
            data.total_score = std::stoll(p["TTS"].get<std::string>());
            data.ranked_score = std::stoll(p["RS"].get<std::string>());
            data.accuracy = std::stod(p["ACC"].get<std::string>());
            data.pp = std::stof(p["PP"].get<std::string>());
            data.level = std::stof(p["Level"].get<std::string>());
            data.global_rank = std::stoi(p["GR"].get<std::string>());
            data.country_rank = std::stoi(p["CR"].get<std::string>());
            data.country = p["Country"].get<std::string>();
            data.playcount = std::stoi(p["PC"].get<std::string>());
            data.xh = std::stoi(p["SSSC"].get<std::string>());
            data.x = std::stoi(p["GSSC"].get<std::string>());
            data.sh = std::stoi(p["SSC"].get<std::string>());
            data.s = std::stoi(p["GSC"].get<std::string>());
            data.a = std::stoi(p["AC"].get<std::string>());
            strptime(p["LastUpdate"].get<std::string>().c_str(),
                     "%Y-%m-%d %H:%M:%S",
                     &time_temp);
            data.update_timestamp = mktime(&time_temp);
            const std::vector<char> mode_char = {'o', 't', 'c', 'm'};
            int m = 0;
            const char ch = p["Mode"].get<std::string>()[0];
            for (auto c : mode_char) {
                if (c == ch) break;
                ++m;
            }
            data.mode = (osu_api::mode)m;

            return 0;
        }

        /*
         * 获取用户自动更新到数据库中的成绩，自定义天数
         * 参数: Username (std::string), days_ago (unsigned), mode (const char,
         * [o,c,t,m] 其中之一) 返回值: json_object数组 可能抛出的错误:
         * Sayobot::QueryException, Sayobot::SqlException
         */
        int GetUserStatus(const std::string& Username, unsigned int days_ago,
                          const char mode, UserStatData& data) {
            if (days_ago == 0) {
                return GetUserLatestedStatus(Username, mode, data);
            }
            char buffer[512];
            sprintf_s(buffer,
                      512,
                      "select * from UserStats_withinfo where UserID in (select UserID from "
                      "UserConfig where "
                      "nick=\"%s\") and to_days(now())-"
                      "to_days(LastUpdate)=%d and mode=\'%c\'",
                      Username.c_str(),
                      days_ago,
                      mode);
            json p;
            try {
                p = Select(buffer)[0];
            } catch (Sayobot::QueryException& ex) {
                return ex.Code();
            }
            tm time_temp;

            data.user_id = std::stoi(p["UserID"].get<std::string>());
            data.qq = std::stoll(p["QQ"].get<std::string>());
            data.username = p["Nick"].get<std::string>();
            data.total_hit = std::stoi(p["TTH"].get<std::string>());
            data.total_score = std::stoll(p["TTS"].get<std::string>());
            data.ranked_score = std::stoll(p["RS"].get<std::string>());
            data.accuracy = std::stod(p["ACC"].get<std::string>());
            data.pp = std::stof(p["PP"].get<std::string>());
            data.level = std::stof(p["Level"].get<std::string>());
            data.global_rank = std::stoi(p["GR"].get<std::string>());
            data.country_rank = std::stoi(p["CR"].get<std::string>());
            data.country = p["Country"].get<std::string>();
            data.playcount = std::stoi(p["PC"].get<std::string>());
            data.xh = std::stoi(p["SSSC"].get<std::string>());
            data.x = std::stoi(p["GSSC"].get<std::string>());
            data.sh = std::stoi(p["SSC"].get<std::string>());
            data.s = std::stoi(p["GSC"].get<std::string>());
            data.a = std::stoi(p["AC"].get<std::string>());
            strptime(p["LastUpdate"].get<std::string>().c_str(),
                     "%Y-%m-%d %H:%M:%S",
                     &time_temp);
            data.update_timestamp = mktime(&time_temp);
            const std::vector<char> mode_char = {'o', 't', 'c', 'm'};
            int m = 0;
            const char ch = p["Mode"].get<std::string>()[0];
            for (auto c : mode_char) {
                if (c == ch) break;
                ++m;
            }
            data.mode = (osu_api::mode)m;

            return 0;
        }

        /*
         * 获取用户自动更新到数据库中的成绩，自定义天数
         * 参数: qq (int64_t), days_ago (unsigned), mode (const char, [o,c,t,m]
         * 其中之一) 返回值: json_object数组 可能抛出的错误: Sayobot::QueryException,
         * Sayobot::SqlException
         */
        int GetUserStatus(int64_t qq, unsigned int days_ago, const char mode,
                          UserStatData& data) {
            if (days_ago == 0) {
                return GetUserLatestedStatus(qq, mode, data);
            }
            char buffer[512];
            sprintf_s(buffer,
                      512,
                      "select * from UserStats_withinfo where UserID in (select UserID from "
                      "UserConfig "
                      "where QQ=%lld) and to_days(now()) - "
                      "to_days(LastUpdate)=%d and mode=\'%c\'",
                      qq,
                      days_ago,
                      mode);
            json p;
            try {
                p = Select(buffer)[0];
            } catch (Sayobot::QueryException& ex) {
                return ex.Code();
            }
            tm time_temp;

            data.user_id = std::stoi(p["UserID"].get<std::string>());
            data.qq = std::stoll(p["QQ"].get<std::string>());
            data.username = p["Nick"].get<std::string>();
            data.total_hit = std::stoi(p["TTH"].get<std::string>());
            data.total_score = std::stoll(p["TTS"].get<std::string>());
            data.ranked_score = std::stoll(p["RS"].get<std::string>());
            data.accuracy = std::stod(p["ACC"].get<std::string>());
            data.pp = std::stof(p["PP"].get<std::string>());
            data.level = std::stof(p["Level"].get<std::string>());
            data.global_rank = std::stoi(p["GR"].get<std::string>());
            data.country_rank = std::stoi(p["CR"].get<std::string>());
            data.country = p["Country"].get<std::string>();
            data.playcount = std::stoi(p["PC"].get<std::string>());
            data.xh = std::stoi(p["SSSC"].get<std::string>());
            data.x = std::stoi(p["GSSC"].get<std::string>());
            data.sh = std::stoi(p["SSC"].get<std::string>());
            data.s = std::stoi(p["GSC"].get<std::string>());
            data.a = std::stoi(p["AC"].get<std::string>());
            strptime(p["LastUpdate"].get<std::string>().c_str(),
                     "%Y-%m-%d %H:%M:%S",
                     &time_temp);
            data.update_timestamp = mktime(&time_temp);
            const std::vector<char> mode_char = {'o', 't', 'c', 'm'};
            int m = 0;
            const char ch = p["Mode"].get<std::string>()[0];
            for (auto c : mode_char) {
                if (c == ch) break;
                ++m;
            }
            data.mode = (osu_api::mode)m;

            return 0;
        }

        /*
         * 新用户绑定
         * 参数: qq (int64_t), Username (std::string), UserID (unsigned)
         * 可能抛出的错误: Sayobot::QueryException
         */
        int NewUser(int64_t qq, const std::string& Username, unsigned int UserID) {
            char buffer[1024];
            sprintf_s(
                buffer,
                1024,
                "insert into UserConfig (nick,qq,UserID) values (\"%s\",%lld,%u)",
                Username.c_str(),
                qq,
                UserID);
            try {
                Insert(buffer);
            } catch (Sayobot::QueryException& ex) {
                if (ex.Code() == 1062) {
                    if (std::string::npos != ex.Message().find("index_of_userid"))
                        throw Sayobot::BaseException("这个人已经绑定了哦阁下",
                                                     Sayobot::USER_OTHER_HAVE_SETTED);
                    else
                        throw Sayobot::BaseException(
                            "阁下已经绑定了哦，不可以再绑定其他id啦",
                            Sayobot::USER_SELF_HAVE_SETTED);
                }
            }
            return 0;
        }

        /*
         * 更新用户设置
         * 参数: qq (int64_t), field (std::string), value (std::string)
         * field为更新项，value为更新值
         * 可能抛出的错误: Sayobot::QueryException
         */
        int UserChangeConfig(int64_t qq, const std::string& field,
                             const std::string& value) {
            char buffer[512];
            sprintf_s(buffer,
                      512,
                      "update UserConfig set %s=\'%s\' where qq=%lld",
                      field.c_str(),
                      value.c_str(),
                      qq);
            try {
                Update(buffer);
            } catch (Sayobot::QueryException& ex) {
                return ex.Code();
            }
            return 0;
        }

        int UserResetConfig(int64_t qq, const std::string& field) {
            char buffer[512];
            sprintf_s(buffer,
                      512,
                      "update userconfig SET %s =( SELECT DEFAULT ( "
                      "%s ) FROM ( SELECT * FROM userconfig LIMIT 1 ) "
                      "AS b ) WHERE qq = %lld ",
                      field.c_str(),
                      field.c_str(),
                      qq);
            try {
                Update(buffer);
            } catch (Sayobot::QueryException& ex) {
                return ex.Code();
            }
            return 0;
        }

        /*
         * 用户取消绑定
         * 参数: qq (int64_t)
         * 可能抛出的错误: Sayobot::QueryException
         */
        int UserUnset(int64_t qq) {
            char buffer[512];
            sprintf_s(buffer, 512, "delete from userConfig where qq=%lld", qq);
            try {
                Delete(buffer);
            } catch (Sayobot::QueryException& ex) {
                return ex.Code();
            }
            return 0;
        }

        bool UserExist(int64_t qq) {
            char buffer[512];
            sprintf_s(buffer,
                      512,
                      "select * from UserConfig where qq=%lld",
                      qq);
            json p;
            try {
                p = Select(buffer)[0];
            } catch (...) {
                return false;
            }
            return true;
        }

        bool UserExist(std::string nick) {
            char buffer[512];
            sprintf_s(buffer,
                      512,
                      "select * from UserConfig where nick=\'%s\'",
                      nick.c_str());
            json p;
            try {
                p = Select(buffer)[0];
            } catch (...) {
                return false;
            }
            return true;
        }
        /*
         * 关闭数据库连接
         * 这是个好习惯
         */
        void Close() {
            if (this->conn.net.vio != NULL) mysql_close(&this->conn);
        }

        ~Database() {
            Close();
        }

    private:
        MYSQL conn;
    };
} // namespace Sayobot
#endif