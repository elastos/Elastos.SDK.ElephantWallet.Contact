#include <ContactTestCmd.hpp>

#include <iostream>
#include <sstream>

/* =========================================== */
/* === static variables initialize =========== */
/* =========================================== */
const std::vector<ContactTestCmd::CommandInfo> ContactTestCmd::gCommandInfoList {
    { 'h', "help",       ContactTestCmd::Help,      "      Print help usages." },
    { 'p', "print-info", ContactTestCmd::PrintInfo, "Print current contact infos." },
    { 'a', "add-friend", ContactTestCmd::AddFriend, "Add a friend by [did, ela address or carrier address]." },
};

/* =========================================== */
/* === static function implement ============= */
/* =========================================== */
int ContactTestCmd::Do(std::shared_ptr<elastos::Contact> contact,
                       const std::string& cmdLine,
                       std::string& errMsg)
{
    auto wsfront=std::find_if_not(cmdLine.begin(), cmdLine.end(),
                                 [](int c){return std::isspace(c);});
    auto wsback=std::find_if_not(cmdLine.rbegin(), cmdLine.rend(),
                                 [](int c){return std::isspace(c);}).base();
    auto trimCmdLine = (wsback <= wsfront ? std::string() : std::string(wsfront, wsback));

    std::istringstream iss(trimCmdLine);
    std::vector<std::string> args {std::istream_iterator<std::string>{iss},
                                   std::istream_iterator<std::string>{}};
    if(args.size() <= 0) {
        return 0;
    }
    const auto& cmd = args[0];

    for(const auto& cmdInfo : gCommandInfoList) {
        if(cmd.compare(0, 1, &cmdInfo.mCmd) != 0
        && cmd != cmdInfo.mLongCmd) {
            continue;
        }

        int ret = cmdInfo.mFunc(contact, args, errMsg);
        return ret;
    }

    errMsg = "Unknown command: " + cmd;
    return -10000;
}

/* =========================================== */
/* === class public function implement  ====== */
/* =========================================== */

/* =========================================== */
/* === class protected function implement  === */
/* =========================================== */


/* =========================================== */
/* === class private function implement  ===== */
/* =========================================== */
int ContactTestCmd::Help(std::shared_ptr<elastos::Contact> contact,
                         const std::vector<std::string>& args,
                         std::string& errMsg)
{
    std::cout << "Usage:" << std::endl;
    for(const auto& cmdInfo : gCommandInfoList) {
        std::cout << "  " << cmdInfo.mCmd << " | " << cmdInfo.mLongCmd << ": " << cmdInfo.mUsage << std::endl;
    }

    return 0;
}

int ContactTestCmd::PrintInfo(std::shared_ptr<elastos::Contact> contact,
                              const std::vector<std::string>& args,
                              std::string& errMsg)
{
    std::string value;

    auto weakUserMgr = contact->getUserManager();
    auto userMgr = weakUserMgr.lock();
    userMgr->serialize(value);
    std::cout << "==========================" << std::endl;
    std::cout << "UserInfo:" << std::endl;
    std::cout << value << std::endl;

    auto weakFriendMgr = contact->getFriendManager();
    auto friendMgr = weakFriendMgr.lock();
    friendMgr->serialize(value);
    std::cout << "FriendInfo:" << std::endl;
    std::cout << value << std::endl;


    return 0;
}

int ContactTestCmd::AddFriend(std::shared_ptr<elastos::Contact> contact,
                              const std::vector<std::string>& args,
                              std::string& errMsg)
{
    auto weakFriendMgr = contact->getFriendManager();
    auto friendMgr = weakFriendMgr.lock();
    if(friendMgr.get() == nullptr) {
        errMsg = "FriendManager has been released.";
        return -1;
    }

    auto friendId = args.size() > 1 ? args[1] : "";
    auto summary = args.size() > 2 ? args[2] : "";

    int ret = friendMgr->tryAddFriend(friendId, summary);
    if(ret < 0) {
        errMsg = "Failed to add friend ret=" + std::to_string(ret);
        return ret;
    }

    return 0;
}

