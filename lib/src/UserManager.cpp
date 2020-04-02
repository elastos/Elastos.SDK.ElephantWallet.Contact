//
//  UserManager.cpp
//
//  Created by mengxk on 19/03/16.
//  Copyright © 2016 mengxk. All rights reserved.
//

#include <UserManager.hpp>

//#include "BlkChnClient.hpp"
//#include "DidChnClient.hpp"
#include <CompatibleFileSystem.hpp>
#include <DateTime.hpp>
#include <Log.hpp>
#include <MD5.hpp>
#include <MessageManager.hpp>
#include <SafePtr.hpp>
#include <Platform.hpp>
#include <iostream>
#include <sstream>
#include <Json.hpp>
#include <RemoteStorageManager.hpp>


namespace elastos {

/***********************************************/
/***** static variables initialize *************/
/***********************************************/


/***********************************************/
/***** static function implement ***************/
/***********************************************/

/***********************************************/
/***** class public function implement  ********/
/***********************************************/
UserManager::UserManager(std::weak_ptr<SecurityManager> sectyMgr)
    : mSecurityManager(sectyMgr)
    , mMessageManager()
    , mRemoteStorageManager()
    , mConfig()
    , mUserListener()
    , mUserInfo()
{
}

UserManager::~UserManager()
{
}

void UserManager::setUserListener(std::shared_ptr<UserListener> listener)
{
    mUserListener = listener;
}

void UserManager::setConfig(std::weak_ptr<Config> config,
                            std::weak_ptr<RemoteStorageManager> rsMgr,
                            std::weak_ptr<MessageManager> msgMgr)
{
    mConfig = config;
    mRemoteStorageManager = rsMgr;
    mMessageManager = msgMgr;
}

int UserManager::loadLocalData()
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

    std::string userData {originData.begin(), originData.end()};
    try {
        ret = mUserInfo->deserialize(userData);
        CHECK_ERROR(ret);
    } catch(const std::exception& ex) {
        Log::E(Log::TAG, "Failed to load local data from: %s.\nex=%s", dataFilePath.c_str(), ex.what());
        return ErrCode::JsonParseException;
    }

    Log::I(Log::TAG, "Success to load local data from: %s.", dataFilePath.c_str());
    return 0;
}

int UserManager::saveLocalData()
{
    std::string userData;
    int ret = serialize(userData);
    CHECK_ERROR(ret);

    auto config = SAFE_GET_PTR(mConfig);
    auto dataFilePath = elastos::filesystem::path(config->mUserDataDir) / DataFileName;

    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);
    std::vector<uint8_t> originData {userData.begin(), userData.end()};
    ret = sectyMgr->saveCryptoFile(dataFilePath.string(), originData);
    CHECK_ERROR(ret);

    Log::D(Log::TAG, "Save local data to: %s, data: %s", dataFilePath.c_str(), userData.c_str());

    return 0;
}

int UserManager::serialize(std::string& value) const
{
    value = "";

    std::string userData;
    int ret = mUserInfo->serialize(userData);
    CHECK_ERROR(ret);

    value = userData;
    return 0;
}

int UserManager::restoreUserInfo()
{
    mUserInfo = std::make_shared<UserInfo>(weak_from_this());

    int ret = loadLocalData();
    if(ret == 0) {
        Log::I(Log::TAG, "UserManager::restoreUserInfo() Success to recover user from local.");
    } else if(ret == ErrCode::FileNotExistsError) {
        auto sectyMgr = SAFE_GET_PTR(mSecurityManager);
        std::string pubKey;
        int ret = sectyMgr->getPublicKey(pubKey);
        CHECK_ERROR(ret);
        ret = mUserInfo->setHumanInfo(UserInfo::Item::ChainPubKey, pubKey);
        CHECK_ERROR(ret);

        Log::I(Log::TAG, "UserManager::restoreUserInfo() Success to create user.");
    }

    ret = mUserInfo->setHumanStatus(HumanInfo::Status::Online, HumanInfo::Status::Offline);
    CHECK_ERROR(ret);

    return 0;
}

