//
//  FriendManager.cpp
//
//  Created by mengxk on 19/03/16.
//  Copyright © 2016 mengxk. All rights reserved.
//

#include <FriendManager.hpp>

#include <algorithm>
//#include "BlkChnClient.hpp"
#include "DidChnClient.hpp"
#include "DidChnDataListener.hpp"
#include "ElaChnClient.hpp"
#include <CompatibleFileSystem.hpp>
#include <DateTime.hpp>
#include <Elastos.SDK.Keypair.C/Elastos.Wallet.Utility.h>
#include <Log.hpp>
#include <JsonDefine.hpp>
#include <MessageManager.hpp>
#include <SafePtr.hpp>

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
FriendManager::FriendManager(std::weak_ptr<SecurityManager> sectyMgr)
    : mSecurityManager(sectyMgr)
    , mMessageManager()
    , mConfig()
    , mMutex()
    , mFriendListener()
    , mFriendList()
{
}

FriendManager::~FriendManager()
{
}

bool FriendManager::contains(const std::string& friendCode)
{
    for(const auto& it: mFriendList) {
        bool find = it->contains(friendCode);
        if(find == true) {
            return true;
        }
    }

    return false;
}

bool FriendManager::contains(const std::shared_ptr<HumanInfo>& friendInfo)
{
    for(const auto& it: mFriendList) {
        bool find = it->contains(friendInfo);
        if(find == true) {
            return true;
        }
    }

    return false;
}


void FriendManager::setFriendListener(std::shared_ptr<FriendListener> listener)
{
    mFriendListener = listener;
}

void FriendManager::setConfig(std::weak_ptr<Config> config, std::weak_ptr<MessageManager> msgMgr)
{
    mConfig = config;
    mMessageManager = msgMgr;
}

int FriendManager::loadLocalData()
{
    auto config = SAFE_GET_PTR(mConfig);
    auto dataFilePath = elastos::filesystem::path(config->mUserDataDir) / DataFileName;

    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);
    std::vector<uint8_t> originData;
    int ret = sectyMgr->loadCryptoFile(dataFilePath.string(), originData);
    if(ret < 0) {
        return 0;
    }

    std::vector<std::shared_ptr<FriendInfo>> friendList;
    std::string friendData {originData.begin(), originData.end()};
    try {
        Json jsonFriend = Json::parse(friendData);
        for(const auto& it: jsonFriend) {
            auto info = std::make_shared<FriendInfo>(weak_from_this());
            ret = info->deserialize(it);
            CHECK_ERROR(ret)
            friendList.push_back(info);
        }
    } catch(const std::exception& ex) {
        Log::E(Log::TAG, "Failed to load local data from: %s.\nex=%s", dataFilePath.c_str(), ex.what());
        return ErrCode::JsonParseException;
    }

    mFriendList.swap(friendList);

    return 0;
}

int FriendManager::saveLocalData()
{
    std::string friendData;
    int ret = serialize(friendData);
    CHECK_ERROR(ret)

    auto config = SAFE_GET_PTR(mConfig);
    auto dataFilePath = elastos::filesystem::path(config->mUserDataDir) / DataFileName;

    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);
    std::vector<uint8_t> originData {friendData.begin(), friendData.end()};
    ret = sectyMgr->saveCryptoFile(dataFilePath.string(), originData);
    CHECK_ERROR(ret)

    Log::D(Log::TAG, "Save local data to: %s, data: %s", dataFilePath.c_str(), friendData.c_str());

    return 0;
}

int FriendManager::serialize(std::string& value) const
{
    Json jsonFriend = Json::array();
    for(const auto& it: mFriendList) {
        std::string data;
        int ret = it->serialize(data);
        CHECK_ERROR(ret)
        jsonFriend.push_back(data);
    }

    value = jsonFriend.dump();

    return 0;
}

