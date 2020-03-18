#include <DidChnClient.hpp>

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
#include "DidChnClient.hpp"


namespace elastos {

/* =========================================== */
/* === static variables initialize =========== */
/* =========================================== */
std::shared_ptr<DidChnClient> DidChnClient::gDidChnClient {};

const struct {
    const char* key;
    bool withHistory;
} DidPropNames [] = {
        { DidChnClient::NamePublicKey, false },
        { DidChnClient::NameCarrierKey, true },
//         ignore to sync for this revision
        { DidChnClient::NameDetailKey, false },
//        { DidChnClient::NameIdentifyKey, true },
        { DidChnClient::NameFriendKey, true },
};


/* =========================================== */
/* === static function implement ============= */
/* =========================================== */
int DidChnClient::InitInstance(std::weak_ptr<Config> config,
                               std::weak_ptr<SecurityManager> sectyMgr) {
    if (gDidChnClient.get() != nullptr) {
        gDidChnClient.reset();
    }

    struct Impl : DidChnClient {
        Impl(std::weak_ptr<Config> config,
             std::weak_ptr<SecurityManager> sectyMgr)
            : DidChnClient(config, sectyMgr) {}
    };

    HttpClient::InitGlobal();
    gDidChnClient = std::make_shared<Impl>(config, sectyMgr);

    int ret = gDidChnClient->loadLocalData();
    if(ret == ErrCode::FileNotExistsError) {
        Log::D(Log::TAG, "DidChnClient::InitInstance() Local data file is not exists.");
        return 0;
    }
    CHECK_ERROR(ret);

    return 0;
}

std::shared_ptr<DidChnClient> DidChnClient::GetInstance()
{
    assert(gDidChnClient.get() != nullptr);
    return gDidChnClient;
}

/* =========================================== */
/* === class public function implement  ====== */
/* =========================================== */
int DidChnClient::setConnectTimeout(uint32_t milliSecond)
{
    mConnectTimeoutMS = milliSecond;
    return 0;
}

int DidChnClient::appendMoniter(const std::string& did, std::shared_ptr<MonitorCallback> callback, bool humanInfoOnly)
{
    if(did.empty() == true) {
        return ErrCode::InvalidArgument;
    }

	std::lock_guard<std::recursive_mutex> lock(mMutex);
    if(mMonitor.get() == nullptr) {
        return ErrCode::BlkChnMonStoppedError;
    }

    callback->mHumanInfoOnly = humanInfoOnly;
    mMonitor->mMonitorCallbackMap[did] = callback;

    return 0;
}

int DidChnClient::removeMoniter(const std::string& did)
{
    if(did.empty() == true) {
        return ErrCode::InvalidArgument;
    }

	std::lock_guard<std::recursive_mutex> lock(mMutex);
    if(mMonitor.get() == nullptr) {
        return ErrCode::BlkChnMonStoppedError;
    }

    mMonitor->mMonitorCallbackMap.erase(did);

    return 0;
}

int DidChnClient::cacheDidProp(const std::string& key, const std::string& value)
{
    bool needCache = false;
    for(const auto& it: DidPropNames) {
        if (key == it.key) {
            needCache = true;
            break;
        }
    }
    if(needCache == false) {
        Log::W(Log::TAG, "DidChnClient::cacheDidProp() Ignore to cache key=%s, value=%s", key.c_str(), value.c_str());
        return 0;
    }

    Log::I(Log::TAG, "DidChnClient::cacheDidProp() key=%s, value=%s", key.c_str(), value.c_str());

    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);
    std::string did;
    int ret = sectyMgr->getDid(did);
    CHECK_ERROR(ret)

    std::lock_guard<std::recursive_mutex> lock(mMutex);

    // refreshUpdateTime(did, key);
    mDidPropCache.push_back({key, value});

    ret = saveLocalData();
    CHECK_ERROR(ret);

    return 0;
}

