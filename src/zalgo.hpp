#ifndef ZALGO_HPP
#define ZALGO_HPP

#include <time.h>

#include <iostream>
#include <random>
#include <string>
#include <cqcppsdk/cqcppsdk.h>

using namespace std;

#define ZALGO_UP_COUNT 50
#define ZALGO_MID_COUNT 23
#define ZALGO_DOWN_COUNT 40

namespace zalgo {
    static default_random_engine rand(time(NULL));
    enum zalgo_opt { mini = 1, normal, max };

    enum zalgo_type { up = 1, mid = 2, down = 4 };

    const wchar_t zalgo_up[ZALGO_UP_COUNT] = {L'\u030d',
                                              /*     ̍     */ L'\u030e',
                                              /*     ̎    */ L'\u0304',
                                              /*     ̄     */ L'\u0305', /*     ̅     */
                                              L'\u033f',
                                              /*     ̿     */ L'\u0311',
                                              /*     ̑    */ L'\u0306',
                                              /*     ̆     */ L'\u0310', /*     ̐     */
                                              L'\u0352',
                                              /*     ͒     */ L'\u0357',
                                              /*     ͗     */ L'\u0351',
                                              /*     ͑     */ L'\u0307', /*     ̇     */
                                              L'\u0308',
                                              /*     ̈     */ L'\u030a',
                                              /*     ̊     */ L'\u0342',
                                              /*     ͂     */ L'\u0343', /*     ̓     */
                                              L'\u0344',
                                              /*     ̈́     */ L'\u034a',
                                              /*     ͊     */ L'\u034b',
                                              /*     ͋     */ L'\u034c', /*     ͌     */
                                              L'\u0303',
                                              /*     ̃     */ L'\u0302',
                                              /*     ̂     */ L'\u030c',
                                              /*     ̌     */ L'\u0350', /*     ͐     */
                                              L'\u0300',
                                              /*     ̀     */ L'\u0301',
                                              /*     ́     */ L'\u030b',
                                              /*     ̋     */ L'\u030f', /*     ̏     */
                                              L'\u0312',
                                              /*     ̒     */ L'\u0313',
                                              /*     ̓     */ L'\u0314',
                                              /*     ̔     */ L'\u033d', /*     ̽     */
                                              L'\u0309',
                                              /*     ̉     */ L'\u0363',
                                              /*     ͣ     */ L'\u0364',
                                              /*     ͤ     */ L'\u0365', /*     ͥ     */
                                              L'\u0366',
                                              /*     ͦ     */ L'\u0367',
                                              /*     ͧ     */ L'\u0368',
                                              /*     ͨ     */ L'\u0369', /*     ͩ     */
                                              L'\u036a',
                                              /*     ͪ     */ L'\u036b',
                                              /*     ͫ     */ L'\u036c',
                                              /*     ͬ     */ L'\u036d', /*     ͭ     */
                                              L'\u036e',
                                              /*     ͮ     */ L'\u036f',
                                              /*     ͯ     */ L'\u033e',
                                              /*     ̾     */ L'\u035b', /*     ͛     */
                                              L'\u0346',
                                              /*     ͆     */ L'\u031a' /*     ̚     */};
    const wchar_t zalgo_mid[ZALGO_MID_COUNT] = {
        L'\u0315',
        /*     ̕     */ L'\u031b',
        /*     ̛     */ L'\u0340',
        /*     ̀     */ L'\u0341', /*     ́     */
        L'\u0358',
        /*     ͘     */ L'\u0321',
        /*     ̡     */ L'\u0322',
        /*     ̢     */ L'\u0327', /*     ̧     */
        L'\u0328',
        /*     ̨     */ L'\u0334',
        /*     ̴     */ L'\u0335',
        /*     ̵     */ L'\u0336', /*     ̶     */
        L'\u034f',
        /*     ͏     */ L'\u035c',
        /*     ͜     */ L'\u035d',
        /*     ͝     */ L'\u035e', /*     ͞     */
        L'\u035f',
        /*     ͟     */ L'\u0360',
        /*     ͠     */ L'\u0362',
        /*     ͢     */ L'\u0338', /*     ̸     */
        L'\u0337',
        /*     ̷     */ L'\u0361',
        /*     ͡     */ L'\u0489' /*     ҉_     */};
    const wchar_t zalgo_down[ZALGO_DOWN_COUNT] = {
        L'\u0316',
        /*     ̖     */ L'\u0317',
        /*     ̗     */ L'\u0318',
        /*     ̘     */ L'\u0319', /*     ̙     */
        L'\u031c',
        /*     ̜     */ L'\u031d',
        /*     ̝     */ L'\u031e',
        /*     ̞     */ L'\u031f', /*     ̟     */
        L'\u0320',
        /*     ̠     */ L'\u0324',
        /*     ̤     */ L'\u0325',
        /*     ̥     */ L'\u0326', /*     ̦     */
        L'\u0329',
        /*     ̩     */ L'\u032a',
        /*     ̪     */ L'\u032b',
        /*     ̫     */ L'\u032c', /*     ̬     */
        L'\u032d',
        /*     ̭     */ L'\u032e',
        /*     ̮     */ L'\u032f',
        /*     ̯     */ L'\u0330', /*     ̰     */
        L'\u0331',
        /*     ̱     */ L'\u0332',
        /*     ̲     */ L'\u0333',
        /*     ̳     */ L'\u0339', /*     ̹     */
        L'\u033a',
        /*     ̺     */ L'\u033b',
        /*     ̻     */ L'\u033c',
        /*     ̼     */ L'\u0345', /*     ͅ     */
        L'\u0347',
        /*     ͇     */ L'\u0348',
        /*     ͈     */ L'\u0349',
        /*     ͉     */ L'\u034d', /*     ͍     */
        L'\u034e',
        /*     ͎     */ L'\u0353',
        /*     ͓     */ L'\u0354',
        /*     ͔     */ L'\u0355', /*     ͕     */
        L'\u0356',
        /*     ͖     */ L'\u0359',
        /*     ͙     */ L'\u035a',
        /*     ͚     */ L'\u0323' /*     ̣     */};

