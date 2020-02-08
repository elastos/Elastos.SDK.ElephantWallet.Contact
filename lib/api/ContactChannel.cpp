//
//  Elastos.SDK.Contact.cpp
//
//  Created by mengxk on 19/03/16.
//  Copyright © 2016 mengxk. All rights reserved.
//

#include <ContactChannel.hpp>
#include "Log.hpp"

#ifdef WITH_CROSSPL
#define ENABLE_PLATFORM_FUNCTION
#include <ContactChannel.proxy.h>
#endif // WITH_CROSSPL

namespace crosspl {
namespace native {

/***********************************************/
/***** static variables initialize *************/
/***********************************************/
//ContactChannel* ContactChannel::sContactChannelInstance = nullptr;

/***********************************************/
/***** static function implement ***************/
/***********************************************/

/***********************************************/
/***** class public function implement  ********/
/***********************************************/
ContactChannel::ContactChannel()
//        : mChannel()
{
    Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);

//    sContactChannelInstance = this;
//
//    mChannel = makeChannel();
}
ContactChannel::~ContactChannel()
{
    Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);
//    if(sContactChannelInstance == this) {
//        sContactChannelInstance = nullptr;
//    }
}

//std::shared_ptr<elastos::MessageManager::Channel> ContactChannel::getChannel()
//{
//    return mChannel;
//}

/***********************************************/
/***** class protected function implement  *****/
/***********************************************/


/***********************************************/
/***** class private function implement  *******/
/***********************************************/
//std::shared_ptr<elastos::MessageManager::Channel> ContactChannel::makeChannel()
//{
//    class Channel final : public elastos::MessageManager::Channel {
//    public:
//        explicit Channel() = default;
//        virtual ~Channel() = default;
//
//        virtual void onNotify(std::shared_ptr<elastos::HumanInfo> humanInfo,
//                              elastos::MessageManager::ChannelType channelType,
//                              const std::string& dataId,
//                              int notify) override {
//            Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);
//            std::string humanCode;
//            int ret = humanInfo->getHumanCode(humanCode);
//            CHECK_RETVAL(ret);
//
//            sContactChannelInstance->onNotify(humanCode,
//                                                   static_cast<ChannelType>(channelType),
//                                                   dataId, notify);
//        }
//
//        virtual int onReadData(std::shared_ptr<elastos::HumanInfo> humanInfo,
//                               elastos::MessageManager::ChannelType channelType,
//                               const std::string& dataId, uint64_t offset,
//                               std::vector<uint8_t>& data) override {
//            Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);
//            std::string humanCode;
//            int ret = humanInfo->getHumanCode(humanCode);
//            CHECK_ERROR(ret);
//
//#ifdef WITH_CROSSPL
//            auto readData = sContactChannelInstance->onReadData(humanCode,
//                                                                     static_cast<ChannelType>(channelType),
//                                                                     dataId, offset);
//            if(readData.get() == nullptr) {
//                return elastos::ErrCode::ChannelFailedReadData;
//            }
//
//            data.clear();
//            data.insert(data.begin(), readData->data(), readData->data() + readData->size());
//            ret = data.size();
//#else
//            ret = sContactChannelInstance->onReadData(humanCode,
//                                                           static_cast<ChannelType>(channelType),
//                                                           dataId, offset, data);
//#endif // WITH_CROSSPL
//
//            return ret;
//        }
//
//        virtual int onWriteData(std::shared_ptr<elastos::HumanInfo> humanInfo,
//                                elastos::MessageManager::ChannelType channelType,
//                                const std::string& dataId, uint64_t offset,
//                                const std::vector<uint8_t>& data) override {
//            Log::I(Log::TAG, "%s", __PRETTY_FUNCTION__);
//            std::string humanCode;
//            int ret = humanInfo->getHumanCode(humanCode);
//            CHECK_ERROR(ret);
//
//#ifdef WITH_CROSSPL
//            std::span<uint8_t> writeData(reinterpret_cast<uint8_t*>(const_cast<uint8_t*>(data.data())), data.size());
//            ret = sContactChannelInstance->onWriteData(humanCode,
//                                                            static_cast<ChannelType>(channelType),
//                                                            dataId, offset, &writeData);
//#else
//            ret = sContactChannelInstance->onWriteData(humanCode,
//                                                            static_cast<ChannelType>(channelType),
//                                                            dataId, offset, data);
//#endif // WITH_CROSSPL
//            return ret;
//        }
//    };
//
//    return std::make_shared<Channel>();
//}

#ifdef WITH_CROSSPL
//void ContactChannel::onNotify(const std::string& humanCode,
//                                   ChannelType channelType,
//                                   const std::string& dataId,
//                                   int status)
//{
//    int64_t platformHandle = getPlatformHandle();
//    crosspl_Proxy_ContactChannel_onNotify(platformHandle,
//                                               humanCode.c_str(), static_cast<int>(channelType),
//                                               dataId.c_str(), status);
//}
//
//std::shared_ptr<std::span<uint8_t>> ContactChannel::onReadData(const std::string& humanCode,
//                                                                    ChannelType channelType,
//                                                                    const std::string& dataId,
//                                                                    uint64_t offset)
//{
//    int64_t platformHandle = getPlatformHandle();
//    auto ret = crosspl_Proxy_ContactChannel_onReadData(platformHandle,
//                                                            humanCode.c_str(), static_cast<int>(channelType),
//                                                            dataId.c_str(), offset);
//
//    return ret;
//}
//
//int ContactChannel::onWriteData(const std::string& humanCode,
//                                     ChannelType channelType,
//                                     const std::string& dataId,
//                                     uint64_t offset,
//                                     const std::span<uint8_t>* data)
//{
//    int64_t platformHandle = getPlatformHandle();
//    auto ret = crosspl_Proxy_ContactChannel_onWriteData(platformHandle,
//                                                             humanCode.c_str(), static_cast<int>(channelType),
//                                                             dataId.c_str(), offset, data);
//
//    return ret;
//}
#endif // WITH_CROSSPL

} //namespace native
} //namespace crosspl
