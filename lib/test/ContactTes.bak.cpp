#include <Elastos.SDK.Contact.hpp>
#include <Elastos.SDK.Keypair.C/Elastos.Wallet.Utility.h>

#include <fstream>
#include <iostream>
#include <thread>
#include <signal.h>

#include <DateTime.hpp>
#include <Platform.hpp>
#include <Log.hpp>
#include <MD5.hpp>
#include "ContactTestCmd.hpp"
#include "ContactTestListener.hpp"

const char* keypairLanguage = "english";
const char* keypairWords = "";

std::string gSavedMnemonic;
std::string gCachedMnemonic;

std::thread gCmdPipeMonitor;
bool gQuitFlag = false;

void loop(const char* fifoFilePath, std::shared_ptr<elastos::Contact> contact);
void signalHandler(int sig);
std::shared_ptr<elastos::SecurityManager::SecurityListener> getSecurityListener(std::weak_ptr<elastos::Contact> contact);
std::shared_ptr<elastos::UserManager::UserListener> getUserListener();
std::shared_ptr<elastos::FriendManager::FriendListener> getFriendListener();
void ensureCachedMnemonic();
std::string getPublicKey();
std::string getPrivateKey();

int main(int argc, char **argv)
{
    signal(SIGSEGV, signalHandler);
    signal(SIGABRT, signalHandler);

	const char* nickname = "Friend";
    const char* fifoFilePath = (argc > 1 ? argv[1] : nullptr);
    if(fifoFilePath != nullptr) {
//        gSavedMnemonic = "bachelor sail glove swing despair lawsuit exhibit travel slot practice latin glass";
        nickname = "Me";
    } else {
       gSavedMnemonic = "month business urban nurse joy derive acquire snap venue hello city buyer";
    }

    Log::I(Log::TAG, "Start Contact Test.");
    Log::I(Log::TAG, "%s\n", (argc > 1 ? argv[1]:""));

    elastos::Contact::Factory::SetLogLevel(4);
    int ret = elastos::Contact::Factory::SetLocalDataDir("/tmp/elastos.sdk.contact/test");
    if(ret < 0) {
        throw std::runtime_error(std::string("Failed to set contact local dir! ret=") + std::to_string(ret));
    }

    auto contact = elastos::Contact::Factory::Create();

    auto sectyListener = getSecurityListener(contact);
    auto userListener = getUserListener();
    auto friendListener = getFriendListener();
    auto msgListener = std::make_shared<ContactTestListener>(contact);
    contact->setListener(sectyListener, userListener, friendListener, msgListener);

    ret = contact->start();
    if(ret < 0) {
        throw std::runtime_error(std::string("Failed to start contact! ret=") + std::to_string(ret));
    }

    gCachedMnemonic.clear();

    auto userMgr = contact->getUserManager().lock();
    std::shared_ptr<elastos::UserInfo> userInfo;
    ret = userMgr->getUserInfo(userInfo);
    if(ret < 0) {
        throw std::runtime_error(std::string("Failed to get user info! ret=") + std::to_string(ret));
    }

    ret = userInfo->setHumanInfo(elastos::UserInfo::Item::Nickname, nickname);
    if(ret < 0) {
        throw std::runtime_error(std::string("Failed to get user info! ret=") + std::to_string(ret));
    }

    std::string value;
    userInfo->getHumanInfo(elastos::UserInfo::Item::Nickname, value);
    Log::I(Log::TAG, "NickName  : %s", value.c_str());
    userInfo->getHumanInfo(elastos::UserInfo::Item::ChainPubKey, value);
    Log::I(Log::TAG, "PubKey    : %s", value.c_str());
    userInfo->getHumanInfo(elastos::UserInfo::Item::Did, value);
    Log::I(Log::TAG, "DID       : %s", value.c_str());
    userInfo->getHumanInfo(elastos::UserInfo::Item::ElaAddress, value);
    Log::I(Log::TAG, "ElaAddress: %s", value.c_str());

    std::vector<elastos::UserInfo::CarrierInfo> carrierInfoArray;
    userInfo->getAllCarrierInfo(carrierInfoArray);
    for(auto& it: carrierInfoArray) {
        Log::I(Log::TAG, "BoundDevId  : %s", it.mDevInfo.mDevId.c_str());
        Log::I(Log::TAG, "BoundDevName: %s", it.mDevInfo.mDevName.c_str());
        Log::I(Log::TAG, "CarrierAddr : %s", it.mUsrAddr.c_str());
        Log::I(Log::TAG, "carrierUsrId: %s", it.mUsrId.c_str());
    }

    //ret = userMgr->uploadUserInfo();
    //if(ret < 0) {
        //throw std::runtime_error(std::string("Failed to upload user info! ret=") + std::to_string(ret));
    //}

    //ret = userMgr->syncUserInfo();
    //if(ret < 0) {
        //throw std::runtime_error(std::string("Failed to get user info! ret=") + std::to_string(ret));
    //}

    loop(fifoFilePath, contact);

    return 0;
}

