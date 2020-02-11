//
//  Elastos.SDK.Contact.cpp
//
//  Created by mengxk on 19/03/16.
//  Copyright © 2016 mengxk. All rights reserved.
//

#include <ContactChannelStrategy.hpp>
#include <ContactListener.hpp>
#include "Log.hpp"
#include "../include/ErrCode.hpp"

#ifdef WITH_CROSSPL
#define ENABLE_PLATFORM_FUNCTION
#include <ContactChannelStrategy.proxy.h>
#endif // WITH_CROSSPL

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
ContactChannelStrategy::ContactChannelStrategy()
        : mMutex(std::make_shared<std::recursive_mutex>())
{
}
ContactChannelStrategy::~ContactChannelStrategy()
{
    auto dataHelper = std::dynamic_pointer_cast<ContactListener::Helper<ContactChannelStrategy>>(mChannelStrategy);
    dataHelper->resetHelperPointer();
}

int ContactChannelStrategy::createChannel(uint32_t channelId, std::shared_ptr<elastos::MessageManager> msgMgr)
{
    std::lock_guard<std::recursive_mutex> lock(*mMutex);
    mChannelStrategy = makeChannelStrategy(channelId, msgMgr);

    return 0;
}

std::shared_ptr<elastos::MessageChannelStrategy> ContactChannelStrategy::getChannel()
{
    return mChannelStrategy;
}

int ContactChannelStrategy::receivedMessage(const std::string& humanCode,
                                            int channelId,
                                            ConstBytesPtr data)
{
    Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);
    LOCK_PTR(mMutex, mChannelStrategy, elastos::ErrCode::PointerReleasedError);

#ifdef WITH_CROSSPL
    const auto msgData = std::vector<uint8_t>(data->data(), data->data() + data->size());
#else
    const auto& msgData = data;
#endif // WITH_CROSSPL

    auto listener = mChannelStrategy->getChannelListener();
    listener->onReceivedMessage(humanCode, static_cast<uint32_t>(channelId), msgData);

    return 0;
}

/***********************************************/
/***** class protected function implement  *****/
/***********************************************/


/***********************************************/
/***** class private function implement  *******/
/***********************************************/
std::shared_ptr<elastos::MessageChannelStrategy> ContactChannelStrategy::makeChannelStrategy(uint32_t chType,
                                                                                             std::shared_ptr<elastos::MessageManager> msgMgr)
{
    class ChannelStrategy final : public elastos::MessageChannelStrategy
                                , public ContactListener::Helper<ContactChannelStrategy> {
    public:
        explicit ChannelStrategy(std::shared_ptr<std::recursive_mutex> mutex,
                                 uint32_t chType)
                : MessageChannelStrategy(chType)
                , Helper(mutex) {
        }
        virtual ~ChannelStrategy() = default;

        virtual int preset(const std::string& profile,
                           std::shared_ptr<ChannelListener> chListener,
                           std::shared_ptr<ChannelDataListener> dataListener) override {
            Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);
            return MessageChannelStrategy::preset(profile, chListener, dataListener);
        };
        virtual int open() override {
            Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);
            LOCK_PTR(mMutex, mHelperPtr, elastos::ErrCode::PointerReleasedError);
            int ret = mHelperPtr->onOpen();
            return ret;
        };
        virtual int close() override {
            Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);
            LOCK_PTR(mMutex, mHelperPtr, elastos::ErrCode::PointerReleasedError);
            int ret = mHelperPtr->onClose();
            return ret;
        };
        virtual int getAddress(std::string& address) override {
            Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);
            return elastos::ErrCode::UnimplementedError;
        };
        virtual bool isReady() override {
            Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);
            return true;
        };
        virtual int requestFriend(const std::string& friendAddr,
                                  const std::string& summary,
                                  bool remoteRequest = true,
                                  bool forceRequest = false) override {
            Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);
            return elastos::ErrCode::UnimplementedError;
        };
        virtual int removeFriend(const std::string& friendAddr) override {
            Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);
            return elastos::ErrCode::UnimplementedError;
        };
        virtual int sendMessage(const std::string& friendCode,
                                const std::vector<uint8_t>& msgContent,
                                bool ignorePackData = false) override {
            Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);
            LOCK_PTR(mMutex, mHelperPtr, elastos::ErrCode::PointerReleasedError);
#ifdef WITH_CROSSPL
            auto data = std::span<uint8_t>(const_cast<uint8_t*>(msgContent.data()), msgContent.size());
            const auto msgData = &data;
#else
            const auto& msgData = msgContent;
#endif // WITH_CROSSPL
            int ret = mHelperPtr->onSendMessage(friendCode, static_cast<int>(this->getChannelType()), msgData);
            return ret;
        };
    };

    auto channelStrategy = std::make_shared<ChannelStrategy>(mMutex, chType);
    channelStrategy->resetHelperPointer(this);
    return channelStrategy;
}


#ifdef WITH_CROSSPL
int ContactChannelStrategy::onOpen()
{
    int64_t platformHandle = getPlatformHandle();
    auto ret = crosspl_Proxy_ContactChannelStrategy_onOpen(platformHandle);
    return ret;
}

int ContactChannelStrategy::onClose()
{
    int64_t platformHandle = getPlatformHandle();
    auto ret = crosspl_Proxy_ContactChannelStrategy_onClose(platformHandle);
    return ret;
}

int ContactChannelStrategy::onSendMessage(const std::string& humanCode,
                                          int channelId,
                                          const std::span<uint8_t>* data)
{
    int64_t platformHandle = getPlatformHandle();
    auto ret = crosspl_Proxy_ContactChannelStrategy_onSendMessage(platformHandle,
                                                                  humanCode.c_str(),
                                                                  channelId,
                                                                  data);
    return ret;
}
#endif // WITH_CROSSPL


} //namespace native
} //namespace crosspl
