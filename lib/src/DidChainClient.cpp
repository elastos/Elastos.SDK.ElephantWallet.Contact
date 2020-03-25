#include <DidChainClient.hpp>

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
#include "DidChainClient.hpp"


namespace elastos {

/* =========================================== */
/* === static variables initialize =========== */
/* =========================================== */
const std::map<std::string, bool> DidChainClient::mDidPropHistoryMap = {
        { PropKey::PublicKey, false },
        { PropKey::CarrierInfo, true },
//         ignore to sync for this revision
        { PropKey::DetailKey, false },
        { PropKey::IdentifyKey, true },
        { PropKey::FriendKey, true },
};


/* =========================================== */
/* === static function implement ============= */
/* =========================================== */

/* =========================================== */
/* === class public function implement  ====== */
/* =========================================== */
DidChainClient::DidChainClient(std::weak_ptr<Config> config, std::weak_ptr<SecurityManager> sectyMgr)
        : mConfig(config)
        , mSecurityManager(sectyMgr)
        , mPropKeyPathPrefix()
{
    Log::I(Log::TAG, FORMAT_METHOD);
}

DidChainClient::~DidChainClient()
{
    Log::I(Log::TAG, FORMAT_METHOD);
}

int DidChainClient::uploadProperties(const std::multimap<std::string, std::string>& changedPropMap,
                                     const std::map<std::string, std::shared_ptr<std::iostream>>& totalPropMap)
{
    if(changedPropMap.size() == 0) {
        Log::I(Log::TAG, "%s Ignore to update empty cache.", FORMAT_METHOD);
        return 0;
    }

    std::string didProtocolData;
    int ret = serializeDidProps(changedPropMap, didProtocolData);
    CHECK_ERROR(ret);

    std::string didAgentData;
    ret = makeDidAgentData(didProtocolData, didAgentData);
    CHECK_ERROR(ret);

    ret = uploadDidPropsByAgent(didAgentData);
    CHECK_ERROR(ret);

    return 0;
}

int DidChainClient::downloadProperties(const std::string& fromDid,
                                       std::multimap<std::string, std::string>& savedPropMap,
                                       std::map<std::string, std::shared_ptr<std::iostream>>& totalPropMap)
{
    std::set<std::string> savedPropKeySet;
    for(const auto& [key, value]: savedPropMap) {
        savedPropKeySet.emplace(key);
    }

    savedPropMap.clear();
    for(const auto& key: savedPropKeySet) {
        bool withHistory = false;
        auto found = mDidPropHistoryMap.find(key);
        if(found != mDidPropHistoryMap.end()) {
            withHistory = found->second;
        }

        std::vector<std::string> propList;
        int ret = downloadDidPropsByAgent(fromDid, key, withHistory, propList);
        if (ret == ErrCode::BlkChnEmptyPropError) {
            Log::I(Log::TAG, "%s Ignore to process empty key: %s.", FORMAT_METHOD, key.c_str());
            continue;
        }
        CHECK_ERROR(ret);

        for(const auto& prop: propList) {
            savedPropMap.emplace(key, prop);
        }
    }

    return 0;
}

/* =========================================== */
/* === class protected function implement  === */
/* =========================================== */


/* =========================================== */
/* === class private function implement  ===== */
/* =========================================== */
int DidChainClient::downloadDidPropsByAgent(const std::string& did, const std::string& key, bool withHistory,
                                            std::vector<std::string>& values)
{
    std::string propArrayStr;
    std::string propCacheArrayStr;

    std::string dataPath;
    int ret = getDidPropPath(did, key, withHistory, dataPath, false);
    CHECK_ERROR(ret);
    ret = downloadDidChnData(dataPath, propArrayStr);
    if(ret == ErrCode::BlkChnEmptyPropError) {
        Log::W(Log::TAG, "Get property from DidChain return empty. urlpath=%s", dataPath.c_str());
        ret = 0;
    }
    if (propArrayStr.empty() == true
        || withHistory == true) { // if history == true or cache is empty
        std::string dataCachePath;
        ret = getDidPropPath(did, key, withHistory, dataCachePath, true);
        CHECK_ERROR(ret);
        ret = downloadDidChnData(dataCachePath, propCacheArrayStr);
        if (ret == ErrCode::BlkChnEmptyPropError) {
            Log::W(Log::TAG, "Get property from DidChain Cache return empty. urlpath=%s", dataCachePath.c_str());
            ret = 0;
        }
    }
    CHECK_ERROR(ret);

    if(propCacheArrayStr.empty() == true
    && propArrayStr.empty() == true) {
        return (ErrCode::BlkChnEmptyPropError);
    }

    try {
        if (propCacheArrayStr.empty() == false) {
            Json jsonPropArray = Json::parse(propCacheArrayStr);
            for (const auto &it: jsonPropArray) {
                std::string value = it["value"];
                if (key == PropKey::FriendKey) {
                    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);
                    std::vector<uint8_t> originData;
                    ret = sectyMgr->decryptData(SecurityManager::DefaultCryptoAlgorithm,
                                                std::vector<uint8_t>{std::begin(value), std::end(value)},
                                                originData);
                    CHECK_ERROR(ret);
                    value = std::string{originData.begin(), originData.end()};
                }
                values.push_back(value);
            }
        }
        if(propArrayStr.empty() == false) {
            Json jsonPropArray = Json::parse(propArrayStr);
            for (const auto &it: jsonPropArray) {
                std::string value = it["value"];
                if (key == PropKey::FriendKey) {
                    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);
                    std::vector<uint8_t> originData;
                    ret = sectyMgr->decryptData(SecurityManager::DefaultCryptoAlgorithm,
                                                std::vector<uint8_t>{std::begin(value), std::end(value)},
                                                originData);
                    CHECK_ERROR(ret);
                    value = std::string{originData.begin(), originData.end()};
                }
                values.push_back(value);
            }
        }
    } catch(const std::exception& ex) {
        Log::E(Log::TAG, "Failed to parse properties from DidChain.\nex=%s", ex.what());
        return ErrCode::JsonParseException;
    }

    return 0;
}

