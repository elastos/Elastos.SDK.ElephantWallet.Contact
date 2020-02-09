//
//  Elastos.SDK.Contact.cpp
//
//  Created by mengxk on 19/03/16.
//  Copyright © 2016 mengxk. All rights reserved.
//

#include <ContactChannelStrategy.hpp>
#include <ContactListener.hpp>
#include "Log.hpp"

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
//ContactChannelStrategy::ContactChannelStrategy()
//        : mMutex(std::make_shared<std::recursive_mutex>())
//{
//    mDataListener = makeDataListener();
//}
//ContactChannelStrategy::~ContactChannelStrategy()
//{
//    auto dataHelper = std::dynamic_pointer_cast<ContactListener::Helper<ContactChannelStrategy>>(mDataListener);
//    dataHelper->resetContactListener();
//}
//
//std::shared_ptr<elastos::MessageManager::DataListener> ContactChannelStrategy::getDataListener()
//{
//    return mDataListener;
//}

/***********************************************/
/***** class protected function implement  *****/
/***********************************************/


/***********************************************/
/***** class private function implement  *******/
/***********************************************/
//std::shared_ptr<elastos::MessageManager::DataListener> ContactChannelStrategy::makeDataListener()
//{
//    class DataListener final : public elastos::MessageManager::DataListener
//                             , public ContactListener::Helper<ContactChannelStrategy> {
//    public:
//        explicit DataListener(std::shared_ptr<std::recursive_mutex> mutex)
//                : elastos::MessageManager::DataListener()
//                , ContactListener::Helper<ContactChannelStrategy>(mutex) {
//        };
//        virtual ~DataListener() = default;
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
//            LOCK_PTR(mMutex, mHelperListener, );
//            mHelperListener->onNotify(humanCode,
//                                      static_cast<ChannelType>(channelType),
//                                      dataId, notify);
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
//            LOCK_PTR(mMutex, mHelperListener, elastos::ErrCode::PointerReleasedError);
//#ifdef WITH_CROSSPL
//            auto readData = mHelperListener->onReadData(humanCode,
//                                                        static_cast<ChannelType>(channelType),
//                                                        dataId, offset);
//            if(readData.get() == nullptr) {
//                return elastos::ErrCode::ChannelFailedReadData;
//            }
//
//            data.clear();
//            data.insert(data.begin(), readData->data(), readData->data() + readData->size());
//            ret = data.size();
//#else
//            ret = mHelperListener->onReadData(humanCode,
//                                              static_cast<ChannelType>(channelType),
//                                              dataId, offset, data);
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
//            LOCK_PTR(mMutex, mHelperListener, elastos::ErrCode::PointerReleasedError);
//#ifdef WITH_CROSSPL
//            std::span<uint8_t> writeData(reinterpret_cast<uint8_t*>(const_cast<uint8_t*>(data.data())), data.size());
//            ret = mHelperListener->onWriteData(humanCode,
//                                               static_cast<ChannelType>(channelType),
//                                               dataId, offset, &writeData);
//#else
//            ret = mHelperListener->onWriteData(humanCode,
//                                               static_cast<ChannelType>(channelType),
//                                               dataId, offset, data);
//#endif // WITH_CROSSPL
//            return ret;
//        }
//    };
//
//    auto listener = std::make_shared<DataListener>(mMutex);
//    listener->resetContactListener(this);
//    return listener;
//}
//

} //namespace native
} //namespace crosspl
