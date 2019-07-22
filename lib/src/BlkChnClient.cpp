#include <BlkChnClient.hpp>

#include <iomanip>

#include <DateTime.hpp>
#include <ErrCode.hpp>
#include <HttpClient.hpp>
#include <Log.hpp>
#include <MD5.hpp>
#include <Platform.hpp>
#include <SafePtr.hpp>


namespace elastos {

/* =========================================== */
/* === static variables initialize =========== */
/* =========================================== */
std::shared_ptr<BlkChnClient> BlkChnClient::gBlkChnClient {};

/* =========================================== */
/* === static function implement ============= */
/* =========================================== */
int BlkChnClient::InitInstance(std::weak_ptr<Config> config, std::weak_ptr<SecurityManager> sectyMgr)
{
    if(gBlkChnClient.get() != nullptr) {
        return 0;
    }

    struct Impl: BlkChnClient {
        Impl(std::weak_ptr<Config> config, std::weak_ptr<SecurityManager> sectyMgr) : BlkChnClient(config, sectyMgr) {
        }
    };

    HttpClient::InitGlobal();
    gBlkChnClient = std::make_shared<Impl>(config, sectyMgr);

//    int ret = gBlkChnClient->startMonitor();
//    if (ret < 0) {
//        return 0;
//    }

    return 0;
}

std::shared_ptr<BlkChnClient> BlkChnClient::GetInstance()
{
    assert(gBlkChnClient.get() != nullptr);
    return gBlkChnClient;
}

/* =========================================== */
/* === class public function implement  ====== */
/* =========================================== */
int BlkChnClient::setConnectTimeout(uint32_t milliSecond)
{
    mConnectTimeoutMS = milliSecond;
    return 0;
}

int BlkChnClient::appendMoniter(const std::string& path, const MonitorCallback& callback)
{
    if(path.empty() == true) {
        return ErrCode::InvalidArgument;
    }

	std::lock_guard<std::recursive_mutex> lock(mMutex);
    mMonitor.mMonitorCallbackMap[path] = callback;

    return 0;
}

int BlkChnClient::removeMoniter(const std::string& path)
{
	std::lock_guard<std::recursive_mutex> lock(mMutex);
    mMonitor.mMonitorCallbackMap.erase(path);

    return 0;
}

int BlkChnClient::downloadAllDidProps(const std::string& did, std::map<std::string, std::string>& propMap)
{
    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);
    auto config = SAFE_GET_PTR(mConfig);

    auto agentGetProps = config->mDidChainConfig->mAgentApi.mGetDidProps;
    std::string agentGetPropsPath = agentGetProps + did;

    std::string propArrayStr;
    int ret = downloadFromDidChn(agentGetPropsPath, propArrayStr);
    CHECK_ERROR(ret)

    std::string keyPath;
    ret = getPropKeyPathPrefix(keyPath);
    CHECK_ERROR(ret)

    Json jsonPropArray = Json::parse(propArrayStr);
    for(const auto& it: jsonPropArray){
        std::string propKey = it["key"];
        std::string propValue = it["value"];

        size_t pos = propKey.find(keyPath);
        if (pos != std::string::npos) {
            propKey.erase(pos, keyPath.length());
        }

        propMap[propKey] = propValue;
    }

    return 0;
}

// int BlkChnClient::uploadAllDidProps(const std::map<std::string, std::string>& propMap)
// {
//     std::string txid;
//     int ret = uploadAllDidProps(propMap, txid);
//     if (ret < 0) {
//         return ret;
//     }

//     return 0;
// }

