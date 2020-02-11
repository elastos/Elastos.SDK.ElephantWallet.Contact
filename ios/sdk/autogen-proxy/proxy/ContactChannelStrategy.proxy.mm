#import <Foundation/Foundation.h>

#import "CrossPLUtils.h"

#define ENABLE_NATIVE_FUNCTION
#define ENABLE_PLATFORM_FUNCTION
#import "ContactChannelStrategy.proxy.h"

#import "CrossPL/CrossPL-Swift.h"
#import "ContactSDK/ContactSDK-Swift.h"
#import "ContactChannelStrategy.hpp"


int crosspl_Proxy_ContactChannelStrategy_onOpen(int64_t platformHandle)
{
  auto ocobj = crosspl::CrossPLUtils::SafeCastCrossObjectToSwift<ContactChannelStrategy>(platformHandle);

  int ocret = [ocobj onOpen];


  int ret = ocret;
  return ret;
}
int crosspl_Proxy_ContactChannelStrategy_onClose(int64_t platformHandle)
{
  auto ocobj = crosspl::CrossPLUtils::SafeCastCrossObjectToSwift<ContactChannelStrategy>(platformHandle);

  int ocret = [ocobj onClose];


  int ret = ocret;
  return ret;
}
int crosspl_Proxy_ContactChannelStrategy_onSendMessage(int64_t platformHandle, const char* var0, int var1, const std::span<uint8_t>* var2)
{
  auto ocobj = crosspl::CrossPLUtils::SafeCastCrossObjectToSwift<ContactChannelStrategy>(platformHandle);
  auto ocvar0 = crosspl::CrossPLUtils::SafeCastString(var0);
  int ocvar1 = var1;
  auto ocvar2 = crosspl::CrossPLUtils::SafeCastByteArray(var2);

  int ocret = [ocobj onSendMessage :ocvar0.get() :ocvar1 :ocvar2.get()];


  int ret = ocret;
  return ret;
}



int crosspl_Proxy_ContactChannelStrategy_receivedMessage(int64_t nativeHandle, NSString* ocvar0, int ocvar1, NSData* ocvar2)
{
  auto var0 = crosspl::CrossPLUtils::SafeCastString(ocvar0);
  int var1 = ocvar1;
  auto var2 = crosspl::CrossPLUtils::SafeCastByteArray(ocvar2);

  auto obj = crosspl::CrossPLUtils::SafeCastCrossObjectToCpp<crosspl::native::ContactChannelStrategy>(nativeHandle);
  int ret = obj->receivedMessage(var0.get(), var1, var2.get());


  int ocret = ret;
  return ocret;
}