int FriendManager::restoreFriendsInfo()
{
    int ret = loadLocalData();
    if(ret == 0) {
        Log::I(Log::TAG, "FriendManager::restoreFriendsInfo() Success to recover friend from local.");
    } else if(ret == ErrCode::FileNotExistsError) {
        Log::I(Log::TAG, "FriendManager::restoreFriendsInfo() Local friend info not exists.");
//        ret = syncDidChainData();
//        if(ret == 0) {
//            Log::I(Log::TAG, "FriendManager::restoreFriendsInfo() Success to recover friend from did chain.");
//        } else if(ret == ErrCode::BlkChnEmptyPropError) {
//            Log::I(Log::TAG, "FriendManager::restoreFriendsInfo() Can't find friend info from local or did chain.");
//            ret = ErrCode::EmptyInfoError;
//        }
        ret = 0;
    }
    if(ret < 0) {
        Log::W(Log::TAG, "FriendManager::restoreFriendsInfo() Failed to restore friend, ret=%d", ret);
        return ret;
    }

    for(auto& it: mFriendList) {
        it->setHumanStatus(HumanInfo::Status::Online, HumanInfo::Status::Offline);
    }

    return 0;
}

int FriendManager::ensureFriendsCarrierInfo(int64_t currCarrierUpdateTime)
{
    Log::V(Log::TAG, "%s", __PRETTY_FUNCTION__);

    for(auto& friendInfo: mFriendList) {
        auto status = friendInfo->getHumanStatus();
        if(status == HumanInfo::Status::Invalid
        || status == HumanInfo::Status::Removed) {
            continue;
        }

        std::vector<FriendInfo::CarrierInfo> carrierInfoArray;
        int ret = friendInfo->getAllCarrierInfo(carrierInfoArray);
        if(ret < 0) {
            Log::W(Log::TAG, "FriendManager::ensureFriendsInfo() Failed to readd friend from local carrier info, ret=%d", ret);
            continue;
        }
        auto friendUpdateTime = friendInfo->getHumanUpdateTime();
        if(friendUpdateTime > currCarrierUpdateTime) {
            continue;
        }

        for(const auto& info: carrierInfoArray) {
            Log::I(Log::TAG, "FriendManager::ensureFriendsInfo() Try to readd friend from carrier address: %s", info.mUsrAddr.c_str());
            if(info.mUsrAddr.empty() == true) {
                Log::W(Log::TAG, "FriendManager::ensureFriendsInfo() Failed to readd friend from empty carrier address");
                continue;
            }
            ret = addFriendByCarrier(info.mUsrAddr, "readd", true, true);
            if(ret < 0) {
                Log::W(Log::TAG, "FriendManager::ensureFriendsInfo() Failed to readd friend from carrier, ret=%d", ret);
                continue;
            }
        }
    }

    return 0;
}

int FriendManager::tryAddFriend(const std::string& friendCode, const std::string& summary,
                                bool remoteRequest, bool forceRequest)
{
    auto kind = HumanInfo::AnalyzeHumanKind(friendCode);
    if(static_cast<int>(kind) < 0) {
        return static_cast<int>(kind);
    }

    int ret = addFriend(kind, friendCode, summary, remoteRequest, forceRequest);
    CHECK_ERROR(ret)

    return 0;
}

int FriendManager::tryAcceptFriend(const std::string& friendCode)
{
    std::shared_ptr<FriendInfo> friendInfo;
    int ret = tryGetFriendInfo(friendCode, friendInfo);
    CHECK_ERROR(ret)

    ret = acceptFriend(friendInfo);
    CHECK_ERROR(ret)

    return 0;
}

int FriendManager::tryRemoveFriend(const std::string& friendCode)
{
    auto kind = HumanInfo::AnalyzeHumanKind(friendCode);
    if(static_cast<int>(kind) < 0) {
        return static_cast<int>(kind);
    }

    int ret = removeFriend(kind, friendCode);
    CHECK_ERROR(ret)

    return 0;
}

int FriendManager::tryGetFriendInfo(const std::string& friendCode, std::shared_ptr<FriendInfo>& friendInfo)
{
    auto kind = HumanInfo::AnalyzeHumanKind(friendCode);
    if(static_cast<int>(kind) < 0) {
        return static_cast<int>(kind);
    }

    int ret = getFriendInfo(kind, friendCode, friendInfo);
    CHECK_ERROR(ret)

    return 0;
}

