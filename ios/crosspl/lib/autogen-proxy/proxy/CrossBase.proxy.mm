#import <Foundation/Foundation.h>

#import "CrossPLUtils.h"

#define ENABLE_PLATFORM_FUNCTION
#import "CrossBase.proxy.h"

#import "CrossPL/CrossPL-Swift.h"
#import "CrossPL/CrossPL-Swift.h"
#import "CrossBase.hpp"




int64_t crosspl_Proxy_CrossBase_CreateNativeObject(NSString* ocvar0)
{
  auto var0 = crosspl::CrossPLUtils::SafeCastString(ocvar0);

  int64_t ret = crosspl::native::CrossBase::CreateNativeObject(var0.get());


  int64_t ocret = ret;
  return ocret;
}
void crosspl_Proxy_CrossBase_DestroyNativeObject(NSString* ocvar0, int64_t ocvar1)
{
  auto var0 = crosspl::CrossPLUtils::SafeCastString(ocvar0);
  int64_t var1 = ocvar1;

  crosspl::native::CrossBase::DestroyNativeObject(var0.get(), var1);



}
void crosspl_Proxy_CrossBase_bindPlatformHandle(int64_t nativeHandle, NSObject* ocvar0)
{
  auto var0 = crosspl::CrossPLUtils::AddGlobalObject(ocvar0);

  auto obj = crosspl::CrossPLUtils::SafeCastCrossObjectToCpp<crosspl::native::CrossBase>(nativeHandle);
  obj->bindPlatformHandle(var0);



}
void crosspl_Proxy_CrossBase_unbindPlatformHandle(int64_t nativeHandle, NSObject* ocvar0)
{
  auto var0 = crosspl::CrossPLUtils::DelGlobalObject(ocvar0);

  auto obj = crosspl::CrossPLUtils::SafeCastCrossObjectToCpp<crosspl::native::CrossBase>(nativeHandle);
  obj->unbindPlatformHandle(var0);



}

