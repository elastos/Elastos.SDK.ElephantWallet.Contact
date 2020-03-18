#include <ContactTest.hpp>
#include <Elastos.SDK.Keypair.C/Elastos.Wallet.Utility.h>

#include <fstream>
#include <iomanip>
#include <Log.hpp>
#include <ContactTestCmd.hpp>
#include <openssl/md5.h>
#include <JsonDefine.hpp>
#include "ContactTest.hpp"

/* =========================================== */
/* === static variables initialize =========== */
/* =========================================== */
const std::string ContactTest::gCacheDir { "/tmp/elastos.sdk.contact/test/" };
std::shared_ptr<ContactTest> ContactTest::gContactTest;

/* =========================================== */
/* === static function implement ============= */
/* =========================================== */
std::shared_ptr<ContactTest> ContactTest::GetInstance() {
    if(gContactTest != nullptr) {
        return gContactTest;
    }

    struct Impl : ContactTest {
    };
    gContactTest = std::make_shared<Impl>();

    return gContactTest;
}

void ContactTest::ShowEvent(const std::string& msg)
{
    Log::V(Log::TAG, "%s", msg.c_str());
}

void ContactTest::ShowError(const std::string& msg)
{
    Log::E(Log::TAG, "Error: %s", msg.c_str());
}

/* =========================================== */
/* === class public function implement  ====== */
/* =========================================== */
int ContactTest::init()
{
    std::ifstream mnemStream(gCacheDir + MnemonicFileName);
    std::getline(mnemStream, mSavedMnemonic);
    mnemStream.close();

    mSavedMnemonic = "dove arctic cute sunset solution invest wasp lawn dawn town snake eight";

    auto ret = newAndSaveMnemonic(mSavedMnemonic);
    return ret;
}

int ContactTest::newAndSaveMnemonic(const std::string& newMnemonic)
{
    mSavedMnemonic = newMnemonic;
    if (mSavedMnemonic.empty()) {
        mSavedMnemonic = generateMnemonic(KeypairLanguage, KeypairWords);
    }

    std::ofstream mnemStream(gCacheDir + MnemonicFileName);
    mnemStream << mSavedMnemonic;
    mnemStream.close();

    Log::I(Log::TAG, "Success to save mnemonic:\n%s", mSavedMnemonic.c_str());

    return 0;
}

