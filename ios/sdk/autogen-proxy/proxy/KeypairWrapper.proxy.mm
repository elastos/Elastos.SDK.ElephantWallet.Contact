#import <Foundation/Foundation.h>

#import "CrossPLUtils.h"

#define ENABLE_NATIVE_FUNCTION
#define ENABLE_PLATFORM_FUNCTION
#import "KeypairWrapper.proxy.h"

#import "CrossPL/CrossPL-Swift.h"
#import "ContactSDK/ContactSDK-Swift.h"
#import "KeypairWrapper.hpp"





int crosspl_Proxy_KeypairWrapper_GetSinglePublicKey(NSData* ocvar0, NSString** ocvar1)
{
  auto var0 = crosspl::CrossPLUtils::SafeCastByteArray(ocvar0);
  auto var1 = crosspl::CrossPLUtils::SafeCastStringBuffer(ocvar1);

  int ret = crosspl::native::KeypairWrapper::GetSinglePublicKey(var0.get(), var1.get());

  crosspl::CrossPLUtils::SafeCopyStringBufferToSwift(ocvar1, var1.get());

  int ocret = ret;
  return ocret;
}
int crosspl_Proxy_KeypairWrapper_GetSinglePrivateKey(NSData* ocvar0, NSString** ocvar1)
{
  auto var0 = crosspl::CrossPLUtils::SafeCastByteArray(ocvar0);
  auto var1 = crosspl::CrossPLUtils::SafeCastStringBuffer(ocvar1);

  int ret = crosspl::native::KeypairWrapper::GetSinglePrivateKey(var0.get(), var1.get());

  crosspl::CrossPLUtils::SafeCopyStringBufferToSwift(ocvar1, var1.get());

  int ocret = ret;
  return ocret;
}
int crosspl_Proxy_KeypairWrapper_GenerateMnemonic(NSString* ocvar0, NSString* ocvar1, NSString** ocvar2)
{
  auto var0 = crosspl::CrossPLUtils::SafeCastString(ocvar0);
  auto var1 = crosspl::CrossPLUtils::SafeCastString(ocvar1);
  auto var2 = crosspl::CrossPLUtils::SafeCastStringBuffer(ocvar2);

  int ret = crosspl::native::KeypairWrapper::GenerateMnemonic(var0.get(), var1.get(), var2.get());

  crosspl::CrossPLUtils::SafeCopyStringBufferToSwift(ocvar2, var2.get());

  int ocret = ret;
  return ocret;
}
int crosspl_Proxy_KeypairWrapper_GetSeedFromMnemonic(NSString* ocvar0, NSString* ocvar1, NSData** ocvar2)
{
  auto var0 = crosspl::CrossPLUtils::SafeCastString(ocvar0);
  auto var1 = crosspl::CrossPLUtils::SafeCastString(ocvar1);
  auto var2 = crosspl::CrossPLUtils::SafeCastByteBuffer(ocvar2);

  int ret = crosspl::native::KeypairWrapper::GetSeedFromMnemonic(var0.get(), var1.get(), var2.get());

  crosspl::CrossPLUtils::SafeCopyByteBufferToSwift(ocvar2, var2.get());

  int ocret = ret;
  return ocret;
}
int crosspl_Proxy_KeypairWrapper_Sign(NSString* ocvar0, NSData* ocvar1, NSData** ocvar2)
{
  auto var0 = crosspl::CrossPLUtils::SafeCastString(ocvar0);
  auto var1 = crosspl::CrossPLUtils::SafeCastByteArray(ocvar1);
  auto var2 = crosspl::CrossPLUtils::SafeCastByteBuffer(ocvar2);

  int ret = crosspl::native::KeypairWrapper::Sign(var0.get(), var1.get(), var2.get());

  crosspl::CrossPLUtils::SafeCopyByteBufferToSwift(ocvar2, var2.get());

  int ocret = ret;
  return ocret;
}
int crosspl_Proxy_KeypairWrapper_EciesEncrypt(NSString* ocvar0, NSData* ocvar1, NSData** ocvar2)
{
  auto var0 = crosspl::CrossPLUtils::SafeCastString(ocvar0);
  auto var1 = crosspl::CrossPLUtils::SafeCastByteArray(ocvar1);
  auto var2 = crosspl::CrossPLUtils::SafeCastByteBuffer(ocvar2);

  int ret = crosspl::native::KeypairWrapper::EciesEncrypt(var0.get(), var1.get(), var2.get());

  crosspl::CrossPLUtils::SafeCopyByteBufferToSwift(ocvar2, var2.get());

  int ocret = ret;
  return ocret;
}
int crosspl_Proxy_KeypairWrapper_EciesDecrypt(NSString* ocvar0, NSData* ocvar1, NSData** ocvar2)
{
  auto var0 = crosspl::CrossPLUtils::SafeCastString(ocvar0);
  auto var1 = crosspl::CrossPLUtils::SafeCastByteArray(ocvar1);
  auto var2 = crosspl::CrossPLUtils::SafeCastByteBuffer(ocvar2);

  int ret = crosspl::native::KeypairWrapper::EciesDecrypt(var0.get(), var1.get(), var2.get());

  crosspl::CrossPLUtils::SafeCopyByteBufferToSwift(ocvar2, var2.get());

  int ocret = ret;
  return ocret;
}

