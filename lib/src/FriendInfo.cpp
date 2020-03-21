//
//  FriendInfo.cpp
//
//  Created by mengxk on 19/03/16.
//  Copyright © 2016 mengxk. All rights reserved.
//

#include <FriendInfo.hpp>
#include <FriendManager.hpp>
#include <JsonDefine.hpp>
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
FriendInfo::FriendInfo(std::weak_ptr<FriendManager> friendMgr)
    : HumanInfo()
    , mAlias()
    , mFriendManager(friendMgr)
{
}

FriendInfo::FriendInfo()
    : HumanInfo()
    , mAlias()
    , mFriendManager()
{
}

FriendInfo::~FriendInfo()
{
}

int FriendInfo::setAlias(const std::string& value)
{
    mAlias = value;

    auto friendMgr = SAFE_GET_PTR(mFriendManager);
    int ret = friendMgr->saveLocalData();
    if(ret < 0) {
        return ret;
    }

    return 0;
}

std::string FriendInfo::getAlias() const
{
    return mAlias;
}

int FriendInfo::serialize(std::string& value, bool summaryOnly) const
{
    Json jsonInfo = Json::object();

    std::string humanInfo, identCode;
    int ret = HumanInfo::serialize(humanInfo, summaryOnly);
    if(ret < 0) {
        return ret;
    }
    jsonInfo[JsonKey::HumanInfo] = humanInfo;

    if(summaryOnly == false) {
        jsonInfo[JsonKey::Alias] = mAlias;
    }

    value = jsonInfo.dump();

    return 0;
}

int FriendInfo::deserialize(const std::string& value, bool summaryOnly)
{
    Json jsonInfo= Json::parse(value);

    std::string humanInfo = jsonInfo[JsonKey::HumanInfo];
    int ret = HumanInfo::deserialize(humanInfo, summaryOnly);
    if(ret < 0) {
        return ret;
    }

    if(summaryOnly == false) {
        mAlias = jsonInfo[JsonKey::Alias];
    }

    return 0;

}

int FriendInfo::toJson(std::shared_ptr<Json>& value) const
{
    if(value.get() == nullptr) {
        return ErrCode::InvalidArgument;
    }

    int ret = HumanInfo::toJson(value);
    CHECK_ERROR(ret);

    (*value)[JsonKey::Alias] = mAlias;
    (*value)[JsonKey::IsFriend] = true;

    return 0;
}

int FriendInfo::addCarrierInfo(const CarrierInfo& info, const Status status)
{
    int ret = HumanInfo::addCarrierInfo(info, status);
    if(ret < 0) { // error or not changed
        return ret;
    }

    auto friendMgr = SAFE_GET_PTR(mFriendManager);
    ret = friendMgr->saveLocalData();
    if(ret < 0) {
        return ret;
    }

    return 0;
}

int FriendInfo::delCarrierInfo(const std::string& carrierUsrId)
{
    int ret = HumanInfo::delCarrierInfo(carrierUsrId);
    if(ret < 0) { // error or not changed
        return ret;
    }

    auto friendMgr = SAFE_GET_PTR(mFriendManager);
    ret = friendMgr->saveLocalData();
    if(ret < 0) {
        return ret;
    }

    return 0;
}

int FriendInfo::setHumanInfo(Item item, const std::string& value)
{
    int ret = HumanInfo::setHumanInfo(item, value);
    Log::D(Log::TAG, "%s ret=%d", FORMAT_METHOD, ret);
    if(ret <= 0) { // error or not changed
        return ret;
    }

    auto friendMgr = SAFE_GET_PTR(mFriendManager);
    ret = friendMgr->saveLocalData();
    if(ret < 0) {
        return ret;
    }

    return 0;
}

int FriendInfo::mergeHumanInfo(const HumanInfo& value, const Status status)
{
    int ret = HumanInfo::mergeHumanInfo(value, status);
    Log::D(Log::TAG, "%s ret=%d", FORMAT_METHOD, ret);
    if(ret < 0) { // error or not changed
        return ret;
    }

    auto friendMgr = SAFE_GET_PTR(mFriendManager);
    ret = friendMgr->saveLocalData();
    if(ret < 0) {
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

} // namespace elastos