int FriendManager::addFriend(FriendInfo::HumanKind friendKind, const std::string& friendCode,
                             const std::string& summary, bool remoteRequest, bool forceRequest)
{
    if(summary.length() > ADDFRIEND_SUMMARY_MAXSIZE) {
        CHECK_ERROR(ErrCode::SizeOverflowError);
    }

    int ret = ErrCode::InvalidArgument;
    if(friendKind == FriendInfo::HumanKind::Brief) {
        ret = addFriendByBrief(friendCode, summary, remoteRequest, forceRequest);
    } else if(friendKind == FriendInfo::HumanKind::Did) {
        ret = addFriendByDid(friendCode, summary, remoteRequest, forceRequest);
    } else if(friendKind == FriendInfo::HumanKind::Ela) {
        ret = addFriendByEla(friendCode, summary, remoteRequest, forceRequest);
    } else if(friendKind == FriendInfo::HumanKind::Carrier) {
        ret = addFriendByCarrier(friendCode, summary, remoteRequest, forceRequest);
    }

    return ret;
}

int FriendManager::removeFriend(FriendInfo::HumanKind friendKind, const std::string& friendCode)
{
    int ret = ErrCode::InvalidArgument;

    if(friendKind == FriendInfo::HumanKind::Did) {
        ret = removeFriendByDid(friendCode);
    } else if(friendKind == FriendInfo::HumanKind::Ela) {
        ret = removeFriendByEla(friendCode);
    } else if(friendKind == FriendInfo::HumanKind::Carrier) {
        ret = removeFriendByCarrier(friendCode);
    }

    return ret;
}

int FriendManager::addFriendInfo(std::shared_ptr<FriendInfo> friendInfo)
{
    std::lock_guard<std::recursive_mutex> lock(mMutex);                              \
    mFriendList.push_back(friendInfo);
    return 0;
}

int FriendManager::getFriendInfo(FriendInfo::HumanKind friendKind, const std::string& friendCode, std::shared_ptr<FriendInfo>& friendInfo)
{
    int ret = ErrCode::InvalidArgument;

    if(friendKind == FriendInfo::HumanKind::Did) {
        ret = getFriendInfoByDid(friendCode, friendInfo);
    } else if(friendKind == FriendInfo::HumanKind::Ela) {
        ret = getFriendInfoByEla(friendCode, friendInfo);
    } else if(friendKind == FriendInfo::HumanKind::Carrier) {
        ret = getFriendInfoByCarrier(friendCode, friendInfo);
    }

    return ret;
}

int FriendManager::getFriendInfoList(std::vector<std::shared_ptr<FriendInfo>>& friendList)
{
    friendList = mFriendList;
    return 0;
}

std::vector<FriendInfo> FriendManager::filterFriends(std::string regex)
{
    throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + " Unimplemented!!!");
}

//int FriendManager::syncDownloadDidChainData()
//{
//    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);
//    std::string did;
//    int ret = sectyMgr->getDid(did);
//    CHECK_ERROR(ret)
//
//    auto bcClient = BlkChnClient::GetInstance();
//
//    std::string keyPath;
//    ret = bcClient->getDidPropHistoryPath(did, "FriendID", keyPath);
//    if (ret < 0) {
//        return ret;
//    }
//
//    std::string result;
//    ret = bcClient->downloadFromDidChn(keyPath, result);
//    CHECK_ERROR(ret);
//
//    ret = mergeFriendInfoFromJsonArray(result);
//    CHECK_ERROR(ret);
//
//    return 0;
//}

