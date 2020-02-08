/**
 * @file	ContactChannel.hpp
 * @brief	Contact
 * @details
 *
 * @author	xxx
 * @author	<xxx@xxx.com>
 * @copyright	(c) 2012 xxx All rights reserved.
 **/

#ifndef _ELASTOS_SDK_JNI_CONTACT_CHANNEL_HPP_
#define _ELASTOS_SDK_JNI_CONTACT_CHANNEL_HPP_

#include <mutex>

#include "ContactTypes.hpp"
#include <Contact.V1.hpp>
//#include <ContactListener.hpp>

namespace crosspl {
namespace native {

class ContactChannel : public CrossBase
                     , public std::enable_shared_from_this<ContactChannel> {
public:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/

#ifdef WITH_CROSSPL
//    void onNotify(const std::string& humanCode, ChannelType channelType,
//                  const std::string& dataId, int status);
//    std::shared_ptr<std::span<uint8_t>> onReadData(const std::string& humanCode, ChannelType channelType,
//                                                   const std::string& dataId, uint64_t offset);
//    int onWriteData(const std::string& humanCode, ChannelType channelType,
//                    const std::string& dataId, uint64_t offset, const std::span<uint8_t>* data);
#else
//    virtual void onNotify(const std::string& humanCode, ChannelType channelType,
//                          const std::string& dataId, int status) = 0;
//    virtual int onReadData(const std::string& humanCode, ChannelType channelType,
//                           const std::string& dataId, uint64_t offset,
//                           std::vector<uint8_t>& data) = 0;
//    virtual int onWriteData(const std::string& humanCode, ChannelType channelType,
//                            const std::string& dataId, uint64_t offset,
//                            const std::vector<uint8_t>& data) = 0;
#endif // WITH_CROSSPL

//    std::shared_ptr<elastos::MessageManager::Channel> getChannel();

PERMISSION:
    explicit ContactChannel();
    virtual ~ContactChannel();

private:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/
//    std::shared_ptr<elastos::MessageManager::Channel> makeChannel();

//    std::shared_ptr<elastos::MessageManager::Channel> mChannel;
}; // class Contact

} //namespace native
} //namespace crosspl

#endif /* _ELASTOS_SDK_JNI_CONTACT_CHANNEL_HPP_ */