int UserManager::ensureUserCarrierInfo()
{
    Log::V(Log::TAG, FORMAT_METHOD);

    auto msgMgr = SAFE_GET_PTR(mMessageManager);
    std::weak_ptr<MessageChannelStrategy> weakChCarrier;
    int ret = msgMgr->getChannel(MessageManager::ChannelType::Carrier, weakChCarrier);
    CHECK_ERROR(ret);
    auto chCarrier = SAFE_GET_PTR(weakChCarrier);
    std::string carrierAddr;
    ret = chCarrier->getAddress(carrierAddr);
    CHECK_ERROR(ret);

    std::string currDevId;
    ret = Platform::GetCurrentDevId(currDevId);
    CHECK_ERROR(ret);

    UserInfo::CarrierInfo info;
    ret = mUserInfo->getCarrierInfoByDevId(currDevId, info);
    if(ret == 0 && info.mUsrAddr == carrierAddr) {
        return 0;
    }

    std::string currDevName;
    ret = Platform::GetCurrentDevName(currDevName);
    CHECK_ERROR(ret);

    HumanInfo::CarrierInfo carrierInfo;
    carrierInfo.mUsrAddr = carrierAddr;
    carrierInfo.mDevInfo = {currDevId, currDevName};
    carrierInfo.mUpdateTime = DateTime::CurrentMS();
    ret = mUserInfo->addCarrierInfo(carrierInfo, UserInfo::Status::Offline);
    CHECK_ERROR(ret);

    std::string carrierInfoStr;
    ret = HumanInfo::SerializeCarrierInfo(carrierInfo, carrierInfoStr);
    CHECK_ERROR(ret);

    std::string pubKey;
    ret = mUserInfo->getHumanInfo(HumanInfo::Item::ChainPubKey, pubKey);
    CHECK_ERROR(ret);

//    auto dcClient = DidChnClient::GetInstance();
//    ret = dcClient->cacheDidProp(DidChnClient::NamePublicKey, pubKey);
//    CHECK_ERROR(ret);
//    ret = dcClient->cacheDidProp(DidChnClient::NameCarrierKey, carrierInfoStr);
//    CHECK_ERROR(ret);

    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);
    auto rsMgr = SAFE_GET_PTR(mRemoteStorageManager);
    std::string did;
    ret = sectyMgr->getDid(did);
    CHECK_ERROR(ret);
    ret = rsMgr->cacheProperty(did, RemoteStorageManager::PropKey::PublicKey);
    CHECK_ERROR(ret);

    std::string devId;
    ret = Platform::GetCurrentDevId(devId);
    CHECK_ERROR(ret);
    ret = rsMgr->cacheProperty(did, RemoteStorageManager::PropKey::CarrierInfo);
    CHECK_ERROR(ret);

    Log::V(Log::TAG, "%s new carrier info: %s", FORMAT_METHOD, carrierInfoStr.c_str());

    return 0;
}


int UserManager::getUserInfo(std::shared_ptr<UserInfo>& userInfo)
{
    if(mUserInfo.get() == nullptr) {
        return ErrCode::NotReadyError;
    }

    userInfo = mUserInfo;

    return 0;
}

int UserManager::setUserInfo(UserInfo::Item item, const std::string& value)
{
    if(mUserInfo.get() == nullptr) {
        return ErrCode::NotReadyError;
    }

    int ret = mUserInfo->setHumanInfo(item, value);
    CHECK_ERROR(ret);

    std::string userDetails;
    ret = mUserInfo->serializeDetails(userDetails);
    if(ret == 0) { // not changed
        return ret;
    }
    CHECK_ERROR(ret);

//    auto dcClient = DidChnClient::GetInstance();
//    ret = dcClient->cacheDidProp(DidChnClient::NameDetailKey, userDetails);
//    CHECK_ERROR(ret);

    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);
    auto rsMgr = SAFE_GET_PTR(mRemoteStorageManager);
    std::string did;
    ret = sectyMgr->getDid(did);
    CHECK_ERROR(ret);
    ret = rsMgr->cacheProperty(did, RemoteStorageManager::PropKey::DetailKey);
    CHECK_ERROR(ret);

    auto msgMgr = SAFE_GET_PTR(mMessageManager);
    ret = msgMgr->broadcastDesc(MessageManager::ChannelType::Carrier);
    CHECK_ERROR(ret);

    return 0;
}

