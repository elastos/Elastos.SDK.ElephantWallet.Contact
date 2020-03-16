#include <ContactTestCmd.hpp>
#include <ContactTest.hpp>

#include <fstream>
#include <iostream>
#include <iterator>
#include <signal.h>
#include <sstream>
#include <Log.hpp>
#include <Platform.hpp>

void signalHandler(int sig) {
    std::cerr << "Error: signal " << sig << std::endl;

    std::string backtrace = elastos::Platform::GetBacktrace();
    std::cerr << backtrace << std::endl;

    exit(sig);
}

int main(int argc, char **argv)
{
    signal(SIGSEGV, signalHandler);
    signal(SIGABRT, signalHandler);

    Log::I(Log::TAG, "Start Contact Test.");
    Log::I(Log::TAG, "%s\n", (argc > 1 ? argv[1]:""));
    ContactTest::GetInstance()->init();

    const char* fifoFilePath = (argc > 1 ? argv[1] : "");

    ContactTestCmd::Loop(fifoFilePath);

    return 0;
}

/* =========================================== */
/* === static variables initialize =========== */
/* =========================================== */
std::thread ContactTestCmd::gCmdPipeMonitor;
bool ContactTestCmd::gQuitFlag;
const std::vector<ContactTestCmd::CommandInfo> ContactTestCmd::gCmdInfoList{
    { 'q', "quit",           nullptr,                             "\t\tQuit." },
    { 'h', "help",           ContactTestCmd::Help,                "\t\tPrint help usages. [h]" },

    { '-', "",               nullptr,                             "\nContact" },
    { 'n', "ns-contact",     ContactTestCmd::NewAndStartContact,  "\tNew and Start Contact [n]" },
    { ' ', "sd-contact",     ContactTestCmd::StopAndDelContact,   "\tStop and Del Contact" },
    { ' ', "rc-contact",     ContactTestCmd::RecreateContact,     "\tRecreate Contact" },
    { ' ', "rs-contact",     ContactTestCmd::RestartContact,      "\tRestart Contact" },

    { '-', "",                nullptr,                             "\n User" },
    { 'g', "get-uinfo",       ContactTestCmd::GetUserInfo,         "\tGet User Info [g]" },
    { 'r', "get-ubrief",      ContactTestCmd::GetUserBrief,         "\tGet User Brief [r]" },
    { ' ', "set-uid",         ContactTestCmd::Unimplemention,      "\t\tSet User Identifycode" },
    { 's', "set-udetails",    ContactTestCmd::SetUserDetails,      "\tSet User Details [s [4:Nickname|7:Description|8:Addition] ${text}]" },
    { ' ', "set-uwaddr",      ContactTestCmd::Unimplemention,      "\tSet User Wallet Address" },
    { 'u', "sync-upload",     ContactTestCmd::SyncUpload,          "\tSync Upload" },
    { ' ', "sync-download",   ContactTestCmd::Unimplemention,      "\tSync Download" },
    { 'w', "export-userdata", ContactTestCmd::ExportUserData,      "\tExport User Data [x ${toFilePath}]" },
    { 'x', "import-userdata", ContactTestCmd::ImportUserData,      "\tImport User Data [w ${fromFilePath}]" },
    { 'o', "loop-message",    ContactTestCmd::LoopMessage,         "\tTest Loop Message" },

    { '-', "",               nullptr,                             "\n Friend" },
    { 'l', "list-finfo",     ContactTestCmd::ListFriendInfo,      "\tList Friend Info [e]" },
    { 'e', "accept-frd",     ContactTestCmd::AcceptFriend,        "\tAccept Friend [e fid]" },
    { 'a', "add-friend",     ContactTestCmd::AddFriend,           "\tAdd Friend [a fid ${summary}]" },
    { 'd', "del-friend",     ContactTestCmd::DelFriend,           "\tDel Friend" },
    { 'i', "set-fdetails",   ContactTestCmd::SetFriendDetails,    "\tSet Friend Details [t fid [4:Nickname|7:Description|8:Addition] ${text}]" },
    { 't', "send-tmsg",      ContactTestCmd::SendTextMessage,     "\tSend Text Message [t fid ${text} ${memo} ${replyTo}] " },
    { 'b', "send-bmsg",      ContactTestCmd::SendBinaryMessage,   "\tSend Binary Message [b fid ${binary}]" },
    { 'f', "send-fmsg",      ContactTestCmd::SendFileMessage,     "\tSend File Message [f fid ${filepath}]" },
    { 'p', "pull-file",      ContactTestCmd::Unimplemention,      "\tPull File" },
    { 'c', "cancel-pfile",   ContactTestCmd::Unimplemention,      "\tCancel Pull File" },


    { '-', "",               nullptr,                              "\n Mnemonic" },
    { 'm', "new-mnemonic",   ContactTestCmd::NewAndSaveMnemonic,   "\t\tnew mnemonic" },

    { '-', "",               nullptr,                              "\n Debug" },
    { 'y', "cached-didprop", ContactTestCmd::ShowCachedDidProp,    "\t\tShow Cached DidProp [y]" },
    { 'z', "thread-sendmsg", ContactTestCmd::ThreadSendMessage,    "\t\tLoop Send Message in new thread [z fid]" },
};

