/**
 * @file	Contact.hpp
 * @brief	Contact
 * @details	
 *
 * @author	xxx
 * @author	<xxx@xxx.com>
 * @copyright	(c) 2012 xxx All rights reserved.
 **/

#ifndef _ELASTOS_SDK_JNI_CONTACT_TYPES_HPP_
#define _ELASTOS_SDK_JNI_CONTACT_TYPES_HPP_

#include <string>
#include <experimental-span.hpp>
#include <Contact.V1.hpp>

#ifdef WITH_CROSSPL
#include "CrossBase.hpp"
#else
class CrossBase {
public:
  explicit CrossBase() = default;
  virtual ~CrossBase() = default;
}; // class CrossBase
#endif // WITH_CROSSPL

namespace crosspl {
namespace native {


class ContactChannelStrategy;
class ContactDataListener;
class ContactListener;
class ContactMessage;

#ifdef WITH_CROSSPL
#define PERMISSION public
#define ABSTRACT

using ChannelStrategyPtr = CrossBase*;
using ListenerPtr = CrossBase*;
using DataListenerPtr = CrossBase*;
using MessagePtr = CrossBase*;
using ConstStringPtr = const char*;
using ConstBytesPtr = const std::span<uint8_t>*;
using HumanInfoPtr = std::stringstream*;
using FriendListPtr = std::stringstream*;
using OutStringPtr = std::stringstream*;
using OutBytesPtr = std::vector<uint8_t>&;
using ChannelType = int;
using StatusType = int;
using InfoItemType = int;
using UserIdentifyType = int;
inline bool IsEmpty(ConstStringPtr str) {
    return (str == nullptr || str[0] == '\0');
}

#else
#define PERMISSION protected
#define ABSTRACT =0

using ChannelStrategyPtr = std::shared_ptr<ContactChannelStrategy>;
using ListenerPtr = std::shared_ptr<ContactListener>;
using DataListenerPtr = std::shared_ptr<ContactDataListener>;
using MessagePtr = std::shared_ptr<crosspl::native::ContactMessage>;
using ConstStringPtr = const std::string&;
using ConstBytesPtr = const std::vector<uint8_t>&;
using HumanInfoPtr = std::shared_ptr<elastos::HumanInfo>&;
using FriendListPtr = std::vector<std::shared_ptr<elastos::FriendInfo>>;
using OutStringPtr = std::string&;
using OutBytesPtr = std::vector<uint8_t>&;
using ChannelType = elastos::MessageManager::ChannelType;
using StatusType = elastos::HumanInfo::Status;
using InfoItemType = elastos::UserInfo::Item;
using UserIdentifyType = elastos::UserInfo::Type;
inline bool IsEmpty(ConstStringPtr str) {
    return str.empty();
}
#endif // WITH_CROSSPL

} //namespace native
} //namespace crosspl

#endif /* _ELASTOS_SDK_JNI_CONTACT_TYPES_HPP_ */
