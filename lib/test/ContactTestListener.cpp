#include <ContactTestListener.hpp>

#include "ContactTestCmd.hpp"
#include <iostream>
#include <Log.hpp>

/* =========================================== */
/* === static variables initialize =========== */
/* =========================================== */

/* =========================================== */
/* === static function implement ============= */
/* =========================================== */

/* =========================================== */
/* === class public function implement  ====== */
/* =========================================== */
void ContactTestListener::onStatusChanged(std::shared_ptr<elastos::UserInfo> userInfo,
                                          elastos::MessageManager::ChannelType channelType,
                                          elastos::UserInfo::Status status)
{
    std::cout << "ContactTestListener ChannelType:" << static_cast<int>(channelType)
              << " Status:" << static_cast<int>(status) << std::endl;
}

void ContactTestListener::onReceivedMessage(std::shared_ptr<elastos::HumanInfo> humanInfo,
                                            elastos::MessageManager::ChannelType channelType,
                                            const std::shared_ptr<elastos::MessageManager::MessageInfo> msgInfo)
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
}

void ContactTestListener::onSentMessage(int msgIndex, int errCode)
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
}

void ContactTestListener::onFriendRequest(std::shared_ptr<elastos::FriendInfo> friendInfo,
                                          elastos::MessageManager::ChannelType channelType,
                                          const std::string& summary)
{
    std::cout << __PRETTY_FUNCTION__ << " summary:" << summary << std::endl;

    std::string errMsg;
    ContactTestCmd::Do("print-info", errMsg);

    std::weak_ptr<elastos::FriendManager> friendMgr = mContact->getFriendManager();
    int ret = friendMgr.lock()->acceptFriend(friendInfo);
    if(ret < 0) {
        Log::E(Log::TAG, "Failed to accept friend. ret = %d", ret);
        return;
    }

    ContactTestCmd::Do("print-info", errMsg);
}

void ContactTestListener::onFriendStatusChanged(std::shared_ptr<elastos::FriendInfo> friendInfo,
                                                elastos::MessageManager::ChannelType channelType,
                                                elastos::FriendInfo::Status status)
{
    std::cout << "ContactTestListener ChannelType:" << static_cast<int>(channelType)
              << " FriendStatus:" << static_cast<int>(status) << std::endl;
}

void ContactTestListener::onHumanInfoChanged(std::shared_ptr<elastos::HumanInfo> humanInfo,
                                             elastos::MessageManager::ChannelType channelType)
{
    std::cout << "ContactTestListener ChannelType:" << static_cast<int>(channelType) << std::endl;
}

/* =========================================== */
/* === class protected function implement  === */
/* =========================================== */

/* =========================================== */
/* === class private function implement  ===== */
/* =========================================== */
