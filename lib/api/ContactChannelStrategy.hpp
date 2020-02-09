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

namespace crosspl {
namespace native {

class ContactChannelStrategy : public CrossBase {
public:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/

//    std::shared_ptr<elastos::MessageManager::DataListener> getDataListener();

PERMISSION:
    explicit ContactChannelStrategy() = default;
    virtual ~ContactChannelStrategy() = default;

private:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/
//    std::shared_ptr<elastos::MessageManager::DataListener> makeDataListener();
//
//    std::shared_ptr<std::recursive_mutex> mMutex;
//    std::shared_ptr<elastos::MessageManager::DataListener> mDataListener;
}; // class Contact

} //namespace native
} //namespace crosspl

#endif /* _ELASTOS_SDK_JNI_CONTACT_CHANNEL_STRATEGY_HPP_ */
