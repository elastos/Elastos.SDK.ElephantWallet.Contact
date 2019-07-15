//
//  Elastos.SDK.Contact.cpp
//
//  Created by mengxk on 19/03/16.
//  Copyright © 2016 mengxk. All rights reserved.
//

#include <ContactListener.hpp>

#include <ContactListener.proxy.hpp>
#include "Log.hpp"

/***********************************************/
/***** static variables initialize *************/
/***********************************************/
ContactListener* ContactListener::sContactListenerInstance = nullptr;

/***********************************************/
/***** static function implement ***************/
/***********************************************/

/***********************************************/
/***** class public function implement  ********/
/***********************************************/
ContactListener::ContactListener()
        : mSecurityListener()
        , mMessageListener()
{
    Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);

    sContactListenerInstance = this;

    mSecurityListener = makeSecurityListener();
    mMessageListener = makeMessageListener();
}
ContactListener::~ContactListener()
{
    Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);
    if(sContactListenerInstance == this) {
        sContactListenerInstance = nullptr;
    }
}

std::shared_ptr<elastos::SecurityManager::SecurityListener> ContactListener::getSecurityListener()
{
    return mSecurityListener;
}
std::shared_ptr<elastos::MessageManager::MessageListener> ContactListener::getMessageListener()
{
    return mMessageListener;
}

/***********************************************/
/***** class protected function implement  *****/
/***********************************************/


/***********************************************/
/***** class private function implement  *******/
/***********************************************/
std::shared_ptr<elastos::SecurityManager::SecurityListener> ContactListener::makeSecurityListener()
{
    class SecurityListener final : public elastos::SecurityManager::SecurityListener {
    public:
        explicit SecurityListener() = default;
        virtual ~SecurityListener() = default;

        std::string onAcquirePublicKey() override {
            Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);
            auto ret = sContactListenerInstance->onAcquire(AcquireType::PublicKey, nullptr, nullptr);
            if(ret.get() == nullptr) {
                return "";
            }

            std::string pubKey(reinterpret_cast<char*>(ret->data()));
            return pubKey;
        }

        std::vector<uint8_t> onEncryptData(const std::string& pubKey, const std::vector<uint8_t>& src) override {
            Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);
            const std::span<uint8_t> data(const_cast<uint8_t*>(src.data()), src.size());
            auto ret = sContactListenerInstance->onAcquire(AcquireType::EncryptData, pubKey.c_str(), &data);
            if(ret.get() == nullptr) {
                return std::vector<uint8_t>();
            }

            std::vector<uint8_t> cryptoData(ret->data(), ret->data() + ret->size());
            return cryptoData;
        }
        std::vector<uint8_t> onDecryptData(const std::vector<uint8_t>& src) override {
            Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);
            const std::span<uint8_t> data(const_cast<uint8_t*>(src.data()), src.size());
            auto ret = sContactListenerInstance->onAcquire(AcquireType::DecryptData, nullptr, &data);
            if(ret.get() == nullptr) {
                return std::vector<uint8_t>();
            }

            std::vector<uint8_t> cryptoData(ret->data(), ret->data() + ret->size());
            return cryptoData;
        }

        std::string onAcquireDidPropAppId() override {
            Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);
            auto ret = sContactListenerInstance->onAcquire(AcquireType::DidPropAppId, nullptr, nullptr);
            if(ret.get() == nullptr) {
                return "";
            }

            std::string appId(reinterpret_cast<char*>(ret->data()));
            return appId;
        }

        std::string onAcquireDidAgentAuthHeader() override {
            Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);
            auto ret = sContactListenerInstance->onAcquire(AcquireType::DidAgentAuthHeader, nullptr, nullptr);
            if(ret.get() == nullptr) {
                return "";
            }

            std::string authHeader(reinterpret_cast<char*>(ret->data()));
            return authHeader;
        }

        std::vector<uint8_t> onSignData(const std::vector<uint8_t>& originData) override {
            Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);
            const std::span<uint8_t> data(const_cast<uint8_t*>(originData.data()), originData.size());
            auto ret = sContactListenerInstance->onAcquire(AcquireType::DecryptData, nullptr, &data);
            if(ret.get() == nullptr) {
                return std::vector<uint8_t>();
            }

            std::vector<uint8_t> unsignedData(ret->data(), ret->data() + ret->size());
            return unsignedData;
        }
    };

    return std::make_shared<SecurityListener>();
}

std::shared_ptr<elastos::MessageManager::MessageListener> ContactListener::makeMessageListener()
{
    class MessageListener final : public elastos::MessageManager::MessageListener {
    public:
        explicit MessageListener() = default;
        virtual ~MessageListener() = default;

        virtual void onStatusChanged(std::shared_ptr<elastos::UserInfo> userInfo,
                                     elastos::MessageManager::ChannelType channelType,
                                     elastos::UserInfo::Status status) override {
            Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);
            std::string humainCode;
            userInfo->getHumanCode(humainCode);
            std::span<uint8_t> data {reinterpret_cast<uint8_t*>(&status), 1 };
            sContactListenerInstance->onEvent(EventType::StatusChanged, humainCode,
                                              static_cast<ContactChannel>(channelType), &data);
        }

        virtual void onReceivedMessage(std::shared_ptr<elastos::HumanInfo> humanInfo,
                                       elastos::MessageManager::ChannelType channelType,
                                       const std::shared_ptr<elastos::MessageManager::MessageInfo> msgInfo) override {
            Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);
        }

        virtual void onSentMessage(int msgIndex, int errCode) override {
            Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);
        }

        virtual void onFriendRequest(std::shared_ptr<elastos::FriendInfo> friendInfo,
                                     elastos::MessageManager::ChannelType channelType,
                                     const std::string& summary) override {
            Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);
            std::string humainCode;
            friendInfo->getHumanCode(humainCode);
            std::span<uint8_t> data {reinterpret_cast<uint8_t*>(const_cast<char*>(summary.c_str())),
                                     summary.length() };
            sContactListenerInstance->onEvent(EventType::FriendReuqest, humainCode,
                                              static_cast<ContactChannel>(channelType), &data);
        }

        virtual void onFriendStatusChanged(std::shared_ptr<elastos::FriendInfo> friendInfo,
                                           elastos::MessageManager::ChannelType channelType,
                                           elastos::FriendInfo::Status status) override {
            Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);
            std::string humainCode;
            friendInfo->getHumanCode(humainCode);
            std::span<uint8_t> data {reinterpret_cast<uint8_t*>(&status), 1 };
            sContactListenerInstance->onEvent(EventType::StatusChanged, humainCode,
                                              static_cast<ContactChannel>(channelType), &data);
        }
    };

    return std::make_shared<MessageListener>();
}

std::shared_ptr<std::span<uint8_t>> ContactListener::onAcquire(AcquireType type,
                                                              const char* pubKey,
                                                              const std::span<uint8_t>* data)
{
    int64_t platformHandle = getPlatformHandle();
    auto ret = crosspl::proxy::ContactListener::onAcquire(platformHandle,
                                                          static_cast<int>(type), pubKey, data);

    return ret;
}

void ContactListener::onEvent(EventType type,
                              const std::string& humanCode,
                              ContactChannel channelType,
                              const std::span<uint8_t>* data)
{
    int64_t platformHandle = getPlatformHandle();
    crosspl::proxy::ContactListener::onEvent(platformHandle,
                                             static_cast<int>(type), humanCode.c_str(), static_cast<int>(channelType), data);
    return;
}