//int FriendManager::monitorDidChainData()
//{
//    class FriendDataMonitor final : public DidChnClient::MonitorCallback {
//    public:
//        explicit FriendDataMonitor(std::weak_ptr<FriendManager> friendManager)
//                : mFriendManager(friendManager) {
//        }
//        virtual ~FriendDataMonitor() = default;
//
//        virtual void onError(const std::string& did, const std::string& key,
//                             int errcode) override {
//            Log::I(Log::TAG, "%s did=%s, key=%s errcode=%d", __PRETTY_FUNCTION__, did.c_str(), key.c_str(), errcode);
//        }
//
//        virtual int onChanged(const std::string& did, const std::string& key,
//                              const std::vector<std::string>& didProps) override {
//            Log::I(Log::TAG, "%s did=%s, key=%s", __PRETTY_FUNCTION__, did.c_str(), key.c_str());
//
//            auto friendMgr = SAFE_GET_PTR(mFriendManager);
//
//            for(const auto& it: didProps) {
//                if(key == DidChnClient::NameCarrierKey) {
//                    HumanInfo::CarrierInfo carrierInfo;
//                    int ret = HumanInfo::deserialize(it, carrierInfo);
//                    CHECK_ERROR(ret);
//
//                    std::shared_ptr<FriendInfo> friendInfo;
//                    ret = friendMgr->tryGetFriendInfo(did, friendInfo);
//                    CHECK_ERROR(ret)
//
//                    ret = friendInfo->addCarrierInfo(carrierInfo, HumanInfo::Status::WaitForAccept);
//                    if(ret == ErrCode::IgnoreMergeOldInfo) {
//                        Log::V(Log::TAG, "MessageManager::monitorDidChainCarrierID() Ignore to sync CarrierId: %s", it.c_str());
//                        continue;
//                    }
//                    CHECK_ERROR(ret);
//                }
//            }
//
//            return 0;
//        }
//
//    private:
//        std::weak_ptr<FriendManager> mFriendManager;
//    };
//    auto callback = std::make_shared<FriendDataMonitor>(shared_from_this());
//
//    auto dcClient = DidChnClient::GetInstance();
//
//    for(auto& it: mFriendList) {
//        std::string did;
//        int ret = it->getHumanInfo(HumanInfo::Item::Did, did);
//        if (ret < 0) {
//            continue;
//        }
//
//        ret = dcClient->appendMoniter(did, callback, true);
//        CHECK_ERROR(ret)
//    }
//
//    return 0;
//}

//{
//    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);
//    auto msgMgr = SAFE_GET_PTR(mMessageManager);
//
//    for(auto& it: mFriendList) {
//        std::string did;
//        int ret = it->getHumanInfo(HumanInfo::Item::Did, did);
//        if(ret < 0) {
//            continue;
//        }
//
//        ret = msgMgr->monitorDidChainCarrierID(did);
//        CHECK_ERROR(ret)
//    }
//
////    int ret = monitorDidChainFriendID();
////    CHECK_ERROR(ret)
//
//    return 0;
//}

//int FriendManager::monitorDidChainFriendID()
//{
//    auto callback = [=](int errcode,
//                        const std::string& keyPath,
//                        const std::string& result) {
//        Log::D(Log::TAG, "FriendManager::monitorDidChainFriendID() ecode=%d, path=%s, result=%s", errcode, keyPath.c_str(), result.c_str());
//        CHECK_RETVAL(errcode);
//
//        int ret = mergeFriendInfoFromJsonArray(result);
//        CHECK_RETVAL(ret);
//
//        return;
//    };
//
//    auto sectyMgr = SAFE_GET_PTR(mSecurityManager);
//    std::string did;
//    int ret = sectyMgr->getDid(did);
//    CHECK_ERROR(ret)
//
//    auto bcClient = BlkChnClient::GetInstance();
//
//    std::string keyPath;
//    ret = bcClient->getDidPropHistoryPath(did, "FriendID", keyPath);
//    CHECK_ERROR(ret)
//
//    Log::I(Log::TAG, "FriendManager::monitorDidChainFriendID() keyPath=%s", keyPath.c_str());
//    ret = bcClient->appendMoniter(keyPath, callback);
//    CHECK_ERROR(ret)
//
//    return 0;
//}


// int FriendManager::uploadFriendInfo()
// {
//     auto bcClient = BlkChnClient::GetInstance();

//     for(const auto& it: mNotUploadFriendList) {
//         std::string did;
//         int ret = it->getHumanInfo(HumanInfo::Item::Did, did);
//         if(ret < 0) {
//             Log::W(Log::TAG, "FriendManager::uploadFriendInfo() Failed get friend did.");
//         }
//         int status = static_cast<int>(it->getHumanStatus());
//         long updateTime = DateTime::CurrentMS();

//         Json jsonInfo = Json::object();
//         jsonInfo["Did"] = did;
//         jsonInfo["Status"] = status;
//         jsonInfo["UpdateTime"] = updateTime;