int BlkChnClient::uploadAllDidProps(const std::multimap<std::string, std::string>& propMap, std::string& txid)
{
    if(propMap.empty() == true) {
        return ErrCode::InvalidArgument;
    }

    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);
    auto config = SAFE_GET_PTR(mConfig);

    Json jsonPropProt = Json::object();
    Json jsonPropArray = Json::array();
    for(const auto& prop: propMap) {
        std::string keyPath;
        int ret = getPropKeyPath(prop.first, keyPath);
        if (ret < 0) {
            return ret;
        }

        std::string propKey = keyPath;
        std::string propValue = prop.second;
        Json jsonProp = Json::object();
        jsonProp[DidProtocol::Name::Key] = propKey;
        jsonProp[DidProtocol::Name::Value] = propValue;
        if(prop.second.empty() == true) {
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

    auto propProtStr = jsonPropProt.dump();
    Log::I(Log::TAG, "BlkChnClient::uploadAllDidProps() propProt: %s", propProtStr.c_str());
    std::vector<uint8_t> originBytes(propProtStr.begin(), propProtStr.end());
    std::vector<uint8_t> signedBytes;
    int ret = sectyMgr->signData(originBytes, signedBytes);
    CHECK_ERROR(ret)

    std::string pubKey;
    ret = sectyMgr->getPublicKey(pubKey);
    CHECK_ERROR(ret)

    std::string msgStr = MD5::MakeHexString(originBytes);
    std::string sigStr = MD5::MakeHexString(signedBytes);

    // did prop key, sign, make {msg, sig, pub}
    std::string reqBody = std::string("{")
        + "\"pub\":\"" + pubKey + "\","
        + "\"msg\":\"" + msgStr + "\","
        + "\"sig\":\"" + sigStr + "\""
        + "}";

    auto didConfigUrl = config->mDidChainConfig->mUrl;
    auto agentUploadPath = config->mDidChainConfig->mAgentApi.mUploadDidProps;
    std::string agentUploadUrl = didConfigUrl + agentUploadPath;
    std::string authHeader;
    ret = sectyMgr->getDidAgentAuthHeader(authHeader);
    CHECK_ERROR(ret)
    Log::I(Log::TAG, "reqBody=%s", reqBody.c_str());

    HttpClient httpClient;
    httpClient.url(agentUploadUrl);
    httpClient.setHeader("Content-Type", "application/json");
    httpClient.setHeader("X-Elastos-Agent-Auth", authHeader);
    ret = httpClient.syncPost(reqBody);
    if(ret < 0) {
        return ErrCode::HttpClientError + ret;
    }

    std::string respBody;
    ret = httpClient.getResponseBody(respBody);
    if(ret < 0) {
        return ErrCode::HttpClientError + ret;
    }
    Log::I(Log::TAG, "respBody=%s", respBody.c_str());

    Json jsonResp = Json::parse(respBody);
    if(jsonResp["status"] != 200) {
        return ErrCode::BlkChnSetPropError;
    }

    txid = jsonResp["result"];
    if(txid.empty() == true) {
        return ErrCode::BlkChnBadTxIdError;
    }

    return 0;
}

int BlkChnClient::downloadDidProp(const std::string& did, const std::string& key, std::string& prop)
{
    prop = "";

    auto config = SAFE_GET_PTR(mConfig);

    std::string keyPath;
    int ret = getPropKeyPath(key, keyPath);
    CHECK_ERROR(ret)

    auto agentGetProps = config->mDidChainConfig->mAgentApi.mGetDidProps;
    auto agentDidProp = config->mDidChainConfig->mAgentApi.mDidProp;
    std::string agentGetPropPath = agentGetProps + did + agentDidProp + keyPath;

    std::string propArrayStr;
    ret = downloadFromDidChn(agentGetPropPath, propArrayStr);
    CHECK_ERROR(ret)

    Json jsonPropArray = Json::parse(propArrayStr);
    for(const auto& it: jsonPropArray) {
        std::string propKey = it["key"];
        if(keyPath != propKey) {
            continue;
        }

        prop = it["value"];
        return 0;
    }

    return ErrCode::BlkChnGetPropError;
}

// int BlkChnClient::uploadDidProp(const std::string& key, const std::string& prop)
// {
//     throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + " Unimplemented!!!");
// }

int BlkChnClient::downloadDidPropHistory(const std::string& did, const std::string& key, std::vector<std::string>& values)
{
    values.clear();

    std::string agentGetPropHistoryPath;
    int ret = getDidPropHistoryPath(did, key, agentGetPropHistoryPath);
    CHECK_ERROR(ret)

    std::string propArrayStr;
    ret = downloadFromDidChn(agentGetPropHistoryPath, propArrayStr);
    CHECK_ERROR(ret)

    Json jsonPropArray = Json::parse(propArrayStr);
    for(const auto& it: jsonPropArray) {
        values.push_back(it["value"]);
    }

    return 0;
}

int BlkChnClient::getDidPropHistoryPath(const std::string& did, const std::string& key, std::string& path)
{
    path.clear();

    auto config = SAFE_GET_PTR(mConfig);

    std::string keyPath;
    int ret = getPropKeyPath(key, keyPath);
    CHECK_ERROR(ret)

    auto agentGetProps = config->mDidChainConfig->mAgentApi.mGetDidProps;
    auto agentDidPropHistory = config->mDidChainConfig->mAgentApi.mDidPropHistory;
    path = agentGetProps + did + agentDidPropHistory + keyPath;

    return 0;
}

int BlkChnClient::downloadHumanInfo(const std::string& did, std::shared_ptr<HumanInfo>& humanInfo)
{
    humanInfo = std::make_shared<HumanInfo>();

    std::string pubKey;
    int ret = downloadDidProp(did, NamePublicKey, pubKey);
    CHECK_ERROR(ret)

    std::string expectedDid;
    ret = SecurityManager::GetDid(pubKey, expectedDid);
    CHECK_ERROR(ret)

    ret = humanInfo->setHumanInfo(HumanInfo::Item::ChainPubKey, pubKey);
    CHECK_ERROR(ret)

    std::vector<std::string> propHistory;
    ret = downloadDidPropHistory(did, NameCarrierId, propHistory);
    CHECK_ERROR(ret)

    for(const auto& it: propHistory) {
        HumanInfo::CarrierInfo carrierInfo;
        ret = humanInfo->deserialize(it, carrierInfo);
        if(ret < 0) {
            Log::W(Log::TAG, "BlkChnClient::downloadHumanInfo() Failed to sync CarrierId: %s", it.c_str());
            continue; // ignore error
        }

        ret = humanInfo->addCarrierInfo(carrierInfo, HumanInfo::Status::Offline);
        if(ret < 0) {
            if(ret == ErrCode::IgnoreMergeOldInfo) {
                Log::I(Log::TAG, "BlkChnClient::downloadHumanInfo() Ignore to sync CarrierId: %s", it.c_str());
            } else {
                Log::W(Log::TAG, "BlkChnClient::downloadHumanInfo() Failed to sync carrier info. CarrierId: %s", it.c_str());
            }
            continue; // ignore error
        }

        Log::I(Log::TAG, "BlkChnClient::downloadHumanInfo() Success to sync CarrierId: %s", it.c_str());
    }

    return 0;
}

// int BlkChnClient::uploadHumanInfo(const std::shared_ptr<HumanInfo>& humanInfo)
// {
//     std::string pubKey;
//     int ret = humanInfo->getHumanInfo(HumanInfo::Item::ChainPubKey, pubKey);
//     CHECK_ERROR(ret)

//     std::string devId;
//     ret = Platform::GetCurrentDevId(devId);
//     CHECK_ERROR(ret)

//     HumanInfo::CarrierInfo carrierInfo;
//     ret = humanInfo->getCarrierInfoByDevId(devId, carrierInfo);
//     CHECK_ERROR(ret)

//     std::string carrierInfoStr;
//     ret = humanInfo->serialize(carrierInfo, carrierInfoStr);
//     CHECK_ERROR(ret)

//     std::map<std::string, std::string> propMap;
//     propMap["PublicKey"] = pubKey;
//     propMap["CarrierID"] = carrierInfoStr;
//     ret = uploadAllDidProps(propMap);
//     CHECK_ERROR(ret)


//     return 0;
// }

int BlkChnClient::cacheDidProp(const std::string& key, const std::string& value)
{
    Log::I(Log::TAG, "BlkChnClient::cacheDidProp() key=%s, value=%s", key.c_str(), value.c_str());

    std::lock_guard<std::recursive_mutex> lock(mMutex);
    mDidPropCache.insert({key, value});

    return 0;
}

int BlkChnClient::uploadCachedDidProp()
{
    std::lock_guard<std::recursive_mutex> lock(mMutex);

    std::string txid;
    int ret = uploadAllDidProps(mDidPropCache, txid);
    CHECK_ERROR(ret)
    mDidPropCache.clear();

    // auto config = SAFE_GET_PTR(mConfig);
    // std::string getTxPath = config->mDidChainConfig->mApi.mGetTx + txid;
    // auto confirmFunc = [=] (int errcode, const std::string& keyPath, const std::string& result) {
    //     std::string result;
    //     int ret = downloadFromDidChain(getTxPath, result);
    //     if(ret < 0) {
    //         return;
    //     }

    //     Json jsonResult = Json::parse(result);
    //     int blockHeight = jsonResult["Height"];

    //     removeMoniter(getTxPath);
    // };

    // ret = BlkChnClient::appendMoniter(getTxPath, confirmFunc);
    // CHECK_ERROR(ret)

    return 0;
}

int BlkChnClient::printCachedDidProp(std::string& output)
{
    std::lock_guard<std::recursive_mutex> lock(mMutex);

    output = "";
    for(const auto& it: mDidPropCache) {
        auto pretty = elastos::Json(it.second);
        output += (it.first + ": " + pretty.dump(2) + "\n");
    }

    return 0;
}

/* =========================================== */
/* === class protected function implement  === */
/* =========================================== */


/* =========================================== */
/* === class private function implement  ===== */
/* =========================================== */
BlkChnClient::BlkChnClient(std::weak_ptr<Config> config, std::weak_ptr<SecurityManager> sectyMgr)
    : mConfig(config)
    , mSecurityManager(sectyMgr)
    , mConnectTimeoutMS(10000)
    , mMutex()
    , mPropKeyPathPrefix()
    , mDidPropCache()
    , mMonitor()
{
}

BlkChnClient::~BlkChnClient()
{
}

int BlkChnClient::startMonitor()
{
    mMonitor.mMonitorLooper = [&]() {
        int64_t current = DateTime::CurrentMS();
        Log::I(Log::TAG, "current timestamp=%lld", current);
        std::map<std::string, MonitorCallback> monitorCallbackMap;
        {
            std::lock_guard<std::recursive_mutex> lock(mMutex);
            monitorCallbackMap = mMonitor.mMonitorCallbackMap;
        }

        for(const auto& it: monitorCallbackMap) {
            auto& keyPath = it.first;
            auto& callback = it.second;

            std::string result;
            int ret = downloadFromDidChn(keyPath, result);
            callback(ret, keyPath, result);
        }

        int ret = mMonitor.mMonitorThread.sleepMS(mMonitor.mMonitorPendingMS);
        mMonitor.mMonitorThread.post(mMonitor.mMonitorLooper);
    };

    mMonitor.mMonitorThread.post(mMonitor.mMonitorLooper);

    return 0;
}

int BlkChnClient::downloadFromDidChn(const std::string& path, std::string& result)
{
    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);
    auto config = SAFE_GET_PTR(mConfig);

    auto didConfigUrl = config->mDidChainConfig->mUrl;
    std::string agentUrl = didConfigUrl + path;

    HttpClient httpClient;
    httpClient.url(agentUrl);
    int ret = httpClient.syncGet();
    if(ret < 0) {
        return ErrCode::HttpClientError + ret;
    }

    std::string respBody;
    ret = httpClient.getResponseBody(respBody);
    if(ret < 0) {
        return ErrCode::HttpClientError + ret;
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

int BlkChnClient::getPropKeyPathPrefix(std::string& keyPathPrefix)
{
    std::lock_guard<std::recursive_mutex> lock(mMutex);
    
    if(mPropKeyPathPrefix.empty() == true) {
        auto sectyMgr = SAFE_GET_PTR(mSecurityManager);

        std::string appId;
        int ret = sectyMgr->getDidPropAppId(appId);
        if (ret < 0) {
            return ret;
        }

        mPropKeyPathPrefix = "Apps/" + appId + "/";
    }

    keyPathPrefix = mPropKeyPathPrefix;

    return 0;
}

int BlkChnClient::getPropKeyPath(const std::string& key, std::string& keyPath)
{
    if(key == "PublicKey") {
        keyPath = key;
        return 0;
    }

    std::string keyPathPrefix;
    int ret = getPropKeyPathPrefix(keyPathPrefix);
    CHECK_ERROR(ret)

    keyPath = (keyPathPrefix + key);
    return 0;
}


} // namespace elastos
