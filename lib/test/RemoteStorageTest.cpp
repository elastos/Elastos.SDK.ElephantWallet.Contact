#include <iostream>
#include <signal.h>

#include <RemoteStorageManager.hpp>
#include <ProofOssClient.hpp>
#include <DateTime.hpp>
#include <Contact.V1.hpp>
#include <Elastos.SDK.Keypair.C/Elastos.Wallet.Utility.h>
#include <Platform.hpp>
#include <Log.hpp>
#include <MD5.hpp>
#include <ErrCode.hpp>

std::string gCachedMnemonic = "reopen mechanic feed suspect bus session write spoon indoor raw apology acquire";
const char* gKeypairLanguage = "english";
const char* gKeypairWords = "";


void signalHandler(int sig);
std::shared_ptr<elastos::SecurityManager::SecurityListener> getSecurityListener();
int testProofClient(std::shared_ptr<elastos::ProofOssClient> paClient);
std::string getDid();
std::string getPublicKey();

int main(int argc, char **argv)
{
    signal(SIGSEGV, signalHandler);

    auto rsMgr = std::make_shared<elastos::RemoteStorageManager>();
    auto config = std::make_shared<elastos::Config>("/tmp/elastos.sdk.contact/test");
    auto sectyMgr = std::make_shared<elastos::SecurityManager>();

    config->load();
    auto sectyListener = getSecurityListener();
    sectyMgr->setSecurityListener(sectyListener);

    auto proofOssClient = std::make_shared<elastos::ProofOssClient>(config, sectyMgr);
    rsMgr->addClient(elastos::RemoteStorageManager::ClientType::Oss, proofOssClient);

    Log::W(Log::TAG, "UserDataDir: %s", config->mUserDataDir.c_str());
    std::filesystem::create_directories(config->mUserDataDir);
    auto dataFileName = "RemoteStorageTest";
    auto dataFilePath = std::fstream();
    dataFilePath.open(config->mUserDataDir + "/" + dataFileName, std::ios::out | std::ios::binary);
    dataFilePath.write((char[]){0, 1, 2, 3}, 4);
    dataFilePath.close();
    int ret = rsMgr->cacheProperty(getDid(), elastos::RemoteStorageManager::PropKey::PublicKey);
    CHECK_ERROR(ret);

//    ret = rsMgr->uploadCachedProp();
//    CHECK_ERROR(ret);

    Log::W(Log::TAG, "XXX to upload data.");

    while (true) {
        std::string input;
        std::cin >> input;
        if(input.find("q") == 0) {
            return 0;
        }
        std::cout << "Unknown input: " << input << std::endl;
    }

    return 0;
}

void signalHandler(int sig) {
    std::string backtrace = elastos::Platform::GetBacktrace();
    std::cerr << backtrace << std::endl;

    exit(0);
}

std::string getDid()
{
    auto pubKey = getPublicKey();

    auto did = ::getDid(pubKey.c_str());
    std::string retval = did;
    freeBuf(did);

    return retval;
}

std::string getPublicKey()
{
    void* seedData = nullptr;
    int seedSize = ::getSeedFromMnemonic(&seedData, gCachedMnemonic.c_str(), gKeypairLanguage);

    auto pubKey = ::getSinglePublicKey(seedData, seedSize);
    freeBuf(seedData);

    std::string retval = pubKey;
    freeBuf(pubKey);

    return retval;

}

std::string getPrivateKey()
{
    void* seedData = nullptr;
    int seedSize = ::getSeedFromMnemonic(&seedData, gCachedMnemonic.c_str(), gKeypairLanguage);

    auto privKey = ::getSinglePrivateKey(seedData, seedSize);
    freeBuf(seedData);

    std::string retval = privKey;
    freeBuf(privKey);

    return retval;

}

std::shared_ptr<elastos::SecurityManager::SecurityListener> getSecurityListener()
{
    class SecurityListener final : public elastos::SecurityManager::SecurityListener {
    public:
        explicit SecurityListener() = default;
        virtual ~SecurityListener() = default;

        std::string onAcquirePublicKey() override {
            auto pubKey = getPublicKey();
            Log::W(Log::TAG, "Did: %s", ::getDid(pubKey.c_str()));
            return pubKey;
        };

        std::vector<uint8_t> onEncryptData(const std::string& pubKey,
                                           const std::string& cryptoAlgorithm,
                                           const std::vector<uint8_t>& src) override {
            //auto dest = std::vector<uint8_t> {src.rbegin(), src.rend()};
            auto dest = src;
            return dest;
        }
        std::vector<uint8_t> onDecryptData(const std::string& cryptoAlgorithm,
                                           const std::vector<uint8_t>& src) override {
            //auto dest = std::vector<uint8_t> {src.rbegin(), src.rend()};
            auto dest = src;
            return dest;
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
    };

    return std::make_shared<SecurityListener>();
}

//int testProofClient(std::shared_ptr<elastos::ProofOssClient> paClient)
//{
//    elastos::ProofOssClient::OssInfo ossInfo;
//    int ret = paClient->getOssInfo(ossInfo);
//    CHECK_ERROR(ret);
//
//    return 0;
//}


