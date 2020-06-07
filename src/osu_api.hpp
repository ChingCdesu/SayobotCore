//
//  osu_api.hpp
//  sayobot_test
//
//  Created by _ChingC on 3/14.
//  Copyright © 2020 _ChingC. All rights reserved.
//

#ifndef osu_api_hpp
#define osu_api_hpp

#include <time.h>

#include <string>
#include <vector>

#include "SayobotHttp.hpp"
#include "json.hpp"

#define OSU_API_V1 "https://osu.ppy.sh/api/"
#define SAYO_PPY_API "https://api.sayobot.cn/ppy/"
#define OSU_FILE_URL "https://osu.ppy.sh/osu/"
#define OSU_KEY "1b801cfce36ac400917b3551af8aca91538e8617"

// only for beatmap info with bid
#define SAYO_API "https://api.sayobot.cn/beatmapinfo?0="

#define OSU_API_V2 "https://osu.ppy.sh/api/v2/"

namespace osu_api {

    enum beatmap_status {
        graveyard = -2,
        WIP,
        pending,
        ranked,
        approved,
        qualified,
        loved
    };

    enum mode { std = 0, taiko, ctb, mania };

    struct beatmap_info {
        int beatmap_id, beatmapset_id;
        // not in Unicode
        std::string artist, title;
        // value is -1 if timestamp not existed
        int64_t submit_timestamp, lastupdate_timestamp, approved_timestamp;
        std::string creator;
        int creator_id;
        float bpm;
        double stars, aim_stars, speed_stars;
        float cs, ar, od, hp;
        std::vector<std::string> tags;
        std::string source;
        std::string version;
        std::string filemd5;
        // in seconds
        int game_length, total_length;
        beatmap_status approved;
        float rating;
        int favourite_count;
        int playcount, passcount;
        int count_circle, count_slider, count_spinner;
        int maxcombo;
        bool hasStoryboard, hasVideo, canDownload;
        mode mode;
    };

    struct user_info {
        int user_id;
        std::string username;
        long registed_timestamp;
        int n300, n100, n50;
        int playcount;
        int64_t total_score, ranked_score;
        int64_t total_hits;
        float pp;
        int country_rank, global_rank;
        int count_ssh, count_ss, count_sh, count_s, count_a;
        int playtime;
        float level;
        double accuracy;
        std::string country;
    };

    struct score_info {
        int beatmap_id;
        int64_t score;
        std::string username;
        int user_id;
        int n300, n100, n50, nmiss;
        // for ctb, taiko, mania
        int nkatu, ngeki;
        int combo;
        int mods;
        long achieved_timestamp;
        double pp;
        bool hasReplay;
        std::string rank;
    };

