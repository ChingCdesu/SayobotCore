#pragma once
#ifndef SAYOBOT_HTTP_HPP
#define SAYOBOT_HTTP_HPP

#include <curl/curl.h>

#include <json.hpp>
#include <sstream>
#include <string>

#include "SayobotException.hpp"

using namespace nlohmann;

namespace Sayobot {
    size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp) {
        std::string data((char *)buffer, size * nmemb);
        *((std::stringstream *)userp) << data;
        return size * nmemb;
    }

    size_t download(void *buffer, size_t size, size_t nmemb, void *userp) {
        char *data = (char *)buffer;
        ((std::ofstream *)userp)->write(data, size * nmemb);
        return size * nmemb;
    }

    class NetConnection {
    public:
        static long Get(const std::string &url, std::string &response,
                        json headers = json::value_t::null) {
            CURL *curl = curl_easy_init();
            long status_code;
            std::stringstream ss;
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            if (!headers.is_null() && headers.is_object()) {
                curl_slist *curlHeaders = NULL;
                for (auto item : headers.items()) {
                    std::stringstream headerss;
                    headerss << item.key() << ":" << item.value();
                    curlHeaders =
                        curl_slist_append(curlHeaders, headerss.str().c_str());
                }
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curlHeaders);
            }
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &write_data);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ss);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
            status_code = curl_easy_perform(curl);
            if (status_code == CURLM_OK)
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);
            curl_easy_cleanup(curl);
            response.clear();
            response = ss.str();
            return status_code;
        }

        static long Post(const std::string &url, std::string &response,
                         json headers = json::value_t::null,
                         json datas = json::value_t::null) {
            CURL *curl = curl_easy_init();
            long status_code;
            std::stringstream ss;
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_POST, 1);
            if (!datas.is_null()) {
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, datas.dump().c_str());
            }
            if (!headers.is_null() && headers.is_object()) {
                curl_slist *curlHeaders = NULL;
                for (auto item : headers.items()) {
                    std::stringstream headerss;
                    headerss << item.key() << ":" << item.value();
                    curlHeaders =
                        curl_slist_append(curlHeaders, headerss.str().c_str());
                }
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curlHeaders);
            }
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &write_data);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ss);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
            status_code = curl_easy_perform(curl);
            if (status_code == CURLM_OK)
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);
            curl_easy_cleanup(curl);
            response.clear();
            response = ss.str();
            return status_code;
        }

        static long Download(const std::string &url, const std::string &filename,
                             json headers = json::value_t::null) {
            CURL *curl = curl_easy_init();
            long status_code;
            std::ofstream file(filename, std::ios::out | std::ios::binary);
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            if (!headers.is_null() && headers.is_object()) {
                curl_slist *curlHeaders = NULL;
                for (auto item : headers.items()) {
                    std::stringstream headerss;
                    headerss << item.key() << ":" << item.value();
                    curlHeaders =
                        curl_slist_append(curlHeaders, headerss.str().c_str());
                }
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curlHeaders);
            }
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &download);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
            status_code = curl_easy_perform(curl);
            if (status_code == CURLM_OK)
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);
            file.close();
            curl_easy_cleanup(curl);
            return status_code;
        }
    };

} // namespace Sayobot
#endif