int DidChnClient::uploadCachedDidProp()
{
    std::lock_guard<std::recursive_mutex> lock(mMutex);

    int ret = uploadDidPropsByAgent(mDidPropCache);
    CHECK_ERROR(ret)

    ret = clearDidPropCache(true);
    CHECK_ERROR(ret)

    return 0;
}

int DidChnClient::printCachedDidProp(std::string& output)
{
    std::lock_guard<std::recursive_mutex> lock(mMutex);

    try {
        Json jsonCache;
        jsonCache[JsonKey::DidPropCache] = mDidPropCache;
        jsonCache[JsonKey::UpdateTime] = mDidPropUpdateTime;
        output = jsonCache.dump(2);
    } catch(const std::exception& ex) {
        Log::E(Log::TAG, "Failed to print cached did prop.\nex=%s", ex.what());
        return ErrCode::JsonParseException;
    }

    return 0;
}

int DidChnClient::downloadDidProp(const std::string& did, bool humanInfoOnly,
                                  std::map<std::string, std::vector<std::string>>& didProps)
{
    didProps.clear();

    for(const auto& it: DidPropNames) {
        if (humanInfoOnly == true) {
            if (it.key == NameFriendKey
            || it.key == NameIdentifyKey) {
                continue;
            }
        }

        std::string key = it.key;

        std::shared_ptr<HttpClient> httpClient;
        std::vector<std::string> props;
        int ret = downloadDidPropsByAgent(httpClient, did, key, it.withHistory, props);
        if (ret == ErrCode::BlkChnEmptyPropError && it.key == NameDetailKey) {
            Log::I(Log::TAG, "Ignore to process empty detail key.");
            continue;
        }
        CHECK_ERROR(ret)

        didProps.insert({ key, std::move(props)} );
    }

    return 0;
}

/* =========================================== */
/* === class protected function implement  === */
/* =========================================== */


/* =========================================== */
/* === class private function implement  ===== */
/* =========================================== */
DidChnClient::DidChnClient(std::weak_ptr<Config> config, std::weak_ptr<SecurityManager> sectyMgr)
    : mConfig(config)
    , mSecurityManager(sectyMgr)
    , mConnectTimeoutMS(10000)
    , mMutex()
    , mPropKeyPathPrefix()
    , mDidPropCache()
    , mMonitor()
{
    Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);
}

DidChnClient::~DidChnClient()
{
    stopMonitor();
    Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);
}

int DidChnClient::startMonitor()
{
    std::lock_guard<std::recursive_mutex> lock(mMutex);
    mMonitor = std::make_shared<Monitor>();
    mMonitor->mMonitorThread = std::make_shared<ThreadPool>("didchn-monitor");

    mMonitor->mMonitorStopFlag = false;
    mMonitor->mMonitorLooper = [&]() {
        int64_t current = DateTime::CurrentMS();
        Log::I(Log::TAG, "%s current timestamp=%lld", __PRETTY_FUNCTION__, current);
        std::shared_ptr<Monitor> monitor;
        std::map<std::string, std::shared_ptr<MonitorCallback>> monitorCallbackMap;
        {
            std::lock_guard<std::recursive_mutex> lock(mMutex);

            monitor = mMonitor;
            if(monitor.get() == nullptr
            || monitor->mMonitorStopFlag == true) {
                Log::W(Log::TAG, "%s:%d Exit DidChnClient::Monitor loop", __PRETTY_FUNCTION__, __LINE__);
                return;
            }

            monitorCallbackMap = monitor->mMonitorCallbackMap;
        }

        for(const auto& it: monitorCallbackMap) {
            auto& did = it.first;
            auto& callback = it.second;

            std::ignore = DidChnClient::checkDidProps(monitor->mHttpClient, monitor->mMonitorStopFlag,
                                                      did, callback);
            if(monitor->mMonitorStopFlag == true) {
                Log::W(Log::TAG, "%s:%d Exit DidChnClient::Monitor loop", __PRETTY_FUNCTION__, __LINE__);
                return;
            }
        }
        if(monitor->mMonitorStopFlag == true) {
            Log::W(Log::TAG, "%s:%d Exit DidChnClient::Monitor loop", __PRETTY_FUNCTION__, __LINE__);
            return;
        }

        constexpr const uint32_t sleepTime = 200; //ms
        uint32_t sleepCount = monitor->MonitorPendingMS / sleepTime;
        for(int idx = 0; idx < sleepCount; idx++) {
            std::ignore = monitor->mMonitorThread->sleepMS(sleepTime);
            if(monitor->mMonitorStopFlag == true) {
                Log::W(Log::TAG, "%s:%d Exit DidChnClient::Monitor loop", __PRETTY_FUNCTION__, __LINE__);
                return;
            }
        }

        {
            std::lock_guard<std::recursive_mutex> lock(mMutex);
            if(monitor->mMonitorThread.get() == nullptr) {
                Log::W(Log::TAG, "%s:%d Exit DidChnClient::Monitor loop", __PRETTY_FUNCTION__, __LINE__);
                return;
            }
            monitor->mMonitorThread->post(monitor->mMonitorLooper);
        }
    };

    mMonitor->mMonitorThread->post(mMonitor->mMonitorLooper);
    Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);

    return 0;
}

