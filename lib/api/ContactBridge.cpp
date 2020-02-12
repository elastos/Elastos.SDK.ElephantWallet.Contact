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
#include "ContactChannelStrategy.hpp"


namespace crosspl {
namespace native {

/***********************************************/
/***** static variables initialize *************/
/***********************************************/

/***********************************************/
/***** static function implement ***************/
/***********************************************/
bool ContactBridge::IsDidFriend(ConstStringPtr friendCode)
{
    auto kind = elastos::HumanInfo::AnalyzeHumanKind(friendCode);

    return (kind == elastos::HumanInfo::HumanKind::Did);
}

/***********************************************/
/***** class public function implement  ********/
/***********************************************/
ContactBridge::ContactBridge()
        : mContactImpl()
        , mListener(nullptr)
        , mDataListener(nullptr)
{
    Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);

    mContactImpl = elastos::ContactV1::Factory::Create();
}
ContactBridge::~ContactBridge()
{
    Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);

    elastos::ErrCode::SetErrorListener(nullptr);
}

int ContactBridge::appendChannelStrategy(ChannelStrategyPtr channelStrategy)
{
    Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);

#ifdef WITH_CROSSPL
    auto channelStrategyPtr = dynamic_cast<ContactChannelStrategy*>(channelStrategy);
#else
    auto channelStrategyPtr = channelStrategy;
#endif // WITH_CROSSPL

    auto weakMsgMgr = mContactImpl->getMessageManager();
    auto msgMgr =  SAFE_GET_PTR(weakMsgMgr);                                                                      \

    auto channel = channelStrategyPtr->getChannel();
    auto channelId = channel->getChannelType();

    int ret = msgMgr->appendChannel(channelId, channel);
    CHECK_ERROR(ret);

    mCustomChannelMap[channelId] = channelStrategy;

    return 0;
}

void ContactBridge::setListener(ListenerPtr listener)
{
    Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);

    mListener = listener;
#ifdef WITH_CROSSPL
    auto listenerPtr = dynamic_cast<ContactListener*>(mListener);
#else
    auto listenerPtr = mListener;
#endif // WITH_CROSSPL
//    mListener->onCallback(0, nullptr);

    auto errorListener = std::bind(&ContactListener::onError, listenerPtr,
                                   std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    elastos::ErrCode::SetErrorListener(errorListener);

    auto sectyListener = listenerPtr->getSecurityListener();
    auto msgListener = listenerPtr->getMessageListener();
    mContactImpl->setListener(sectyListener, nullptr, nullptr, msgListener);

    return;
}

void ContactBridge::setDataListener(DataListenerPtr listener)
{
    Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);

    mDataListener = listener;
#ifdef WITH_CROSSPL
    auto listenerPtr = dynamic_cast<ContactDataListener*>(mDataListener);
#else
    auto listenerPtr = mDataListener;
#endif // WITH_CROSSPL
//    mDataListener->onCallback(0, nullptr);

    auto msgDataListener = listenerPtr->getDataListener();

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
int ContactBridge::setIdentifyCode(UserIdentifyType type, ConstStringPtr value)
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

int ContactBridge::setHumanInfo(ConstStringPtr humanCode, InfoItemType item, ConstStringPtr value)
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

int ContactBridge::getHumanInfo(ConstStringPtr humanCode, HumanInfoPtr info)
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

#ifdef WITH_CROSSPL
    auto jsonInfo = std::make_shared<elastos::Json>();
    ret = humanInfo->toJson(jsonInfo);
    CHECK_ERROR(ret);
    info->str(jsonInfo->dump());
#else
    info = humanInfo;
#endif // WITH_CROSSPL
    return 0;
}


int ContactBridge::findAvatarFile(ConstStringPtr avatar, OutStringPtr filepath)
{
    if(mContactImpl->isStarted() == false) {
        return elastos::ErrCode::NotReadyError;
    }

    if(IsEmpty(avatar) == true) {
        return elastos::ErrCode::InvalidArgument;
    }

    auto weakUserMgr = mContactImpl->getUserManager();
    auto userMgr =  SAFE_GET_PTR(weakUserMgr);                                                                      \


    std::string avatarPath;
    int ret = userMgr->getAvatarFile(avatar, avatarPath);

#ifdef WITH_CROSSPL
    filepath->str(avatarPath);
#else
    filepath = avatarPath;
#endif // WITH_CROSSPL
    return 0;
}