//         std::map<std::string, std::string> propMap;
//         propMap["FriendID"] = jsonInfo.dump();
//         ret = bcClient->uploadAllDidProps(propMap);
//         if(ret < 0) {
//             Log::I(Log::TAG, "FriendManager::uploadFriendInfo() Failed to upload did: %s", did.c_str());
//             continue;
//         }

//         Log::I(Log::TAG, "FriendManager::uploadFriendInfo() Success to upload did: %s", did.c_str());
//     }

//     return 0;
// }

int FriendManager::acceptFriend(std::shared_ptr<FriendInfo> friendInfo)
{
    Log::I(Log::TAG, "==== %s", __PRETTY_FUNCTION__);
    FriendInfo::Status oldStatus = friendInfo->getHumanStatus();

    std::vector<FriendInfo::CarrierInfo> carrierInfoArray;
    int ret = friendInfo->getAllCarrierInfo(carrierInfoArray);
    CHECK_ERROR(ret)

    for(const auto& info: carrierInfoArray) {
        FriendInfo::Status status;
        ret = friendInfo->getCarrierStatus(info.mUsrId, status);
        CHECK_ERROR(ret)

        if(status != FriendInfo::Status::WaitForAccept) {
            continue;
        }

        ret = addFriendByCarrier(info.mUsrId, "", false);
        CHECK_ERROR(ret)

        ret = friendInfo->setCarrierStatus(info.mUsrId, FriendInfo::Status::Offline);
        CHECK_ERROR(ret)
    }

    if(oldStatus == FriendInfo::Status::WaitForAccept) {
        ret = cacheFriendToDidChain(friendInfo);
        CHECK_ERROR(ret)
    }

    return 0;
}

int FriendManager::cacheFriendToDidChain(std::shared_ptr<FriendInfo> friendInfo)
{
    std::string humanCode;
    int ret = friendInfo->getHumanCode(humanCode);
    if (ret < 0) {
        return ret;
    }

    Json jsonInfo = Json::object();
    jsonInfo[JsonKey::FriendCode] = humanCode;
    jsonInfo[JsonKey::Status] = friendInfo->getHumanStatus();
    jsonInfo[JsonKey::UpdateTime] = DateTime::CurrentMS();

    std::string friendID = jsonInfo.dump();

    auto dcClient = DidChnClient::GetInstance();
    ret = dcClient->cacheDidProp(DidChnClient::NameFriendKey, friendID);
    if (ret < 0) {
        return ret;
    }

    return 0;
}

/***********************************************/
/***** class protected function implement  *****/
/***********************************************/


/***********************************************/
/***** class private function implement  *******/
/***********************************************/

int FriendManager::addFriendByBrief(const std::string& brief, const std::string& summary, bool remoteRequest, bool forceRequest)
{
    auto newFriendInfo = std::make_shared<FriendInfo>(weak_from_this());
    int ret = newFriendInfo->setHumanBrief(brief);
    CHECK_ERROR(ret);
    std::string did;
    ret = newFriendInfo->getHumanInfo(FriendInfo::Item::Did, did);
    CHECK_ERROR(ret);

    std::shared_ptr<FriendInfo> friendInfo;
    ret = tryGetFriendInfo(did, friendInfo);
    if(ret < 0) {
        friendInfo = newFriendInfo;
        ret = friendInfo->setHumanStatus(FriendInfo::Status::Invalid, FriendInfo::Status::WaitForAccept);
        CHECK_ERROR(ret);

        addFriendInfo(friendInfo);
    } else {
        ret = friendInfo->mergeHumanInfo(*newFriendInfo, FriendInfo::Status::WaitForAccept);
        CHECK_ERROR(ret);
    }

    std::vector<FriendInfo::CarrierInfo> carrierInfoList;
    ret = friendInfo->getAllCarrierInfo(carrierInfoList);
    CHECK_ERROR(ret);
    for(const auto& carrierInfo: carrierInfoList) {
        ret = addFriendByCarrier(carrierInfo.mUsrAddr, summary, true, true);
        CHECK_ERROR(ret);
    }
    return 0;
}

