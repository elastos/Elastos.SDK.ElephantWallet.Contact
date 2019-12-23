//
//  Elastos.SDK.Contact.cpp
//
//  Created by mengxk on 19/03/16.
//  Copyright © 2016 mengxk. All rights reserved.
//

#include <ContactBridge.hpp>
#include <SafePtr.hpp>

#include "Log.hpp"
#include "Json.hpp"
#include <JsonDefine.hpp>
#include "ContactMessage.hpp"

namespace crosspl {
namespace native {

/***********************************************/
/***** static variables initialize *************/
/***********************************************/

/***********************************************/
/***** static function implement ***************/
/***********************************************/


/***********************************************/
/***** class public function implement  ********/
/***********************************************/
ContactBridge::ContactBridge()
        : mContactImpl()
        , mListener(nullptr)
        , mDataListener(nullptr)
{
    Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);

    mContactImpl = elastos::Contact::Factory::Create();
}
ContactBridge::~ContactBridge()
{
    Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);

    elastos::ErrCode::SetErrorListener(nullptr);
}

void ContactBridge::setListener(ListenerPtr listener)
{
    Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);

    mListener = dynamic_cast<ContactListener*>(listener);
//    mListener->onCallback(0, nullptr);

    auto errorListener = std::bind(&ContactListener::onError, mListener,
                                   std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    elastos::ErrCode::SetErrorListener(errorListener);

    auto sectyListener = mListener->getSecurityListener();
    auto msgListener = mListener->getMessageListener();
    mContactImpl->setListener(sectyListener, nullptr, nullptr, msgListener);

    return;
}

void ContactBridge::setDataListener(DataListenerPtr listener)
{
    Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);

    mDataListener = dynamic_cast<ContactDataListener*>(listener);
//    mDataListener->onCallback(0, nullptr);

    auto msgDataListener = mDataListener->getDataListener();

    auto weakMsgMgr = mContactImpl->getMessageManager();
    auto msgMgr =  SAFE_GET_PTR_NO_RETVAL(weakMsgMgr);                                                                      \
    msgMgr->setDataListener(msgDataListener);

    return;
}

int ContactBridge::start()
{
    Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);

    int ret = mContactImpl->start();

    return ret;
}

int ContactBridge::stop()
{
    Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);

    int ret = mContactImpl->stop();

    return ret;
}

//int ContactBridge::setUserInfo(int item, ConstStringPtr value)
//{
//    if(mContactImpl->isStarted() == false) {
//        return elastos::ErrCode::NotReadyError;
//    }
//
//    auto weakUserMgr = mContactImpl->getUserManager();
//    auto userMgr =  SAFE_GET_PTR(weakUserMgr);                                                                      \
//
//    int ret = userMgr->setUserInfo(static_cast<elastos::UserInfo::Item>(item), value);
//    CHECK_ERROR(ret);
//
//    return 0;
//}
//
int ContactBridge::setIdentifyCode(int type, ConstStringPtr value)
{
    if(mContactImpl->isStarted() == false) {
        return elastos::ErrCode::NotReadyError;
    }

    auto weakUserMgr = mContactImpl->getUserManager();
    auto userMgr =  SAFE_GET_PTR(weakUserMgr);                                                                      \

    int ret = userMgr->setIdentifyCode(static_cast<elastos::UserInfo::Type>(type), value);
    CHECK_ERROR(ret);

    return 0;
}

int ContactBridge::setHumanInfo(ConstStringPtr humanCode, int item, ConstStringPtr value)
{
    if(mContactImpl->isStarted() == false) {
        return elastos::ErrCode::NotReadyError;
    }

    if(IsEmpty(humanCode) == true) {
        return elastos::ErrCode::InvalidArgument;
    }

    auto weakUserMgr = mContactImpl->getUserManager();
    auto userMgr =  SAFE_GET_PTR(weakUserMgr);                                                                      \
    auto weakFriendMgr = mContactImpl->getFriendManager();
    auto friendMgr =  SAFE_GET_PTR(weakFriendMgr);                                                                      \

    if(std::string("-user-info-") == humanCode
    || userMgr->contains(humanCode) == true) {
        int ret = userMgr->setUserInfo(static_cast<elastos::UserInfo::Item>(item), value);
        CHECK_ERROR(ret);
    } else if (friendMgr->contains(humanCode) == true) {
        std::shared_ptr<elastos::FriendInfo> friendInfo;
        int ret = friendMgr->tryGetFriendInfo(humanCode, friendInfo);
        CHECK_ERROR(ret);
        ret = friendInfo->setHumanInfo(static_cast<elastos::UserInfo::Item>(item), value);
        CHECK_ERROR(ret);
    } else {
        return elastos::ErrCode::NotFoundError;
    }

    return 0;
}

