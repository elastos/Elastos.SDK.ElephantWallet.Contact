#include <RemoteStorageManager.hpp>

#include <CompatibleFileSystem.hpp>
#include <DateTime.hpp>
#include <ErrCode.hpp>
#include <FriendInfo.hpp>
#include <Json.hpp>
#include <JsonDefine.hpp>
#include <Log.hpp>
#include <Platform.hpp>
#include <SafePtr.hpp>
#include <UserInfo.hpp>
#include <UserManager.hpp>

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
int RemoteStorageManager::setConfig(std::weak_ptr<Config> config,
                                    std::weak_ptr<SecurityManager> sectyMgr)
{
    mConfig = config;
    mSecurityManager = sectyMgr;

    int ret = loadLocalData();
    if(ret == ErrCode::FileNotExistsError) {
        Log::D(Log::TAG, "DidChnClient::InitInstance() Local data file is not exists.");
        return 0;
    }
    CHECK_ERROR(ret);

    return 0;
}

void RemoteStorageManager::addClient(ClientType type, std::shared_ptr<RemoteStorageClient> client)
{
    Log::I(Log::TAG, FORMAT_METHOD);

    std::lock_guard<std::recursive_mutex> lock(mMutex);
    mRemoteStorageClientMap[type] = client;
}

int RemoteStorageManager::getClient(ClientType type, std::shared_ptr<RemoteStorageClient>& client)
{
    std::lock_guard<std::recursive_mutex> lock(mMutex);
    if(mRemoteStorageClientMap.find(type) == mRemoteStorageClientMap.end()) {
        CHECK_ERROR(ErrCode::NotFoundError);
    }

    client = mRemoteStorageClientMap[type];

    return 0;
}

int RemoteStorageManager::cacheProperty(const std::string& did, const char* key)
{
    Log::I(Log::TAG, FORMAT_METHOD);
    if(did.empty() == true) {
        CHECK_ERROR(ErrCode::InvalidArgument);
    }
    if(key == nullptr) {
        CHECK_ERROR(ErrCode::InvalidArgument);
    }

    std::lock_guard<std::recursive_mutex> lock(mMutex);
    mPropCache[did] = key;

    int ret = saveLocalData();
    CHECK_ERROR(ret);

    return 0;
}

int RemoteStorageManager::uploadData(const std::vector<ClientType>& toClient,
                                     const std::shared_ptr<UserInfo> userInfo,
                                     const std::vector<std::shared_ptr<FriendInfo>>& friendInfoList,
                                     const std::shared_ptr<std::fstream> carrierData)
{
    Log::I(Log::TAG, FORMAT_METHOD);

    auto config = SAFE_GET_PTR(mConfig);

    std::multimap<std::string, std::string> changedPropMap;
    std::map<std::string, std::shared_ptr<std::iostream>> totalPropMap;
    {
        std::lock_guard<std::recursive_mutex> lock(mMutex);
        for(const auto& [did, propKey]: mPropCache) {
            std::string segment;
            std::string path;
            if (propKey == PropKey::FriendKey) {
                int ret = packFriendSegment(friendInfoList, did, segment);
                CHECK_ERROR(ret);
                path = FriendManager::DataFileName;
            } else if (propKey == PropKey::PublicKey
            || propKey == PropKey::DetailKey
            || propKey == PropKey::IdentifyKey
            || propKey == PropKey::CarrierInfo){
                int ret = packUserSegment(userInfo, propKey, segment);
                CHECK_ERROR(ret);
                path = UserManager::DataFileName;
            } else {
                CHECK_ERROR(ErrCode::NotExpectedReachedError);
            }

            changedPropMap.emplace(propKey, segment);
            totalPropMap[path] = std::make_shared<std::fstream>(config->mUserDataDir + "/" + path);

            if (propKey == PropKey::CarrierInfo) {
                if(carrierData.get() == nullptr) {
                    CHECK_ERROR(ErrCode::InvalidArgument);
                }

                std::string currDevId;
                int ret = Platform::GetCurrentDevId(currDevId);
                CHECK_ERROR(ret);
                path = currDevId + "/carrier.data";
                totalPropMap[path] = carrierData;
            }
        }
    }

    for (const auto& [type, client]: mRemoteStorageClientMap){
        int ret = client->uploadProperties(changedPropMap, totalPropMap);
        if(ErrCode::AdditivityIndex < ret && ret < 0) {
            ret += ErrCode::RemoteStorageClientErrorIndex;
        }
        CHECK_ERROR(ret);
    }

    {
        std::lock_guard<std::recursive_mutex> lock(mMutex);
        mPropCache.clear();
        int ret = saveLocalData();
        CHECK_ERROR(ret);
    }

    return 0;
}

