#include <ContactTest.hpp>
#include <Elastos.SDK.Keypair.C/Elastos.Wallet.Utility.h>

#include <fstream>
#include <Log.hpp>
#include <ContactTestCmd.hpp>
#include <openssl/md5.h>

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
    ElaphantContact::Factory::SetLogLevel(7);

    // ElaphantContact::Factory::SetDeviceId(getDeviceId());

    int ret = ElaphantContact::Factory::SetLocalDataDir(gCacheDir);
    if (ret < 0) {
        Log::E(Log::TAG, "Failed to call Contact.Factory.SetLocalDataDir() ret=%d", ret);
    }

    mContact = ElaphantContact::Factory::Create();
    if (mContact == nullptr) {
        Log::E(Log::TAG, "Failed to call Contact.Factory.Create()");
    }

    class Listener: public ElaphantContact::Listener {
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
                                       ContactChannel channelType,
                                       std::shared_ptr<ElaphantContact::Message> msgInfo) override {
            auto msg = std::string("onRcvdMsg(): humanCode=") + humanCode.c_str() + "\n";
            msg += "onRcvdMsg(): data=" + msgInfo->data->toString() + "\n";
            msg += "onRcvdMsg(): type=" + std::to_string(static_cast<int>(msgInfo->type)) + "\n";
            msg += "onRcvdMsg(): crypto=" + msgInfo->cryptoAlgorithm + "\n";
            ShowEvent(msg);

            if (msgInfo->type == ElaphantContact::Message::Type::MsgFile) {
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
    mContact->setListener(mContactListener.get());

    class DataListener: public ElaphantContact::DataListener {
        virtual void onNotify(const std::string& humanCode,
                              ContactChannel channelType,
                              const std::string& dataId, int status) override {
            std::string msg = "onNotify(): dataId=" + dataId
                            + ", status=" + std::to_string(status) + "\n";
            ShowEvent(msg);
        }
        virtual int onReadData(const std::string& humanCode,
                               ContactChannel channelType,
                               const std::string& dataId, uint64_t offset,
                               std::vector<uint8_t>& data) override {
            std::string msg = "onReadData(): dataId=" + dataId
                            + ", offset=" + std::to_string(offset) + "\n";
            ShowEvent(msg);
        }
        virtual int onWriteData(const std::string& humanCode,
                                ContactChannel channelType,
                                const std::string& dataId, uint64_t offset,
                                const std::vector<uint8_t>& data) override {
            std::string msg = "onWriteData(): dataId=" + dataId
                            + ", offset=" + std::to_string(offset) + "\n";
            ShowEvent(msg);
        }
    };
    mContactDataListener = std::make_shared<DataListener>();
    mContact->setDataListener(mContactDataListener.get());

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

int ContactTest::doSendMessage(const std::string& friendCode, const std::string& text)
{
    if (mContact == nullptr) {
        ShowError("Contact is null.");
        return -1;
    }

    auto msgInfo = ElaphantContact::MakeTextMessage(text);
    if(msgInfo == nullptr) {
        ShowError("Failed to make text message.");
        return -1;
    }

    auto ret = mContact->sendMessage(friendCode, ContactChannel::Carrier, msgInfo);

    Log::V(Log::TAG, "Success send text to friend: %s", friendCode.c_str());

    return 0;
}

int ContactTest::doSendMessage(const std::string& friendCode, const std::vector<uint8_t>& binary)
{
    if (mContact == nullptr) {
        ShowError("Contact is null.");
        return -1;
    }

    auto msgInfo = ElaphantContact::MakeBinaryMessage(binary);
    if(msgInfo == nullptr) {
        ShowError("Failed to make binary message.");
        return -1;
    }

    auto ret = mContact->sendMessage(friendCode, ContactChannel::Carrier, msgInfo);

    Log::V(Log::TAG, "Success send binary to friend: %s", friendCode.c_str());

    return 0;
}

int ContactTest::doSendMessage(const std::string& friendCode, const std::filesystem::path& file)
{
    if (mContact == nullptr) {
        ShowError("Contact is null.");
        return -1;
    }

    auto msgInfo = ElaphantContact::MakeFileMessage(file.string());
    if(msgInfo == nullptr) {
        ShowError("Failed to make binary message.");
        return -1;
    }

    auto ret = mContact->sendMessage(friendCode, ContactChannel::Carrier, msgInfo);

    Log::V(Log::TAG, "Success send binary to friend: %s", friendCode.c_str());

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

std::shared_ptr<std::vector<uint8_t>> ContactTest::processAcquire(const ElaphantContact::Listener::AcquireArgs& request)
{
    Log::D(Log::TAG, "%s", __PRETTY_FUNCTION__);
    std::shared_ptr<std::vector<uint8_t>> response;

    switch(request.type) {
    case ElaphantContact::Listener::AcquireType::PublicKey:
    {
        auto pubKey = getPublicKey();
        response = std::make_shared<std::vector<uint8_t>>(pubKey.begin(), pubKey.end());
        break;
    }
    case ElaphantContact::Listener::AcquireType::EncryptData:
        response = std::make_shared<std::vector<uint8_t>>(request.data); // ignore encrypt
        break;
    case ElaphantContact::Listener::AcquireType::DecryptData:
        response = std::make_shared<std::vector<uint8_t>>(request.data); // ignore decrypt
        break;
    case ElaphantContact::Listener::AcquireType::DidPropAppId:
    {
        std::string appId = "DC92DEC59082610D1D4698F42965381EBBC4EF7DBDA08E4B3894D530608A64AA"
                            "A65BB82A170FBE16F04B2AF7B25D88350F86F58A7C1F55CC29993B4C4C29E405";
        response = std::make_shared<std::vector<uint8_t>>(appId.begin(), appId.end());
        break;
    }
    case ElaphantContact::Listener::AcquireType::DidAgentAuthHeader:
    {
        std::string authHeader = getAgentAuthHeader();
        response = std::make_shared<std::vector<uint8_t>>(authHeader.begin(), authHeader.end());
        break;
    }
    case ElaphantContact::Listener::AcquireType::SignData:
        break;
    }

    return response;
}


void ContactTest::processEvent(ElaphantContact::Listener::EventArgs& event)
{
    switch (event.type) {
    case ElaphantContact::Listener::EventType::StatusChanged:
        break;
    case ElaphantContact::Listener::EventType::FriendRequest:
    {
        auto requestEvent = dynamic_cast<ElaphantContact::Listener::RequestEvent*>(&event);
        Log::V(Log::TAG, "Friend request from: %s, summary: %s",
                         requestEvent->humanCode.c_str(), requestEvent->summary.c_str());
        break;
    }
    case ElaphantContact::Listener::EventType::HumanInfoChanged:
    {
        auto infoEvent = dynamic_cast<ElaphantContact::Listener::InfoEvent*>(&event);
        auto msg = event.humanCode + " info changed: " + infoEvent->toString();
        ShowEvent(msg);
        break;
    }
    default:
        Log::W(Log::TAG, "Unprocessed event: %d", static_cast<int>(event.type));
            break;
    }
}