/* =========================================== */
/* === static function implement ============= */
/* =========================================== */
void ContactTestCmd::Loop(const std::string& fifoFilePath)
{
    MonitorCmdPipe(fifoFilePath);

    while (true) {
        std::string cmdLine;
        std::getline(std::cin, cmdLine);

        if (cmdLine == "q" || cmdLine == "quit") {
            gQuitFlag = true;
            return;
        }
        ProcessCmd(cmdLine);
    }
}

int ContactTestCmd::Do(const std::string& cmdLine,
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
    if (args.size() <= 0) {
        return 0;
    }
    const auto& cmd = args[0];

    for(const auto& cmdInfo : gCmdInfoList) {
        if(cmdInfo.mCmd == ' '
        || cmdInfo.mFunc == nullptr) {
            continue;
        }
        if(cmd.compare(0, 1, &cmdInfo.mCmd) != 0
        && cmd != cmdInfo.mLongCmd) {
            continue;
        }

        int ret = cmdInfo.mFunc(args, errMsg);
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
void ContactTestCmd::ProcessCmd(const std::string& cmdLine)
{
    if (cmdLine.empty() == true) {
        std::cout << "# ";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return;
    }
    Log::I(Log::TAG, "==> Received Command: %s", cmdLine.c_str());

    std::string errMsg;
    int ret = ContactTestCmd::Do(cmdLine, errMsg);
    if (ret < 0) {
        Log::E(Log::TAG, "ErrCode(%d): %s", ret, errMsg.c_str());
    } else {
        Log::I(Log::TAG, "Success to exec: %s", cmdLine.c_str());
    }
    std::cout << "# ";
}

void ContactTestCmd::MonitorCmdPipe(const std::string& fifoFilePath)
{
    if (fifoFilePath.empty()) {
        return;
    }

    auto funcPipeMonitor = [=] {
        while (true) {
            std::string cmdLine;
            std::ifstream fifoStream(fifoFilePath, std::ifstream::in);
            std::getline(fifoStream, cmdLine);
            fifoStream.close();

            if (cmdLine == "q" || cmdLine == "quit") {
                gQuitFlag = true;
                return;
            }
            ProcessCmd(cmdLine);
        };
    };

    gCmdPipeMonitor = std::thread (funcPipeMonitor);
}

int ContactTestCmd::Unimplemention(const std::vector<std::string>& args,
                                   std::string& errMsg)
{
    errMsg = "Unimplemention Command!!!";
    return -1;
}

int ContactTestCmd::Help(const std::vector<std::string>& args,
                         std::string& errMsg)
{
    std::cout << "Usage:" << std::endl;
    for(const auto& cmdInfo : gCmdInfoList) {
        if(cmdInfo.mCmd == '-') {
            std::cout << cmdInfo.mUsage << std::endl;
        } else {
            std::cout << "  " << cmdInfo.mCmd << " | " << cmdInfo.mLongCmd << ": " << cmdInfo.mUsage << std::endl;
        }
    }
    std::cout << std::endl;

    return 0;
}

int ContactTestCmd::NewAndStartContact(const std::vector<std::string>& args,
                                       std::string& errMsg)
{
    auto ret = ContactTest::GetInstance()->testNewContact();
    if(ret < 0) {
        return ret;
    }

    ret = ContactTest::GetInstance()->testStartContact();

    return ret;
}

int ContactTestCmd::StopAndDelContact(const std::vector<std::string>& args,
                                      std::string& errMsg)
{
    auto ret = ContactTest::GetInstance()->testStopContact();
    if(ret < 0) {
        return ret;
    }

    ret = ContactTest::GetInstance()->testDelContact();

    return ret;
}

int ContactTestCmd::RecreateContact(const std::vector<std::string>& args,
                                    std::string& errMsg)
{
    auto ret = StopAndDelContact(args, errMsg);
    if(ret < 0) {
        return ret;
    }

    ret = NewAndStartContact(args, errMsg);

    return ret;
}

int ContactTestCmd::RestartContact(const std::vector<std::string>& args,
                                   std::string& errMsg)
{
    auto ret = ContactTest::GetInstance()->testStopContact();
    if(ret < 0) {
        return ret;
    }

    ret = ContactTest::GetInstance()->testStartContact();

    return ret;
}

int ContactTestCmd::GetUserInfo(const std::vector<std::string>& args,
                                std::string& errMsg)
{
    auto ret = ContactTest::GetInstance()->showGetUserInfo();

    return ret;
}

int ContactTestCmd::GetUserBrief(const std::vector<std::string>& args,
                                 std::string& errMsg)
{
    auto ret = ContactTest::GetInstance()->showGetUserBrief();

    return ret;
}

int ContactTestCmd::SetUserDetails(const std::vector<std::string>& args,
                                     std::string& errMsg)
{
    if(args.size() < 3) {
        errMsg = "Bad input count: " + std::to_string(args.size());
        return -1;
    }

    auto infoType = std::stoi(args[1]);
    auto infoData = args[2];

    auto ret = ContactTest::GetInstance()->doSetHumanDetails("-user-info-", infoType, infoData);
    return ret;
}

int ContactTestCmd::SyncUpload(const std::vector<std::string>& args,
                               std::string& errMsg)
{
    auto ret = ContactTest::GetInstance()->doSyncUpload();

    return ret;
}

int ContactTestCmd::LoopMessage(const std::vector<std::string>& args,
                                std::string& errMsg)
{
    auto ret = ContactTest::GetInstance()->doLoopMessage();

    return ret;
}

int ContactTestCmd::ListFriendInfo(const std::vector<std::string>& args,
                                   std::string& errMsg)
{
    auto ret = ContactTest::GetInstance()->listFriendInfo();

    return ret;
}

int ContactTestCmd::AcceptFriend(const std::vector<std::string>& args,
                                 std::string& errMsg)
{
    if(args.size() < 2) {
        errMsg = "Bad input count: " + std::to_string(args.size());
        return -1;
    }

    auto ret = ContactTest::GetInstance()->doAcceptFriend(args[1]);
    return ret;
}

int ContactTestCmd::AddFriend(const std::vector<std::string>& args,
                              std::string& errMsg)
{
    if(args.size() < 3) {
        errMsg = "Bad input count: " + std::to_string(args.size());
        return -1;
    }

    auto ret = ContactTest::GetInstance()->doAddFriend(args[1], args[2]);
    return ret;
}

int ContactTestCmd::DelFriend(const std::vector<std::string>& args,
                              std::string& errMsg)
{
    if(args.size() < 2) {
        errMsg = "Bad input count: " + std::to_string(args.size());
        return -1;
    }

    auto ret = ContactTest::GetInstance()->doDelFriend(args[1]);
    return ret;
}

int ContactTestCmd::SetFriendDetails(const std::vector<std::string>& args,
                                     std::string& errMsg)
{
    if(args.size() < 4) {
        errMsg = "Bad input count: " + std::to_string(args.size());
        return -1;
    }

    auto fid = args[1];
    auto infoType = std::stoi(args[2]);
    auto infoData = args[3];

    auto ret = ContactTest::GetInstance()->doSetHumanDetails(fid, infoType, infoData);
    return ret;

}

int ContactTestCmd::SendTextMessage(const std::vector<std::string>& args,
                                    std::string& errMsg)
{
    if(args.size() < 3) {
        errMsg = "Bad input count: " + std::to_string(args.size());
        return -1;
    }

    std::string  memo;
    if(args.size() > 3) {
        memo = args[3];
    }

    uint64_t replyTo = 0;
    if(args.size() > 4) {
        replyTo = std::stoll(args[4]);
    }

    auto ret = ContactTest::GetInstance()->doSendMessage(args[1], args[2], memo, replyTo);
    return ret;
}

int ContactTestCmd::SendBinaryMessage(const std::vector<std::string>& args,
                                    std::string& errMsg)
{
    if(args.size() < 2) {
        errMsg = "Bad input count: " + std::to_string(args.size());
        return -1;
    }

    std::vector<uint8_t> data = {255, 1, 0, 1, 255};
    auto ret = ContactTest::GetInstance()->doSendMessage(args[1], data);
    return ret;
}

int ContactTestCmd::SendFileMessage(const std::vector<std::string>& args,
                                    std::string& errMsg)
{
    if(args.size() < 3) {
        errMsg = "Bad input count: " + std::to_string(args.size());
        return -1;
    }

    auto file = elastos::filesystem::path(args[2]);
    auto ret = ContactTest::GetInstance()->doSendMessage(args[1], file);
    return ret;
}

int ContactTestCmd::NewAndSaveMnemonic(const std::vector<std::string>& args,
                                       std::string& errMsg) {
    auto ret = ContactTest::GetInstance()->newAndSaveMnemonic();
    return ret;
}

int ContactTestCmd::ImportUserData(const std::vector<std::string> &args,
                                   std::string& errMsg) {
    if(args.size() < 2) {
        errMsg = "Bad input count: " + std::to_string(args.size());
        return -1;
    }

    auto file = elastos::filesystem::path(args[1]);
    auto ret = ContactTest::GetInstance()->importUserData(file);
    return ret;
}
int ContactTestCmd::ExportUserData(const std::vector<std::string> &args,
                                   std::string& errMsg) {
    if(args.size() < 2) {
        errMsg = "Bad input count: " + std::to_string(args.size());
        return -1;
    }

    auto file = elastos::filesystem::path(args[1]);
    auto ret = ContactTest::GetInstance()->exportUserData(file);
    return ret;
}

int ContactTestCmd::ShowCachedDidProp(const std::vector<std::string>& args,
                                      std::string& errMsg) {
    auto ret = ContactTest::GetInstance()->showCachedProp();
    return ret;
}


int ContactTestCmd::ThreadSendMessage(const std::vector<std::string>& args,
                      std::string& errMsg) {
    if(args.size() < 2) {
        errMsg = "Bad input count: " + std::to_string(args.size());
        return -1;
    }

    auto funcMsgSendLooper = [=] {
        std::string msg;
        for(auto idx = 0; idx < 1000; idx++) {
            msg += "0123456789";
        }

        while (true) {
            auto ret = ContactTest::GetInstance()->doSendMessage(args[1], msg);
            if(ret < 0) {
                Log::E(Log::TAG, "Failed to send message in thread: 0x%x, ret=%d.", std::this_thread::get_id(), ret);
                std::this_thread::sleep_for(std::chrono::milliseconds(3000));
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        };
    };

    new std::thread(funcMsgSendLooper); // never released

    return 0;
}