int RemoteStorageManager::downloadData(const std::vector<ClientType>& fromClient,
                                       std::shared_ptr<UserInfo>& userInfo,
                                       std::vector<std::shared_ptr<FriendInfo>>& friendInfoList,
                                       std::shared_ptr<std::fstream>& carrierData)
{
    Log::I(Log::TAG, FORMAT_METHOD);

    std::string currDevId;
    int ret = Platform::GetCurrentDevId(currDevId);
    CHECK_ERROR(ret);
    std::string carrierDataPath = currDevId + "/carrier.data";

    std::multimap<std::string, std::string> savedPropMap {
            {PropKey::PublicKey, ""},
            {PropKey::CarrierInfo, ""},
            {PropKey::DetailKey, ""},
            {PropKey::IdentifyKey, ""},
            {PropKey::FriendKey, ""},
    };
    std::map<std::string, std::shared_ptr<std::iostream>> totalPropMap {
            {UserManager::DataFileName, nullptr},
            {FriendManager::DataFileName, nullptr},
    };
    if(carrierData.get() != nullptr) {
        totalPropMap[carrierDataPath] = nullptr;
    }

    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);
    std::string did;
    ret = sectyMgr->getDid(did);
    CHECK_ERROR(ret);
    for (const auto& [type, client]: mRemoteStorageClientMap){
        int ret = client->downloadProperties(did, savedPropMap, totalPropMap);
        if(ErrCode::AdditivityIndex < ret && ret < 0) {
            ret += ErrCode::RemoteStorageClientErrorIndex;
        }
        CHECK_ERROR(ret);
    }

    for(const auto& [path, content]: totalPropMap) {
        if(content == nullptr) {
            Log::W(Log::TAG, "%s Ignore to process empty content from: %s", FORMAT_METHOD, path.c_str());
            continue;
        }

        if(path == UserManager::DataFileName) {
            std::string data;
            (*content) >> data;
            int ret = unpackUserData(data, userInfo);
            CHECK_ERROR(ret);
        } else if(path == UserManager::DataFileName) {
            std::string data;
            (*content) >> data;
            int ret = unpackFriendData(data, friendInfoList);
            CHECK_ERROR(ret);
        } else if(path == carrierDataPath) {
            if(carrierData.get() == nullptr) {
                CHECK_ERROR(ErrCode::InvalidArgument);
            }
            (*carrierData) << content->rdbuf();
        } else {
            CHECK_ERROR(ErrCode::NotExpectedReachedError);
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
int RemoteStorageManager::loadLocalData()
{
    auto config = SAFE_GET_PTR(mConfig);
    auto dataFilePath = elastos::filesystem::path(config->mUserDataDir) / DataFileName;

    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);
    std::vector<uint8_t> originData;
    int ret = sectyMgr->loadCryptoFile(dataFilePath.string(), originData);
    CHECK_ERROR(ret);

    std::string cacheData {originData.begin(), originData.end()};
    try {
        Json jsonCache = Json::parse(cacheData);

        mPropCache = jsonCache.get<std::map<std::string, std::string>>();
    } catch(const std::exception& ex) {
        Log::E(Log::TAG, "Failed to load local data from: %s.\nex=%s", dataFilePath.c_str(), ex.what());
        return ErrCode::JsonParseException;
    }

    Log::I(Log::TAG, "Success to load local data from: %s.", dataFilePath.c_str());
    return 0;
}

int RemoteStorageManager::saveLocalData()
{
    auto config = SAFE_GET_PTR(mConfig);
    auto dataFilePath = elastos::filesystem::path(config->mUserDataDir) / DataFileName;

    std::string cacheData;
    try {
        Json jsonCache = Json(mPropCache);
        cacheData = jsonCache.dump();
    } catch(const std::exception& ex) {
        Log::E(Log::TAG, "Failed to save local data to: %s.\nex=%s", dataFilePath.c_str(), ex.what());
        return ErrCode::JsonParseException;
    }

    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);
    std::vector<uint8_t> originData {cacheData.begin(), cacheData.end()};
    int ret = sectyMgr->saveCryptoFile(dataFilePath.string(), originData);
    CHECK_ERROR(ret);

    Log::D(Log::TAG, "Save local data to: %s, data: %s", dataFilePath.c_str(), cacheData.c_str());

    return 0;
}

