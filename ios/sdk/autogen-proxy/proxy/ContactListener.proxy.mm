#import <Foundation/Foundation.h>

#import "CrossPLUtils.h"

#define ENABLE_NATIVE_FUNCTION
#define ENABLE_PLATFORM_FUNCTION
#import "ContactListener.proxy.h"

#import "CrossPL/CrossPL-Swift.h"
#import "ContactSDK/ContactSDK-Swift.h"
#import "ContactListener.hpp"


std::shared_ptr<std::span<uint8_t>> crosspl_Proxy_ContactListener_onAcquire(int64_t platformHandle, int var0, const char* var1, const std::span<uint8_t>* var2, const char* var3)
{
  auto ocobj = crosspl::CrossPLUtils::SafeCastCrossObjectToSwift<ContactListener>(platformHandle);
  int ocvar0 = var0;
  auto ocvar1 = crosspl::CrossPLUtils::SafeCastString(var1);
  auto ocvar2 = crosspl::CrossPLUtils::SafeCastByteArray(var2);
  auto ocvar3 = crosspl::CrossPLUtils::SafeCastString(var3);

  NSData* ocret = [ocobj onAcquire :ocvar0 :ocvar1.get() :ocvar2.get() :ocvar3.get()];


  auto ret = crosspl::CrossPLUtils::SafeCastByteArray(ocret);
  return ret;
}
void crosspl_Proxy_ContactListener_onEvent(int64_t platformHandle, int var0, const char* var1, int var2, const std::span<uint8_t>* var3)
{
  auto ocobj = crosspl::CrossPLUtils::SafeCastCrossObjectToSwift<ContactListener>(platformHandle);
  int ocvar0 = var0;
  auto ocvar1 = crosspl::CrossPLUtils::SafeCastString(var1);
  int ocvar2 = var2;
  auto ocvar3 = crosspl::CrossPLUtils::SafeCastByteArray(var3);

  [ocobj onEvent :ocvar0 :ocvar1.get() :ocvar2 :ocvar3.get()];



}
void crosspl_Proxy_ContactListener_onReceivedMessage(int64_t platformHandle, const char* var0, int var1, int var2, const std::span<uint8_t>* var3, const char* var4, int64_t var5, int64_t var6)
{
  auto ocobj = crosspl::CrossPLUtils::SafeCastCrossObjectToSwift<ContactListener>(platformHandle);
  auto ocvar0 = crosspl::CrossPLUtils::SafeCastString(var0);
  int ocvar1 = var1;
  int ocvar2 = var2;
  auto ocvar3 = crosspl::CrossPLUtils::SafeCastByteArray(var3);
  auto ocvar4 = crosspl::CrossPLUtils::SafeCastString(var4);
  int64_t ocvar5 = var5;
  int64_t ocvar6 = var6;

  [ocobj onReceivedMessage :ocvar0.get() :ocvar1 :ocvar2 :ocvar3.get() :ocvar4.get() :ocvar5 :ocvar6];



}
void crosspl_Proxy_ContactListener_onError(int64_t platformHandle, int32_t var0, const char* var1, const char* var2)
{
  auto ocobj = crosspl::CrossPLUtils::SafeCastCrossObjectToSwift<ContactListener>(platformHandle);
  int32_t ocvar0 = var0;
  auto ocvar1 = crosspl::CrossPLUtils::SafeCastString(var1);
  auto ocvar2 = crosspl::CrossPLUtils::SafeCastString(var2);

  [ocobj onError :ocvar0 :ocvar1.get() :ocvar2.get()];



}




