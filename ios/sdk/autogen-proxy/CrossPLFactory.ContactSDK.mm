#import "CrossPLFactory.ContactSDK.h"
#import <memory>

#import "CrossBase.hpp"
#import "ContactDebug.proxy.h"
#import "ContactMessage.proxy.h"
#import "ContactChannelStrategy.proxy.h"
#import "ContactBridge.proxy.h"
#import "ContactListener.proxy.h"
#import "ContactFactory.proxy.h"
#import "KeypairWrapper.proxy.h"
#import "ContactDataListener.proxy.h"

#import "ContactDebug.hpp"
#import "ContactMessage.hpp"
#import "ContactChannelStrategy.hpp"
#import "ContactBridge.hpp"
#import "ContactListener.hpp"
#import "ContactFactory.hpp"
#import "KeypairWrapper.hpp"
#import "ContactDataListener.hpp"


static int64_t createCppObject(const char* swiftClassName)
{
  printf("%s %s", __PRETTY_FUNCTION__, swiftClassName);
  void *ptr = nullptr;
  
  if(std::strcmp(swiftClassName, "ContactDebug") == 0) {
    ptr = new crosspl::native::ContactDebug();
  }
  if(std::strcmp(swiftClassName, "ContactMessage") == 0) {
    ptr = new crosspl::native::ContactMessage();
  }
  if(std::strcmp(swiftClassName, "ContactChannelStrategy") == 0) {
    ptr = new crosspl::native::ContactChannelStrategy();
  }
  if(std::strcmp(swiftClassName, "ContactBridge") == 0) {
    ptr = new crosspl::native::ContactBridge();
  }
  if(std::strcmp(swiftClassName, "ContactListener") == 0) {
    ptr = new crosspl::native::ContactListener();
  }
  if(std::strcmp(swiftClassName, "ContactFactory") == 0) {
    ptr = new crosspl::native::ContactFactory();
  }
  if(std::strcmp(swiftClassName, "KeypairWrapper") == 0) {
    ptr = new crosspl::native::KeypairWrapper();
  }
  if(std::strcmp(swiftClassName, "ContactDataListener") == 0) {
    ptr = new crosspl::native::ContactDataListener();
  }

  
  
  auto cppHandle = reinterpret_cast<int64_t>(ptr);
  return cppHandle;

}

static int destroyCppObject(const char* swiftClassName, int64_t cppHandle)
{
  if(cppHandle == 0) {
    return -1;
  }

  printf("%s %s", __PRETTY_FUNCTION__, swiftClassName);

  if(std::strcmp(swiftClassName, "ContactDebug") == 0) {
    delete reinterpret_cast<crosspl::native::ContactDebug*>(cppHandle);
    return 0;
  }
  if(std::strcmp(swiftClassName, "ContactMessage") == 0) {
    delete reinterpret_cast<crosspl::native::ContactMessage*>(cppHandle);
    return 0;
  }
  if(std::strcmp(swiftClassName, "ContactChannelStrategy") == 0) {
    delete reinterpret_cast<crosspl::native::ContactChannelStrategy*>(cppHandle);
    return 0;
  }
  if(std::strcmp(swiftClassName, "ContactBridge") == 0) {
    delete reinterpret_cast<crosspl::native::ContactBridge*>(cppHandle);
    return 0;
  }
  if(std::strcmp(swiftClassName, "ContactListener") == 0) {
    delete reinterpret_cast<crosspl::native::ContactListener*>(cppHandle);
    return 0;
  }
  if(std::strcmp(swiftClassName, "ContactFactory") == 0) {
    delete reinterpret_cast<crosspl::native::ContactFactory*>(cppHandle);
    return 0;
  }
  if(std::strcmp(swiftClassName, "KeypairWrapper") == 0) {
    delete reinterpret_cast<crosspl::native::KeypairWrapper*>(cppHandle);
    return 0;
  }
  if(std::strcmp(swiftClassName, "ContactDataListener") == 0) {
    delete reinterpret_cast<crosspl::native::ContactDataListener*>(cppHandle);
    return 0;
  }


  return -1;
}

__attribute__((constructor)) static void OnLoad() {
  printf("%s %s\n", "ContactSDK", __PRETTY_FUNCTION__);

  RegCreateCppObjFunc(createCppObject);
  RegDestroyCppObjFunc(destroyCppObject);
}
