#pragma once
#ifndef SAYOBOT_SHARED_MEMORY_HPP
#define SAYOBOT_SHARED_MEMORY_HPP

#ifdef WIN32
#include <Windows.h>
#else
#include <random>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#endif

#include "SayobotException.hpp"
#include <iostream>
#include <string>

namespace Sayobot
{
    // 环状不带头序列
    struct GroupMessage
    {
        int64_t group_id;
        char Message[4][512];
        int64_t user_id[4];
        int nowIndex;
    };

    struct Errors
    {
        int32_t errorCodes[5];
        int nowIndex;
    };

    // Usage: auto memory = SharedMemory<int>("test"); (Windows)
    //		  auto memory = SharedMemory<int>("/tmp", 0600) (Linux)
    // NOTICE: NO STL, NO CONST

    template <class _Tc> class SharedMemory
    {
    public:
#ifdef WIN32
        SharedMemory(const char *Name)
        {
            memset(this->name, 0, 512);
            strcpy(this->name, Name);
            this->hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, Name);
            if (NULL == this->hMap)
                throw Sayobot::SharedMemoryException(
                    "Unable to get SharedMemory!",
                    Sayobot::SHARED_MEMORY_CANNOT_GET_BLOCK);
            this->lpMemory = MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);

            if (NULL == this->lpMemory)
            {
                throw Sayobot::SharedMemoryException(
                    "Unable to get space!", Sayobot::SHARED_MEMORY_CANNOT_GET_SPACE);
            }
        }
#else
        SharedMemory(const char *Path, int privileges = 0600)
        {
            std::default_random_engine random;
            std::uniform_int_distribution<int> dist(0, 255);
            int id = dist(random);
            strcpy(this->path, Path);
            this->key = ftok(path, id);
            if (-1 == this->key)
                throw SharedMemoryException("Unable to get key!", 10005);
            this->shmid = shmget(this->key, sizeof(_Tc), IPC_CREAT | privileges);
            if (-1 == this->shmid)
                throw SharedMemoryException("Unable to get shmid!", 10005);
            this->lpMemory = shmat(this->shmid, NULL, 0);
            if ((void*)-1 == this->lpMemory)
                throw SharedMemoryException("Unable to get space!", 10005);
        }

#endif
        void *GetPtr()
        {
            return this->lpMemory;
        }
#ifdef WIN32
        const char *GetName()
        {
            return this->name;
        }
#else
        const char *GetPath()
        {
            return this->path;
        }
#endif

        void Disconnect()
        {
#ifdef WIN32
            Destory();
#else
            if (this->lpMemory)
            {
                if (shmdt(this->lpMemory))
                    throw SharedMemoryException("Unable to release space!", 10005);
            }

#endif
        }

        void Destory()
        {
#ifdef WIN32
            if (NULL != this->lpMemory)
            {
                UnmapViewOfFile(this->lpMemory);
                this->lpMemory = NULL;
            }
            if (INVALID_HANDLE_VALUE != this->hMap)
            {
                CloseHandle(this->hMap);
                this->hMap = INVALID_HANDLE_VALUE;
            }
#else
            if (shmctl(this->shmid, IPC_RMID, NULL))
                throw SharedMemoryException("Unable to delete shared memory!", 10006);
#endif
        }

        ~SharedMemory()
        {
            Disconnect();
        }

    private:
        void *lpMemory;
#ifdef WIN32
        char name[512];
        HANDLE hMap;
#else
        key_t key;
        char path[512];
        int shmid;
#endif
    };

    class MessageListener
    {
    public:
        MessageListener()
        {
            for (int i = 0; i < 512; ++i)
                this->boxes[i] = GroupMessage();

            this->groupCount = 0;
            this->es.nowIndex = 0;
            for (int i = 0; i < 5; ++i)
            {
                this->es.errorCodes[i] = 0;
            }
        }
        // 插件启动的时候添加群
        void AddGroup(const int64_t group_id)
        {
            for (int i = 0; i < this->groupCount; ++i)
                if (group_id == this->boxes[i].group_id)
                    return;

            this->boxes[this->groupCount].group_id = group_id;
            for (int i = 0; i < 4; ++i)
            {
                memset(this->boxes[this->groupCount].Message[i], 0, 512);
            }
            ++this->groupCount;
        }

        // 群内新增消息
        bool PushMessage(const int64_t group_id, const int64_t user_id,
                         const char *msg)
        {
            int group_index = 0;
            for (; group_index < this->groupCount; ++group_index)
                if (group_id == this->boxes[group_index].group_id)
                    break;

            if (group_index != this->groupCount)
            {
                this->boxes[group_index].user_id[this->boxes[group_index].nowIndex] =
                    user_id;
                strcpy(this->boxes[group_index]
                             .Message[this->boxes[group_index].nowIndex],
                         msg);
                ++this->boxes[group_index].nowIndex;
                if (4 == this->boxes[group_index].nowIndex)
                    this->boxes[group_index].nowIndex = 0;
                return IsNeedBan(group_index);
            }
            return false;
        }

        bool PushError(int32_t errorCode)
        {
            this->es.errorCodes[this->es.nowIndex] = errorCode;
            ++this->es.nowIndex %= 5;
            return IsErrors();
        }

        void Display()
        {
            system("cls");
            std::cout << "监控的群数量：" << this->groupCount << std::endl;
        }

        void ListGroup()
        {
            system("cls");
            for (int i = 0; i < this->groupCount; ++i)
            {
                std::cout << this->boxes[i].group_id << std::endl;
            }
        }

    private:
        // 判断是否要被ban
        bool IsNeedBan(const unsigned group_index)
        {
            bool same[3];
            for (int i = 0; i < 3; ++i)
            {
                same[i] = !strcmp(this->boxes[group_index].Message[i],
                                  this->boxes[group_index].Message[i + 1]);
            }
            return same[0] & same[1] & same[2];
        }

        bool IsErrors()
        {
            bool flag = true;
            for (int i = 0; i < 4; ++i)
            {
                if (this->es.errorCodes[i] != this->es.errorCodes[i + 1])
                {
                    flag = false;
                    break;
                }
                if (this->es.errorCodes[i] == 0)
                {
                    flag = false;
                    break;
                }
            }
            if (flag)
            {
                for (int i = 0; i < 5; ++i)
                {
                    this->es.errorCodes[i] = 0;
                }
                this->es.nowIndex = 0;
            }
            return flag;
        }

    private:
        unsigned groupCount;
        GroupMessage boxes[512];
        Errors es;
    };
} // namespace Sayobot

#endif // !SAYOBOT_SHARED_MEMORY_HPP