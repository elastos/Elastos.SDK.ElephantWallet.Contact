/**
 * @file	ContactMessage.hpp
 * @brief	Contact
 * @details
 *
 * @author	xxx
 * @author	<xxx@xxx.com>
 * @copyright	(c) 2012 xxx All rights reserved.
 **/

#ifndef _ELASTOS_SDK_JNI_CONTACT_MESSAGE_HPP_
#define _ELASTOS_SDK_JNI_CONTACT_MESSAGE_HPP_

#include "ContactTypes.hpp"
#include <MessageManager.hpp>
#include "experimental-span.hpp"

namespace crosspl {
namespace native {

class ContactMessage : public CrossBase {
public:
    /*** type define ***/
    using Type = elastos::MessageManager::MessageType;

    /*** static function and variable ***/

    /*** class function and variable ***/

    int syncMessageToNative(int type,
                            const std::span<uint8_t>* data,
                            ConstStringPtr cryptoAlgorithm,
                            int64_t nanoTime,
                            int64_t replyToNanoTime);

    std::shared_ptr<elastos::MessageManager::MessageInfo> mMessageInfo;

PERMISSION:
    explicit ContactMessage();
    virtual ~ContactMessage();

private:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/
}; // class Contact

} //namespace native
} //namespace crosspl

#endif /* _ELASTOS_SDK_JNI_CONTACT_MESSAGE_HPP_ */

