#ifndef SAYOBOT_COMMAND
#define SAYOBOT_COMMAND

#include <string>
#include <vector>

using namespace std;

class Command {
public:
    virtual string Operation();
    void SetArguments(const vector<string>& args) {
        _args.clear();
        for (auto i : args) {
            _args.push_back(i);
        }
    }

protected:
    vector<string> _args;
};

class UserStatusCommand : public Command {
public:
    string Operation() {
    }

    static Command* GetInstance() {
        return new UserStatusCommand();
    }
};

#endif