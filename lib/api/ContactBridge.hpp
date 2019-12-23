/**
 * @file	Contact.hpp
 * @brief	Contact
 * @details	
 *
 * @author	xxx
 * @author	<xxx@xxx.com>
 * @copyright	(c) 2012 xxx All rights reserved.
 **/

#ifndef _ELASTOS_SDK_JNI_CONTACT_BRIDGE_HPP_
#define _ELASTOS_SDK_JNI_CONTACT_BRIDGE_HPP_

#include <sstream>

#include <ContactTypes.hpp>
#include <Elastos.SDK.Contact.hpp>

namespace crosspl {
namespace native {

class ContactBridge {
#ifdef WITH_CROSSPL
public:
#else
protected:
#endif // WITH_CROSSPL
    explicit ContactBridge();
    virtual ~ContactBridge();


public:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/

    void setListener(ListenerPtr listener);
    void setDataListener(DataListenerPtr listener);
    int start();
    int stop();

    int setIdentifyCode(int type, ConstStringPtr value);

    int setHumanInfo(ConstStringPtr humanCode, int item, ConstStringPtr value);
    int getHumanInfo(ConstStringPtr humanCode, std::stringstream* info);
    int getHumanStatus(ConstStringPtr humanCode);

    int addFriend(ConstStringPtr friendCode, ConstStringPtr summary);
    int removeFriend(ConstStringPtr friendCode);
    int acceptFriend(ConstStringPtr friendCode);
    int getFriendList(std::stringstream* info);

    int sendMessage(ConstStringPtr friendCode, int chType, CrossBase* message);
    int pullData(ConstStringPtr humanCode, int chType, ConstStringPtr devId, ConstStringPtr dataId);
    int cancelPullData(ConstStringPtr humanCode, int chType, ConstStringPtr devId, ConstStringPtr dataId);

    int syncInfoDownloadFromDidChain();
    int syncInfoUploadToDidChain();

    int setWalletAddress(ConstStringPtr name, ConstStringPtr value);

protected:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/
    std::shared_ptr<elastos::Contact> mContactImpl;

private:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/
    ContactListener* mListener;
    ContactDataListener* mDataListener;

}; // class Contact

} //namespace native
} //namespace crosspl

#endif /* _ELASTOS_SDK_JNI_CONTACT_BRIDGE_HPP_ */
