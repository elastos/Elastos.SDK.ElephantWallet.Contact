#include <ProofOssClient.hpp>

#include <fstream>

#include <CompatibleFileSystem.hpp>
#include <DateTime.hpp>
#include <ErrCode.hpp>
#include <HttpClient.hpp>
#include <Log.hpp>
#include <MD5.hpp>
#include <Platform.hpp>
#include <SafePtr.hpp>
#include "JsonDefine.hpp"
#include "ProofOssClient.hpp"
#include <PersonalStorage.SDK.OSS.hpp>

namespace elastos {

/* =========================================== */
/* === static variables initialize =========== */
/* =========================================== */

/* =========================================== */
/* === static function implement ============= */
/* =========================================== */

/* =========================================== */
/* === class public function implement  ====== */
/* =========================================== */
ProofOssClient::ProofOssClient(std::weak_ptr<Config> config, std::weak_ptr<SecurityManager> sectyMgr)
        : mConfig(config)
        , mSecurityManager(sectyMgr)
{
    Log::I(Log::TAG, FORMAT_METHOD);
}

ProofOssClient::~ProofOssClient()
{
    Log::I(Log::TAG, FORMAT_METHOD);
}

int ProofOssClient::uploadProperties(const std::map<std::string, std::string>& changedPropMap,
                                     const std::map<std::string, std::shared_ptr<std::iostream>>& totalPropMap)
{
    Log::I(Log::TAG, "%s Start upload data.", FORMAT_METHOD);
    if(totalPropMap.size() == 0) {
        Log::W(Log::TAG, "%s No data need to upload", FORMAT_METHOD);
        return 0;
    }

    int ret = ossLogin();
    CHECK_ERROR(ret);

    for(const auto& [path, content]: totalPropMap) {
        ret = ossWrite(path, content);
        CHECK_ERROR(ret);
    }

    return 0;
}

int ProofOssClient::downloadProperties(std::map<std::string, std::string>& changedPropMap,
                                       std::map<std::string, std::shared_ptr<std::iostream>>& totalPropMap)
{
    Log::I(Log::TAG, "%s Start download data.", FORMAT_METHOD);
    if(totalPropMap.size() == 0) {
        Log::W(Log::TAG, "%s No data need to download", FORMAT_METHOD);
        return 0;
    }

    int ret = ossLogin();
    CHECK_ERROR(ret);

    for(auto& [path, placeholder]: totalPropMap) {
        auto content = std::make_shared<std::stringstream>();
        ret = ossRead(path, content);
        if(ret < 0) {
            Log::W(Log::TAG, "%s Failed to read oss content from: %s", FORMAT_METHOD, path.c_str());
            continue;
        }
//        CHECK_ERROR(ret);
        totalPropMap[path] = content;
    }

    return 0;
}

/* =========================================== */
/* === class protected function implement  === */
/* =========================================== */


/* =========================================== */
/* === class private function implement  ===== */
/* =========================================== */
int ProofOssClient::getOssInfo(OssInfo& ossInfo)
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
    Log::I(Log::TAG, "ProofOssClient::getOssInfo() %s", signedVerifyCode.c_str());

    ret = getOssInfo(httpClient, signedVerifyCode, ossInfo);
    CHECK_ERROR(ret);

    return 0;
}

int ProofOssClient::getVerifyCode(std::shared_ptr<HttpClient> httpClient, std::string& verifyCode)
{
    auto config = SAFE_GET_PTR(mConfig);

    std::string verifyCodeUrl = config->mProofConfig->mUrl + config->mProofConfig->mApi.mGetVerifyCode;
    httpClient->url(verifyCodeUrl);
    Log::I(Log::TAG, "%s time=%lld", FORMAT_METHOD, DateTime::CurrentMS());
    int ret = httpClient->syncGet();
    Log::I(Log::TAG, "%s time=%lld", FORMAT_METHOD, DateTime::CurrentMS());
    if(ret < 0) {
        Log::I(Log::TAG, "%s ret=%d", FORMAT_METHOD, ret);
        CHECK_ERROR(ErrCode::HttpClientErrorIndex + ret);
    }

    std::string respBody;
    ret = httpClient->getResponseBody(respBody);
    if(ret < 0) {
        CHECK_ERROR(ErrCode::HttpClientErrorIndex + ret);
    }
    Log::I(Log::TAG, "DidChnClient::getVerifyCode() respBody=%s", respBody.c_str());

    try {
        Json jsonResp = Json::parse(respBody);
        int status = jsonResp["state"];
        if(status != 200) {
            CHECK_ERROR(ErrCode::ProofApiGetPropError);
        }

        std::string result = jsonResp["data"];
        if(result.empty() == true) {
            CHECK_ERROR(ErrCode::ProofApiEmptyPropError);
        }

        verifyCode = result;
    } catch(const std::exception& ex) {
        Log::E(Log::TAG, "Failed to parse verify code from: %s.\nex=%s", respBody.c_str(), ex.what());
        CHECK_ERROR(ErrCode::JsonParseException);
    }

    return 0;
}