int DidChainClient::downloadDidChnData(const std::string& path, std::string& result)
{
    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);
    auto config = SAFE_GET_PTR(mConfig);

    auto didConfigUrl = config->mDidChainConfig->mUrl;
    std::string agentUrl = didConfigUrl + path;

    HttpClient httpClient;
    httpClient.url(agentUrl);
    httpClient.setHeader("Content-Type", "application/json");
    int ret = httpClient.syncGet();
    if(ret < 0) {
        CHECK_ERROR(ErrCode::HttpClientErrorIndex + ret);
    }

    std::string respBody;
    ret = httpClient.getResponseBody(respBody);
    if(ret < 0) {
        CHECK_ERROR(ErrCode::HttpClientErrorIndex + ret);
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

int DidChainClient::serializeDidProps(const std::multimap<std::string, std::string>& changedPropMap,
                                      std::string& result)
{
    if(changedPropMap.size() == 0) {
        return ErrCode::BlkChnEmptyPropError;
    }

    Json jsonPropProt = Json::object();
    Json jsonPropArray = Json::array();
    for(const auto& [key, value]: changedPropMap) {
        std::string propKey;
        std::string propValue = value;
        int ret = getPropKeyPath(key, propKey);
        if (ret < 0) {
            return ret;
        }

        if(key == PropKey::FriendKey) {
            auto sectyMgr = SAFE_GET_PTR(mSecurityManager);
            std::string pubKey;
            int ret = sectyMgr->getPublicKey(pubKey);
            CHECK_ERROR(ret);
            std::vector<uint8_t> cryptoData;
            ret = sectyMgr->encryptData(pubKey, SecurityManager::DefaultCryptoAlgorithm, std::vector<uint8_t>{std::begin(value), std::end(value)} , cryptoData);
            CHECK_ERROR(ret);
            propValue = std::string{cryptoData.begin(), cryptoData.end()};
        }

        Json jsonProp = Json::object();
        jsonProp[DidProtocol::Name::Key] = propKey;
        jsonProp[DidProtocol::Name::Value] = propValue;
        if(value.empty() == true) {
            jsonProp[DidProtocol::Name::Status] = DidProtocol::Value::Status::Deprecated;
        } else {
            jsonProp[DidProtocol::Name::Status] = DidProtocol::Value::Status::Normal;
        }

        jsonPropArray.push_back(jsonProp);
    }

    jsonPropProt[DidProtocol::Name::Tag] = DidProtocol::Value::Tag;
    jsonPropProt[DidProtocol::Name::Ver] = DidProtocol::Value::Ver;
    jsonPropProt[DidProtocol::Name::Status] = DidProtocol::Value::Status::Normal;
    jsonPropProt[DidProtocol::Name::Properties] = jsonPropArray;

    result = jsonPropProt.dump();
    Log::I(Log::TAG, "%s result=%s", FORMAT_METHOD, result.c_str());

    return 0;
}

int DidChainClient::makeDidAgentData(const std::string& didProtocolData, std::string& result)
{
    if(didProtocolData.empty() == true) {
        return ErrCode::BlkChnEmptyPropError;
    }

    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);

    std::vector<uint8_t> originBytes(didProtocolData.begin(), didProtocolData.end());
    std::vector<uint8_t> signedBytes;
    int ret = sectyMgr->signData(originBytes, signedBytes);
    CHECK_ERROR(ret);

    std::string pubKey;
    ret = sectyMgr->getPublicKey(pubKey);
    CHECK_ERROR(ret);

    std::string msgStr = MD5::MakeHexString(originBytes);
    std::string sigStr = MD5::MakeHexString(signedBytes);

    // did prop key, sign, make {msg, sig, pub}
    result = std::string("{")
             + "\"pub\":\"" + pubKey + "\","
             + "\"msg\":\"" + msgStr + "\","
             + "\"sig\":\"" + sigStr + "\""
             + "}";
    return 0;
}