int UserManager::setIdentifyCode(elastos::IdentifyCode::Type type, const std::string &value)
{
    Log::V(Log::TAG, "%s %d:%s", FORMAT_METHOD, type, value.c_str());
    if(mUserInfo.get() == nullptr) {
        CHECK_ERROR(ErrCode::NotReadyError);
    }

    int ret = mUserInfo->setIdentifyCode(type, value);
    Log::V(Log::TAG, "%s ret=%d", FORMAT_METHOD, ret);
    if(ret == 0) { // not changed
        return ret;
    }
    CHECK_ERROR(ret);

    Log::V(Log::TAG, "%s %d", FORMAT_METHOD, __LINE__);

    std::string userIdentify;
    ret = mUserInfo->IdentifyCode::serialize(userIdentify);
    CHECK_ERROR(ret);

//    auto dcClient = DidChnClient::GetInstance();
//    ret = dcClient->cacheDidProp(DidChnClient::NameIdentifyKey, userIdentify);
//    CHECK_ERROR(ret);

    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);
    auto rsMgr = SAFE_GET_PTR(mRemoteStorageManager);
    std::string did;
    ret = sectyMgr->getDid(did);
    CHECK_ERROR(ret);
    Log::V(Log::TAG, "%s %d", FORMAT_METHOD, __LINE__);
    ret = rsMgr->cacheProperty(did, RemoteStorageManager::PropKey::IdentifyKey);
    CHECK_ERROR(ret);

//    auto msgMgr = SAFE_GET_PTR(mMessageManager);
//    ret = msgMgr->broadcastDesc(MessageManager::ChannelType::Carrier);
//    CHECK_ERROR(ret);

    return 0;
}

int UserManager::setWalletAddress(const std::string& name, const std::string& value)
{
    if(mUserInfo.get() == nullptr) {
        return ErrCode::NotReadyError;
    }

    int ret = mUserInfo->setWalletAddress(name, value);
    CHECK_ERROR(ret);

    std::string userDetails;
    ret = mUserInfo->serializeDetails(userDetails);
    CHECK_ERROR(ret);

//    auto dcClient = DidChnClient::GetInstance();
//    ret = dcClient->cacheDidProp(DidChnClient::NameDetailKey, userDetails);
//    CHECK_ERROR(ret);

    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);
    auto rsMgr = SAFE_GET_PTR(mRemoteStorageManager);
    std::string did;
    ret = sectyMgr->getDid(did);
    CHECK_ERROR(ret);
    ret = rsMgr->cacheProperty(did, RemoteStorageManager::PropKey::DetailKey);
    CHECK_ERROR(ret);

    auto msgMgr = SAFE_GET_PTR(mMessageManager);
    ret = msgMgr->broadcastDesc(MessageManager::ChannelType::Carrier);
    CHECK_ERROR(ret);

    return 0;
}

bool UserManager::contains(const std::string& userCode)
{
    return mUserInfo->contains(userCode);
}

bool UserManager::contains(const std::shared_ptr<HumanInfo>& userInfo)
{
    return mUserInfo->contains(userInfo);
}

