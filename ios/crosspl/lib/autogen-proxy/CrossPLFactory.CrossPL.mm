#import "CrossPLFactory.CrossPL.h"
#import <memory>

#import "CrossBase.hpp"
#import "CrossBase.proxy.h"

#import "CrossBase.hpp"


static int64_t createCppObject(const char* swiftClassName)
{
  printf("%s %s", __PRETTY_FUNCTION__, swiftClassName);
  void *ptr = nullptr;
  
  if(std::strcmp(swiftClassName, "CrossBase") == 0) {
    ptr = new crosspl::native::CrossBase();
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

  if(std::strcmp(swiftClassName, "CrossBase") == 0) {
    delete reinterpret_cast<crosspl::native::CrossBase*>(cppHandle);
    return 0;
  }


  return -1;
}

__attribute__((constructor)) static void OnLoad() {
  printf("%s %s\n", "CrossPL", __PRETTY_FUNCTION__);

  RegCreateCppObjFunc(createCppObject);
  RegDestroyCppObjFunc(destroyCppObject);
}
