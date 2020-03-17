//
//  Elastos.SDK.Contact.cpp
//
//  Created by mengxk on 19/03/16.
//  Copyright © 2016 mengxk. All rights reserved.
//

#include <ContactDataListener.hpp>
#include <ContactListener.hpp>
#include "Log.hpp"

#ifdef WITH_CROSSPL
#define ENABLE_PLATFORM_FUNCTION
#include <ContactDataListener.proxy.h>
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
ContactDataListener::ContactDataListener()
        : mMutex(std::make_shared<std::recursive_mutex>())
{
    mDataListener = makeDataListener();
}
ContactDataListener::~ContactDataListener()
{
    auto dataHelper = std::dynamic_pointer_cast<ContactListener::Helper<ContactDataListener>>(mDataListener);
    dataHelper->resetHelperPointer();
}

std::shared_ptr<elastos::MessageManager::DataListener> ContactDataListener::getDataListener()
{
    return mDataListener;
}

/***********************************************/
/***** class protected function implement  *****/
/***********************************************/


/***********************************************/
/***** class private function implement  *******/
/***********************************************/
std::shared_ptr<elastos::MessageManager::DataListener> ContactDataListener::makeDataListener()
{
    class DataListener final : public elastos::MessageManager::DataListener
                             , public ContactListener::Helper<ContactDataListener> {
    public:
        explicit DataListener(std::shared_ptr<std::recursive_mutex> mutex)
                : elastos::MessageManager::DataListener()
                , ContactListener::Helper<ContactDataListener>(mutex) {
        };
        virtual ~DataListener() = default;

        virtual void onNotify(std::shared_ptr<elastos::HumanInfo> humanInfo,
                              elastos::MessageManager::ChannelType channelType,
                              const std::string& dataId,
                              int notify) override {
            Log::I(Log::TAG, FORMAT_METHOD);
            std::string humanCode;
            int ret = humanInfo->getHumanCode(humanCode);
            CHECK_RETVAL(ret);

            LOCK_PTR(mMutex, mHelperPtr, );
            mHelperPtr->onNotify(humanCode,
                                      static_cast<ChannelType>(channelType),
                                      dataId, notify);
        }

        virtual int onReadData(std::shared_ptr<elastos::HumanInfo> humanInfo,
                               elastos::MessageManager::ChannelType channelType,
                               const std::string& dataId, uint64_t offset,
                               std::vector<uint8_t>& data) override {
            Log::I(Log::TAG, FORMAT_METHOD);
            std::string humanCode;
            int ret = humanInfo->getHumanCode(humanCode);
            CHECK_ERROR(ret);

            LOCK_PTR(mMutex, mHelperPtr, elastos::ErrCode::PointerReleasedError);
#ifdef WITH_CROSSPL
            auto readData = mHelperPtr->onReadData(humanCode,
                                                   static_cast<ChannelType>(channelType),
                                                   dataId, offset);
            if(readData.get() == nullptr) {
                return elastos::ErrCode::ChannelFailedReadData;
            }

            data.clear();
            data.insert(data.begin(), readData->data(), readData->data() + readData->size());
            ret = data.size();
#else
            ret = mHelperPtr->onReadData(humanCode,
                                              static_cast<ChannelType>(channelType),
                                              dataId, offset, data);
#endif // WITH_CROSSPL

            return ret;
        }

        virtual int onWriteData(std::shared_ptr<elastos::HumanInfo> humanInfo,
                                elastos::MessageManager::ChannelType channelType,
                                const std::string& dataId, uint64_t offset,
                                const std::vector<uint8_t>& data) override {
            Log::I(Log::TAG, FORMAT_METHOD);
            std::string humanCode;
            int ret = humanInfo->getHumanCode(humanCode);
            CHECK_ERROR(ret);

            LOCK_PTR(mMutex, mHelperPtr, elastos::ErrCode::PointerReleasedError);
#ifdef WITH_CROSSPL
            std::span<uint8_t> writeData(reinterpret_cast<uint8_t*>(const_cast<uint8_t*>(data.data())), data.size());
            ret = mHelperPtr->onWriteData(humanCode,
                                               static_cast<ChannelType>(channelType),
                                               dataId, offset, &writeData);
#else
            ret = mHelperPtr->onWriteData(humanCode,
                                               static_cast<ChannelType>(channelType),
                                               dataId, offset, data);
#endif // WITH_CROSSPL
            return ret;
        }
    };

    auto listener = std::make_shared<DataListener>(mMutex);
    listener->resetHelperPointer(this);
    return listener;
}

#ifdef WITH_CROSSPL
void ContactDataListener::onNotify(const std::string& humanCode,
                                   ChannelType channelType,
                                   const std::string& dataId,
                                   int status)
{
    int64_t platformHandle = getPlatformHandle();
    crosspl_Proxy_ContactDataListener_onNotify(platformHandle,
                                               humanCode.c_str(), static_cast<int>(channelType),
                                               dataId.c_str(), status);
}

std::shared_ptr<std::span<uint8_t>> ContactDataListener::onReadData(const std::string& humanCode,
                                                                    ChannelType channelType,
                                                                    const std::string& dataId,
                                                                    uint64_t offset)
{
    int64_t platformHandle = getPlatformHandle();
    auto ret = crosspl_Proxy_ContactDataListener_onReadData(platformHandle,
                                                            humanCode.c_str(), static_cast<int>(channelType),
                                                            dataId.c_str(), offset);

    return ret;
}

int ContactDataListener::onWriteData(const std::string& humanCode,
                                     ChannelType channelType,
                                     const std::string& dataId,
                                     uint64_t offset,
                                     const std::span<uint8_t>* data)
{
    int64_t platformHandle = getPlatformHandle();
    auto ret = crosspl_Proxy_ContactDataListener_onWriteData(platformHandle,
                                                             humanCode.c_str(), static_cast<int>(channelType),
                                                             dataId.c_str(), offset, data);

    return ret;
}
#endif // WITH_CROSSPL

} //namespace native
} //namespace crosspl
