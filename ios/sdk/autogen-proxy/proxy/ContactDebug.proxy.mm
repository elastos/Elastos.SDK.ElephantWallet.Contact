#import <Foundation/Foundation.h>

#import "CrossPLUtils.h"

#define ENABLE_PLATFORM_FUNCTION
#import "ContactDebug.proxy.h"

#import "CrossPL/CrossPL-Swift.h"
#import "ContactSDK/ContactSDK-Swift.h"
#import "ContactDebug.hpp"




int crosspl_Proxy_ContactDebug_GetCachedDidProp(NSString** ocvar0)
{
  auto var0 = crosspl::CrossPLUtils::SafeCastStringBuffer(ocvar0);

  int ret = crosspl::native::ContactDebug::GetCachedDidProp(var0.get());

  crosspl::CrossPLUtils::SafeCopyStringBufferToSwift(ocvar0, var0.get());

  int ocret = ret;
  return ocret;
}

