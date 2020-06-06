#pragma once
#ifndef SAYOBOT_HTTP_HPP
#define SAYOBOT_HTTP_HPP

#include <Magick++.h>
#include <Windows.h>
#include <schannel.h>
#include <winhttp.h>

#include <iostream>
#include <json.hpp>
#include <sstream>
#include <string>

#pragma comment(lib, "winhttp.lib")

#include "SayobotException.hpp"

using namespace nlohmann;

namespace Sayobot
{
    class NetConnection
    {
    public:
        static std::string HttpsGet(const std::string& url)
        {
            std::string ret("");

            std::wstring wUrl = cq::utils::s2ws(url);
            URL_COMPONENTS urlComp;
            ZeroMemory(&urlComp, sizeof(urlComp));
            urlComp.dwStructSize = sizeof(urlComp);

            urlComp.dwSchemeLength = (DWORD)-1;
            urlComp.dwHostNameLength = (DWORD)-1;
            urlComp.dwUrlPathLength = (DWORD)-1;
            urlComp.dwExtraInfoLength = (DWORD)-1;

            if (!WinHttpCrackUrl(wUrl.c_str(), (DWORD)wUrl.size(), 0, &urlComp))
                throw NetException("[Network] Error in WinHttpCrackUrl",
                                   GetLastError());

            DWORD dwSize = 0;
            DWORD dwDownloaded = 0;
            bool bResults = false;
            HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;
            hSession = WinHttpOpen(L"Sayobot with WinHttp API",
                                   WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                   WINHTTP_NO_PROXY_NAME,
                                   WINHTTP_NO_PROXY_BYPASS,
                                   0);
            if (hSession)
            {
                std::wstring hostname(urlComp.lpszHostName);
                hostname = hostname.substr(0, hostname.find_first_of(L'/'));
                hostname = hostname.substr(0, hostname.find_first_of(L':'));
                hConnect =
                    WinHttpConnect(hSession, hostname.c_str(), urlComp.nPort, 0);
            }

            if (hConnect)
            {
                hRequest = WinHttpOpenRequest(hConnect,
                                              L"GET",
                                              urlComp.lpszUrlPath,
                                              NULL,
                                              WINHTTP_NO_REFERER,
                                              WINHTTP_DEFAULT_ACCEPT_TYPES,
                                              WINHTTP_FLAG_SECURE);
            }
            if (hRequest)
            {
                bResults = WinHttpSendRequest(hRequest,
                                              WINHTTP_NO_ADDITIONAL_HEADERS,
                                              0,
                                              WINHTTP_NO_REQUEST_DATA,
                                              0,
                                              0,
                                              0);
            }
            if (bResults)
                bResults = WinHttpReceiveResponse(hRequest, NULL);
            if (bResults)
            {
                do
                {
                    LPSTR inBuf;
                    dwSize = 0;
                    if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
                        throw NetException(
                            "[Network] Error in WinHttpQueryDataAvailable",
                            GetLastError());
                    inBuf = new char[dwSize + 1];
                    if (!inBuf)
                        throw NetException("[Network] Out of Memory!", 1001);
                    ZeroMemory(inBuf, dwSize + 1);
                    if (!WinHttpReadData(
                            hRequest, (LPVOID)inBuf, dwSize, &dwDownloaded))
                        throw NetException("[Network] Error in WinHttpReadData!",
                                           GetLastError());
                    ret += inBuf;
                } while (dwSize > 0);
            }
            if (!bResults)
                throw NetException("[Network] Error has occurred!", GetLastError());

            if (hSession)
                WinHttpCloseHandle(hSession);
            if (hConnect)
                WinHttpCloseHandle(hConnect);
            if (hRequest)
                WinHttpCloseHandle(hRequest);
            return ret;
        }

        static void DownloadPic(const std::string& url, const std::string& path)
        {
            auto image = Magick::Image(url);
            image.quality(100);
            image.write(path);
        }