int FriendManager::addFriendByDid(const std::string& did, const std::string& summary, bool remoteRequest, bool forceRequest)
{
    auto dcClient = DidChnClient::GetInstance();

    std::map<std::string, std::vector<std::string>> didProps;
    int ret = dcClient->downloadDidProp(did, true, didProps);
    if(ret < 0) {
        Log::W(Log::TAG, "FriendManager::addFriendByDid() Failed to add friend did: %s.", did.c_str());
        return ret;
    }

    std::shared_ptr<FriendInfo> friendInfo;
    ret = tryGetFriendInfo(did, friendInfo);
    if(ret < 0) {
        friendInfo = std::make_shared<FriendInfo>(weak_from_this());
        addFriendInfo(friendInfo);
    }

    auto dcDataListener = DidChnDataListener::GetInstance();
    for (auto& [key, values]: didProps) {
        ret = dcDataListener->mergeHumanInfo(friendInfo, key, values);
        CHECK_ERROR(ret)
    }

    std::vector<FriendInfo::CarrierInfo> carrierInfoArray;
    ret = friendInfo->getAllCarrierInfo(carrierInfoArray);
    CHECK_ERROR(ret)
    for(const auto& info: carrierInfoArray) {
        ret = addFriendByCarrier(info.mUsrAddr, summary, true, forceRequest);
        if(ret == ErrCode::ChannelFailedFriendExists) {
            Log::I(Log::TAG, "Ignore to add friend %s, is already exists.", info.mUsrAddr.c_str());
            ret = 0;
        }
        CHECK_ERROR(ret)
    }

    Log::I(Log::TAG, "FriendManager::addFriendByDid() Add friend did: %s.", did.c_str());

    return 0;
}

int FriendManager::addFriendByCarrier(const std::string& carrierAddress, const std::string& summary, bool remoteRequest, bool forceRequest)
{
    auto msgMgr = SAFE_GET_PTR(mMessageManager);
    int ret = msgMgr->requestFriend(carrierAddress, MessageManager::ChannelType::Carrier, summary, remoteRequest, forceRequest);
    CHECK_ERROR(ret)

    std::shared_ptr<FriendInfo> friendInfo;
    ret = tryGetFriendInfo(carrierAddress, friendInfo);
    if(ret < 0) { // not found
        friendInfo = std::make_shared<FriendInfo>(weak_from_this());
        mFriendList.push_back(friendInfo);
        FriendInfo::CarrierInfo carrierInfo;
        carrierInfo.mUsrAddr = carrierAddress;
        ret = friendInfo->addCarrierInfo(carrierInfo, FriendInfo::Status::WaitForAccept);
        CHECK_ERROR(ret)
    } else {
        // change removed friend to WaitForAccept
        ret = friendInfo->setHumanStatus(FriendInfo::Status::Removed, FriendInfo::Status::WaitForAccept);
        CHECK_ERROR(ret)
    }

    return 0;
}

int FriendManager::addFriendByEla(const std::string& elaAddress, const std::string& summary, bool remoteRequest, bool forceRequest)
{
    if(::isAddressValid(elaAddress.c_str()) == false) {
        return ErrCode::InvalidArgument;
    }

    auto ecClient = ElaChnClient::GetInstance();

    std::string publicKey;
    int ret = ecClient->downloadPublicKey(elaAddress, publicKey);
    CHECK_ERROR(ret);

    std::string did;
    ret = SecurityManager::GetDid(publicKey, did);
    CHECK_ERROR(ret);

    ret = addFriendByDid(did, summary, remoteRequest, forceRequest);
    CHECK_ERROR(ret);


//    bool hasExists = contains(elaAddress);
//    if(hasExists == true) {
//        Log::I(Log::TAG, "Friend %s has been exists.", elaAddress.c_str());
//        return 0;
//    }
//
//    auto friendInfo = std::make_shared<FriendInfo>(weak_from_this());
//    int ret = friendInfo->setHumanInfo(HumanInfo::Item::ElaAddress, elaAddress);
//    CHECK_ERROR(ret)
//
//    ret = friendInfo->setHumanStatus(HumanInfo::HumanKind ::Ela, HumanInfo::Status::Offline);
//    CHECK_ERROR(ret)
//
//    mFriendList.push_back(friendInfo);

    return 0;
}

