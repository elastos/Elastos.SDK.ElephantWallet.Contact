/**
 * @file	ContactListener.hpp
 * @brief	Contact
 * @details
 *
 * @author	xxx
 * @author	<xxx@xxx.com>
 * @copyright	(c) 2012 xxx All rights reserved.
 **/

#ifndef _ELASTOS_SDK_JNI_CONTACT_LISTENER_HPP_
#define _ELASTOS_SDK_JNI_CONTACT_LISTENER_HPP_

#include <mutex>

#include "ContactTypes.hpp"
#include <Contact.V1.hpp>
#include "experimental-span.hpp"

namespace crosspl {
namespace native {

#define LOCK_PTR(mutex, ptr, retval)                                                 \
    std::lock_guard<std::recursive_mutex> lock(*mutex);                              \
    if(ptr == nullptr) {                                                             \
        Log::W(Log::TAG, #ptr " has been released at %s(%d)", __FILE__, __LINE__);   \
        return retval;                                                               \
    }


class ContactListener : public CrossBase {
public:
    /*** type define ***/
    enum class AcquireType: int {
        PublicKey = 201,
        EncryptData = 202,
        DecryptData = 203,
        DidPropAppId = 204,
        DidAgentAuthHeader = 205,
        SignData = 206,
    };

    enum class EventType: int {
        StatusChanged = 101,
//        ReceivedMessage = 102,
//        SentMessage = 103,
        FriendRequest = 104,
//        FriendStatusChanged = 105,
        HumanInfoChanged = 106,
        MessageAck = 107,
    };

    class AcquireArgs {
    public:
        using Type = AcquireType;

        explicit AcquireArgs(Type type, const std::string& pubKey,
                             const std::vector<uint8_t>& data, const std::string& extra) {
            this->type = type;
            this->publicKey = pubKey;
            this->data = std::move(data);
            this->extra = extra;
        }
        virtual ~AcquireArgs() = default;

        std::string toString() const {
            return std::string("AcquireArgs")
                   + "[type=" + std::to_string(static_cast<int>(type))
                   + ",publicKey=" + publicKey
                   + ",data={unprintable}"
                   + ",extra=" + extra
                   +"]";
        }

        Type type;
        std::string publicKey;
        std::vector<uint8_t> data;
        std::string extra;
    };

    class EventArgs {
    public:
        using Type = EventType;

        explicit EventArgs(Type type, const std::string& humanCode, ChannelType channelType)
                : type(type)
                , humanCode(humanCode)
                , channelType(channelType) {
        }
        virtual ~EventArgs() = default;

        virtual std::string toString() const {
            return std::string("EventArgs")
                   + "[type=" + std::to_string(static_cast<int>(type))
                   + ",humanCode=" + humanCode
                   + ",channelType=" + std::to_string(static_cast<int>(channelType))
                   +"]";
        }

        Type type;
        std::string humanCode;
        ChannelType channelType;
    };

    class StatusEvent: public EventArgs {
    public:
        using Status = elastos::HumanInfo::Status;
        explicit StatusEvent(Type type, const std::string& humanCode, ChannelType channelType, Status status)
                : EventArgs(type, humanCode, channelType)
                , status(status) {
        }
        virtual ~StatusEvent() = default;

        virtual std::string toString() const override {
            return std::string("StatusEvent")
                   + "[status=" + std::to_string(static_cast<int>(status))
                   + ", " + EventArgs::toString()
                   +"]";
        }

        Status status;
    };

    class RequestEvent: public EventArgs {
    public:
        explicit RequestEvent(Type type, const std::string& humanCode, ChannelType channelType, const std::string& summary)
                : EventArgs(type, humanCode, channelType)
                , summary(summary) {
        }
        virtual ~RequestEvent() = default;

        virtual std::string toString() const override  {
            return std::string("RequestEvent")
                   + "[summary=" + summary
                   + ", " + EventArgs::toString()
                   +"]";
        }

        std::string summary;
    };

    class InfoEvent: public EventArgs {
    public:
        explicit InfoEvent(Type type, const std::string& humanCode, ChannelType channelType, std::shared_ptr<elastos::HumanInfo> humanInfo)
                : EventArgs(type, humanCode, channelType) {
            this->humanInfo = humanInfo;
        }
        virtual ~InfoEvent() = default;

        virtual std::string toString() const override {
            auto jsonInfo = std::make_shared<elastos::Json>();
            int ret = humanInfo->toJson(jsonInfo);
            if(ret < 0) {
                return "Error toString";
            }

            return std::string("RequestEvent")
                   + "[humanInfo=" + jsonInfo->dump(2)
                   + ", " + EventArgs::toString()
                   +"]";
        }

        std::shared_ptr<elastos::HumanInfo> humanInfo;
    };

    class MsgAckEvent: public EventArgs {
    public:
        explicit MsgAckEvent(Type type, const std::string& humanCode,
                             ChannelType channelType, const std::string& memo, int64_t ackToNanoTime)
                : EventArgs(type, humanCode, channelType)
                , memo(memo)
                , ackToNanoTime(ackToNanoTime) {
        }
        virtual ~MsgAckEvent() = default;

        virtual std::string toString() const override  {
            return std::string("MsgAckEvent")
                   + "[memo=" + memo
                   + ", ackToNanoTime=" + std::to_string(ackToNanoTime)
                   + ", " + EventArgs::toString()
                   +"]";
        }

        std::string memo;
        int64_t ackToNanoTime;
    };

    template <class T>
    class Helper {
    public:
        explicit Helper(std::shared_ptr<std::recursive_mutex> mutex)
                : mMutex(mutex)
                , mHelperPtr(nullptr) {
        };
        virtual ~Helper() = default;
        void resetHelperPointer(T* ptr = nullptr) {
            std::lock_guard<std::recursive_mutex> lg(*mMutex);
            mHelperPtr = ptr;
        }

    protected:
        std::shared_ptr<std::recursive_mutex> mMutex;
        T* mHelperPtr;
    };

    /*** static function and variable ***/
    const std::string DIDPROP_APPID_DEFVAL = "DidFriend";

    /*** class function and variable ***/

#ifdef WITH_CROSSPL
    std::shared_ptr<std::span<uint8_t>> onAcquire(AcquireType type, const char* pubKey, const std::span<uint8_t>* data, const char* extra);
    void onEvent(EventType type, const std::string& humanCode, ChannelType channelType, const std::span<uint8_t>* data);
    void onReceivedMessage(const std::string& humanCode, ChannelType channelType,
                           std::shared_ptr<elastos::MessageManager::MessageInfo> msgInfo);
    void onError(int errCode, const std::string& errStr, const std::string& ext);
#else
    virtual std::shared_ptr<std::vector<uint8_t>> onAcquire(const AcquireArgs& request) = 0;
    virtual void onEvent(EventArgs& event) = 0;
    virtual void onReceivedMessage(const std::string& humanCode, ChannelType channelType,
                                   std::shared_ptr<elastos::MessageManager::MessageInfo> msgInfo) = 0;
    virtual void onError(int errCode, const std::string& errStr, const std::string& ext) = 0;
#endif // WITH_CROSSPL

    std::shared_ptr<elastos::SecurityManager::SecurityListener> getSecurityListener();
    std::shared_ptr<elastos::MessageManager::MessageListener> getMessageListener();

PERMISSION:
    explicit ContactListener();
    virtual ~ContactListener();

private:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/
    std::shared_ptr<elastos::SecurityManager::SecurityListener> makeSecurityListener();
    std::shared_ptr<elastos::MessageManager::MessageListener> makeMessageListener();

    std::shared_ptr<std::recursive_mutex> mMutex;
    std::shared_ptr<elastos::SecurityManager::SecurityListener> mSecurityListener;
    std::shared_ptr<elastos::MessageManager::MessageListener> mMessageListener;
}; // class Contact

} //namespace native
} //namespace crosspl

#endif /* _ELASTOS_SDK_JNI_CONTACT_LISTENER_HPP_ */