    class api_v1 {
    public:
        static void GetBeatmap(int bid, beatmap_info *info) {
            char url[512];
            sprintf(url, SAYO_PPY_API "get_beatmaps?b=%d", bid);
            std::string response;
            long status_code = Sayobot::NetConnection::Get(url, response);
            if (status_code != 200) {
                char msg[1024];
                sprintf(msg, "%s\nStatus Code: %ld", url, status_code);
                throw Sayobot::NetException(msg, status_code);
            }
            json data = json::parse(response);
            if (data.size() == 0) {
                throw Sayobot::BaseException("阁下，小夜找不到这个人呢",
                                             Sayobot::USER_BANNED);
            }
            data = data[0];
            info->approved =
                (beatmap_status)std::stoi(data["approved"].get<std::string>());
            tm time_temp;
            std::string temp_str;
            if (!data["submit_date"].is_null()) {
                strptime(data["submit_date"].get<std::string>().c_str(),
                         "%Y-%m-%d %H:%M:%S",
                         &time_temp);
                info->submit_timestamp = mktime(&time_temp) + 8 * 3600;
            } else {
                info->submit_timestamp = -1;
            }
            if (!data["approved_date"].is_null()) {
                strptime(data["approved_date"].get<std::string>().c_str(),
                         "%Y-%m-%d %H:%M:%S",
                         &time_temp);
                info->approved_timestamp = mktime(&time_temp) + 8 * 3600;
            } else {
                info->approved_timestamp = -1;
            }
            if (!data["last_update"].is_null()) {
                strptime(data["last_update"].get<std::string>().c_str(),
                         "%Y-%m-%d %H:%M:%S",
                         &time_temp);
                info->lastupdate_timestamp = mktime(&time_temp) + 8 * 3600;
            } else {
                info->lastupdate_timestamp = -1;
            }
            info->artist = data["artist_unicode"].is_null()
                               ? data["artist"].get<std::string>()
                               : data["artist_unicode"].get<std::string>();
            info->title = data["title_unicode"].is_null()
                              ? data["title"].get<std::string>()
                              : data["title_unicode"].get<std::string>();

            info->beatmap_id = std::stoi(data["beatmap_id"].get<std::string>());
            info->beatmapset_id = std::stoi(data["beatmapset_id"].get<std::string>());
            info->bpm = std::stof(data["bpm"].get<std::string>());
            info->creator = data["creator"].get<std::string>();
            info->creator_id = std::stoi(data["creator_id"].get<std::string>());
            info->stars = std::stod(data["difficultyrating"].get<std::string>());
            info->aim_stars = std::stod(data["diff_aim"].get<std::string>());
            info->speed_stars = std::stod(data["diff_speed"].get<std::string>());
            info->cs = std::stof(data["diff_size"].get<std::string>());
            info->ar = std::stof(data["diff_approach"].get<std::string>());
            info->od = std::stof(data["diff_overall"].get<std::string>());
            info->hp = std::stof(data["diff_drain"].get<std::string>());
            info->game_length = std::stoi(data["hit_length"].get<std::string>());
            info->total_length = std::stoi(data["total_length"].get<std::string>());
            info->version = data["version"].get<std::string>();
            info->filemd5 = data["file_md5"].get<std::string>();
            info->mode = (mode)std::stoi(data["mode"].get<std::string>());
            info->tags.clear();
            std::stringstream ss(data["tags"].get<std::string>());
            std::string temp;
            while (!ss.eof()) {
                ss >> temp;
                info->tags.push_back(temp);
            }
            info->favourite_count =
                std::stoi(data["favourite_count"].get<std::string>());
            info->rating = std::stof(data["rating"].get<std::string>());
            info->playcount = std::stoi(data["playcount"].get<std::string>());
            info->passcount = std::stoi(data["passcount"].get<std::string>());
            info->count_circle = std::stoi(data["count_normal"].get<std::string>());
            info->count_slider = std::stoi(data["count_slider"].get<std::string>());
            info->count_spinner = std::stoi(data["count_spinner"].get<std::string>());
            info->maxcombo = std::stoi(data["max_combo"].get<std::string>());
            info->hasStoryboard = std::stoi(data["storyboard"].get<std::string>());
            info->hasVideo = std::stoi(data["video"].get<std::string>());
            info->canDownload =
                std::stoi(data["download_unavailable"].get<std::string>()) != 0;
        }