int ContactTest::testNewContact()
{
    elastos::sdk::Contact::Factory::SetLogLevel(7);

    // elastos::sdk::Contact::Factory::SetDeviceId(getDeviceId());

    int ret = elastos::sdk::Contact::Factory::SetLocalDataDir(gCacheDir);
    if (ret < 0) {
        Log::E(Log::TAG, "Failed to call Contact.Factory.SetLocalDataDir() ret=%d", ret);
    }

    mContact = elastos::sdk::Contact::Factory::Create();
    if (mContact == nullptr) {
        Log::E(Log::TAG, "Failed to call Contact.Factory.Create()");
    }

    class ChannelStrategy: public elastos::sdk::Contact::ChannelStrategy {
    public:
        explicit ChannelStrategy()
                : elastos::sdk::Contact::ChannelStrategy(elastos::sdk::Contact::Channel::CustomId,
                                                         "LoopMessage") {
        }
        virtual ~ChannelStrategy() = default;

        virtual int onSendMessage(const std::string& humanCode,
                                  elastos::sdk::Contact::Channel channelType,
                                  const std::vector<uint8_t>& data) override {
            auto ret = this->receivedMessage(humanCode, channelType, data);
            return ret;
        }
    };

    mCustomChannelStrategy = std::make_shared<ChannelStrategy>();
    mContact->appendChannelStrategy(mCustomChannelStrategy);

    class Listener: public elastos::sdk::Contact::Listener {
        virtual std::shared_ptr<std::vector<uint8_t>> onAcquire(const AcquireArgs& request) override {
            auto ret = ContactTest::GetInstance()->processAcquire(request);

            auto msg = std::string("onAcquire(): req=") + request.toString() + "\n";
            // msg += "onAcquire(): resp=" + std::string(ret->data()) + "\n";
            ShowEvent(msg);

            return ret;
        }
        virtual void onEvent(EventArgs& event) override {
            ContactTest::GetInstance()->processEvent(event);

            std::string msg = "onEvent(): ev=" + event.toString() + "\n";
            ShowEvent(msg);
        }
        virtual void onReceivedMessage(const std::string& humanCode,
                                       elastos::sdk::Contact::Channel channelType,
                                       std::shared_ptr<elastos::sdk::Contact::Message> msgInfo) override {
            std::stringstream msg;
            msg << "onRcvdMsg(): humanCode=" << humanCode << "\n";
            msg << "onRcvdMsg(): data=" << msgInfo->data->toString() << "\n";
            msg << "onRcvdMsg(): type=" << std::to_string(static_cast<int>(msgInfo->type)) << "\n";
            msg << "onRcvdMsg(): crypto=" << msgInfo->cryptoAlgorithm << "\n";
            msg << "onRcvdMsg(): memo=" << msgInfo->memo << "\n";
            msg << "onRcvdMsg(): nanoTime=" << std::to_string(msgInfo->nanoTime) << "\n";
            msg << "onRcvdMsg(): replyTo=" << std::to_string(msgInfo->replyToNanoTime) << "\n";
            ShowEvent(msg.str());

            if (msgInfo->type == elastos::sdk::Contact::Message::Type::MsgFile) {
                //             mContactRecvFileMap.put(humanCode, (Contact.Message.FileData)message.data);
            }
        }
        virtual void onError(int errCode, const std::string& errStr,
                             const std::string& ext) override {
            auto msg = std::to_string(errCode) + ": " + errStr;
            msg += "\n" + ext;
            ShowError(msg);
        }
    };
    mContactListener = std::make_shared<Listener>();
    mContact->setListener(mContactListener);

    class DataListener: public elastos::sdk::Contact::DataListener {
        virtual void onNotify(const std::string& humanCode,
                              elastos::sdk::Contact::Channel channelType,
                              const std::string& dataId, int status) override {
            std::string msg = "onNotify(): dataId=" + dataId
                            + ", status=" + std::to_string(status) + "\n";
            ShowEvent(msg);
        }
        virtual int onReadData(const std::string& humanCode,
                               elastos::sdk::Contact::Channel channelType,
                               const std::string& dataId, uint64_t offset,
                               std::vector<uint8_t>& data) override {
            std::string msg = "onReadData(): dataId=" + dataId
                            + ", offset=" + std::to_string(offset) + "\n";
            ShowEvent(msg);
        }
        virtual int onWriteData(const std::string& humanCode,
                                elastos::sdk::Contact::Channel channelType,
                                const std::string& dataId, uint64_t offset,
                                const std::vector<uint8_t>& data) override {
            std::string msg = "onWriteData(): dataId=" + dataId
                            + ", offset=" + std::to_string(offset) + "\n";
            ShowEvent(msg);
        }
    };
    mContactDataListener = std::make_shared<DataListener>();
    mContact->setDataListener(mContactDataListener);

    Log::I(Log::TAG, "Success to create a contact instance.");
    return 0;
}

int ContactTest::testStartContact()
{
    if (mContact == nullptr) {
        ShowError("Contact is null.");
        return -1;
    }

    int ret = mContact->start();
    if (ret < 0) {
        ShowError("Failed to start contact instance. ret=" + std::to_string(ret));
        return -1;
    }

    Log::I(Log::TAG, "Success to start contact instance.");
    return 0;
}

int ContactTest::testStopContact()
{
    if (mContact == nullptr) {
        ShowError("Contact is null.");
        return -1;
    }

    int ret = mContact->stop();
    if (ret < 0) {
        ShowError("Failed to stop contact instance. ret=" + std::to_string(ret));
        return -1;
    }

    Log::I(Log::TAG, "Success to stop contact instance.");
    return 0;
}

int ContactTest::testDelContact()
{
    if (mContact == nullptr) {
        ShowError("Contact is null.");
        return -1;
    }

    mContact.reset();
    Log::I(Log::TAG, "Success to delete contact instance.");
    return 0;
}

int ContactTest::doSetHumanDetails(const std::string& friendCode, int key, const std::string& value)
{
    if (mContact == nullptr) {
        ShowError("Contact is null.");
        return -1;
    }

    int ret = mContact->setHumanInfo(friendCode, static_cast<elastos::sdk::Contact::UserInfo::Item>(key), value);
    CHECK_ERROR(ret);

    Log::V(Log::TAG, "Success to set human details");

    return 0;

}

