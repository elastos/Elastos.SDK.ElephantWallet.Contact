#import <Foundation/Foundation.h>

#import "CrossPLUtils.h"

#define ENABLE_PLATFORM_FUNCTION
#import "ContactFactory.proxy.h"

#import "CrossPL/CrossPL-Swift.h"
#import "ContactSDK/ContactSDK-Swift.h"
#import "ContactFactory.hpp"




void crosspl_Proxy_ContactFactory_SetLogLevel(int32_t ocvar0)
{
  int32_t var0 = ocvar0;

  crosspl::native::ContactFactory::SetLogLevel(var0);



}
void crosspl_Proxy_ContactFactory_SetDeviceId(NSString* ocvar0)
{
  auto var0 = crosspl::CrossPLUtils::SafeCastString(ocvar0);

  crosspl::native::ContactFactory::SetDeviceId(var0.get());



}
int32_t crosspl_Proxy_ContactFactory_SetLocalDataDir(NSString* ocvar0)
{
  auto var0 = crosspl::CrossPLUtils::SafeCastString(ocvar0);

  int32_t ret = crosspl::native::ContactFactory::SetLocalDataDir(var0.get());


  int32_t ocret = ret;
  return ocret;
}