        static void GetBeatmapset(int sid, std::vector<beatmap_info> &info) {
            info.clear();
            char url[512];
            sprintf(url, SAYO_PPY_API "get_beatmaps?s=%d", sid);
            std::string response;
            long status_code = Sayobot::NetConnection::Get(url, response);
            if (status_code != 200) {
                char msg[1024];
                sprintf(msg, "%s\nStatus Code: %ld", url, status_code);
                throw Sayobot::NetException(msg, status_code);
            }
            json data = json::parse(response);
            std::cout << data.dump(2) << std::endl;
            for (auto it : data) {
                beatmap_info single;
                single.approved =
                    (beatmap_status)std::stoi(it["approved"].get<std::string>());
                tm time_temp;
                std::string temp_str;
                if (!it["submit_date"].is_null()) {
                    strptime(it["submit_date"].get<std::string>().c_str(),
                             "%Y-%m-%d %H:%M:%S",
                             &time_temp);
                    single.submit_timestamp = mktime(&time_temp) + 16 * 3600;
                } else {
                    single.submit_timestamp = -1;
                }
                if (!it["approved_date"].is_null()) {
                    strptime(it["approved_date"].get<std::string>().c_str(),
                             "%Y-%m-%d %H:%M:%S",
                             &time_temp);
                    single.approved_timestamp = mktime(&time_temp) + 16 * 3600;
                } else {
                    single.approved_timestamp = -1;
                }
                if (!it["last_update"].is_null()) {
                    strptime(it["last_update"].get<std::string>().c_str(),
                             "%Y-%m-%d %H:%M:%S",
                             &time_temp);
                    single.lastupdate_timestamp = mktime(&time_temp) + 16 * 3600;
                } else {
                    single.lastupdate_timestamp = -1;
                }
                single.artist = it["artist_unicode"].is_null()
                                    ? it["artist"].get<std::string>()
                                    : it["artist_unicode"].get<std::string>();
                single.title = it["title_unicode"].is_null()
                                   ? it["title"].get<std::string>()
                                   : it["title_unicode"].get<std::string>();

                single.beatmap_id = std::stoi(it["beatmap_id"].get<std::string>());
                single.beatmapset_id =
                    std::stoi(it["beatmapset_id"].get<std::string>());
                single.bpm = std::stof(it["bpm"].get<std::string>());
                single.creator = it["creator"].get<std::string>();
                single.creator_id = std::stoi(it["creator_id"].get<std::string>());
                single.stars = std::stod(it["difficultyrating"].get<std::string>());
                single.aim_stars = std::stod(it["diff_aim"].get<std::string>());
                single.speed_stars = std::stod(it["diff_speed"].get<std::string>());
                single.cs = std::stof(it["diff_size"].get<std::string>());
                single.ar = std::stof(it["diff_approach"].get<std::string>());
                single.od = std::stof(it["diff_overall"].get<std::string>());
                single.hp = std::stof(it["diff_drain"].get<std::string>());
                single.game_length = std::stoi(it["hit_length"].get<std::string>());
                single.total_length = std::stoi(it["total_length"].get<std::string>());
                single.version = it["version"].get<std::string>();
                single.filemd5 = it["file_md5"].get<std::string>();
                single.mode = (mode)std::stoi(it["mode"].get<std::string>());
                single.tags.clear();
                std::stringstream ss(it["tags"].get<std::string>());
                std::string temp;
                while (!ss.eof()) {
                    ss >> temp;
                    single.tags.push_back(temp);
                }
                single.favourite_count =
                    std::stoi(it["favourite_count"].get<std::string>());
                single.rating = std::stof(it["rating"].get<std::string>());
                single.playcount = std::stoi(it["playcount"].get<std::string>());
                single.passcount = std::stoi(it["passcount"].get<std::string>());
                single.count_circle = std::stoi(it["count_normal"].get<std::string>());
                single.count_slider = std::stoi(it["count_slider"].get<std::string>());
                single.count_spinner =
                    std::stoi(it["count_spinner"].get<std::string>());
                single.maxcombo = std::stoi(it["max_combo"].get<std::string>());
                single.hasStoryboard = std::stoi(it["storyboard"].get<std::string>());
                single.hasVideo = std::stoi(it["video"].get<std::string>());
                single.canDownload =
                    std::stoi(it["download_unavailable"].get<std::string>()) != 0;
                info.push_back(single);
            }
        }

        static void GetUser(int uid, mode mode, user_info *info) {
            char url[512];
            sprintf(url, SAYO_PPY_API "get_user?u=%d&m=%d", uid, (int)mode);
            std::string response;
            long status_code = Sayobot::NetConnection::Get(url, response);
            if (status_code != 200) {
                char msg[1024];
                sprintf(msg, "%s\nStatus Code: %ld", url, status_code);
                throw Sayobot::NetException(msg, status_code);
            }
            json data = json::parse(response);
            if (data.size() == 0) {
                throw Sayobot::BaseException("阁下，小夜找不到这个人呢",
                                             Sayobot::USER_BANNED);
            }
            data = data[0];
            tm time_temp;
            info->user_id = std::stoi(data["user_id"].get<std::string>());
            info->username = data["username"].get<std::string>();
            strptime(data["join_date"].get<std::string>().c_str(),
                     "%Y-%m-%d %H:%M:%S",
                     &time_temp);
            info->registed_timestamp = mktime(&time_temp) + 16 * 3600;
            ;
            info->n50 = std::stoi(data["count50"].get<std::string>());
            info->n100 = std::stoi(data["count100"].get<std::string>());
            info->n300 = std::stoi(data["count300"].get<std::string>());
            info->playcount = std::stoi(data["playcount"].get<std::string>());
            info->ranked_score = std::stoll(data["ranked_score"].get<std::string>());
            info->total_score = std::stoll(data["total_score"].get<std::string>());
            info->global_rank = std::stoi(data["pp_rank"].get<std::string>());
            info->level = std::stof(data["level"].get<std::string>());
            info->pp = std::stof(data["pp_raw"].get<std::string>());
            info->accuracy = std::stod(data["accuracy"].get<std::string>());
            info->count_ss = std::stoi(data["count_rank_ss"].get<std::string>());
            info->count_ssh = std::stoi(data["count_rank_ssh"].get<std::string>());
            info->count_s = std::stoi(data["count_rank_s"].get<std::string>());
            info->count_sh = std::stoi(data["count_rank_sh"].get<std::string>());
            info->count_a = std::stoi(data["count_rank_a"].get<std::string>());
            info->country = data["country"].get<std::string>();
            info->playtime =
                std::stoi(data["total_seconds_played"].get<std::string>());
            info->country_rank = std::stoi(data["pp_country_rank"].get<std::string>());
            info->total_hits = std::stoll(data["count300"].get<std::string>())
                               + std::stoll(data["count100"].get<std::string>())
                               + std::stoll(data["count50"].get<std::string>());

        }

