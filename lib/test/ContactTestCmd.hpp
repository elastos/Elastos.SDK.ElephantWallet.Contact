#ifndef _CONTACT_TEST_CMD_HPP_
#define _CONTACT_TEST_CMD_HPP_

#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>
//#include <CompatibleFileSystem.hpp>

class ContactTestCmd {
public:
    /*** type define ***/

    /*** static function and variable ***/
    static void Loop(const std::string& fifoFilePath);
    static int Do(const std::string& cmdLine, std::string& errMsg);

    /*** class function and variable ***/

protected:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/

private:
    /*** type define ***/
    struct CommandInfo {
        char mCmd;
        std::string mLongCmd;
        std::function<int(const std::vector<std::string>&, std::string&)> mFunc;
        std::string mUsage;
    };

    /*** static function and variable ***/
    static void ProcessCmd(const std::string& cmdLine);
    static void MonitorCmdPipe(const std::string& fifoFilePath);
    static int Unimplemention(const std::vector<std::string>& args,
                              std::string& errMsg);
    static int Help(const std::vector<std::string>& args,
                    std::string& errMsg);

    static int NewAndStartContact(const std::vector<std::string>& args,
                                  std::string& errMsg);
    static int StopAndDelContact(const std::vector<std::string>& args,
                                  std::string& errMsg);
    static int RecreateContact(const std::vector<std::string>& args,
                               std::string& errMsg);
    static int RestartContact(const std::vector<std::string>& args,
                              std::string& errMsg);

    static int GetUserInfo(const std::vector<std::string>& args,
                           std::string& errMsg);


    static int ListFriendInfo(const std::vector<std::string>& args,
                              std::string& errMsg);
    static int AcceptFriend(const std::vector<std::string>& args,
                            std::string& errMsg);
    static int AddFriend(const std::vector<std::string>& args,
                         std::string& errMsg);
    static int DelFriend(const std::vector<std::string>& args,
                         std::string& errMsg);
    static int SendTextMessage(const std::vector<std::string>& args,
                               std::string& errMsg);
    static int SendBinaryMessage(const std::vector<std::string>& args,
                                 std::string& errMsg);
    static int SendFileMessage(const std::vector<std::string>& args,
                               std::string& errMsg);

    static int NewAndSaveMnemonic(const std::vector<std::string>& args,
                                  std::string& errMsg);

    static std::thread gCmdPipeMonitor;
    static bool gQuitFlag;
    static const std::vector<CommandInfo> gCmdInfoList;

    /*** class function and variable ***/
    explicit ContactTestCmd() = delete;
    virtual ~ContactTestCmd() = delete;
};

/***********************************************/
/***** class template function implement *******/
/***********************************************/

/***********************************************/
/***** macro definition ************************/
/***********************************************/

#endif /* _CONTACT_TEST_CMD_HPP_ */

