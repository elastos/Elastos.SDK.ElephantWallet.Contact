#import <Foundation/Foundation.h>

#import "CrossPLUtils.h"

#define ENABLE_NATIVE_FUNCTION
#define ENABLE_PLATFORM_FUNCTION
#import "ContactBridge.proxy.h"

#import "CrossPL/CrossPL-Swift.h"
#import "ContactSDK/ContactSDK-Swift.h"
#import "ContactBridge.hpp"





bool crosspl_Proxy_ContactBridge_IsDidFriend(NSString* ocvar0)
{
  auto var0 = crosspl::CrossPLUtils::SafeCastString(ocvar0);

  bool ret = crosspl::native::ContactBridge::IsDidFriend(var0.get());


  bool ocret = ret;
  return ocret;
}
int crosspl_Proxy_ContactBridge_start(int64_t nativeHandle)
{

  auto obj = crosspl::CrossPLUtils::SafeCastCrossObjectToCpp<crosspl::native::ContactBridge>(nativeHandle);
  int ret = obj->start();


  int ocret = ret;
  return ocret;
}
int crosspl_Proxy_ContactBridge_stop(int64_t nativeHandle)
{

  auto obj = crosspl::CrossPLUtils::SafeCastCrossObjectToCpp<crosspl::native::ContactBridge>(nativeHandle);
  int ret = obj->stop();


  int ocret = ret;
  return ocret;
}
int crosspl_Proxy_ContactBridge_addFriend(int64_t nativeHandle, NSString* ocvar0, NSString* ocvar1)
{
  auto var0 = crosspl::CrossPLUtils::SafeCastString(ocvar0);
  auto var1 = crosspl::CrossPLUtils::SafeCastString(ocvar1);

  auto obj = crosspl::CrossPLUtils::SafeCastCrossObjectToCpp<crosspl::native::ContactBridge>(nativeHandle);
  int ret = obj->addFriend(var0.get(), var1.get());


  int ocret = ret;
  return ocret;
}
int crosspl_Proxy_ContactBridge_removeFriend(int64_t nativeHandle, NSString* ocvar0)
{
  auto var0 = crosspl::CrossPLUtils::SafeCastString(ocvar0);

  auto obj = crosspl::CrossPLUtils::SafeCastCrossObjectToCpp<crosspl::native::ContactBridge>(nativeHandle);
  int ret = obj->removeFriend(var0.get());


  int ocret = ret;
  return ocret;
}
int crosspl_Proxy_ContactBridge_acceptFriend(int64_t nativeHandle, NSString* ocvar0)
{
  auto var0 = crosspl::CrossPLUtils::SafeCastString(ocvar0);

  auto obj = crosspl::CrossPLUtils::SafeCastCrossObjectToCpp<crosspl::native::ContactBridge>(nativeHandle);
  int ret = obj->acceptFriend(var0.get());


  int ocret = ret;
  return ocret;
}
int crosspl_Proxy_ContactBridge_syncInfoDownloadFromDidChain(int64_t nativeHandle)
{

  auto obj = crosspl::CrossPLUtils::SafeCastCrossObjectToCpp<crosspl::native::ContactBridge>(nativeHandle);
  int ret = obj->syncInfoDownloadFromDidChain();


  int ocret = ret;
  return ocret;
}
int crosspl_Proxy_ContactBridge_syncInfoUploadToDidChain(int64_t nativeHandle)
{

  auto obj = crosspl::CrossPLUtils::SafeCastCrossObjectToCpp<crosspl::native::ContactBridge>(nativeHandle);
  int ret = obj->syncInfoUploadToDidChain();


  int ocret = ret;
  return ocret;
}
int crosspl_Proxy_ContactBridge_setWalletAddress(int64_t nativeHandle, NSString* ocvar0, NSString* ocvar1)
{
  auto var0 = crosspl::CrossPLUtils::SafeCastString(ocvar0);
  auto var1 = crosspl::CrossPLUtils::SafeCastString(ocvar1);

  auto obj = crosspl::CrossPLUtils::SafeCastCrossObjectToCpp<crosspl::native::ContactBridge>(nativeHandle);
  int ret = obj->setWalletAddress(var0.get(), var1.get());


  int ocret = ret;
  return ocret;
}
int crosspl_Proxy_ContactBridge_appendChannelStrategy(int64_t nativeHandle, int ocvar0, NSObject* ocvar1)
{
  int var0 = ocvar0;
  auto var1 = crosspl::CrossPLUtils::SafeCastCrossObjectToNative(ocvar1);

  auto obj = crosspl::CrossPLUtils::SafeCastCrossObjectToCpp<crosspl::native::ContactBridge>(nativeHandle);
  int ret = obj->appendChannelStrategy(var0, var1);


  int ocret = ret;
  return ocret;
}
void crosspl_Proxy_ContactBridge_setListener(int64_t nativeHandle, NSObject* ocvar0)
{
  auto var0 = crosspl::CrossPLUtils::SafeCastCrossObjectToNative(ocvar0);

  auto obj = crosspl::CrossPLUtils::SafeCastCrossObjectToCpp<crosspl::native::ContactBridge>(nativeHandle);
  obj->setListener(var0);



}
void crosspl_Proxy_ContactBridge_setDataListener(int64_t nativeHandle, NSObject* ocvar0)
{
  auto var0 = crosspl::CrossPLUtils::SafeCastCrossObjectToNative(ocvar0);

  auto obj = crosspl::CrossPLUtils::SafeCastCrossObjectToCpp<crosspl::native::ContactBridge>(nativeHandle);
  obj->setDataListener(var0);



}
int crosspl_Proxy_ContactBridge_setHumanInfo(int64_t nativeHandle, NSString* ocvar0, int ocvar1, NSString* ocvar2)
{
  auto var0 = crosspl::CrossPLUtils::SafeCastString(ocvar0);
  int var1 = ocvar1;
  auto var2 = crosspl::CrossPLUtils::SafeCastString(ocvar2);

  auto obj = crosspl::CrossPLUtils::SafeCastCrossObjectToCpp<crosspl::native::ContactBridge>(nativeHandle);
  int ret = obj->setHumanInfo(var0.get(), var1, var2.get());


  int ocret = ret;
  return ocret;
}
int crosspl_Proxy_ContactBridge_setIdentifyCode(int64_t nativeHandle, int ocvar0, NSString* ocvar1)
{
  int var0 = ocvar0;
  auto var1 = crosspl::CrossPLUtils::SafeCastString(ocvar1);

  auto obj = crosspl::CrossPLUtils::SafeCastCrossObjectToCpp<crosspl::native::ContactBridge>(nativeHandle);
  int ret = obj->setIdentifyCode(var0, var1.get());


  int ocret = ret;
  return ocret;
}
int crosspl_Proxy_ContactBridge_getHumanInfo(int64_t nativeHandle, NSString* ocvar0, NSString** ocvar1)
{
  auto var0 = crosspl::CrossPLUtils::SafeCastString(ocvar0);
  auto var1 = crosspl::CrossPLUtils::SafeCastStringBuffer(ocvar1);

  auto obj = crosspl::CrossPLUtils::SafeCastCrossObjectToCpp<crosspl::native::ContactBridge>(nativeHandle);
  int ret = obj->getHumanInfo(var0.get(), var1.get());

  crosspl::CrossPLUtils::SafeCopyStringBufferToSwift(ocvar1, var1.get());

  int ocret = ret;
  return ocret;
}
int crosspl_Proxy_ContactBridge_getFriendList(int64_t nativeHandle, NSString** ocvar0)
{
  auto var0 = crosspl::CrossPLUtils::SafeCastStringBuffer(ocvar0);

  auto obj = crosspl::CrossPLUtils::SafeCastCrossObjectToCpp<crosspl::native::ContactBridge>(nativeHandle);
  int ret = obj->getFriendList(var0.get());

  crosspl::CrossPLUtils::SafeCopyStringBufferToSwift(ocvar0, var0.get());

  int ocret = ret;
  return ocret;
}
int crosspl_Proxy_ContactBridge_getHumanStatus(int64_t nativeHandle, NSString* ocvar0)
{
  auto var0 = crosspl::CrossPLUtils::SafeCastString(ocvar0);

  auto obj = crosspl::CrossPLUtils::SafeCastCrossObjectToCpp<crosspl::native::ContactBridge>(nativeHandle);
  int ret = obj->getHumanStatus(var0.get());


  int ocret = ret;
  return ocret;
}
int crosspl_Proxy_ContactBridge_sendMessage(int64_t nativeHandle, NSString* ocvar0, int ocvar1, NSObject* ocvar2)
{
  auto var0 = crosspl::CrossPLUtils::SafeCastString(ocvar0);
  int var1 = ocvar1;
  auto var2 = crosspl::CrossPLUtils::SafeCastCrossObjectToNative(ocvar2);

  auto obj = crosspl::CrossPLUtils::SafeCastCrossObjectToCpp<crosspl::native::ContactBridge>(nativeHandle);
  int ret = obj->sendMessage(var0.get(), var1, var2);


  int ocret = ret;
  return ocret;
}
int crosspl_Proxy_ContactBridge_pullData(int64_t nativeHandle, NSString* ocvar0, int ocvar1, NSString* ocvar2, NSString* ocvar3)
{
  auto var0 = crosspl::CrossPLUtils::SafeCastString(ocvar0);
  int var1 = ocvar1;
  auto var2 = crosspl::CrossPLUtils::SafeCastString(ocvar2);
  auto var3 = crosspl::CrossPLUtils::SafeCastString(ocvar3);

  auto obj = crosspl::CrossPLUtils::SafeCastCrossObjectToCpp<crosspl::native::ContactBridge>(nativeHandle);
  int ret = obj->pullData(var0.get(), var1, var2.get(), var3.get());


  int ocret = ret;
  return ocret;
}
int crosspl_Proxy_ContactBridge_cancelPullData(int64_t nativeHandle, NSString* ocvar0, int ocvar1, NSString* ocvar2, NSString* ocvar3)
{
  auto var0 = crosspl::CrossPLUtils::SafeCastString(ocvar0);
  int var1 = ocvar1;
  auto var2 = crosspl::CrossPLUtils::SafeCastString(ocvar2);
  auto var3 = crosspl::CrossPLUtils::SafeCastString(ocvar3);

  auto obj = crosspl::CrossPLUtils::SafeCastCrossObjectToCpp<crosspl::native::ContactBridge>(nativeHandle);
  int ret = obj->cancelPullData(var0.get(), var1, var2.get(), var3.get());


  int ocret = ret;
  return ocret;
}