StatusType ContactBridge::getHumanStatus(ConstStringPtr humanCode)
{
    auto invalidStatus = static_cast<StatusType>(elastos::HumanInfo::Status::Invalid);
    if(mContactImpl->isStarted() == false) {
        return invalidStatus;
    }

    auto weakUserMgr = mContactImpl->getUserManager();
    auto userMgr =  SAFE_GET_PTR_DEF_RETVAL(weakUserMgr, invalidStatus);
    auto weakFriendMgr = mContactImpl->getFriendManager();
    auto friendMgr =  SAFE_GET_PTR_DEF_RETVAL(weakFriendMgr, invalidStatus);

    std::shared_ptr<elastos::HumanInfo> humanInfo;
    if(std::string("-user-info-") == humanCode
    || userMgr->contains(humanCode) == true) {
        std::shared_ptr<elastos::UserInfo> userInfo;
        int ret = userMgr->getUserInfo(userInfo);
        CHECK_AND_RETDEF(ret, invalidStatus);
        humanInfo = userInfo;
    } else if (friendMgr->contains(humanCode) == true) {
        std::shared_ptr<elastos::FriendInfo> friendInfo;
        int ret = friendMgr->tryGetFriendInfo(humanCode, friendInfo);
        CHECK_AND_RETDEF(ret, invalidStatus);
        humanInfo = friendInfo;
    } else {
        return invalidStatus;
    }

    auto status = humanInfo->getHumanStatus();

    return static_cast<StatusType>(status);
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

int ContactBridge::getFriendList(FriendListPtr info)
{
    if(mContactImpl->isStarted() == false) {
        return elastos::ErrCode::NotReadyError;
    }

    auto weakFriendMgr = mContactImpl->getFriendManager();
    auto friendMgr = SAFE_GET_PTR(weakFriendMgr);                                                                      \

    std::vector<std::shared_ptr<elastos::FriendInfo>> friendList;
    int ret = friendMgr->getFriendInfoList(friendList);
    CHECK_ERROR(ret);

#ifdef WITH_CROSSPL
    elastos::Json friendJsonArray = elastos::Json::array();
    for(const auto& it: friendList) {
        auto jsonInfo = std::make_shared<elastos::Json>();
        ret = it->toJson(jsonInfo);
        CHECK_ERROR(ret);

        friendJsonArray.push_back(*jsonInfo);
    }

    info->str(friendJsonArray.dump());
#else
    info = std::move(friendList);
#endif // WITH_CROSSPL
    return 0;
}

int ContactBridge::sendMessage(ConstStringPtr friendCode, ChannelType chType, MessagePtr message)
{
    if(mContactImpl->isStarted() == false) {
        return elastos::ErrCode::NotReadyError;
    }

#ifdef WITH_CROSSPL
    auto msgInfo = dynamic_cast<ContactMessage*>(message);
#else
    auto msgInfo = message;
#endif // WITH_CROSSPL
    if(msgInfo == nullptr) {
        return elastos::ErrCode::InvalidArgument;
    }

    auto weakUserMgr = mContactImpl->getUserManager();
    auto userMgr =  SAFE_GET_PTR(weakUserMgr);
    auto weakFriendMgr = mContactImpl->getFriendManager();
    auto friendMgr =  SAFE_GET_PTR(weakFriendMgr);                                                                      \
    auto weakMsgMgr = mContactImpl->getMessageManager();
    auto msgMgr =  SAFE_GET_PTR(weakMsgMgr);                                                                      \

    std::shared_ptr<elastos::HumanInfo> humanInfo;
    if(std::string("-user-info-") == friendCode
    || userMgr->contains(friendCode) == true) {
        std::shared_ptr<elastos::UserInfo> userInfo;
        int ret = userMgr->getUserInfo(userInfo);
        CHECK_ERROR(ret);
        humanInfo = userInfo;
    } else if (friendMgr->contains(friendCode) == true) {
        std::shared_ptr<elastos::FriendInfo> friendInfo;
        int ret = friendMgr->tryGetFriendInfo(friendCode, friendInfo);
        CHECK_ERROR(ret);
        humanInfo = friendInfo;
    } else {
        return elastos::ErrCode::NotFoundError;
    }

    int ret = msgMgr->sendMessage(humanInfo, static_cast<elastos::MessageManager::ChannelType>(chType), msgInfo->mMessageInfo);
    CHECK_ERROR(ret);

    return ret;
}

int ContactBridge::pullData(ConstStringPtr humanCode, ChannelType chType,
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

int ContactBridge::cancelPullData(ConstStringPtr humanCode, ChannelType chType,
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

int ContactBridge::importUserData(ConstStringPtr fromFile)
{
//    if(mContactImpl->isStarted() == false) {
//        return elastos::ErrCode::NotReadyError;
//    }
    int ret = mContactImpl->importUserData(fromFile);
    CHECK_ERROR(ret);

    return 0;
}

int ContactBridge::exportUserData(ConstStringPtr toFile)
{
//    if(mContactImpl->isStarted() == false) {
//        return elastos::ErrCode::NotReadyError;
//    }
    int ret = mContactImpl->exportUserData(toFile);
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