int ContactTest::showGetUserInfo()
{
    if (mContact == nullptr) {
        ShowError("Contact is null.");
        return -1;
    }

    auto info = mContact->getUserInfo();
    if (info == nullptr) {
        ShowError("Failed to get user info.");
        return -1;
    }

    auto jsonInfo = std::make_shared<nlohmann::json>();
    int ret = info->toJson(jsonInfo);
    CHECK_ERROR(ret);

    Log::V(Log::TAG, "UserInfo: %s", jsonInfo->dump(2).c_str());

    return 0;
}

int ContactTest::showGetUserBrief()
{
    if (mContact == nullptr) {
        ShowError("Contact is null.");
        return -1;
    }

    std::string brief;
    int ret = mContact->getUserBrief(brief);
    if (ret < 0) {
        ShowError("Failed to get user brief.");
        return ret;
    }

    Log::V(Log::TAG, "UserBrief: %s", brief.c_str());

    return 0;
}

int ContactTest::doSyncUpload()
{
    if (mContact == nullptr) {
        ShowError("Contact is null.");
        return -1;
    }

    int ret = mContact->syncInfoUploadToDidChain();
    CHECK_ERROR(ret);

    return 0;
}

int ContactTest::doLoopMessage()
{
    if (mContact == nullptr) {
        ShowError("Contact is null.");
        return -1;
    }

    auto userInfo = mContact->getUserInfo();
    if (userInfo == nullptr) {
        ShowError("Failed to get user info.");
        return -1;
    }

    std::string humanCode;
    int ret = userInfo->getHumanCode(humanCode);
    CHECK_ERROR(ret);

    auto msgInfo = elastos::sdk::Contact::MakeTextMessage("test loop message");
    Log::V(Log::TAG, "===========================: %lld", msgInfo->nanoTime);
    ret = mContact->sendMessage(humanCode,
                                mCustomChannelStrategy->getChannelId(),
                                msgInfo);
    CHECK_ERROR(ret);

    Log::V(Log::TAG, "Success send loop message");
    return 0;
}

int ContactTest::listFriendInfo()
{
    if (mContact == nullptr) {
        ShowError("Contact is null.");
        return -1;
    }

    auto infoArray = mContact->listFriendInfo();

    auto jsonInfo = std::make_shared<nlohmann::json>();
    for(auto const& item: infoArray) {
        auto jsonItem = std::make_shared<nlohmann::json>();
        item->toJson(jsonItem);
        jsonInfo->push_back(*jsonItem);
    }
    Log::V(Log::TAG, "FriendInfo: %s", jsonInfo->dump(2).c_str());

    return 0;
}

int ContactTest::doAcceptFriend(const std::string& friendCode)
{
    if (mContact == nullptr) {
        ShowError("Contact is null.");
        return -1;
    }

    auto ret = mContact->acceptFriend(friendCode);
    CHECK_ERROR(ret);

    Log::V(Log::TAG, "Success to accept friend: %s", friendCode.c_str());

    return 0;
}

int ContactTest::doAddFriend(const std::string& friendCode, const std::string& summary)
{
    if (mContact == nullptr) {
        ShowError("Contact is null.");
        return -1;
    }

    auto ret = mContact->addFriend(friendCode, summary);
    CHECK_ERROR(ret);

    Log::V(Log::TAG, "Success to add friend request: %s", friendCode.c_str());

    return 0;
}

int ContactTest::doDelFriend(const std::string& friendCode)
{
    if (mContact == nullptr) {
        ShowError("Contact is null.");
        return -1;
    }

    auto ret = mContact->removeFriend(friendCode);
    CHECK_ERROR(ret);

    Log::V(Log::TAG, "Success to del friend: %s", friendCode.c_str());

    return 0;
}

int ContactTest::doSendMessage(const std::string& friendCode, const std::string& text, const std::string& memo, int64_t replyTo)
{
    if (mContact == nullptr) {
        ShowError("Contact is null.");
        return -1;
    }

    auto msgInfo = elastos::sdk::Contact::MakeTextMessage(text, "", memo);
    if(msgInfo == nullptr) {
        ShowError("Failed to make text message.");
        return -1;
    }

    msgInfo->replyTo(replyTo);

    auto ret = mContact->sendMessage(friendCode, elastos::sdk::Contact::Channel::Carrier, msgInfo);

    Log::V(Log::TAG, "Success send text to friend: %s", friendCode.c_str());

    return 0;
}