int DidChainClient::uploadDidPropsByAgent(const std::string& didAgentData)
{
    if(didAgentData.empty() == true) {
        return ErrCode::BlkChnEmptyPropError;
    }

    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);
    auto config = SAFE_GET_PTR(mConfig);

    auto didConfigUrl = config->mDidChainConfig->mUrl;
    auto agentUploadPath = config->mDidChainConfig->mAgentApi.mUploadDidProps;
    std::string agentUploadUrl = didConfigUrl + agentUploadPath;
    std::string authHeader;
    int ret = sectyMgr->getDidAgentAuthHeader(authHeader);
    CHECK_ERROR(ret);

    const auto& reqBody = didAgentData;
    Log::I(Log::TAG, "%s reqBody=%s", FORMAT_METHOD, reqBody.c_str());

    HttpClient httpClient;
    httpClient.url(agentUploadUrl);
    httpClient.setHeader("Content-Type", "application/json");
    httpClient.setHeader("X-Elastos-Agent-Auth", authHeader);
    ret = httpClient.syncPost(reqBody);
    if(ret < 0) {
        CHECK_ERROR(ErrCode::HttpClientErrorIndex + ret);
    }

    std::string respBody;
    ret = httpClient.getResponseBody(respBody);
    if(ret < 0) {
        CHECK_ERROR(ErrCode::HttpClientErrorIndex + ret);
    }
    Log::I(Log::TAG, "%s respBody=%s", FORMAT_METHOD, respBody.c_str());

    Json jsonResp = Json::parse(respBody);
    if(jsonResp["status"] != 200) {
        return ErrCode::BlkChnSetPropError;
    }

    std::string txid = jsonResp["result"];
    if(txid.empty() == true) {
        return ErrCode::BlkChnBadTxIdError;
    }

    return 0;
}

int DidChainClient::getDidPropPath(const std::string& did, const std::string& key, bool withHistory,
                                   std::string& path, bool useCache)
{
    path.clear();

    auto config = SAFE_GET_PTR(mConfig);

    std::string keyPath;
    int ret = getPropKeyPath(key, keyPath);
    CHECK_ERROR(ret);

    std::string agentGetProps;
    if(useCache == true) {
        agentGetProps = config->mDidChainConfig->mAgentApi.mGetCacheDidProps;
    } else {
        agentGetProps = config->mDidChainConfig->mAgentApi.mGetDidProps;
    }
    auto agentDidPropApi = (withHistory == true
                            ? config->mDidChainConfig->mAgentApi.mDidPropHistory
                            : config->mDidChainConfig->mAgentApi.mDidProp);
    path = agentGetProps + did + agentDidPropApi + keyPath;

    return 0;
}

int DidChainClient::getPropKeyPath(const std::string& key, std::string& keyPath)
{
    if(key == PropKey::PublicKey) {
        keyPath = key;
    } else if(key == PropKey::CarrierInfo) {
        auto keyPathPrefix = "DID/DidFriend/";
        keyPath = (keyPathPrefix + key);
    } else {
        if(mPropKeyPathPrefix.empty() == true) {
            auto sectyMgr = SAFE_GET_PTR(mSecurityManager);

            std::string appId;
            int ret = sectyMgr->getDidPropAppId(appId);
            CHECK_ERROR(ret);

            mPropKeyPathPrefix = "Apps/" + appId + "/";
        }

        keyPath = (mPropKeyPathPrefix + key);
    }

    return 0;
}

} // namespace elastos
