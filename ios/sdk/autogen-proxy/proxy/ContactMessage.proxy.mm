#import <Foundation/Foundation.h>

#import "CrossPLUtils.h"

#define ENABLE_PLATFORM_FUNCTION
#import "ContactMessage.proxy.h"

#import "CrossPL/CrossPL-Swift.h"
#import "ContactSDK/ContactSDK-Swift.h"
#import "ContactMessage.hpp"




int crosspl_Proxy_ContactMessage_syncMessageToNative(int64_t nativeHandle, int ocvar0, NSData* ocvar1, NSString* ocvar2, int64_t ocvar3)
{
  int var0 = ocvar0;
  auto var1 = crosspl::CrossPLUtils::SafeCastByteArray(ocvar1);
  auto var2 = crosspl::CrossPLUtils::SafeCastString(ocvar2);
  int64_t var3 = ocvar3;

  auto obj = crosspl::CrossPLUtils::SafeCastCrossObjectToCpp<crosspl::native::ContactMessage>(nativeHandle);
  int ret = obj->syncMessageToNative(var0, var1.get(), var2.get(), var3);


  int ocret = ret;
  return ocret;
}