int ProofOssClient::getOssInfo(std::shared_ptr<HttpClient> httpClient, const std::string signedVerifyCode,
                               OssInfo& ossInfo)
{
    auto config = SAFE_GET_PTR(mConfig);

    std::string ossInfoUrl = config->mProofConfig->mUrl + config->mProofConfig->mApi.mOssInfo;
    httpClient->url(ossInfoUrl);
    httpClient->setHeader("Content-Type", "application/json");

    Log::I(Log::TAG, "%s time=%lld", FORMAT_METHOD, DateTime::CurrentMS());
    int ret = httpClient->syncPost(signedVerifyCode);
    Log::I(Log::TAG, "%s time=%lld", FORMAT_METHOD, DateTime::CurrentMS());
    if(ret < 0) {
        CHECK_ERROR(ErrCode::HttpClientErrorIndex + ret);
    }

    std::string respBody;
    ret = httpClient->getResponseBody(respBody);
    if(ret < 0) {
        CHECK_ERROR(ErrCode::HttpClientErrorIndex + ret);
    }
    Log::I(Log::TAG, "DidChnClient::getOssInfo() respBody=%s", respBody.c_str());

    try {
        Json jsonResp = Json::parse(respBody);
        int status = jsonResp["state"];
        if(status != 200) {
            CHECK_ERROR(ErrCode::ProofApiGetPropError);
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
        CHECK_ERROR(ErrCode::JsonParseException);
    }

    return 0;
}

int ProofOssClient::ossLogin()
{
    Log::D(Log::TAG, FORMAT_METHOD);
    if(mExpiredTime > DateTime::CurrentMS()) {
        Log::I(Log::TAG, "%s Expired time not reached, ignore to login.", FORMAT_METHOD);
        return 0;
    }

    ProofOssClient::OssInfo ossInfo;
    int ret = getOssInfo(ossInfo);
    CHECK_ERROR(ret);

    auto disk = elastos::sdk::CloudDisk::Find(elastos::sdk::CloudDisk::Domain::AliOss);
    ret = disk->login(ossInfo.mDisk, ossInfo.mUser, ossInfo.mPassword, ossInfo.mToken);
    CHECK_ERROR(ret);
    ret = disk->getPartition(ossInfo.mPartition, mOssPartition);
    CHECK_ERROR(ret);

    mExpiredTime = DateTime::CurrentMS() + 30 * 60 * 1000; // half of hour

        // Partition is already exists, ignore mount it.
//        ret = partition->mount();
//        CHECK_ERROR(ret);

    return 0;
}

int ProofOssClient::ossList(std::vector<std::string>& pathList)
{
    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);

    std::string did;
    int ret = sectyMgr->getDid(did);
    CHECK_ERROR(ret);

    auto ossFile = std::make_shared<elastos::sdk::CloudFile>();
    ret = ossFile->open(mOssPartition, did + "/", elastos::sdk::CloudMode::UserAll);
    CHECK_ERROR(ret);
    ret = ossFile->list(pathList);
    CHECK_ERROR(ret);
    ret = ossFile->close();
    CHECK_ERROR(ret);

    return ret;
}

int ProofOssClient::ossWrite(const std::string& path, std::shared_ptr<std::iostream> content)
{
    auto config = SAFE_GET_PTR(mConfig);
    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);

    std::string did;
    int ret = sectyMgr->getDid(did);
    CHECK_ERROR(ret);

    Log::D(Log::TAG, "%s filepath=%s", FORMAT_METHOD, (did + "/" + path).c_str());

    auto ossFile = std::make_shared<elastos::sdk::CloudFile>();
    ret = ossFile->open(mOssPartition, did + "/" + path, elastos::sdk::CloudMode::UserAll);
    CHECK_ERROR(ret);
    ret = ossFile->write(content);
    CHECK_ERROR(ret);
    ret = ossFile->close();
    CHECK_ERROR(ret);

    return ret;
}

int ProofOssClient::ossRead(const std::string& path, std::shared_ptr<std::iostream> content)
{
    auto config = SAFE_GET_PTR(mConfig);
    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);

    std::string did;
    int ret = sectyMgr->getDid(did);
    CHECK_ERROR(ret);

    Log::D(Log::TAG, "%s filepath=%s", FORMAT_METHOD, (did + "/" + path).c_str());

    auto ossFile = std::make_shared<elastos::sdk::CloudFile>();
    ret = ossFile->open(mOssPartition, did + "/" + path, elastos::sdk::CloudMode::UserAll);
    CHECK_ERROR(ret);
    ret = ossFile->read(content);
    CHECK_ERROR(ret);
    ret = ossFile->close();
    CHECK_ERROR(ret);

    return ret;
}

} // namespace elastos