int ContactTest::doSendMessage(const std::string& friendCode, const std::vector<uint8_t>& binary)
{
    if (mContact == nullptr) {
        ShowError("Contact is null.");
        return -1;
    }

    auto msgInfo = elastos::sdk::Contact::MakeBinaryMessage(binary);
    if(msgInfo == nullptr) {
        ShowError("Failed to make binary message.");
        return -1;
    }

    auto ret = mContact->sendMessage(friendCode, elastos::sdk::Contact::Channel::Carrier, msgInfo);

    Log::V(Log::TAG, "Success send binary to friend: %s", friendCode.c_str());

    return 0;
}

int ContactTest::doSendMessage(const std::string& friendCode, const elastos::filesystem::path& file)
{
    if (mContact == nullptr) {
        ShowError("Contact is null.");
        return -1;
    }

    auto msgInfo = elastos::sdk::Contact::MakeFileMessage(file.string());
    if(msgInfo == nullptr) {
        ShowError("Failed to make binary message.");
        return -1;
    }

    auto ret = mContact->sendMessage(friendCode, elastos::sdk::Contact::Channel::Carrier, msgInfo);

    Log::V(Log::TAG, "Success send binary to friend: %s", friendCode.c_str());

    return 0;
}

int ContactTest::importUserData(const elastos::filesystem::path& fromFile)
{
    if (mContact != nullptr) {
        ShowError("Contact is not null.");
        return -1;
    }
    if (mContact == nullptr) {
        testNewContact();
    }

    int ret = mContact->importUserData(fromFile.string());
    return ret;
}
int ContactTest::exportUserData(const elastos::filesystem::path& toFile)
{
    if (mContact == nullptr) {
        testNewContact();
    }

    int ret = mContact->exportUserData(toFile.string());
    return ret;
}

int ContactTest::showCachedProp()
{
    if (mContact == nullptr) {
        ShowError("Contact is null.");
        return -1;
    }

    std::stringstream cachedDidProp;
    elastos::sdk::Contact::Debug::GetCachedDidProp(&cachedDidProp);
    Log::V(Log::TAG, "CachedDidProp: %s", cachedDidProp.str().c_str());

    return 0;
}


/* =========================================== */
/* === class protected function implement  === */
/* =========================================== */

/* =========================================== */
/* === class private function implement  ===== */
/* =========================================== */

std::string ContactTest::getPublicKey()
{
    void* seedData = nullptr;
    int seedSize = ::getSeedFromMnemonic(&seedData, mSavedMnemonic.c_str(), KeypairLanguage);

    auto pubKey = ::getSinglePublicKey(seedData, seedSize);
    freeBuf(seedData);

    std::string retval = pubKey;
    freeBuf(pubKey);

    Log::D(Log::TAG, "%s %d, pubkey=%s", __PRETTY_FUNCTION__, __LINE__, retval.c_str());
    return retval;

}

std::string ContactTest::getPrivateKey()
{
    void* seedData = nullptr;
    int seedSize = ::getSeedFromMnemonic(&seedData, mSavedMnemonic.c_str(), KeypairLanguage);

    auto privKey = ::getSinglePrivateKey(seedData, seedSize);
    freeBuf(seedData);

    std::string retval = privKey;
    freeBuf(privKey);

    return retval;

}

std::string ContactTest::getAgentAuthHeader()
{
    std::string appid = "org.elastos.debug.didplugin";
    std::string appkey = "b2gvzUM79yLhCbbGNWCuhSsGdqYhA7sS";
    uint64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    std::string auth = ContactTest::GetMd5Sum(appkey + std::to_string(timestamp));
    std::string headerValue = "id=" + appid + ";time=" + std::to_string(timestamp) + ";auth=" + auth;
    Log::I(Log::TAG, "getAgentAuthHeader() headerValue=%s", headerValue.c_str());

    return headerValue;
}

std::string ContactTest::GetMd5Sum(const std::string& data)
{
    uint8_t result[MD5_DIGEST_LENGTH];
    MD5((uint8_t*)data.data(), data.size(), result);

    std::ostringstream sstream;
    sstream<< std::hex<< std::setfill('0');
    for(auto i: result) {
        sstream << std::setw(2) <<(int)i;
    }

    return sstream.str();
}