        static void GetUser(const std::string &username, mode mode, user_info *info) {
            char url[512];
            sprintf(url, SAYO_PPY_API "get_user?u=%s&m=%d", username.c_str(),
                    (int)mode);
            std::string response;
            long status_code = Sayobot::NetConnection::Get(url, response);
            if (status_code != 200) {
                char msg[1024];
                sprintf(msg, "%s\nStatus Code: %ld", url, status_code);
                throw Sayobot::NetException(msg, status_code);
            }
            json data = json::parse(response);
            if (data.size() == 0) {
                throw Sayobot::BaseException("阁下，小夜找不到这个人呢",
                                             Sayobot::USER_BANNED);
            }
            data = data[0];
            for (auto it = data.begin(); it != data.end(); ++it) {
                if (it.value() == json::value_t::null) {
                    (*it) = "0";
                }
            }
            tm time_temp;
            info->user_id = std::stoi(data["user_id"].get<std::string>());
            info->username = data["username"].get<std::string>();
            strptime(data["join_date"].get<std::string>().c_str(),
                     "%Y-%m-%d %H:%M:%S",
                     &time_temp);
            info->registed_timestamp = mktime(&time_temp) + 16 * 3600;
            ;
            info->n50 = std::stoi(data["count50"].get<std::string>());
            info->n100 = std::stoi(data["count100"].get<std::string>());
            info->n300 = std::stoi(data["count300"].get<std::string>());
            info->playcount = std::stoi(data["playcount"].get<std::string>());
            info->ranked_score = std::stoll(data["ranked_score"].get<std::string>());
            info->total_score = std::stoll(data["total_score"].get<std::string>());
            info->global_rank = std::stoi(data["pp_rank"].get<std::string>());
            info->level = std::stof(data["level"].get<std::string>());
            info->pp = std::stof(data["pp_raw"].get<std::string>());
            info->accuracy = std::stod(data["accuracy"].get<std::string>());
            info->count_ss = std::stoi(data["count_rank_ss"].get<std::string>());
            info->count_ssh = std::stoi(data["count_rank_ssh"].get<std::string>());
            info->count_s = std::stoi(data["count_rank_s"].get<std::string>());
            info->count_sh = std::stoi(data["count_rank_sh"].get<std::string>());
            info->count_a = std::stoi(data["count_rank_a"].get<std::string>());
            info->country = data["country"].get<std::string>();
            info->playtime =
                std::stoi(data["total_seconds_played"].get<std::string>());
            info->country_rank = std::stoi(data["pp_country_rank"].get<std::string>());
            info->total_hits = std::stoll(data["count300"].get<std::string>())
                               + std::stoll(data["count100"].get<std::string>())
                               + std::stoll(data["count50"].get<std::string>());
        }

        static void GetUserBest(int uid, int count, mode mode,
                                std::vector<score_info> &info) {
            char url[512];
            sprintf(url, SAYO_PPY_API "get_user_best?u=%d&m=%d&limit=%d", uid,
                    (int)mode,
                    count);
            std::string response;
            long status_code = Sayobot::NetConnection::Get(url, response);
            if (status_code != 200) {
                char msg[1024];
                sprintf(msg, "%s\nStatus Code: %ld", url, status_code);
                throw Sayobot::NetException(msg, status_code);
            }
            json data = json::parse(response);
            if (data.size() == 0) {
                throw Sayobot::BaseException("阁下，小夜找不到这个人呢",
                                             Sayobot::USER_BANNED);
            }
            info.clear();
            for (auto it : data) {
                score_info single;
                single.beatmap_id = std::stoi(it["beatmap_id"].get<std::string>());
                single.score = std::stoll(it["score"].get<std::string>());
                single.combo = std::stoi(it["maxcombo"].get<std::string>());
                single.n50 = std::stoi(it["count50"].get<std::string>());
                single.n100 = std::stoi(it["count100"].get<std::string>());
                single.n300 = std::stoi(it["count300"].get<std::string>());
                single.nkatu = std::stoi(it["countkatu"].get<std::string>());
                single.ngeki = std::stoi(it["countgeki"].get<std::string>());
                single.nmiss = std::stoi(it["countmiss"].get<std::string>());
                single.mods = std::stoi(it["enabled_mods"].get<std::string>());
                single.user_id = std::stoi(it["user_id"].get<std::string>());
                single.rank = it["rank"].get<std::string>();
                tm time_temp;
                strptime(it["date"].get<std::string>().c_str(),
                         "%Y-%m-%d %H:%M:%S",
                         &time_temp);
                single.achieved_timestamp = mktime(&time_temp) + 16 * 3600;
                info.push_back(single);
            }
        }