int FriendManager::removeFriendByDid(const std::string& did)
{
    std::shared_ptr<FriendInfo> friendInfo;
    int ret = tryGetFriendInfo(did, friendInfo);
    CHECK_ERROR(ret);

    std::vector<FriendInfo::CarrierInfo> carrierInfoArray;
    ret = friendInfo->getAllCarrierInfo(carrierInfoArray);
    CHECK_ERROR(ret)
    for(const auto& info: carrierInfoArray) {
        ret = removeFriendByCarrier(info.mUsrId);
        CHECK_ERROR(ret)
    }

    ret = friendInfo->setHumanStatus(HumanInfo::HumanKind::Did, HumanInfo::Status::Removed);
    CHECK_ERROR(ret)

    ret = cacheFriendToDidChain(friendInfo);
    CHECK_ERROR(ret)

    Log::I(Log::TAG, "FriendManager::removeFriendByDid() Remove friend did: %s.", did.c_str());

    return 0;
}

int FriendManager::removeFriendByCarrier(const std::string& carrierUsrId)
{
    auto msgMgr = SAFE_GET_PTR(mMessageManager);
    int ret = msgMgr->removeFriend(carrierUsrId, MessageManager::ChannelType::Carrier);
    CHECK_ERROR(ret)

    std::shared_ptr<FriendInfo> friendInfo;
    ret = tryGetFriendInfo(carrierUsrId, friendInfo);
    CHECK_ERROR(ret)

    ret = friendInfo->delCarrierInfo(carrierUsrId);
    CHECK_ERROR(ret)

    return 0;
}

int FriendManager::removeFriendByEla(const std::string& elaAddress)
{
    throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + " Unimplemented!!!");
}

int FriendManager::getFriendInfoByDid(const std::string& did, std::shared_ptr<FriendInfo>& friendInfo)
{
    for(auto idx = 0; idx < mFriendList.size(); idx++) {
        std::string value;
        mFriendList[idx]->getHumanInfo(HumanInfo::Item::Did, value);

        if(value == did) {
            friendInfo = mFriendList[idx];
            return idx;
        }
    }

    return ErrCode::NotFoundError;
}

int FriendManager::getFriendInfoByCarrier(const std::string& carrierUsrId, std::shared_ptr<FriendInfo>& friendInfo)
{
    friendInfo.reset();

    for(auto idx = 0; idx < mFriendList.size(); idx++) {
        auto info = mFriendList[idx];
        std::vector<FriendInfo::CarrierInfo> carrierInfoArray;
        int ret = info->getAllCarrierInfo(carrierInfoArray);
        CHECK_ERROR(ret)

        for(const auto& carrierInfo: carrierInfoArray) {
            if(carrierInfo.mUsrAddr == carrierUsrId
            || carrierInfo.mUsrId == carrierUsrId) {
                friendInfo = info;
                return idx;
            }
        }
    }

    return ErrCode::NotFoundError;
}

int FriendManager::getFriendInfoByEla(const std::string& elaAddress, std::shared_ptr<FriendInfo>& friendInfo)
{
    throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + " Unimplemented!!!");
}

int FriendManager::mergeFriendInfoFromJsonArray(const std::string& jsonArray)
{
    std::vector<std::string> values;
    Json jsonPropArray = Json::parse(jsonArray);
    for (const auto& it : jsonPropArray) {
        values.push_back(it["value"]);
    }

    std::vector<std::string> friendCodeArray;
    for(const auto& it: values) {
        Json jsonInfo = Json::parse(it);
        std::string friendCode = jsonInfo["FriendCode"];
        HumanInfo::Status status = jsonInfo["Status"];
        //int64_t updateTime = jsonInfo["UpdateTime"];

        if(status == HumanInfo::Status::Removed) {
            continue;
        }
        friendCodeArray.push_back(friendCode);
    }

    for(const auto& it: friendCodeArray) {
        int ret = tryAddFriend(it, "", false);
        if(ret < 0) {
            Log::W(Log::TAG, "FriendManager::mergeFriendInfoFromJsonArray() Failed to add friend code: %s.", it.c_str());
            continue;
        }

        Log::I(Log::TAG, "FriendManager::mergeFriendInfoFromJsonArray() Add friend did: %s.", it.c_str());
    }

    return 0;
}


} // namespace elastos