int ContactBridge::getHumanInfo(ConstStringPtr humanCode, std::stringstream* info)
{
    if(mContactImpl->isStarted() == false) {
        return elastos::ErrCode::NotReadyError;
    }

    if(IsEmpty(humanCode) == true) {
        return elastos::ErrCode::InvalidArgument;
    }

    auto weakUserMgr = mContactImpl->getUserManager();
    auto userMgr =  SAFE_GET_PTR(weakUserMgr);                                                                      \
    auto weakFriendMgr = mContactImpl->getFriendManager();
    auto friendMgr =  SAFE_GET_PTR(weakFriendMgr);                                                                      \

    int ret = elastos::ErrCode::UnknownError;
    std::shared_ptr<elastos::HumanInfo> humanInfo;
    if(std::string("-user-info-") == humanCode
    || userMgr->contains(humanCode) == true) {
        std::shared_ptr<elastos::UserInfo> userInfo;
        ret = userMgr->getUserInfo(userInfo);
        CHECK_ERROR(ret);
        humanInfo = userInfo;
    } else if (friendMgr->contains(humanCode) == true) {
        std::shared_ptr<elastos::FriendInfo> friendInfo;
        ret = friendMgr->tryGetFriendInfo(humanCode, friendInfo);
        CHECK_ERROR(ret);
        humanInfo = friendInfo;
    } else {
        return elastos::ErrCode::NotFoundError;
    }

    auto jsonInfo = std::make_shared<elastos::Json>();
    ret = humanInfo->toJson(jsonInfo);
    CHECK_ERROR(ret);

    info->str(jsonInfo->dump());
    return 0;
}

int ContactBridge::getHumanStatus(ConstStringPtr humanCode)
{
    if(mContactImpl->isStarted() == false) {
        return elastos::ErrCode::NotReadyError;
    }

    auto weakUserMgr = mContactImpl->getUserManager();
    auto userMgr =  SAFE_GET_PTR(weakUserMgr);                                                                      \
    auto weakFriendMgr = mContactImpl->getFriendManager();
    auto friendMgr =  SAFE_GET_PTR(weakFriendMgr);                                                                      \

    std::shared_ptr<elastos::HumanInfo> humanInfo;
    if(std::string("-user-info-") == humanCode
    || userMgr->contains(humanCode) == true) {
        std::shared_ptr<elastos::UserInfo> userInfo;
        int ret = userMgr->getUserInfo(userInfo);
        CHECK_ERROR(ret);
        humanInfo = userInfo;
    } else if (friendMgr->contains(humanCode) == true) {
        std::shared_ptr<elastos::FriendInfo> friendInfo;
        int ret = friendMgr->tryGetFriendInfo(humanCode, friendInfo);
        CHECK_ERROR(ret);
        humanInfo = friendInfo;
    } else {
        return elastos::ErrCode::NotFoundError;
    }

    auto status = humanInfo->getHumanStatus();

    return static_cast<int>(status);
}

int ContactBridge::addFriend(ConstStringPtr friendCode, ConstStringPtr summary)
{
    if(mContactImpl->isStarted() == false) {
        return elastos::ErrCode::NotReadyError;
    }

    auto weakFriendMgr = mContactImpl->getFriendManager();
    auto friendMgr =  SAFE_GET_PTR(weakFriendMgr);                                                                      \

    int ret = friendMgr->tryAddFriend(friendCode, summary, true, true);
    CHECK_ERROR(ret);

    return 0;
}

int ContactBridge::removeFriend(ConstStringPtr friendCode)
{
    if(mContactImpl->isStarted() == false) {
        return elastos::ErrCode::NotReadyError;
    }

    auto weakFriendMgr = mContactImpl->getFriendManager();
    auto friendMgr =  SAFE_GET_PTR(weakFriendMgr);                                                                      \

    int ret = friendMgr->tryRemoveFriend(friendCode);
    CHECK_ERROR(ret);

    return 0;
}

int ContactBridge::acceptFriend(ConstStringPtr friendCode)
{
    if(mContactImpl->isStarted() == false) {
        return elastos::ErrCode::NotReadyError;
    }

    auto weakFriendMgr = mContactImpl->getFriendManager();
    auto friendMgr =  SAFE_GET_PTR(weakFriendMgr);                                                                      \

    int ret = friendMgr->tryAcceptFriend(friendCode);
    CHECK_ERROR(ret);

    return 0;
}

int ContactBridge::getFriendList(std::stringstream* info)
{
    if(mContactImpl->isStarted() == false) {
        return elastos::ErrCode::NotReadyError;
    }

    auto weakFriendMgr = mContactImpl->getFriendManager();
    auto friendMgr = SAFE_GET_PTR(weakFriendMgr);                                                                      \

    std::vector<std::shared_ptr<elastos::FriendInfo>> friendList;
    int ret = friendMgr->getFriendInfoList(friendList);
    CHECK_ERROR(ret);

    elastos::Json friendJsonArray = elastos::Json::array();
    for(const auto& it: friendList) {
        auto jsonInfo = std::make_shared<elastos::Json>();
        ret = it->toJson(jsonInfo);
        CHECK_ERROR(ret);

        friendJsonArray.push_back(*jsonInfo);
    }

    info->str(friendJsonArray.dump());
    return 0;
}

