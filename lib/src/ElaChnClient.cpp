#include <ElaChnClient.hpp>

#include <iomanip>

#include <CompatibleFileSystem.hpp>
#include <DateTime.hpp>
#include <ErrCode.hpp>
#include <HttpClient.hpp>
#include <Log.hpp>
#include <MD5.hpp>
#include <Platform.hpp>
#include <SafePtr.hpp>
#include "JsonDefine.hpp"
#include "ElaChnClient.hpp"


namespace elastos {

/* =========================================== */
/* === static variables initialize =========== */
/* =========================================== */
std::shared_ptr<ElaChnClient> ElaChnClient::gElaChnClient {};

/* =========================================== */
/* === static function implement ============= */
/* =========================================== */
int ElaChnClient::InitInstance(std::weak_ptr<Config> config,
                               std::weak_ptr<SecurityManager> sectyMgr) {
    if (gElaChnClient.get() != nullptr) {
        gElaChnClient.reset();
    }

    struct Impl : ElaChnClient {
        Impl(std::weak_ptr<Config> config,
             std::weak_ptr<SecurityManager> sectyMgr)
            : ElaChnClient(config, sectyMgr) {}
    };

    HttpClient::InitGlobal();
    gElaChnClient = std::make_shared<Impl>(config, sectyMgr);

    return 0;
}

std::shared_ptr<ElaChnClient> ElaChnClient::GetInstance()
{
    assert(gElaChnClient.get() != nullptr);
    return gElaChnClient;
}

/* =========================================== */
/* === class public function implement  ====== */
/* =========================================== */
int ElaChnClient::setConnectTimeout(uint32_t milliSecond)
{
    mConnectTimeoutMS = milliSecond;
    return 0;
}

int ElaChnClient::downloadPublicKey(const std::string& elaAddr, std::string& pubKey)
{
    auto config = SAFE_GET_PTR(mConfig);

    std::shared_ptr<HttpClient> httpClient;
    auto path = config->mElaChainConfig->mApi.mGetPubKey + elaAddr;
    std::string result;
    int ret = downloadElaChnData(httpClient, path, result);
    CHECK_ERROR(ret);

    if(result.find(' ') != std::string::npos) {
        Log::W(Log::TAG, "Failed to download pubkey for ela addr: %s, result=%s", elaAddr.c_str(), result.c_str());
        return ErrCode::BlkChnGetPropError;
    }

    Log::D(Log::TAG, "Success to download pubkey for ela addr: %s", elaAddr.c_str());
    return 0;
}

/* =========================================== */
/* === class protected function implement  === */
/* =========================================== */


/* =========================================== */
/* === class private function implement  ===== */
/* =========================================== */
ElaChnClient::ElaChnClient(std::weak_ptr<Config> config, std::weak_ptr<SecurityManager> sectyMgr)
    : mConfig(config)
    , mSecurityManager(sectyMgr)
    , mConnectTimeoutMS(10000)
{
    Log::I(Log::TAG, FORMAT_METHOD);

}

ElaChnClient::~ElaChnClient()
{
    Log::I(Log::TAG, FORMAT_METHOD);
}

int ElaChnClient::downloadElaChnData(std::shared_ptr<HttpClient>& httpClient,
                                     const std::string& path, std::string& result)
{
    auto config = SAFE_GET_PTR(mConfig);

    auto didConfigUrl = config->mElaChainConfig->mUrl;
    std::string agentUrl = didConfigUrl + path;

    httpClient = std::make_shared<HttpClient>();
    httpClient->url(agentUrl);
    int ret = httpClient->syncGet();
    if(ret < 0) {
        return ErrCode::HttpClientErrorIndex + ret;
    }

    std::string respBody;
    ret = httpClient->getResponseBody(respBody);
    if(ret < 0) {
        return ErrCode::HttpClientErrorIndex + ret;
    }
    Log::I(Log::TAG, "respBody=%s", respBody.c_str());

    Json jsonResp = Json::parse(respBody);
    int status = jsonResp["status"];
    if(status != 200) {
        return ErrCode::BlkChnGetPropError;
    }

    result = jsonResp["result"];
    if(result.empty() == true) {
        return ErrCode::BlkChnEmptyPropError;
    }

    return 0;
}

} // namespace elastos