int DidChnClient::stopMonitor()
{
    std::lock_guard<std::recursive_mutex> lock(mMutex);
    if(mMonitor.get() == nullptr) {
        return 0;
    }

    mMonitor->mMonitorCallbackMap.clear();

    if(mMonitor->mHttpClient.get() != nullptr) {
        Log::I(Log::TAG, "%s cancel HttpClient", __PRETTY_FUNCTION__);
        mMonitor->mHttpClient->cancel();
    }

    mMonitor->mMonitorStopFlag = true;

    mMonitor->mMonitorThread = nullptr;
    mMonitor = nullptr;
    Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);

    return 0;
}

void DidChnClient::refreshUpdateTime(const std::string& did, const std::string& key, int64_t updateTime)
{
    std::string updateTimeKey = (did + ":" + key);

    mDidPropUpdateTime.insert({updateTimeKey, updateTime});
}

bool DidChnClient::checkUpdateTime(const std::string& did, const std::string& key, int64_t updateTime)
{
    std::string updateTimeKey = (did + ":" + key);

    if(mDidPropUpdateTime.find(updateTimeKey) == mDidPropUpdateTime.end()) {
        return true;
    }

    int64_t lastUpdateTime = mDidPropUpdateTime[updateTimeKey];
    if(lastUpdateTime < updateTime) {
        return true;
    }

    return false;
}

int DidChnClient::uploadDidPropsByAgent(const std::vector<std::pair<std::string, std::string>>& didProps)
{
    if(didProps.size() == 0) {
        Log::I(Log::TAG, "DidChnClient::uploadCachedDidProp() Ignore to update empty cache.");
        return 0;
    }

    std::string didProtocolData;
    int ret = serializeDidProps(didProps, didProtocolData);
    CHECK_ERROR(ret)

    std::string didAgentData;
    ret = makeDidAgentData(didProtocolData, didAgentData);
    CHECK_ERROR(ret)

    ret = uploadDidAgentData(didAgentData);
    CHECK_ERROR(ret)

    return 0;
}