    int randint(int min, int max) {
        uniform_int_distribution<int> u(min, max);
        return u(rand);
    }

    wchar_t rand_zalgo(const wchar_t arr[], int len) {
        return arr[randint(0, len - 1)];
    }

    std::wstring StringToWstring(const std::string& strInput, unsigned int uCodePage) {
#ifndef _WIN32
        if (strInput.empty()) {
            return L"";
        }
        std::string strLocale = setlocale(LC_ALL, "");
        const char* pSrc = strInput.c_str();
        unsigned int iDestSize = mbstowcs(NULL, pSrc, 0) + 1;
        wchar_t* szDest = new wchar_t[iDestSize];
        wmemset(szDest, 0, iDestSize);
        mbstowcs(szDest, pSrc, iDestSize);
        std::wstring wstrResult = szDest;
        delete[] szDest;
        setlocale(LC_ALL, strLocale.c_str());
        return wstrResult;
#else
        std::wstring strUnicode = L"";
        if (strInput.length() == 0) {
            return strUnicode;
        }
        int iLength =
            ::MultiByteToWideChar(uCodePage, 0, strInput.c_str(), -1, NULL, 0);
        wchar_t* szDest = new wchar_t[iLength + 1];
        memset(szDest, 0, (iLength + 1) * sizeof(wchar_t));

        ::MultiByteToWideChar(
            uCodePage, 0, strInput.c_str(), -1, (wchar_t*)szDest, iLength);
        strUnicode = szDest;
        delete[] szDest;
        return strUnicode;
#endif
    }

    std::string WstringToString(const std::wstring& wstrInput,
                                unsigned int uCodePage) {
#ifndef _WIN32
        if (wstrInput.empty()) {
            return "";
        }
        std::string strLocale = setlocale(LC_ALL, "");
        const wchar_t* pSrc = wstrInput.c_str();
        unsigned int iDestSize = wcstombs(NULL, pSrc, 0) + 1;
        char* szDest = new char[iDestSize];
        memset(szDest, 0, iDestSize);
        wcstombs(szDest, pSrc, iDestSize);
        std::string strResult = szDest;
        delete[] szDest;
        setlocale(LC_ALL, strLocale.c_str());
        return strResult;
#else
        std::string strAnsi = "";
        if (wstrInput.length() == 0) {
            return strAnsi;
        }

        int iLength = ::WideCharToMultiByte(
            uCodePage, 0, wstrInput.c_str(), -1, NULL, 0, NULL, NULL);
        char* szDest = new char[iLength + 1];
        memset((void*)szDest, 0, (iLength + 1) * sizeof(char));

        ::WideCharToMultiByte(
            uCodePage, 0, wstrInput.c_str(), -1, szDest, iLength, NULL, NULL);

        strAnsi = szDest;

        delete[] szDest;
        return strAnsi;
#endif
    }

    bool is_zalgo_char(wchar_t c) {
        for (int i = 0; i < ZALGO_UP_COUNT; ++i) {
            if (c == zalgo_up[i]) return true;
        }
        for (int i = 0; i < ZALGO_MID_COUNT; ++i) {
            if (c == zalgo_mid[i]) return true;
        }
        for (int i = 0; i < ZALGO_DOWN_COUNT; ++i) {
            if (c == zalgo_down[i]) return true;
        }
        return false;
    }

    string zalgo(const string& str, const zalgo_opt opt, unsigned type) {
        wstring temp = cq::utils::s2ws(str);
        string ret = "";
        int num_up, num_mid, num_down;
        for (auto it : temp) {
            wstring middle_str = L"";
            middle_str.push_back(it);
            switch (opt) {
            case zalgo_opt::mini:
                num_up = randint(0, 8);
                num_mid = randint(0, 2);
                num_down = randint(0, 8);
                break;
            case zalgo_opt::normal:
                num_up = randint(0, 16) / 2 + 1;
                num_mid = randint(0, 6) / 2;
                num_down = randint(0, 16) / 2 + 1;
                break;
            case zalgo_opt::max:
                num_up = randint(0, 64) / 4 + 3;
                num_mid = randint(0, 16) / 4 + 3;
                num_down = randint(0, 64) / 4 + 3;
                break;
            default:
                break;
            }

            if (type & zalgo_type::up) {
                for (int i = 0; i < num_up; ++i) {
                    middle_str.push_back(rand_zalgo(zalgo_up, ZALGO_UP_COUNT));
                }
            }
            if (type & zalgo_type::mid) {
                for (int i = 0; i < num_mid; ++i) {
                    middle_str.push_back(rand_zalgo(zalgo_mid, ZALGO_MID_COUNT));
                }
            }
            if (type & zalgo_type::down) {
                for (int i = 0; i < num_down; ++i) {
                    middle_str.push_back(rand_zalgo(zalgo_down, ZALGO_DOWN_COUNT));
                }
            }
            ret += cq::utils::ws2s(middle_str);
        }
        return ret;
    }

} // namespace zalgo

#endif