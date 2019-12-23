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
#include <ContactListener.hpp>
#include <ContactDataListener.hpp>
#include <CrossBase.hpp>

namespace crosspl {
namespace native {

#ifdef WITH_CROSSPL
using ListenerPtr = CrossBase*;
using DataListenerPtr = CrossBase*;
using ConstStringPtr = const char*;
inline bool IsEmpty(ConstStringPtr str) {
    return (str == nullptr || str[0] == '\0');
}
#else
using ListenerPtr = ContactListener*;
using DataListenerPtr = ContactDataListener*;
using ConstStringPtr = const std::string&;
inline bool IsEmpty(ConstStringPtr str) {
    return str.empty();
}
#endif // WITH_CROSSPL

} //namespace native
} //namespace crosspl

#endif /* _ELASTOS_SDK_JNI_CONTACT_TYPES_HPP_ */
