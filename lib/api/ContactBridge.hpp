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

#include "ContactChannelStrategy.hpp"
#include "ContactDataListener.hpp"
#include "ContactListener.hpp"
#include "ContactTypes.hpp"
#include <Contact.V1.hpp>

namespace crosspl {
namespace native {

class ContactBridge {
PERMISSION:
    explicit ContactBridge();
    virtual ~ContactBridge();

public:
    /*** type define ***/

    /*** static function and variable ***/
    static bool IsDidFriend(ConstStringPtr friendCode);

    /*** class function and variable ***/

    int appendChannelStrategy(ChannelStrategyPtr channelStrategy);
    int setListener(ListenerPtr listener);
    void setDataListener(DataListenerPtr listener);
    int start();
    int stop();

    int setIdentifyCode(UserIdentifyType type, ConstStringPtr value);

    int setHumanInfo(ConstStringPtr humanCode, InfoItemType item, ConstStringPtr value);
    int getHumanInfo(ConstStringPtr humanCode, HumanInfoPtr info);
    int getHumanBrief(ConstStringPtr humanCode, ConstStringPtr devId, OutStringPtr brief);
    int findAvatarFile(ConstStringPtr avatar, OutStringPtr filepath);
    StatusType getHumanStatus(ConstStringPtr humanCode);

    int addFriend(ConstStringPtr friendCode, ConstStringPtr summary);
    int removeFriend(ConstStringPtr friendCode);
    int acceptFriend(ConstStringPtr friendCode);
    int getFriendList(FriendListPtr info);

    int sendMessage(ConstStringPtr friendCode, ChannelType chType, MessagePtr message);
    int pullData(ConstStringPtr humanCode, ChannelType chType, ConstStringPtr devId, ConstStringPtr dataId);
    int cancelPullData(ConstStringPtr humanCode, ChannelType chType, ConstStringPtr devId, ConstStringPtr dataId);

    int syncInfoMigrateOss(ConstStringPtr user, ConstStringPtr password, ConstStringPtr token,
                           ConstStringPtr disk, ConstStringPtr partition, ConstStringPtr rootdir);
    int syncInfoAuthOss(ConstStringPtr user, ConstStringPtr password, ConstStringPtr token,
                        ConstStringPtr disk, ConstStringPtr partition, ConstStringPtr rootdir);
    int syncInfoDownload(int fromClient);
    int syncInfoUpload(int toClient);
    int exportUserData(ConstStringPtr toFile);
    int importUserData(ConstStringPtr fromFile);

    int setWalletAddress(ConstStringPtr name, ConstStringPtr value);

protected:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/
    std::shared_ptr<elastos::ContactV1> mContactImpl;

private:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/
    std::map<uint32_t, ChannelStrategyPtr> mCustomChannelMap;
    ListenerPtr mListener;
    DataListenerPtr mDataListener;

}; // class Contact

} //namespace native
} //namespace crosspl

#endif /* _ELASTOS_SDK_JNI_CONTACT_BRIDGE_HPP_ */