int UserManager::saveAvatarFile(const std::string& filepath, std::string& md5)
{
    auto avatarFrom = elastos::filesystem::path(filepath);
    bool ret = elastos::filesystem::exists(avatarFrom);
    if(ret == false) {
        return ErrCode::FileNotExistsError;
    }
    md5 = elastos::MD5::Get(avatarFrom);

    auto config = SAFE_GET_PTR(mConfig);
    auto avatarDir = elastos::filesystem::path(config->mUserDataDir) / "avatar";

    std::error_code stdErrCode;
    ret = elastos::filesystem::create_directories(avatarDir, stdErrCode);
    if(ret == false
    || stdErrCode.value() != 0) {
        return ErrCode::StdSystemErrorIndex - stdErrCode.value();
    }

    auto avatarTo = avatarDir / md5;
    elastos::filesystem::copy(avatarFrom, avatarTo, stdErrCode);
    if(stdErrCode.value() != 0) {
        return ErrCode::StdSystemErrorIndex - stdErrCode.value();
    }

    Log::D(Log::TAG, "Save avatar file to: %s", avatarTo.c_str());

    return 0;
}

int UserManager::getAvatarFile(const std::string& md5, std::string& filepath)
{
    auto config = SAFE_GET_PTR(mConfig);

    auto avatarDir = elastos::filesystem::path(config->mUserDataDir) / "avatar";
    auto avatarFile = avatarDir / md5;

    bool ret = elastos::filesystem::exists(avatarFile);
    if(ret == false) {
        return ErrCode::FileNotExistsError;
    }

    filepath = avatarFile.string();

    return 0;
}

//int UserManager::monitorDidChainData()
//{
//    class UserDataMonitor final : public DidChnClient::MonitorCallback {
//    public:
//        explicit UserDataMonitor(std::weak_ptr<UserManager> friendManager)
//        : mUserManager(friendManager) {
//        }
//        virtual ~UserDataMonitor() = default;
//
//        virtual void onError(const std::string& did, const std::string& key,
//                             int errcode) override {
//            Log::I(Log::TAG, "%s did=%s, key=%s errcode=%d", FORMAT_METHOD, did.c_str(), key.c_str(), errcode);
//        }
//
//        virtual int onChanged(const std::string& did, const std::string& key,
//                               const std::vector<std::string>& didProps) override {
//            Log::I(Log::TAG, "%s did=%s, key=%s", FORMAT_METHOD, did.c_str(), key.c_str());
//
//            auto userMgr = SAFE_GET_PTR(mUserManager);
//
//            std::shared_ptr<UserInfo> userInfo;
//            int ret = userMgr->getUserInfo(userInfo);
//            CHECK_ERROR(ret);
//
//            for(const auto& it: didProps) {
//                if(key == DidChnClient::NameCarrierKey) {
//                    HumanInfo::CarrierInfo carrierInfo;
//                    ret = HumanInfo::deserialize(it, carrierInfo);
//                    CHECK_ERROR(ret);
//
//                    ret = userInfo->addCarrierInfo(carrierInfo, HumanInfo::Status::WaitForAccept);
//                    if(ret == ErrCode::IgnoreMergeOldInfo) {
//                        Log::V(Log::TAG, "MessageManager::monitorDidChainCarrierID() Ignore to sync CarrierId: %s", it.c_str());
//                        continue;
//                    }
//                    CHECK_ERROR(ret);
//                } else if(key == DidChnClient::NameIdentifyKey) {
//                    int ret = userMgr->mergeIdentifyCodeFromJsonArray(it);
//                    CHECK_ERROR(ret);
//                }
//            }
//
//            return 0;
//        }
//    private:
//        std::weak_ptr<UserManager> mUserManager;
//    };
//    auto callback = std::make_shared<UserDataMonitor>(shared_from_this());
//
//    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);
//    std::string did;
//    int ret = sectyMgr->getDid(did);
//    CHECK_ERROR(ret);
//
//    auto dcClient = DidChnClient::GetInstance();
//    ret = dcClient->appendMoniter(did, callback, false);
//    CHECK_ERROR(ret);
//
//    return 0;
//}

