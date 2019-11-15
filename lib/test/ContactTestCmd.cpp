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
    { 'q', "quit",          nullptr,                             "\t\tQuit." },
    { 'h', "help",          ContactTestCmd::Help,                "\t\tPrint help usages." },

    { '-', "",              nullptr,                             "\nContact" },
    { 'n', "ns-contact",    ContactTestCmd::NewAndStartContact,  "\tNew and Start Contact" },
    { 'd', "sd-contact",    ContactTestCmd::StopAndDelContact,   "\tStop and Del Contact" },
    { 'c', "rc-contact",    ContactTestCmd::RecreateContact,     "\tRecreate Contact" },
    { 's', "rs-contact",    ContactTestCmd::RestartContact,      "\tRestart Contact" },

    { '-', "",              nullptr,                             "\n User" },
    { 'i', "get-uinfo",     ContactTestCmd::GetUserInfo,         "\tGet User Info" },
    { ' ', "set-uid",       ContactTestCmd::Unimplemention,      "\t\tSet User Identifycode" },
    { ' ', "set-udetails",  ContactTestCmd::Unimplemention,      "\tSet User Details" },
    { ' ', "set-uwaddr",    ContactTestCmd::Unimplemention,      "\tSet User Wallet Address" },
    { ' ', "set-uwaddr",    ContactTestCmd::Unimplemention,      "\tSet User Wallet Address" },
    { 'l', "sync-upload",   ContactTestCmd::Unimplemention,      "\tSync Upload" },
    { ' ', "sync-download", ContactTestCmd::Unimplemention,      "\tSync Download" },

    { '-', "",              nullptr,                             "\n Friend" },
    { 'g', "get-finfo",     ContactTestCmd::Unimplemention,      "\tGet Friend Info" },
    { 'f', "accept-frd",    ContactTestCmd::AcceptFriend,        "\tAccept Friend" },
    { 'a', "add-frd",       ContactTestCmd::Unimplemention,      "\t\tAdd Friend" },
    { ' ', "del-frd",       ContactTestCmd::Unimplemention,      "\t\tDel Friend" },
    { 't', "send-tmsg",     ContactTestCmd::Unimplemention,      "\tSend Text Message" },
    { 'b', "send-bmsg",     ContactTestCmd::Unimplemention,      "\tSend Binary Message" },
    { 'f', "send-fmsg",     ContactTestCmd::Unimplemention,      "\tSend File Message" },
    { 'p', "pull-file",     ContactTestCmd::Unimplemention,      "\tPull File" },
    { ' ', "cancel-pfile",  ContactTestCmd::Unimplemention,      "\tCancel Pull File" },


    { '-', "",             nullptr,                             "\n Mnemonic" },
    { 'm', "new-mnemonic", ContactTestCmd::NewAndSaveMnemonic,  "\t\tnew mnemonic" },
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

int ContactTestCmd::NewAndSaveMnemonic(const std::vector<std::string>& args,
                                       std::string& errMsg) {
    auto ret = ContactTest::GetInstance()->newAndSaveMnemonic();
    return ret;
}