int DidChnClient::serializeDidProps(const std::vector<std::pair<std::string, std::string>>& didProps,
                                     std::string& result)
{
    if(didProps.size() == 0) {
        return ErrCode::BlkChnEmptyPropError;
    }

    Json jsonPropProt = Json::object();
    Json jsonPropArray = Json::array();
    for(const auto& prop: didProps) {
        std::string key = prop.first;
        std::string value = prop.second;

        std::string propKey;
        std::string propValue = value;
        int ret = getPropKeyPath(key, propKey);
        if (ret < 0) {
            return ret;
        }

        if(key == NameFriendKey) {
            auto sectyMgr = SAFE_GET_PTR(mSecurityManager);
            std::string pubKey;
            int ret = sectyMgr->getPublicKey(pubKey);
            CHECK_ERROR(ret)
            std::vector<uint8_t> cryptoData;
            ret = sectyMgr->encryptData(pubKey, SecurityManager::DefaultCryptoAlgorithm, std::vector<uint8_t>{std::begin(value), std::end(value)} , cryptoData);
            CHECK_ERROR(ret);
            propValue = std::string{cryptoData.begin(), cryptoData.end()};
        }

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

    result = jsonPropProt.dump();
    Log::I(Log::TAG, "DidChnClient::serializeDidProps() result=%s", result.c_str());

    return 0;
}

int DidChnClient::makeDidAgentData(const std::string& didProtocolData, std::string& result)
{
    if(didProtocolData.empty() == true) {
        return ErrCode::BlkChnEmptyPropError;
    }

    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);

    std::vector<uint8_t> originBytes(didProtocolData.begin(), didProtocolData.end());
    std::vector<uint8_t> signedBytes;
    int ret = sectyMgr->signData(originBytes, signedBytes);
    CHECK_ERROR(ret)

    std::string pubKey;
    ret = sectyMgr->getPublicKey(pubKey);
    CHECK_ERROR(ret)

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

int DidChnClient::uploadDidAgentData(const std::string& didAgentData)
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
    CHECK_ERROR(ret)

    const auto& reqBody = didAgentData;
    Log::I(Log::TAG, "DidChnClient::uploadDidAgentData() reqBody=%s", reqBody.c_str());

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
    Log::I(Log::TAG, "DidChnClient::uploadDidAgentData() respBody=%s", respBody.c_str());

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

int DidChnClient::checkDidProps(std::shared_ptr<HttpClient>& httpClient, const bool& stopFlag,
                                const std::string& did, std::shared_ptr<MonitorCallback> callback)
{
    if(callback.get() == nullptr) {
        return ErrCode::InvalidArgument;
    }

    for(const auto& it: DidPropNames) {
        if(callback->mHumanInfoOnly == true) {
            if(it.key == NameFriendKey
            || it.key == NameIdentifyKey) {
                continue;
            }
        }

        std::string key = it.key;

        std::vector<std::string> didProps;
        int ret = downloadDidPropsByAgent(httpClient, did, key, it.withHistory, didProps);
        if(stopFlag == true) {
            Log::V(Log::TAG, "DidChnClient::checkDidProps() Stop to check %s: %s", did.c_str(), key.c_str());
            return ErrCode::BlkChnMonStoppedError;
        }

        if(ret < 0) {
            Log::V(Log::TAG, "DidChnClient::checkDidProps() Failed to check %s: %s", did.c_str(), key.c_str());
            callback->onError(did, key, ret);
            continue;
        }

        int64_t lastUpdateTime  = checkDidProps(did, key, didProps);
        if(lastUpdateTime == ErrCode::BlkChnOldUpdateTimeError) { // not changed
            continue;
        }
        if(lastUpdateTime < 0) {
            callback->onError(did, key, (int)lastUpdateTime);
            continue;
        }

        ret = callback->onChanged(did, key, didProps);
        if(ret < 0) {
            callback->onError(did, key, ret);
            continue;
        }

        refreshUpdateTime(did, key, lastUpdateTime);
    }

    return 0;
}

int64_t DidChnClient::checkDidProps(const std::string& did, const std::string& key, const std::vector<std::string>& didProps)
{
    int64_t lastUpdateTime = -1;
    for(const auto& prop: didProps) {
        if(key == NamePublicKey) {
            lastUpdateTime = 0;
            break;
        }

        Json jsonProp = Json::parse(prop);
        if(jsonProp.find(JsonKey::UpdateTime) == jsonProp.end()) {
            Log::V(Log::TAG, "DidChnClient::checkDidProps() Failed to get updatetime from %s: %s -> %s", did.c_str(), key.c_str(), prop.c_str());
            return ErrCode::BlkChnBadUpdateTimeError;
        }

        int64_t updateTime = jsonProp[JsonKey::UpdateTime];
        if(lastUpdateTime < updateTime) {
            lastUpdateTime = updateTime;
        }
    }

    bool changed = checkUpdateTime(did, key, lastUpdateTime);
    Log::V(Log::TAG, "DidChnClient::checkDidProps() %s: %s changed=%d.", did.c_str(), key.c_str(), changed);

    return (changed == true ? lastUpdateTime : ErrCode::BlkChnOldUpdateTimeError);
}

int DidChnClient::downloadDidPropsByAgent(std::shared_ptr<HttpClient>& httpClient,
                                          const std::string& did, const std::string& key, bool withHistory,
                                          std::vector<std::string>& values) {
    std::string propArrayStr;
    std::string propCacheArrayStr;

    std::string dataPath;
    int ret = getDidPropPath(did, key, withHistory, dataPath, false);
    CHECK_ERROR(ret)
    ret = downloadDidChnData(httpClient, dataPath, propArrayStr);
    if(ret == ErrCode::BlkChnEmptyPropError) {
        Log::W(Log::TAG, "Get property from DidChain return empty. urlpath=%s", dataPath.c_str());
        ret = 0;
    }
    if (propArrayStr.empty() == true
    || withHistory == true) { // if history == true or cache is empty
        std::string dataCachePath;
        ret = getDidPropPath(did, key, withHistory, dataCachePath, true);
        CHECK_ERROR(ret)
        ret = downloadDidChnData(httpClient, dataCachePath, propCacheArrayStr);
        if (ret == ErrCode::BlkChnEmptyPropError) {
            Log::W(Log::TAG, "Get property from DidChain Cache return empty. urlpath=%s", dataCachePath.c_str());
            ret = 0;
        }
    }
    CHECK_ERROR(ret);

    if(propCacheArrayStr.empty() == true
    && propArrayStr.empty() == true) {
        CHECK_ERROR(ErrCode::BlkChnEmptyPropError);
    }

    try {
        if (propCacheArrayStr.empty() == false) {
            Json jsonPropArray = Json::parse(propCacheArrayStr);
            for (const auto &it: jsonPropArray) {
                std::string value = it["value"];
                if (key == NameFriendKey) {
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
                if (key == NameFriendKey) {
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

int DidChnClient::downloadDidChnData(std::shared_ptr<HttpClient>& httpClient,
                                     const std::string& path, std::string& result)
{
    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);
    auto config = SAFE_GET_PTR(mConfig);

    auto didConfigUrl = config->mDidChainConfig->mUrl;
    std::string agentUrl = didConfigUrl + path;

    httpClient = std::make_shared<HttpClient>();
    httpClient->url(agentUrl);
    httpClient->setHeader("Content-Type", "application/json");
    int ret = httpClient->syncGet();
    if(ret < 0) {
        return ErrCode::HttpClientError + ret;
    }

    std::string respBody;
    ret = httpClient->getResponseBody(respBody);
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

int DidChnClient::getDidPropPath(const std::string& did, const std::string& key, bool withHistory,
                                  std::string& path, bool useCache)
{
    path.clear();

    auto config = SAFE_GET_PTR(mConfig);

    std::string keyPath;
    int ret = getPropKeyPath(key, keyPath);
    CHECK_ERROR(ret)

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

int DidChnClient::clearDidPropCache(bool refreshUpdateTime)
{
    Log::I(Log::TAG, "DidChnClient::clearDidPropCache() refreshUpdateTime=%d", refreshUpdateTime);

    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);
    std::string did;
    int ret = sectyMgr->getDid(did);
    CHECK_ERROR(ret)

    std::lock_guard<std::recursive_mutex> lock(mMutex);

    if(refreshUpdateTime == true) {
        for (const auto &it: mDidPropCache) {
            const auto &key = it.first;

            std::vector<std::string> props = {it.second};
            int64_t ret = DidChnClient::checkDidProps(did, key, props);
            if (ret == ErrCode::BlkChnOldUpdateTimeError) { // cached did prop is old
                continue;
            }
            if (ret < 0) {  // cached did prop is old=
                Log::E(Log::TAG, "DidChnClient::clearDidPropCache() Failed to check %s:%s", did.c_str(), key.c_str());
                continue;
            }
            int64_t lastUpdateTime = ret;

            DidChnClient::refreshUpdateTime(did, key, lastUpdateTime);
        }
    }

    mDidPropCache.clear();

    ret = saveLocalData();
    CHECK_ERROR(ret);

    return 0;
}

int DidChnClient::loadLocalData()
{
    auto config = SAFE_GET_PTR(mConfig);
    auto dataFilePath = elastos::filesystem::path(config->mUserDataDir) / DataFileName;

    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);
    std::vector<uint8_t> originData;
    int ret = sectyMgr->loadCryptoFile(dataFilePath.string(), originData);
    CHECK_ERROR(ret)

    std::string cacheData {originData.begin(), originData.end()};
    try {
        Json jsonCache = Json::parse(cacheData);

        mDidPropCache = jsonCache[JsonKey::DidPropCache].get<std::vector<std::pair<std::string, std::string>>>();
        mDidPropUpdateTime = jsonCache[JsonKey::UpdateTime].get<std::map<std::string, int64_t>>();
    } catch(const std::exception& ex) {
        Log::E(Log::TAG, "Failed to load local data from: %s.\nex=%s", dataFilePath.c_str(), ex.what());
        return ErrCode::JsonParseException;
    }

    Log::I(Log::TAG, "Success to load local data from: %s.", dataFilePath.c_str());
    return 0;
}

int DidChnClient::saveLocalData()
{
    auto config = SAFE_GET_PTR(mConfig);
    auto dataFilePath = elastos::filesystem::path(config->mUserDataDir) / DataFileName;

    std::string cacheData;
    try {
        Json jsonCache;
        jsonCache[JsonKey::DidPropCache] = mDidPropCache;
        jsonCache[JsonKey::UpdateTime] = mDidPropUpdateTime;
        cacheData = jsonCache.dump();
    } catch(const std::exception& ex) {
        Log::E(Log::TAG, "Failed to save local data to: %s.\nex=%s", dataFilePath.c_str(), ex.what());
        return ErrCode::JsonParseException;
    }

    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);
    std::vector<uint8_t> originData {cacheData.begin(), cacheData.end()};
    int ret = sectyMgr->saveCryptoFile(dataFilePath.string(), originData);
    CHECK_ERROR(ret)

    Log::D(Log::TAG, "Save local data to: %s, data: %s", dataFilePath.c_str(), cacheData.c_str());

    return 0;
}

int DidChnClient::getPropKeyPath(const std::string& key, std::string& keyPath)
{
    if(key == NamePublicKey) {
        keyPath = key;
    } else if(key == NameCarrierKey) {
        auto keyPathPrefix = "DID/DidFriend/";
        keyPath = (keyPathPrefix + key);
    } else {
        if(mPropKeyPathPrefix.empty() == true) {
            auto sectyMgr = SAFE_GET_PTR(mSecurityManager);

            std::string appId;
            int ret = sectyMgr->getDidPropAppId(appId);
            CHECK_ERROR(ret)

            std::lock_guard<std::recursive_mutex> lock(mMutex);
            mPropKeyPathPrefix = "Apps/" + appId + "/";
        }

        keyPath = (mPropKeyPathPrefix + key);
    }

    return 0;
}

} // namespace elastos