        static void GetUserRecent(int uid, mode mode, score_info *info) {
            char url[512];
            sprintf(url, SAYO_PPY_API "get_user_recent?u=%d&m=%d", uid,
                    (int)mode);
            std::string response;
            long status_code = Sayobot::NetConnection::Get(url, response);
            if (status_code != 200) {
                char msg[1024];
                sprintf(msg, "%s\nStatus Code: %ld", url, status_code);
                throw Sayobot::NetException(msg, status_code);
            }
            json data = json::parse(response);
            if (data.size() == 0) {
                throw Sayobot::BaseException("阁下，小夜找不到这个人呢",
                                             Sayobot::USER_BANNED);
            }
            data = data[0];
            info->beatmap_id = std::stoi(data["beatmap_id"].get<std::string>());
            info->score = std::stoll(data["score"].get<std::string>());
            info->combo = std::stoi(data["maxcombo"].get<std::string>());
            info->n50 = std::stoi(data["count50"].get<std::string>());
            info->n100 = std::stoi(data["count100"].get<std::string>());
            info->n300 = std::stoi(data["count300"].get<std::string>());
            info->nkatu = std::stoi(data["countkatu"].get<std::string>());
            info->ngeki = std::stoi(data["countgeki"].get<std::string>());
            info->nmiss = std::stoi(data["countmiss"].get<std::string>());
            info->mods = std::stoi(data["enabled_mods"].get<std::string>());
            info->user_id = std::stoi(data["user_id"].get<std::string>());
            info->rank = data["rank"].get<std::string>();
            tm time_temp;
            strptime(data["date"].get<std::string>().c_str(),
                     "%Y-%m-%d %H:%M:%S",
                     &time_temp);
            info->achieved_timestamp = mktime(&time_temp) + 16 * 3600;
        }

        static void GetUserRecent(const std::string &username, mode mode,
                                  score_info *info) {
            char url[512];
            sprintf(url, SAYO_PPY_API "get_user_recent?u=%s&m=%d", username.c_str(),
                    (int)mode);
            std::string response;
            long status_code = Sayobot::NetConnection::Get(url, response);
            if (status_code != 200) {
                char msg[1024];
                sprintf(msg, "%s\nStatus Code: %ld", url, status_code);
                throw Sayobot::NetException(msg, status_code);
            }
            json data = json::parse(response);
            if (data.size() == 0) {
                throw Sayobot::BaseException("阁下，小夜找不到这个人呢",
                                             Sayobot::USER_BANNED);
            }
            data = data[0];
            info->beatmap_id = std::stoi(data["beatmap_id"].get<std::string>());
            info->score = std::stoll(data["score"].get<std::string>());
            info->combo = std::stoi(data["maxcombo"].get<std::string>());
            info->n50 = std::stoi(data["count50"].get<std::string>());
            info->n100 = std::stoi(data["count100"].get<std::string>());
            info->n300 = std::stoi(data["count300"].get<std::string>());
            info->nkatu = std::stoi(data["countkatu"].get<std::string>());
            info->ngeki = std::stoi(data["countgeki"].get<std::string>());
            info->nmiss = std::stoi(data["countmiss"].get<std::string>());
            info->mods = std::stoi(data["enabled_mods"].get<std::string>());
            info->user_id = std::stoi(data["user_id"].get<std::string>());
            info->rank = data["rank"].get<std::string>();
            info->username = username;
            tm time_temp;
            strptime(data["date"].get<std::string>().c_str(),
                     "%Y-%m-%d %H:%M:%S",
                     &time_temp);
            info->achieved_timestamp = mktime(&time_temp) + 16 * 3600;
        }
    };

    // class api_v2 {
    //  public:
    //    api_v2(const std::string &oauth) { this->_oAuthKey = oauth; }
    //
    //    beatmap_info GetBeatmap(int bid) {}
    //
    //    static std::vector<beatmap_info> GetBeatmapset(int sid) {}
    //
    //    user_info GetUser(int uid) {}
    //
    //    user_info GetUser(const std::string &username) {}
    //
    //    std::vector<score_info> GetUserBest(int uid, int count) {}
    //
    //  private:
    //    std::string _oAuthKey;
    //};
} // namespace osu_api
#endif /* osu_apiv2_hpp */