int RemoteStorageManager::packUserSegment(const std::shared_ptr<UserInfo> userInfo,
                                          const std::string& propKey,
                                          std::string& segment)
{
    if(propKey == PropKey::PublicKey) {
        int ret = userInfo->getHumanInfo(HumanInfo::Item::ChainPubKey, segment);
    } else if(propKey == PropKey::CarrierInfo) {
        std::string currDevId;
        int ret = Platform::GetCurrentDevId(currDevId);
        CHECK_ERROR(ret);
        HumanInfo::CarrierInfo carrierInfo;
        ret = userInfo->getCarrierInfoByDevId(currDevId, carrierInfo);
        CHECK_ERROR(ret);

        ret = HumanInfo::SerializeCarrierInfo(carrierInfo, segment);
        CHECK_ERROR(ret);
    } else if(propKey == PropKey::DetailKey) {
        int ret = userInfo->serializeDetails(segment);
        CHECK_ERROR(ret);
    } else if(propKey == PropKey::IdentifyKey) {
        int ret = userInfo->IdentifyCode::serialize(segment);
        CHECK_ERROR(ret);
    } else {
        CHECK_ERROR(ErrCode::InvalidArgument);
    }

    return 0;
}

int RemoteStorageManager::packFriendSegment(const std::vector<std::shared_ptr<FriendInfo>>& friendInfoList,
                                            const std::string& friendCode,
                                            std::string& segment)
{
    std::shared_ptr<FriendInfo> friendInfo;
    for(auto& it: friendInfoList) {
        if(it->contains(friendCode) == true) {
            friendInfo = it;
            break;
        }
    }
    if(friendInfo == nullptr) {
        CHECK_ERROR(ErrCode::NotFoundError);
    }

    std::string humanCode;
    int ret = friendInfo->getHumanCode(humanCode);
    CHECK_ERROR(ret);

    Json jsonInfo = Json::object();
    jsonInfo[JsonKey::FriendCode] = humanCode;
    jsonInfo[JsonKey::Status] = friendInfo->getHumanStatus();
    jsonInfo[JsonKey::UpdateTime] = DateTime::CurrentMS();
    segment = jsonInfo.dump();

    return 0;
}

int RemoteStorageManager::unpackUserData(const std::string& data,
                                         std::shared_ptr<UserInfo>& userInfo)
{
    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);

    std::string originData;
    int ret = sectyMgr->decryptString(SecurityManager::DefaultCryptoAlgorithm,
                                      data, originData);
    CHECK_ERROR(ret);

    userInfo = std::make_shared<UserInfo>();
    try {
        ret = userInfo->deserialize(originData);
        CHECK_ERROR(ret);
    } catch(const std::exception& ex) {
        Log::E(Log::TAG, "%s Failed to deserialize user data.\nex=%s", FORMAT_METHOD, ex.what());
        return ErrCode::JsonParseException;
    }
    Log::I(Log::TAG, "%s Success to deserialize user data.", FORMAT_METHOD);

    return 0;
}

int RemoteStorageManager::unpackFriendData(const std::string& data,
                                           std::vector<std::shared_ptr<FriendInfo>>& friendInfoList)
{
    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);

    std::string originData;
    int ret = sectyMgr->decryptString(SecurityManager::DefaultCryptoAlgorithm,
                                      data, originData);
    CHECK_ERROR(ret);

    friendInfoList.clear();
    try {
        Json jsonFriend = Json::parse(originData);
        for(const auto& it: jsonFriend) {
            auto info = std::make_shared<FriendInfo>();
            ret = info->deserialize(it);
            CHECK_ERROR(ret);
            friendInfoList.push_back(info);
        }
    } catch(const std::exception& ex) {
        Log::E(Log::TAG, "%s Failed to deserialize friend data.\nex=%s", FORMAT_METHOD, ex.what());
        return ErrCode::JsonParseException;
    }
    Log::I(Log::TAG, "%s Success to deserialize friend data.", FORMAT_METHOD);

    return 0;
}

} // namespace elastos