std::shared_ptr<std::vector<uint8_t>> ContactTest::processAcquire(const elastos::sdk::Contact::Listener::AcquireArgs& request)
{
    Log::D(Log::TAG, "%s", __PRETTY_FUNCTION__);
    std::shared_ptr<std::vector<uint8_t>> response;

    switch(request.type) {
    case elastos::sdk::Contact::Listener::AcquireType::PublicKey:
    {
        auto pubKey = getPublicKey();
        response = std::make_shared<std::vector<uint8_t>>(pubKey.begin(), pubKey.end());
        break;
    }
    case elastos::sdk::Contact::Listener::AcquireType::EncryptData:
        if(request.extra == "DefaultAlgorithm") {
            const char* cryptoBuf = ::eciesEncrypt(request.publicKey.c_str(), request.data.data(), request.data.size());
            if(cryptoBuf != nullptr) {
                std::string cryptoData {cryptoBuf};
                response = std::make_shared<std::vector<uint8_t>>(cryptoData.begin(), cryptoData.end());
                ::freeBuf((void*)cryptoBuf);
            } else {
                response = std::make_shared<std::vector<uint8_t>>(request.data); // ignore encrypt
            }
        } else {
            response = std::make_shared<std::vector<uint8_t>>(request.data); // ignore encrypt
        }
        break;
    case elastos::sdk::Contact::Listener::AcquireType::DecryptData:
        if(request.extra == "DefaultAlgorithm") {
            std::string privKey = getPrivateKey();
            int plainLen = 0;
            unsigned char* plainBuf = ::eciesDecrypt(privKey.c_str(), (char*)request.data.data(), &plainLen);
            if(plainBuf != nullptr) {
                std::vector <uint8_t> plainData {plainBuf, plainBuf + plainLen};
                response = std::make_shared<std::vector<uint8_t>>(plainData);
                ::freeBuf((void*)plainBuf);
            } else {
                response = std::make_shared<std::vector<uint8_t>>(request.data); // ignore decrypt
            }
        } else {
            response = std::make_shared<std::vector<uint8_t>>(request.data); // ignore encrypt
        }
        break;
    case elastos::sdk::Contact::Listener::AcquireType::DidPropAppId:
    {
        std::string appId = "DC92DEC59082610D1D4698F42965381EBBC4EF7DBDA08E4B3894D530608A64AA"
                            "A65BB82A170FBE16F04B2AF7B25D88350F86F58A7C1F55CC29993B4C4C29E405";
        response = std::make_shared<std::vector<uint8_t>>(appId.begin(), appId.end());
        response.reset(); // return null will use `DidFriend`
        break;
    }
    case elastos::sdk::Contact::Listener::AcquireType::DidAgentAuthHeader:
    {
        std::string authHeader = getAgentAuthHeader();
        response = std::make_shared<std::vector<uint8_t>>(authHeader.begin(), authHeader.end());
        break;
    }
    case elastos::sdk::Contact::Listener::AcquireType::SignData:
    {
        uint8_t* signedData = nullptr;
        auto privKey = getPrivateKey();
        int signedSize = sign(privKey.c_str(), request.data.data(), request.data.size(), (void**)&signedData);
        if(signedSize > 0 && signedData != nullptr) {
            response = std::make_shared<std::vector<uint8_t>>(signedData, signedData + signedSize);
            freeBuf(signedData);
        }
        break;
    }
    }

    return response;
}


void ContactTest::processEvent(elastos::sdk::Contact::Listener::EventArgs& event)
{
    switch (event.type) {
    case elastos::sdk::Contact::Listener::EventType::StatusChanged:
        break;
    case elastos::sdk::Contact::Listener::EventType::FriendRequest:
    {
        auto requestEvent = dynamic_cast<elastos::sdk::Contact::Listener::RequestEvent*>(&event);
        std::string nickname;
        std::shared_ptr<elastos::sdk::Contact::HumanInfo> friendInfo;
        int ret = mContact->getHumanInfo(requestEvent->humanCode, friendInfo);
        if(ret >= 0) {
            int ret = friendInfo->getHumanInfo(elastos::sdk::Contact::HumanInfo::Item::Nickname, nickname);
        }

        Log::V(Log::TAG, "New friend request\n from:     %s\n nickname: %s\n summary:  %s",
                         requestEvent->humanCode.c_str(), nickname.c_str(), requestEvent->summary.c_str());
        break;
    }
    case elastos::sdk::Contact::Listener::EventType::HumanInfoChanged:
    {
        auto infoEvent = dynamic_cast<elastos::sdk::Contact::Listener::InfoEvent*>(&event);
        auto msg = event.humanCode + " info changed: " + infoEvent->toString();
        ShowEvent(msg);
        break;
    }
    default:
        Log::W(Log::TAG, "Unprocessed event: %d", static_cast<int>(event.type));
            break;
    }
}
