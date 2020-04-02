#include "DidChnDataListener.hpp"

#include <Log.hpp>
#include <SafePtr.hpp>
#include "JsonDefine.hpp"

namespace elastos {

/* =========================================== */
/* === static variables initialize =========== */
/* =========================================== */
std::shared_ptr<DidChnDataListener> DidChnDataListener::gDidChnDataListener;

/* =========================================== */
/* === static function implement ============= */
/* =========================================== */
int DidChnDataListener::InitInstance(std::weak_ptr<UserManager> userMgr,
                                     std::weak_ptr<FriendManager> friendMgr,
                                     std::weak_ptr<MessageManager> msgMgr)
{
//    if (gDidChnDataListener.get() != nullptr) {
//        gDidChnDataListener.reset();
//    }

    struct Impl : DidChnDataListener {
        Impl(std::weak_ptr<UserManager> userMgr,
             std::weak_ptr<FriendManager> friendMgr,
             std::weak_ptr<MessageManager> msgMgr)
                : DidChnDataListener(userMgr, friendMgr, msgMgr) {}
    };

    gDidChnDataListener = std::make_shared<Impl>(userMgr, friendMgr, msgMgr);

    return 0;
}

std::shared_ptr<DidChnDataListener> DidChnDataListener::GetInstance()
{
    assert(gDidChnDataListener.get() != nullptr);
    return gDidChnDataListener;
}

/* =========================================== */
/* === class public function implement  ====== */
/* =========================================== */
int DidChnDataListener::mergeHumanInfo(std::shared_ptr<HumanInfo> humanInfo,
                                       const std::string& key,
                                       const std::vector<std::string>& didProps)
{
    if(key == DidChnClient::NamePublicKey) {
        int ret = processPublicKeyChanged(humanInfo, didProps);
        CHECK_ERROR(ret);
    } else if(key == DidChnClient::NameCarrierKey) {
        int ret = processCarrierKeyChanged(humanInfo, didProps);
        CHECK_ERROR(ret);
    } else if(key == DidChnClient::NameDetailKey) {
        int ret = processDetailKeyChanged(humanInfo, didProps);
        CHECK_ERROR(ret);
    } else if(key == DidChnClient::NameIdentifyKey) {
        int ret = processIdentifyKeyChanged(humanInfo, didProps);
        CHECK_ERROR(ret);
    } else if(key == DidChnClient::NameFriendKey) {
        int ret = processFriendKeyChanged(humanInfo, didProps);
        CHECK_ERROR(ret);
    } else {
        CHECK_ERROR(ErrCode::InvalidFriendCode);
    }

    return 0;
}

void DidChnDataListener::onError(const std::string& did, const std::string& key, int errcode)
{
    std::string msg = std::string("DidChnDataListener::onError did=") + did + ", key=" + key + " errcode=" + std::to_string(errcode);
    Log::W(Log::TAG, msg.c_str());
    CHECK_AND_NOTIFY_RETVAL(errcode);
}

int DidChnDataListener::onChanged(const std::string& did, const std::string& key, const std::vector<std::string>& didProps)
{
    Log::I(Log::TAG, "%s did=%s, key=%s", FORMAT_METHOD, did.c_str(), key.c_str());

    auto userMgr = SAFE_GET_PTR(mUserManager);
    auto friendMgr = SAFE_GET_PTR(mFriendManager);

    std::shared_ptr<HumanInfo> humanInfo;
    if (userMgr->contains(did)) {
        std::shared_ptr<elastos::UserInfo> userInfo;
        int ret = userMgr->getUserInfo(userInfo);
        CHECK_AND_NOTIFY_ERROR(ret);
        humanInfo = userInfo;
    } else if (friendMgr->contains(did)) {
        std::shared_ptr<FriendInfo> friendInfo;
        int ret  = friendMgr->tryGetFriendInfo(did, friendInfo);
        CHECK_AND_NOTIFY_ERROR(ret);
        humanInfo = friendInfo;
    } else {
        CHECK_AND_NOTIFY_ERROR(ErrCode::InvalidFriendCode);
    }

    int ret = mergeHumanInfo(humanInfo, key, didProps);
    CHECK_AND_NOTIFY_ERROR(ret);

    return 0;
}

/* =========================================== */
/* === class protected function implement  === */
/* =========================================== */


/* =========================================== */
/* === class private function implement  ===== */
/* =========================================== */
DidChnDataListener::DidChnDataListener(std::weak_ptr<UserManager> userMgr,
                                       std::weak_ptr<FriendManager> friendMgr,
                                       std::weak_ptr<MessageManager> msgMgr)
        : mUserManager(userMgr)
        , mFriendManager(friendMgr)
        , mMessageManager(msgMgr)
{
}

DidChnDataListener::~DidChnDataListener()
{
}

int DidChnDataListener::processPublicKeyChanged(std::shared_ptr<elastos::HumanInfo> humanInfo,
                                                const std::vector<std::string> &didProps)
{
    for(const auto& it: didProps) {
        int ret = humanInfo->setHumanInfo(HumanInfo::Item::ChainPubKey, it);
        CHECK_ERROR(ret);
    }

    return 0;
}

int DidChnDataListener::processCarrierKeyChanged(std::shared_ptr<HumanInfo> humanInfo,
                                                 const std::vector<std::string>& didProps)
{
    for(const auto& it: didProps) {
        HumanInfo::CarrierInfo carrierInfo;
        int ret = HumanInfo::DeserializeCarrierInfo(it, carrierInfo);
        CHECK_ERROR(ret);

        ret = humanInfo->addCarrierInfo(carrierInfo, HumanInfo::Status::WaitForAccept);
        if(ret == ErrCode::IgnoreMergeOldInfo) {
            Log::V(Log::TAG, "DidChnDataListener::processCarrierInfoChanged() Ignore to sync CarrierId: %s", it.c_str());
            continue;
        }
        CHECK_ERROR(ret);

        Log::I(Log::TAG, "DidChnDataListener::processCarrierInfoChanged() Success to sync CarrierId: %s", it.c_str());
    }

    return 0;
}

int DidChnDataListener::processDetailKeyChanged(std::shared_ptr<HumanInfo> humanInfo,
                                                const std::vector<std::string>& didProps)
{
    for(const auto& it: didProps) {
        int ret = humanInfo->deserializeDetails(it);
        if(ret == ErrCode::IgnoreMergeOldInfo) {
            Log::V(Log::TAG, "DidChnDataListener::processCarrierInfoChanged() Ignore to sync CarrierId: %s", it.c_str());
            continue;
        }
        CHECK_ERROR(ret);

        Log::I(Log::TAG, "DidChnDataListener::processCarrierInfoChanged() Success to sync CarrierId: %s", it.c_str());
    }

    return 0;
}

int DidChnDataListener::processIdentifyKeyChanged(std::shared_ptr<HumanInfo> humanInfo, const std::vector<std::string>& didProps)
{
    std::shared_ptr<UserInfo> userInfo;
    userInfo = std::dynamic_pointer_cast<UserInfo>(humanInfo);
    if(userInfo.get() == nullptr) {
        return ErrCode::BlkChnSetPropError;
    }

    for(const auto& it: didProps) {
        IdentifyCode identifyCode;
        int ret = identifyCode.deserialize(it);
        CHECK_ERROR(ret);

        ret = userInfo->mergeIdentifyCode(identifyCode);
        if(ret == ErrCode::IgnoreMergeOldInfo) {
            Log::V(Log::TAG, "DidChnDataListener::processIdentifyKeyChanged() Ignore to sync IdentifyCode: %s", it.c_str());
            continue;
        }
        CHECK_ERROR(ret);
    }

    return 0;
}

int DidChnDataListener::processFriendKeyChanged(std::shared_ptr<HumanInfo> humanInfo, const std::vector<std::string>& didProps)
{
    auto friendMgr = SAFE_GET_PTR(mFriendManager);

    std::vector<std::string> friendCodeArray;
    for(const auto& it: didProps) {
        Json jsonInfo = Json::parse(it);
        std::string friendCode = jsonInfo[JsonKey::FriendCode];
        HumanInfo::Status status = jsonInfo[JsonKey::Status];
        //int64_t updateTime = jsonInfo[JsonKey::UpdateTime];

        if(status == HumanInfo::Status::Removed) {
            continue;
        }
        friendCodeArray.push_back(friendCode);
    }

    for(const auto& it: friendCodeArray) {
        int ret = friendMgr->tryAddFriend(it, "", false);
        CHECK_ERROR(ret);

        Log::I(Log::TAG, "DidChnDataListener::processFriendKeyChanged() Add friend did: %s.", it.c_str());
    }

    return 0;
}


} // namespace elastos