void processCmd(const std::string& cmdLine, std::shared_ptr<elastos::Contact> contact)
{
    if (cmdLine.empty() == true) {
        std::cout << "# ";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return;
    }
    Log::I(Log::TAG, "==> Received Command: %s", cmdLine.c_str());

    std::string errMsg;
    int ret = ContactTestCmd::Do(contact, cmdLine, errMsg);
    if (ret < 0) {
        Log::E(Log::TAG, "ErrCode(%d): %s", ret, errMsg.c_str());
    } else {
        Log::I(Log::TAG, "Success to exec: %s", cmdLine.c_str());
    }
    std::cout << "# ";
}

void monitorCmdPipe(const char* fifoFilePath, std::shared_ptr<elastos::Contact> contact) {
    if (fifoFilePath == nullptr) {
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
            processCmd(cmdLine, contact);
        };
    };

    gCmdPipeMonitor = std::thread (funcPipeMonitor);
}

void loop(const char* fifoFilePath, std::shared_ptr<elastos::Contact> contact)
{
    monitorCmdPipe(fifoFilePath, contact);

    while (true) {
        std::string cmdLine;
        std::getline(std::cin, cmdLine);

        if (cmdLine == "q" || cmdLine == "quit") {
            gQuitFlag = true;
            return;
        }
        processCmd(cmdLine, contact);
    }
}

void signalHandler(int sig) {
    std::cerr << "Error: signal " << sig << std::endl;

    std::string backtrace = elastos::Platform::GetBacktrace();
    std::cerr << backtrace << std::endl;

    exit(sig);
}

