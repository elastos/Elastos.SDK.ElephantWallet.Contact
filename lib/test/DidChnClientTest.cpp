#include <iostream>
#include <signal.h>

#include <DidChnClient.hpp>
#include <DateTime.hpp>
#include <Contact.V1.hpp>
#include <Elastos.SDK.Keypair.C/Elastos.Wallet.Utility.h>
#include <Platform.hpp>
#include <Log.hpp>
#include <MD5.hpp>

std::string gCachedMnemonic = "reopen mechanic feed suspect bus session write spoon indoor raw apology acquire";
const char* gKeypairLanguage = "english";
const char* gKeypairWords = "";

void signalHandler(int sig);
std::shared_ptr<elastos::SecurityManager::SecurityListener> getSecurityListener();
int testMonitor(std::shared_ptr<elastos::DidChnClient> bcClient);

int main(int argc, char **argv)
{
    signal(SIGSEGV, signalHandler);

    auto config = std::make_shared<elastos::Config>("/tmp/elastos.sdk.contact/test");
    auto sectyMgr = std::make_shared<elastos::SecurityManager>();

    config->load();
    auto sectyListener = getSecurityListener();
    sectyMgr->setSecurityListener(sectyListener);

    int ret = elastos::DidChnClient::InitInstance(config, sectyMgr);
    if(ret < 0) {
        throw std::runtime_error(std::string("Failed to init instance! ret=") + std::to_string(ret));
    }

    auto bcClient = elastos::DidChnClient::GetInstance();;

    ret = testMonitor(bcClient);
    if(ret < 0) {
        throw std::runtime_error(std::string("Failed to test monitor! ret=") + std::to_string(ret));
    }

    // std::map<std::string, std::string> propMap;
    // propMap["key1"] = "value1";
    // propMap["key2"] = "value2";
    // ret = bcClient->uploadAllDidProps(propMap);
    // if(ret < 0) {
    //     throw std::runtime_error(std::string("Failed to upload did props! ret=") + std::to_string(ret));
    // }

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

std::string getDid()
{
    std::string pubKey = getPublicKey();

    auto did = ::getDid(pubKey.data());
    std::string retval = did;
    freeBuf(did);

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
            //std::cout << __PRETTY_FUNCTION__ << " pubKey:" << pubKey << std::endl;
            return pubKey;
        };

        std::vector<uint8_t> onEncryptData(const std::string& pubKey, const std::vector<uint8_t>& src) override {
            //auto dest = std::vector<uint8_t> {src.rbegin(), src.rend()};
            auto dest = src;
            return dest;
        }
        std::vector<uint8_t> onDecryptData(const std::vector<uint8_t>& src) override {
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

int testMonitor(std::shared_ptr<elastos::DidChnClient> bcClient)
{
    //std::string did = getDid();

    //std::string carrierIdHistoryPath;
    //int ret = bcClient->getDidPropHistoryPath(did, elastos::DidChnClient::NameCarrierId, carrierIdHistoryPath);
    //if(ret < 0) {
        //return ret;
    //}

    //// remove keypath which not exists.
    //ret = bcClient->removeMoniter(carrierIdHistoryPath);
    //if(ret < 0) {
        //return ret;
    //}

    //auto monitorCallback = [](int errcode, const std::string& keyPath, const std::string& result) {
        //std::cout << __PRETTY_FUNCTION__ << std::endl;
        //std::cout << " errcode=" << errcode << std::endl;
        //std::cout << " keyPath=" << keyPath << std::endl;
        //std::cout << " result=" << result << std::endl;
    //};

    //ret = bcClient->appendMoniter(carrierIdHistoryPath, monitorCallback);
    //if(ret < 0) {
        //return ret;
    //}

    return 0;
}
