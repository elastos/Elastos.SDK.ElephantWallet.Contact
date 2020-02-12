/**
 * @file	ContactDataListener.hpp
 * @brief	Contact
 * @details
 *
 * @author	xxx
 * @author	<xxx@xxx.com>
 * @copyright	(c) 2012 xxx All rights reserved.
 **/

#ifndef _ELASTOS_SDK_JNI_CONTACT_CHANNEL_STRATEGY_HPP_
#define _ELASTOS_SDK_JNI_CONTACT_CHANNEL_STRATEGY_HPP_

#include "ContactTypes.hpp"
#include <Contact.V1.hpp>
#include <MessageChannelStrategy.hpp>

namespace crosspl {
namespace native {

class ContactChannelStrategy : public CrossBase {
public:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/
    std::shared_ptr<elastos::MessageChannelStrategy> getChannel();
    ChannelType getChannelId();

    virtual int syncChannelToNative(int channelId, ConstStringPtr name);
    virtual int receivedMessage(const std::string& humanCode, ChannelType chType, ConstBytesPtr data);

    virtual int onOpen();
    virtual int onClose();
    virtual int onSendMessage(const std::string& humanCode, ChannelType chType, ConstBytesPtr data) ABSTRACT;

PERMISSION:
#ifdef WITH_CROSSPL
    explicit ContactChannelStrategy();
#else
    explicit ContactChannelStrategy(int channelId, const std::string& name);
#endif // WITH_CROSSPL

    virtual ~ContactChannelStrategy();

private:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/
    std::shared_ptr<elastos::MessageChannelStrategy> makeChannelStrategy(uint32_t chType);
//
    std::shared_ptr<std::recursive_mutex> mMutex;
    std::shared_ptr<elastos::MessageChannelStrategy> mChannelStrategy;
}; // class Contact

} //namespace native
} //namespace crosspl

#endif /* _ELASTOS_SDK_JNI_CONTACT_CHANNEL_STRATEGY_HPP_ */
