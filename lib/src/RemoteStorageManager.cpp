#include <RemoteStorageManager.hpp>

#include <sstream>
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

int RemoteStorageManager::ensureRemoteStorageHash()
{
    Log::I(Log::TAG, "%s ensure oss auth hash...", FORMAT_METHOD);
    std::shared_ptr<RemoteStorageClient> client;
    int ret = RemoteStorageManager::getClient(ClientType::Oss, client);
    CHECK_ERROR(ret);

    auto hash = client->getAuthHash();
    if(hash.empty() == false) {
        Log::I(Log::TAG, "%s oss auth hash has been set, use it directly.", FORMAT_METHOD);
        return 0;
    }

    // if auth hash is empty, try to get it from did chain.
    Log::W(Log::TAG, "%s search oss auth hash from did chain ...", FORMAT_METHOD);
    std::vector<std::string> savedPropKeyList { PropKey::IdentifyKey };
    std::vector<std::string> totalPropFileList;
    std::shared_ptr<UserInfo> remoteUserInfo = std::make_shared<UserInfo>();
    std::vector <std::shared_ptr<FriendInfo>> remoteFriendInfoList;
    std::shared_ptr<std::iostream> remoteCarrierData;
    ret = downloadData(ClientType::DidChain,
                       savedPropKeyList, totalPropFileList,
                       remoteUserInfo, remoteFriendInfoList, remoteCarrierData);
    CHECK_ERROR(ret);

    std::string authHash;
    ret = remoteUserInfo->getIdentifyCode(UserInfo::Type::RemoteStorage, authHash);
    if(ret == ErrCode::NotFoundError) { // ignore not found
        ret = 0;
    }
    CHECK_ERROR(ret);
    if(authHash.empty() == false) {
        Log::I(Log::TAG, "%s oss auth hash is updated from did chain.", FORMAT_METHOD);
        client->setAuthHash(authHash);
        return 0;
    }

    // if auth hash on didchain is empty, try to generate it from default elaphant oss.
    Log::W(Log::TAG, "%s generate oss auth hash from defalut elaphant oss ...", FORMAT_METHOD);
    ret = client->genAuthHash();
    if(ret == 0) {
        Log::I(Log::TAG, "%s oss auth hash is updated from default settings.", FORMAT_METHOD);
        return 0;
    }

    CHECK_ERROR(ErrCode::NotExpectedReachedError);
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
    Log::I(Log::TAG, "%s %s:%s", FORMAT_METHOD, did.c_str(), key);
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

int RemoteStorageManager::uploadData(const std::vector<ClientType>& toClientList,
                                     const std::shared_ptr<UserInfo> userInfo,
                                     const std::vector<std::shared_ptr<FriendInfo>>& friendInfoList,
                                     const std::shared_ptr<std::fstream> carrierData)
{
    Log::I(Log::TAG, FORMAT_METHOD);

    int ret = ensureRemoteStorageHash();
    CHECK_ERROR(ret);

    auto config = SAFE_GET_PTR(mConfig);
    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);
    std::string did;
    ret = sectyMgr->getDid(did);
    CHECK_ERROR(ret);

    std::multimap<std::string, std::string> changedPropMap;
    std::map<std::string, std::shared_ptr<std::iostream>> totalPropMap;
    {
        std::lock_guard<std::recursive_mutex> lock(mMutex);
        for(const auto& [did, propKey]: mPropCache) {
            std::string segment;
            std::string path;
            if (propKey == PropKey::FriendKey) {
                ret = packFriendSegment(did, friendInfoList, segment);
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
            totalPropMap[did + "/" + path] = std::make_shared<std::fstream>(config->mUserDataDir + "/" + path);

            if (propKey == PropKey::CarrierInfo) {
                if(carrierData.get() == nullptr) {
                    CHECK_ERROR(ErrCode::InvalidArgument);
                }

                std::string currDevId;
                int ret = Platform::GetCurrentDevId(currDevId);
                CHECK_ERROR(ret);
                path = currDevId + "/carrier.data";
                totalPropMap[did + "/" + path] = carrierData;
            }
        }
    }

    for (const auto& type : toClientList){
        auto it = mRemoteStorageClientMap.find(type);
        if(it == mRemoteStorageClientMap.end()) {
            CHECK_ERROR(ErrCode::InvalidArgument);
        }
        auto client = it->second;

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

int RemoteStorageManager::downloadData(const std::vector<ClientType>& fromClientList,
                                       std::shared_ptr<UserInfo>& userInfo,
                                       std::vector<std::shared_ptr<FriendInfo>>& friendInfoList,
                                       std::shared_ptr<std::fstream>& carrierData)
{
    Log::I(Log::TAG, FORMAT_METHOD);

    int ret = ensureRemoteStorageHash();
    CHECK_ERROR(ret);

    for (const auto& type : fromClientList) {
        std::shared_ptr<UserInfo> remoteUserInfo = std::make_shared<UserInfo>();
        std::vector <std::shared_ptr<FriendInfo>> remoteFriendInfoList;
        std::shared_ptr<std::iostream> remoteCarrierData;
        if(carrierData != nullptr) {
            remoteCarrierData = std::make_shared<std::stringstream>();
        }

        int ret = downloadData(type, remoteUserInfo, remoteFriendInfoList, remoteCarrierData);
        if (ErrCode::AdditivityIndex < ret && ret < 0) {
            ret += ErrCode::RemoteStorageClientErrorIndex;
        }
        CHECK_ERROR(ret);

        ret = userInfo->mergeIdentifyCode(*remoteUserInfo);
        if(GET_ERRCODE(ret) == ErrCode::IgnoreMergeOldInfo) {
            ret = 0;
        }
        ret = userInfo->mergeHumanInfo(*remoteUserInfo, HumanInfo::Status::Invalid);
        if(GET_ERRCODE(ret) == ErrCode::IgnoreMergeOldInfo) {
            ret = 0;
        }
        CHECK_ERROR(ret);
        if(carrierData != nullptr
        && remoteCarrierData != nullptr) {
            (*carrierData) << remoteCarrierData->rdbuf();
            auto dataLen = remoteCarrierData->tellg();
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
    if(ret == ErrCode::FileNotExistsError) {
        return ret;
    }
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

int RemoteStorageManager::downloadData(ClientType fromClient,
                                       std::shared_ptr<UserInfo>& userInfo,
                                       std::vector<std::shared_ptr<FriendInfo>>& friendInfoList,
                                       std::shared_ptr<std::iostream>& carrierData)
{
    Log::I(Log::TAG, "%s %d", FORMAT_METHOD, __LINE__);
    auto it = mRemoteStorageClientMap.find(fromClient);
    if(it == mRemoteStorageClientMap.end()) {
        CHECK_ERROR(ErrCode::InvalidArgument);
    }
    auto client = it->second;

    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);
    std::string did;
    int ret = sectyMgr->getDid(did);
    CHECK_ERROR(ret);

    std::string currDevId;
    ret = Platform::GetCurrentDevId(currDevId);
    CHECK_ERROR(ret);
    std::string carrierDataPath = currDevId + "/carrier.data";

    std::vector<std::string> savedPropKeyList {
            PropKey::PublicKey,
            PropKey::CarrierInfo,
            PropKey::DetailKey,
            PropKey::IdentifyKey,
            PropKey::FriendKey,
    };
    std::vector<std::string> totalPropFileList {
            did + "/" + UserManager::DataFileName,
            did + "/" + FriendManager::DataFileName,
    };
    if(carrierData.get() != nullptr) {
        totalPropFileList.push_back(did + "/" + carrierDataPath);
    }

//    Log::I(Log::TAG, "%s %d", FORMAT_METHOD, __LINE__);
    ret = downloadData(fromClient, savedPropKeyList, totalPropFileList, userInfo, friendInfoList, carrierData);
    CHECK_ERROR(ret);
//    Log::I(Log::TAG, "%s %d", FORMAT_METHOD, __LINE__);

    return 0;
}

int RemoteStorageManager::downloadData(ClientType fromClient,
                                       const std::vector<std::string>& propKeyList,
                                       const std::vector<std::string>& propFileList,
                                       std::shared_ptr<UserInfo>& userInfo,
                                       std::vector<std::shared_ptr<FriendInfo>>& friendInfoList,
                                       std::shared_ptr<std::iostream>& carrierData)
{
    Log::I(Log::TAG, "%s %d", FORMAT_METHOD, __LINE__);
    auto it = mRemoteStorageClientMap.find(fromClient);
    if(it == mRemoteStorageClientMap.end()) {
        CHECK_ERROR(ErrCode::InvalidArgument);
    }
    auto client = it->second;

    std::string currDevId;
    int ret = Platform::GetCurrentDevId(currDevId);
    CHECK_ERROR(ret);
    std::string carrierDataPath = currDevId + "/carrier.data";

    std::multimap<std::string, std::string> savedPropMap;
    for(const auto& key: propKeyList) {
        savedPropMap.emplace(key, "");
    }
    std::map<std::string, std::shared_ptr<std::iostream>> totalPropMap;
    for(const auto& file: propFileList) {
        totalPropMap.emplace(file, nullptr);
    }
//    Log::I(Log::TAG, "%s %d", FORMAT_METHOD, __LINE__);

    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);
    std::string did;
    ret = sectyMgr->getDid(did);
    CHECK_ERROR(ret);

    ret = client->downloadProperties(did, savedPropMap, totalPropMap);
    if(ErrCode::AdditivityIndex < ret && ret < 0) {
        ret += ErrCode::RemoteStorageClientErrorIndex;
    }
    CHECK_ERROR(ret);

    for(const auto& [propKey, segment]: savedPropMap) {
        if(segment.empty() == true) {
            continue;
        }
        if (propKey == PropKey::FriendKey) {
            int ret = unpackFriendSegment(segment, friendInfoList);
            CHECK_ERROR(ret);
        } else if (propKey == PropKey::PublicKey
        || propKey == PropKey::DetailKey
        || propKey == PropKey::IdentifyKey
        || propKey == PropKey::CarrierInfo){
            auto newUserInfo = std::make_shared<UserInfo>();
            int ret = unpackUserSegment(segment, propKey, newUserInfo);
            if(GET_ERRCODE(ret) == ErrCode::IgnoreMergeOldInfo) {
                Log::V(Log::TAG, "%s Ignore to sync old %s: %s", FORMAT_METHOD, propKey.c_str(), segment.c_str());
                continue;
            }
            CHECK_ERROR(ret);
            ret = mergeUserInfo(newUserInfo, userInfo);
            CHECK_ERROR(ret);
        } else {
            CHECK_ERROR(ErrCode::NotExpectedReachedError);
        }
    }

    carrierData.reset();
    for(const auto& [path, content]: totalPropMap) {
        if(content == nullptr) {
            Log::W(Log::TAG, "%s Ignore to process empty content from: %s", FORMAT_METHOD, path.c_str());
            continue;
        }

        if(path == (did + "/" + UserManager::DataFileName)) {
            std::string data;
            (*content) >> data;
            auto newUserInfo = std::make_shared<UserInfo>();
            int ret = unpackUserData(data, newUserInfo);
            CHECK_ERROR(ret);
            ret = mergeUserInfo(newUserInfo, userInfo);
            CHECK_ERROR(ret);
        } else if(path == (did + "/" + FriendManager::DataFileName)) {
            std::string data;
            (*content) >> data;
            int ret = unpackFriendData(data, friendInfoList);
            CHECK_ERROR(ret);
        } else if(path == (did + "/" + carrierDataPath)) {
            carrierData = content;
        } else {
            Log::E(Log::TAG, "%s Failed to process file path: ", path.c_str());
            CHECK_ERROR(ErrCode::NotExpectedReachedError);
        }
    }

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

int RemoteStorageManager::unpackUserSegment(const std::string& segment,
                                            const std::string& propKey,
                                            std::shared_ptr<UserInfo>& userInfo)
{
    Log::I(Log::TAG, "%s %d %s:%s", FORMAT_METHOD, __LINE__, propKey.c_str(), segment.c_str());

    if(propKey == PropKey::PublicKey) {
        int ret = userInfo->HumanInfo::setHumanInfo(HumanInfo::Item::ChainPubKey, segment);
    } else if(propKey == PropKey::CarrierInfo) {
        HumanInfo::CarrierInfo carrierInfo;
        int ret = HumanInfo::DeserializeCarrierInfo(segment, carrierInfo);
        CHECK_ERROR(ret);

        ret = userInfo->HumanInfo::addCarrierInfo(carrierInfo, HumanInfo::Status::WaitForAccept);
        if(GET_ERRCODE(ret) == ErrCode::IgnoreMergeOldInfo) {
            return 0;
        }
        CHECK_ERROR(ret);
//        Log::I(Log::TAG, "DidChnDataListener::processCarrierInfoChanged() Success to sync CarrierId: %s", it.c_str());
    } else if(propKey == PropKey::DetailKey) {
        int ret = userInfo->deserializeDetails(segment);
        if(GET_ERRCODE(ret) == ErrCode::IgnoreMergeOldInfo) {
            return 0;
        }
        CHECK_ERROR(ret);
    } else if(propKey == PropKey::IdentifyKey) {
        IdentifyCode identifyCode;
        int ret = identifyCode.deserialize(segment);
        CHECK_ERROR(ret);

        ret = userInfo->IdentifyCode::mergeIdentifyCode(identifyCode);
        if(GET_ERRCODE(ret) == ErrCode::IgnoreMergeOldInfo) {
            return 0;
        }
        CHECK_ERROR(ret);
    } else {
        CHECK_ERROR(ErrCode::InvalidArgument);
    }

    return 0;
}

int RemoteStorageManager::packFriendSegment(const std::string& friendCode,
                                            const std::vector<std::shared_ptr<FriendInfo>>& friendInfoList,
                                            std::string& segment)
{
    throw std::runtime_error(std::string(FORMAT_METHOD) + " Unimplemented!!!");
//    std::shared_ptr<FriendInfo> friendInfo;
//    for(auto& it: friendInfoList) {
//        if(it->contains(friendCode) == true) {
//            friendInfo = it;
//            break;
//        }
//    }
//    if(friendInfo == nullptr) {
//        CHECK_ERROR(ErrCode::NotFoundError);
//    }
//
//    std::string humanCode;
//    int ret = friendInfo->getHumanCode(humanCode);
//    CHECK_ERROR(ret);
//
//    Json jsonInfo = Json::object();
//    jsonInfo[JsonKey::FriendCode] = humanCode;
//    jsonInfo[JsonKey::Status] = friendInfo->getHumanStatus();
//    jsonInfo[JsonKey::UpdateTime] = DateTime::CurrentMS();
//    segment = jsonInfo.dump();
//
//    return 0;
}

int RemoteStorageManager::unpackFriendSegment(const std::string& segment,
                                              std::vector<std::shared_ptr<FriendInfo>>& friendInfoList)
{
    throw std::runtime_error(std::string(FORMAT_METHOD) + " Unimplemented!!!");
//    Json jsonInfo = Json::parse(segment);
//    std::string friendCode = jsonInfo[JsonKey::FriendCode];
//    HumanInfo::Status status = jsonInfo[JsonKey::Status];
//    int64_t updateTime = jsonInfo[JsonKey::UpdateTime];

//    if(status == HumanInfo::Status::Removed) {
//        return 0;
//    }

//    int ret = friendMgr->tryAddFriend(it, "", false);
//    CHECK_ERROR(ret);

//        Log::I(Log::TAG, "DidChnDataListener::processFriendKeyChanged() Add friend did: %s.", it.c_str());

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

int RemoteStorageManager::mergeUserInfo(const std::shared_ptr<UserInfo>& from,
                                        const std::shared_ptr<UserInfo>& to) {
    auto status = to->getHumanStatus();

    int ret = to->HumanInfo::mergeHumanInfo(*from, status);
    if (GET_ERRCODE(ret) == 0) {
        Log::I(Log::TAG, "%s Success to merge human info.", FORMAT_METHOD);
    } else if(GET_ERRCODE(ret) == ErrCode::IgnoreMergeOldInfo) {
        Log::I(Log::TAG, "%s Ignore to merge old human info.", FORMAT_METHOD);
        ret = 0;
    }
    CHECK_ERROR(ret);

    ret = to->mergeIdentifyCode(*from);
    if (GET_ERRCODE(ret) == 0) {
        Log::I(Log::TAG, "%s Success to merge indentify info.", FORMAT_METHOD);
    } else if(GET_ERRCODE(ret) == ErrCode::IgnoreMergeOldInfo) {
        Log::I(Log::TAG, "%s Ignore to merge old indentify code.", FORMAT_METHOD);
        ret = 0;
    }
    CHECK_ERROR(ret);

    return 0;
}

} // namespace elastos
