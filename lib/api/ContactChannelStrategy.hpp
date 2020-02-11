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
    int createChannel(uint32_t channelId, std::shared_ptr<elastos::MessageManager> msgMgr);
    std::shared_ptr<elastos::MessageChannelStrategy> getChannel();

    virtual int receivedMessage(const std::string& humanCode, int channelId, ConstBytesPtr data);

    virtual int onOpen() ABSTRACT;
    virtual int onClose() ABSTRACT;
    virtual int onSendMessage(const std::string& humanCode, int channelId, ConstBytesPtr data) ABSTRACT;

PERMISSION:
    explicit ContactChannelStrategy();
    virtual ~ContactChannelStrategy();

private:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/
    std::shared_ptr<elastos::MessageChannelStrategy> makeChannelStrategy(uint32_t chType,
                                                                         std::shared_ptr<elastos::MessageManager> msgMgr);
//
    std::shared_ptr<std::recursive_mutex> mMutex;
    std::shared_ptr<elastos::MessageChannelStrategy> mChannelStrategy;
}; // class Contact

} //namespace native
} //namespace crosspl

#endif /* _ELASTOS_SDK_JNI_CONTACT_CHANNEL_STRATEGY_HPP_ */