std::shared_ptr<elastos::SecurityManager::SecurityListener> getSecurityListener(std::weak_ptr<elastos::Contact> contact)
{
    class SecurityListener final : public elastos::SecurityManager::SecurityListener {
    public:
        explicit SecurityListener(std::weak_ptr<elastos::Contact> contact)
            : mContact(contact) {
        };
        virtual ~SecurityListener() = default;

        std::string onAcquirePublicKey() override {
            auto pubKey = getPublicKey();
            //std::cout << __PRETTY_FUNCTION__ << " pubKey:" << pubKey << std::endl;
            return pubKey;
        };

        std::vector<uint8_t> onEncryptData(const std::string& pubKey, const std::vector<uint8_t>& src) override {
            //auto dest = std::vector<uint8_t> {src.rbegin(), src.rend()};
            //return dest;
            return src;
        }
        std::vector<uint8_t> onDecryptData(const std::vector<uint8_t>& src) override {
            //auto dest = std::vector<uint8_t> {src.rbegin(), src.rend()};
            //return dest;
            return src;
        }

        std::string onAcquireDidPropAppId() override {
            return "DC92DEC59082610D1D4698F42965381EBBC4EF7DBDA08E4B3894D530608A64AAA65BB82A170FBE16F04B2AF7B25D88350F86F58A7C1F55CC29993B4C4C29E405";
        }

        std::string onAcquireDidAgentAuthHeader() override {
            std::string appid = "org.elastos.debug.didplugin";
            std::string appkey = "b2gvzUM79yLhCbbGNWCuhSsGdqYhA7sS";
            std::string timestamp = std::to_string(elastos::DateTime::CurrentMS());
            std::string auth = elastos::MD5::Get(appkey + timestamp);
            std::string headerValue = "id=" + appid + ";time=" + timestamp + ";auth=" + auth;
            Log::I(Log::TAG, "onAcquireDidAgentAuthHeader() headerValue=%s", headerValue.c_str());

            return headerValue;
        }

        std::vector<uint8_t> onSignData(const std::vector<uint8_t>& originData) override {
            std::string privKey = getPrivateKey();

            std::vector<uint8_t> signedData;

            void* keypairSignedData = nullptr;
            int keypairSignedSize = ::sign(privKey.c_str(), originData.data(), originData.size(), &keypairSignedData);
            if(keypairSignedSize <= 0) {
                return signedData;
            }

            uint8_t* keypairSignedDataPtr = reinterpret_cast<uint8_t*>(keypairSignedData);
            signedData = std::vector<uint8_t>(keypairSignedDataPtr, keypairSignedDataPtr + keypairSignedSize);

            return signedData;
        }

    private:
        std::weak_ptr<elastos::Contact> mContact;
    };

    return std::make_shared<SecurityListener>(contact);
}


std::shared_ptr<elastos::UserManager::UserListener> getUserListener()
{
    class UserListener : public elastos::UserManager::UserListener {
    public:
        explicit UserListener() = default;
        virtual ~UserListener() = default;

        //virtual int onSigninDevicesOverflow(const std::weak_ptr<elastos::HumanInfo> userInfo, int capacity) override {
            //std::cout << __PRETTY_FUNCTION__ << std::endl;
            //return 0;
        //};
    };

    return std::make_shared<UserListener>();
}

std::shared_ptr<elastos::FriendManager::FriendListener> getFriendListener()
{
    class FriendListener : public elastos::FriendManager::FriendListener {
    public:
        explicit FriendListener() = default;
        virtual ~FriendListener() = default;

        //virtual int onFriendRequest(elastos::FriendInfo friendInfo, std::string message) override {
            //std::cout << __PRETTY_FUNCTION__ << " message:" << message << std::endl;
            //return 0;
        //};

        //virtual void onStatusChanged(elastos::FriendInfo friendInfo, uint64_t status) override {
            //std::cout << __PRETTY_FUNCTION__ << std::endl;
        //};
    };

    return std::make_shared<FriendListener>();
}

void ensureCachedMnemonic()
{
    if(gCachedMnemonic.empty() == true) {
        //std::string password;
        //std::cout << "input password: ";
        //std::getline(std::cin, password);

        if(gSavedMnemonic.empty() == true) {
            gSavedMnemonic = ::generateMnemonic(keypairLanguage, keypairWords);
            Log::I(Log::TAG, "generate mnemonic: %s", gSavedMnemonic.c_str());
        }
        gCachedMnemonic = gSavedMnemonic;
    }

}

std::string getPublicKey()
{
    ensureCachedMnemonic();

    void* seedData = nullptr;
    int seedSize = ::getSeedFromMnemonic(&seedData, gCachedMnemonic.c_str(), keypairLanguage);

    auto pubKey = ::getSinglePublicKey(seedData, seedSize);
    freeBuf(seedData);

    std::string retval = pubKey;
    freeBuf(pubKey);

    return retval;

}

std::string getPrivateKey()
{
    ensureCachedMnemonic();

    void* seedData = nullptr;
    int seedSize = ::getSeedFromMnemonic(&seedData, gCachedMnemonic.c_str(), keypairLanguage);

    auto privKey = ::getSinglePrivateKey(seedData, seedSize);
    freeBuf(seedData);

    std::string retval = privKey;
    freeBuf(privKey);

    return retval;

}
