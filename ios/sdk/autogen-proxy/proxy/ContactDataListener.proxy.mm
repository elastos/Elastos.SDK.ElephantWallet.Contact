#import <Foundation/Foundation.h>

#import "CrossPLUtils.h"

#define ENABLE_NATIVE_FUNCTION
#define ENABLE_PLATFORM_FUNCTION
#import "ContactDataListener.proxy.h"

#import "CrossPL/CrossPL-Swift.h"
#import "ContactSDK/ContactSDK-Swift.h"
#import "ContactDataListener.hpp"


void crosspl_Proxy_ContactDataListener_onNotify(int64_t platformHandle, const char* var0, int var1, const char* var2, int var3)
{
  auto ocobj = crosspl::CrossPLUtils::SafeCastCrossObjectToSwift<ContactDataListener>(platformHandle);
  auto ocvar0 = crosspl::CrossPLUtils::SafeCastString(var0);
  int ocvar1 = var1;
  auto ocvar2 = crosspl::CrossPLUtils::SafeCastString(var2);
  int ocvar3 = var3;

  [ocobj onNotify :ocvar0.get() :ocvar1 :ocvar2.get() :ocvar3];



}
std::shared_ptr<std::span<uint8_t>> crosspl_Proxy_ContactDataListener_onReadData(int64_t platformHandle, const char* var0, int var1, const char* var2, int64_t var3)
{
  auto ocobj = crosspl::CrossPLUtils::SafeCastCrossObjectToSwift<ContactDataListener>(platformHandle);
  auto ocvar0 = crosspl::CrossPLUtils::SafeCastString(var0);
  int ocvar1 = var1;
  auto ocvar2 = crosspl::CrossPLUtils::SafeCastString(var2);
  int64_t ocvar3 = var3;

  NSData* ocret = [ocobj onReadData :ocvar0.get() :ocvar1 :ocvar2.get() :ocvar3];


  auto ret = crosspl::CrossPLUtils::SafeCastByteArray(ocret);
  return ret;
}
int crosspl_Proxy_ContactDataListener_onWriteData(int64_t platformHandle, const char* var0, int var1, const char* var2, int64_t var3, const std::span<uint8_t>* var4)
{
  auto ocobj = crosspl::CrossPLUtils::SafeCastCrossObjectToSwift<ContactDataListener>(platformHandle);
  auto ocvar0 = crosspl::CrossPLUtils::SafeCastString(var0);
  int ocvar1 = var1;
  auto ocvar2 = crosspl::CrossPLUtils::SafeCastString(var2);
  int64_t ocvar3 = var3;
  auto ocvar4 = crosspl::CrossPLUtils::SafeCastByteArray(var4);

  int ocret = [ocobj onWriteData :ocvar0.get() :ocvar1 :ocvar2.get() :ocvar3 :ocvar4.get()];


  int ret = ocret;
  return ret;
}