        static void DownloadFile(const std::string& url, const std::string& path)
        {
            std::string data_str = HttpsGet(url);
            BYTE* data = (BYTE*)data_str.c_str();

            std::ofstream file(path, std::ios::binary | std::ios::out);
            file << data;
            file.close();
            delete[] data;
        }

        static std::string HttpsPost(const std::string& url, const json& Data)
        {
#ifdef WIN32
            std::string ret("");

            std::string data_str = Data.dump();
            std::wstring wUrl = cq::utils::s2ws(url);
            URL_COMPONENTS urlComp;
            ZeroMemory(&urlComp, sizeof(urlComp));
            urlComp.dwStructSize = sizeof(urlComp);

            urlComp.dwSchemeLength = (DWORD)-1;
            urlComp.dwHostNameLength = (DWORD)-1;
            urlComp.dwUrlPathLength = (DWORD)-1;
            urlComp.dwExtraInfoLength = (DWORD)-1;

            if (!WinHttpCrackUrl(wUrl.c_str(), (DWORD)wUrl.size(), 0, &urlComp))
                throw NetException("[Network] Error in WinHttpCrackUrl",
                                     GetLastError());

            DWORD dwSize = 0;
            DWORD dwDownloaded = 0;
            bool bResults = false;
            HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;
            hSession = WinHttpOpen(L"Sayobot with WinHttp API",
                                   WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                   WINHTTP_NO_PROXY_NAME,
                                   WINHTTP_NO_PROXY_BYPASS,
                                   0);
            if (hSession)
            {
                std::wstring hostname(urlComp.lpszHostName);
                hostname = hostname.substr(0, hostname.find_first_of(L'/'));
                hostname = hostname.substr(0, hostname.find_first_of(L':'));
                hConnect =
                    WinHttpConnect(hSession, hostname.c_str(), urlComp.nPort, 0);
            }

            if (hConnect)
            {
                hRequest = WinHttpOpenRequest(hConnect,
                                              L"POST",
                                              urlComp.lpszUrlPath,
                                              L"HTTPS/1.1",
                                              WINHTTP_NO_REFERER,
                                              WINHTTP_DEFAULT_ACCEPT_TYPES,
                                              WINHTTP_FLAG_SECURE);
            }

            if (hRequest)
            {
                WinHttpAddRequestHeaders(
                    hRequest,
                    L"Content-Type: application/json;charset=utf-8",
                    -1L,
                    WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE);
                bResults = WinHttpSendRequest(hRequest,
                                              WINHTTP_NO_ADDITIONAL_HEADERS,
                                              0,
                                              (LPVOID)data_str.c_str(),
                                              data_str.length(),
                                              data_str.length(),
                                              0);
            }

            if (bResults)
                bResults = WinHttpReceiveResponse(hRequest, NULL);
            if (bResults)
            {
                do
                {
                    LPSTR inBuf;
                    dwSize = 0;
                    if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
                        throw NetException(
                            "[Network] Error in WinHttpQueryDataAvailable",
                            GetLastError());
                    inBuf = new char[dwSize + 1];
                    if (!inBuf)
                        throw NetException("[Network] Out of Memory!", 1001);
                    ZeroMemory(inBuf, dwSize + 1);
                    if (!WinHttpReadData(
                            hRequest, (LPVOID)inBuf, dwSize, &dwDownloaded))
                        throw NetException("[Network] Error in WinHttpReadData!",
                                             GetLastError());
                    ret += inBuf;
                } while (dwSize > 0);
            }
            if (!bResults)
                throw NetException("[Network] Error has occurred!", GetLastError());

            if (hSession)
                WinHttpCloseHandle(hSession);
            if (hConnect)
                WinHttpCloseHandle(hConnect);
            if (hRequest)
                WinHttpCloseHandle(hRequest);
            return ret;

#endif
        }
    };
} // namespace Sayobot
#endif