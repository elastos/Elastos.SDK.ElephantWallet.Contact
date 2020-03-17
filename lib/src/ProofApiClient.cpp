#include <ProofApiClient.hpp>

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
#include "ProofApiClient.hpp"
#include "../common/DateTime.hpp"


namespace elastos {

/* =========================================== */
/* === static variables initialize =========== */
/* =========================================== */
std::shared_ptr<ProofApiClient> ProofApiClient::gProofApiClient {};

/* =========================================== */
/* === static function implement ============= */
/* =========================================== */
int ProofApiClient::InitInstance(std::weak_ptr<Config> config,
                               std::weak_ptr<SecurityManager> sectyMgr) {
    if (gProofApiClient.get() != nullptr) {
        gProofApiClient.reset();
    }

    struct Impl : ProofApiClient {
        Impl(std::weak_ptr<Config> config,
             std::weak_ptr<SecurityManager> sectyMgr)
            : ProofApiClient(config, sectyMgr) {}
    };

    HttpClient::InitGlobal();
    gProofApiClient = std::make_shared<Impl>(config, sectyMgr);

    return 0;
}

std::shared_ptr<ProofApiClient> ProofApiClient::GetInstance()
{
    assert(gProofApiClient.get() != nullptr);
    return gProofApiClient;
}

/* =========================================== */
/* === class public function implement  ====== */
/* =========================================== */
int ProofApiClient::setConnectTimeout(uint32_t milliSecond)
{
    mConnectTimeoutMS = milliSecond;
    return 0;
}

int ProofApiClient::getOssInfo(OssInfo& ossInfo)
{
    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);
    auto httpClient = std::make_shared<HttpClient>();

    std::string verifyCode;
    int ret = getVerifyCode(httpClient, verifyCode);
    CHECK_ERROR(ret);

    std::string signedVerifyCode;
    ret = sectyMgr->signDataSelfVerifiable(std::vector<uint8_t>(verifyCode.begin(), verifyCode.end()),
                                           signedVerifyCode);
    CHECK_ERROR(ret);
    Log::I(Log::TAG, "ProofApiClient::getOssInfo() %s", signedVerifyCode.c_str());

    ret = getOssInfo(httpClient, signedVerifyCode, ossInfo);
    CHECK_ERROR(ret);

    return 0;
}

/* =========================================== */
/* === class protected function implement  === */
/* =========================================== */


/* =========================================== */
/* === class private function implement  ===== */
/* =========================================== */
ProofApiClient::ProofApiClient(std::weak_ptr<Config> config, std::weak_ptr<SecurityManager> sectyMgr)
    : mConfig(config)
    , mSecurityManager(sectyMgr)
    , mConnectTimeoutMS(10000)
{
    Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);
}

ProofApiClient::~ProofApiClient()
{
    Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);
}

int ProofApiClient::getVerifyCode(std::shared_ptr<HttpClient> httpClient, std::string& verifyCode)
{
    auto config = SAFE_GET_PTR(mConfig);

    std::string verifyCodeUrl = config->mProofConfig->mUrl + config->mProofConfig->mApi.mGetVerifyCode;
    httpClient->url(verifyCodeUrl);
    Log::I(Log::TAG, "%s time=%lld", __PRETTY_FUNCTION__, DateTime::CurrentMS());
    int ret = httpClient->syncGet();
    Log::I(Log::TAG, "%s time=%lld", __PRETTY_FUNCTION__, DateTime::CurrentMS());
    if(ret < 0) {
        Log::I(Log::TAG, "%s ret=%d", __PRETTY_FUNCTION__, ret);
        CHECK_ERROR(ErrCode::HttpClientError + ret);
    }

    std::string respBody;
    ret = httpClient->getResponseBody(respBody);
    if(ret < 0) {
        CHECK_ERROR(ErrCode::HttpClientError + ret);
    }
    Log::I(Log::TAG, "DidChnClient::getVerifyCode() respBody=%s", respBody.c_str());

    try {
        Json jsonResp = Json::parse(respBody);
        int status = jsonResp["state"];
        if(status != 200) {
            return ErrCode::ProofApiGetPropError;
        }

        std::string result = jsonResp["data"];
        if(result.empty() == true) {
            return ErrCode::ProofApiEmptyPropError;
        }

        verifyCode = result;
    } catch(const std::exception& ex) {
        Log::E(Log::TAG, "Failed to parse verify code from: %s.\nex=%s", respBody.c_str(), ex.what());
        return ErrCode::JsonParseException;
    }

    return 0;
}

int ProofApiClient::getOssInfo(std::shared_ptr<HttpClient> httpClient, const std::string signedVerifyCode,
                               OssInfo& ossInfo)
{
    auto config = SAFE_GET_PTR(mConfig);

    std::string ossInfoUrl = config->mProofConfig->mUrl + config->mProofConfig->mApi.mOssInfo;
    httpClient->url(ossInfoUrl);
    httpClient->setHeader("Content-Type", "application/json");

    Log::I(Log::TAG, "%s time=%lld", __PRETTY_FUNCTION__, DateTime::CurrentMS());
    int ret = httpClient->syncPost(signedVerifyCode);
    Log::I(Log::TAG, "%s time=%lld", __PRETTY_FUNCTION__, DateTime::CurrentMS());
    if(ret < 0) {
        CHECK_ERROR(ErrCode::HttpClientError + ret)
    }

    std::string respBody;
    ret = httpClient->getResponseBody(respBody);
    if(ret < 0) {
        CHECK_ERROR(ErrCode::HttpClientError + ret)
    }
    Log::I(Log::TAG, "DidChnClient::getOssInfo() respBody=%s", respBody.c_str());

    try {
        Json jsonResp = Json::parse(respBody);
        int status = jsonResp["state"];
        if(status != 200) {
            return ErrCode::ProofApiGetPropError;
        }

        Json data = jsonResp["data"];
        Json sts = data["sts"];
        ossInfo.mUser = sts["accessKeyId"];
        ossInfo.mPassword = sts["accessKeySecret"];
        ossInfo.mToken = sts["securityToken"];
        ossInfo.mDisk = data["endpoint"];
        ossInfo.mPartition = data["bucket"];
        ossInfo.mPath = data["path"];
    } catch(const std::exception& ex) {
        Log::E(Log::TAG, "Failed to parse oss info data from: %s.\nex=%s", respBody.c_str(), ex.what());
        return ErrCode::JsonParseException;
    }

    return 0;
}


} // namespace elastos