//int UserManager::monitorDidChainIdentifyCode()
//{
//    auto callback = [=](int errcode,
//                        const std::string& keyPath,
//                        const std::string& result) {
//        Log::D(Log::TAG, "UserManager::monitorDidChainIdentifyCode() ecode=%d, path=%s, result=%s", errcode, keyPath.c_str(), result.c_str());
//        CHECK_RETVAL(errcode);
//
//        int ret = mergeIdentifyCodeFromJsonArray(result);
//        CHECK_RETVAL(ret);
//
//        return;
//    };
//
//    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);
//    std::string did;
//    int ret = sectyMgr->getDid(did);
//    CHECK_ERROR(ret);
//
//    auto bcClient = BlkChnClient::GetInstance();
//
//    std::string keyPath;
//    ret = bcClient->getDidPropHistoryPath(did, "IdentifyCode", keyPath);
//    CHECK_ERROR(ret);
//
//    Log::I(Log::TAG, "UserManager::monitorDidChainIdentifyCode() keyPath=%s", keyPath.c_str());
//    ret = bcClient->appendMoniter(keyPath, callback);
//    CHECK_ERROR(ret);
//
//    return 0;
//}

// int UserManager::uploadUserInfo()
// {
//     auto bcClient = BlkChnClient::GetInstance();

//     int ret = bcClient->uploadHumanInfo(mUserInfo);
//     CHECK_ERROR(ret);

//     return 0;
// }

int UserManager::setupMultiDevChannels()
{
    auto msgMgr = SAFE_GET_PTR(mMessageManager);

    std::vector<HumanInfo::CarrierInfo> carrierInfoArray;
    int ret = mUserInfo->getAllCarrierInfo(carrierInfoArray);
    CHECK_ERROR(ret);

    for(const auto& carrierInfo: carrierInfoArray) {
        int ret = msgMgr->requestFriend(carrierInfo.mUsrAddr,
                                        MessageManager::ChannelType::Carrier,
                                        "", true);
        Log::I(Log::TAG, "UserManager::setupMultiDevChannels() add %s", carrierInfo.mUsrAddr.c_str());
        if(ret < 0
        && ret != ErrCode::ChannelFailedFriendSelf
        && ret != ErrCode::ChannelFailedFriendExists) {
            Log::W(Log::TAG, "UserManager::setupMultiDevChannels() Failed to add %s ret=%d", carrierInfo.mUsrAddr.c_str(), ret);
        }
    }

    return 0;
}

//int UserManager::syncDownloadDidChainData()
//{
//    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);
//    std::string did;
//    int ret = sectyMgr->getDid(did);
//    CHECK_ERROR(ret);
//
//    auto dcClient = DidChnClient::GetInstance();
//
//    std::map<std::string, std::vector<std::string>> didProps;
//    ret = dcClient->downloadDidProp(did, false, didProps);
//    CHECK_ERROR(ret);
//
////    std::string keyPath;
////    ret = bcClient->getDidPropHistoryPath(did, "IdentifyCode", keyPath);
////    if (ret < 0) {
////        return ret;
////    }
////
////    std::string result;
////    ret = bcClient->downloadFromDidChn(keyPath, result);
////    CHECK_ERROR(ret);
////
////    ret = mergeIdentifyCodeFromJsonArray(result);
////    CHECK_ERROR(ret);
//
//    return 0;
//}


/***********************************************/
/***** class protected function implement  *****/
/***********************************************/


/***********************************************/
/***** class private function implement  *******/
/***********************************************/
int UserManager::mergeIdentifyCodeFromJsonArray(const std::string& jsonArray)
{
    std::vector<std::string> values;
    Json jsonPropArray = Json::parse(jsonArray);
    for (const auto& it : jsonPropArray) {
        values.push_back(it["value"]);
    }

    for(const auto& it: values) {
        IdentifyCode identifyCode;
        int ret = identifyCode.deserialize(it);
        if (ret < 0) {
            Log::W(Log::TAG, "UserManager::mergeFriendInfoFromJsonArray() Failed to deserialize IdentifyCode: %s.", it.c_str());
            continue;
        }

        ret = mUserInfo->mergeIdentifyCode(identifyCode);
        if (ret < 0) {
            Log::W(Log::TAG, "UserManager::mergeFriendInfoFromJsonArray() Failed to merge IdentifyCode: %s.", it.c_str());
            continue;
        }
    }

    return 0;
}

} // namespace elastos