int ContactBridge::sendMessage(ConstStringPtr friendCode, int chType, CrossBase* message)
{
    if(mContactImpl->isStarted() == false) {
        return elastos::ErrCode::NotReadyError;
    }

    auto msgInfo = dynamic_cast<ContactMessage*>(message);
    if(msgInfo == nullptr) {
        return elastos::ErrCode::InvalidArgument;
    }

    auto weakFriendMgr = mContactImpl->getFriendManager();
    auto friendMgr =  SAFE_GET_PTR(weakFriendMgr);                                                                      \
    auto weakMsgMgr = mContactImpl->getMessageManager();
    auto msgMgr =  SAFE_GET_PTR(weakMsgMgr);                                                                      \

    std::shared_ptr<elastos::FriendInfo> friendInfo;
    int ret = friendMgr->tryGetFriendInfo(friendCode, friendInfo);
    CHECK_ERROR(ret);

    ret = msgMgr->sendMessage(friendInfo, static_cast<elastos::MessageManager::ChannelType>(chType), msgInfo->mMessageInfo);
    CHECK_ERROR(ret);

    return ret;
}

int ContactBridge::pullData(ConstStringPtr humanCode, int chType,
                            ConstStringPtr devId, ConstStringPtr dataId)
{
    if(IsEmpty(dataId) == true) {
        return elastos::ErrCode::InvalidArgument;
    }

    if(mContactImpl->isStarted() == false) {
        return elastos::ErrCode::NotReadyError;
    }

    auto weakUserMgr = mContactImpl->getUserManager();
    auto userMgr =  SAFE_GET_PTR(weakUserMgr);                                                                      \
    auto weakFriendMgr = mContactImpl->getFriendManager();
    auto friendMgr =  SAFE_GET_PTR(weakFriendMgr);                                                                      \
    auto weakMsgMgr = mContactImpl->getMessageManager();
    auto msgMgr =  SAFE_GET_PTR(weakMsgMgr);                                                                      \

    std::shared_ptr<elastos::HumanInfo> humanInfo;
    if(userMgr->contains(humanCode) == true) {
        std::shared_ptr<elastos::UserInfo> userInfo;
        int ret = userMgr->getUserInfo(userInfo);
        CHECK_ERROR(ret);
        humanInfo = userInfo;
    } else if (friendMgr->contains(humanCode) == true) {
        std::shared_ptr<elastos::FriendInfo> friendInfo;
        int ret = friendMgr->tryGetFriendInfo(humanCode, friendInfo);
        CHECK_ERROR(ret);
        humanInfo = friendInfo;
    } else {
        return elastos::ErrCode::NotFoundError;
    }

    int ret = msgMgr->pullData(humanInfo, static_cast<elastos::MessageManager::ChannelType>(chType),
                               devId, dataId);
    CHECK_ERROR(ret);

    return ret;
}

int ContactBridge::cancelPullData(ConstStringPtr humanCode, int chType,
                                  ConstStringPtr devId, ConstStringPtr dataId)
{
    if(IsEmpty(dataId) == true) {
        return elastos::ErrCode::InvalidArgument;
    }

    if(mContactImpl->isStarted() == false) {
        return elastos::ErrCode::NotReadyError;
    }

    auto weakFriendMgr = mContactImpl->getFriendManager();
    auto friendMgr =  SAFE_GET_PTR(weakFriendMgr);                                                                      \
    auto weakMsgMgr = mContactImpl->getMessageManager();
    auto msgMgr =  SAFE_GET_PTR(weakMsgMgr);                                                                      \

    std::shared_ptr<elastos::FriendInfo> friendInfo;
    int ret = friendMgr->tryGetFriendInfo(humanCode, friendInfo);
    CHECK_ERROR(ret);

    ret = msgMgr->cancelPullData(friendInfo, static_cast<elastos::MessageManager::ChannelType>(chType),
                                 devId, dataId);
    CHECK_ERROR(ret);

    return ret;
}

int ContactBridge::syncInfoDownloadFromDidChain()
{
    if(mContactImpl->isStarted() == false) {
        return elastos::ErrCode::NotReadyError;
    }

    int ret = mContactImpl->syncInfoDownloadFromDidChain();
    CHECK_ERROR(ret);

    return 0;
}

int ContactBridge::syncInfoUploadToDidChain()
{
    if(mContactImpl->isStarted() == false) {
        return elastos::ErrCode::NotReadyError;
    }

    int ret = mContactImpl->syncInfoUploadToDidChain();
    CHECK_ERROR(ret);

    return 0;
}

int ContactBridge::setWalletAddress(ConstStringPtr name, ConstStringPtr value)
{
    if(mContactImpl->isStarted() == false) {
        return elastos::ErrCode::NotReadyError;
    }

    auto weakUserMgr = mContactImpl->getUserManager();
    auto userMgr =  SAFE_GET_PTR(weakUserMgr);                                                                      \

    int ret = userMgr->setWalletAddress(name, value);
    CHECK_ERROR(ret);

    return 0;
}

/***********************************************/
/***** class protected function implement  *****/
/***********************************************/


/***********************************************/
/***** class private function implement  *******/
/***********************************************/


} //namespace native
} //namespace crosspl
