/**
 * @file	Contact.hpp
 * @brief	Contact
 * @details	
 *
 * @author	xxx
 * @author	<xxx@xxx.com>
 * @copyright	(c) 2012 xxx All rights reserved.
 **/

#ifndef _ELASTOS_SDK_CONTACT_HPP_
#define _ELASTOS_SDK_CONTACT_HPP_

#include <sstream>

#include <ContactListener.hpp>
#include <ContactDataListener.hpp>
#include "CrossBase.hpp"
#include <Elastos.SDK.Contact.hpp>

#ifdef WITH_CROSSPL
namespace crosspl {
namespace native {
#endif // WITH_CROSSPL

class ContactBridge {
public:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/
    explicit ContactBridge();
    virtual ~ContactBridge();

    void setListener(CrossBase* listener);
    void setDataListener(CrossBase* listener);
    int start();
    int stop();

    int setUserInfo(int item, const char* value);
    int setIdentifyCode(int type, const char* value);

    int getHumanInfo(const char* humanCode, std::stringstream* info);
    int getHumanStatus(const char* humanCode);

    int addFriend(const char* friendCode, const char* summary);
    int removeFriend(const char* friendCode);
    int acceptFriend(const char* friendCode);
    int getFriendList(std::stringstream* info);

    int sendMessage(const char* friendCode, int chType, CrossBase* message);
    int pullData(const char* humanCode, int chType, const char* devId, const char* dataId);
    int cancelPullData(const char* humanCode, int chType, const char* devId, const char* dataId);

    int syncInfoDownloadFromDidChain();
    int syncInfoUploadToDidChain();

    int setWalletAddress(const char* name, const char* value);

private:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/
    std::shared_ptr<elastos::Contact> mContactImpl;
    ContactListener* mListener;
    ContactDataListener* mDataListener;

}; // class Contact

#ifdef WITH_CROSSPL
} //namespace native
} //namespace crosspl
#endif // WITH_CROSSPL

#endif /* _ELASTOS_SDK_CONTACT_HPP_ */
