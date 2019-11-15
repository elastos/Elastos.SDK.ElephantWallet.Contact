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

#include "CrossBase.hpp"
#include <Elastos.SDK.Contact.hpp>
#include "experimental-span.hpp"

#ifdef WITH_CROSSPL
namespace crosspl {
namespace native {
#endif // WITH_CROSSPL

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
    };

    enum class ContactChannel: int { //TODO: Same as elastos::MessageManager::ChannelType, need merge to one.
        Carrier = 1,
        ElaChain = 2,
        Email = 3,
    };

    /*** static function and variable ***/

    /*** class function and variable ***/
    explicit ContactListener();
    virtual ~ContactListener();

#ifdef WITH_CROSSPL
    std::shared_ptr<std::span<uint8_t>> onAcquire(AcquireType type, const char* pubKey, const std::span<uint8_t>* data);
    void onEvent(EventType type, const std::string& humanCode, ContactChannel channelType, const std::span<uint8_t>* data);
    void onReceivedMessage(const std::string& humanCode, ContactChannel channelType,
                           std::shared_ptr<elastos::MessageManager::MessageInfo> msgInfo);
    void onError(int errCode, const std::string& errStr, const std::string& ext);
#else
    virtual std::shared_ptr<std::vector<uint8_t>> onAcquire(AcquireType type, const std::string& pubKey,
                                                            const std::vector<uint8_t>& data) = 0;
    virtual void onEvent(EventType type, const std::string& humanCode, ContactChannel channelType,
                         const std::vector<uint8_t>& data) = 0;
    virtual void onReceivedMessage(const std::string& humanCode, ContactChannel channelType,
                                   std::shared_ptr<elastos::MessageManager::MessageInfo> msgInfo) = 0;
    virtual void onError(int errCode, const std::string& errStr, const std::string& ext) = 0;
#endif // WITH_CROSSPL

    std::shared_ptr<elastos::SecurityManager::SecurityListener> getSecurityListener();
    std::shared_ptr<elastos::MessageManager::MessageListener> getMessageListener();

private:
    /*** type define ***/

    /*** static function and variable ***/
    static ContactListener* sContactListenerInstance;

    /*** class function and variable ***/
    std::shared_ptr<elastos::SecurityManager::SecurityListener> makeSecurityListener();
    std::shared_ptr<elastos::MessageManager::MessageListener> makeMessageListener();

    std::shared_ptr<elastos::SecurityManager::SecurityListener> mSecurityListener;
    std::shared_ptr<elastos::MessageManager::MessageListener> mMessageListener;
}; // class Contact

#ifdef WITH_CROSSPL
} //namespace native
} //namespace crosspl
#endif // WITH_CROSSPL

#endif /* _ELASTOS_SDK_JNI_CONTACT_LISTENER_HPP_ */
