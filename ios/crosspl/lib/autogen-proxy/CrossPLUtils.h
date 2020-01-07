#ifndef _CROSSPL_CrossPLUtils_HPP_
#define _CROSSPL_CrossPLUtils_HPP_

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <sstream>
#include <thread>
#include <vector>
#include <type_traits>

#include "experimental-span.hpp"

#import "CrossBase.hpp"
@class CrossBase;

namespace crosspl {

class CrossPLUtils {
public:
    /*** type define ***/

    /*** static function and variable ***/
//    static void SetJavaVM(JavaVM* jvm);
//    static std::unique_ptr<JNIEnv, std::function<void(JNIEnv*)>> SafeGetEnv();
//    static jclass FindJavaClass(JNIEnv* jenv, const char* className);
//
    static std::shared_ptr<const char> SafeCastString(NSString* ocdata);
    static std::shared_ptr<std::span<uint8_t>> SafeCastByteArray(NSData* ocdata);
//    static std::shared_ptr<std::function<void()>> SafeCastFunction(JNIEnv* jenv, jobject jdata);
    static std::shared_ptr<std::stringstream> SafeCastStringBuffer(NSString** ocdata);
    static std::shared_ptr<std::vector<uint8_t>> SafeCastByteBuffer(NSData** ocdata);
//
    static std::shared_ptr<NSString> SafeCastString(const char* data);
    static std::shared_ptr<NSData> SafeCastByteArray(const std::span<uint8_t>* data);
//    static std::shared_ptr<_jobject> SafeCastFunction(JNIEnv* jenv, const std::function<void()>* data);
//    static std::shared_ptr<_jobject> SafeCastStringBuffer(JNIEnv* jenv, const std::stringstream* data);
//    static std::shared_ptr<_jobject> SafeCastByteBuffer(JNIEnv* jenv, const std::vector<uint8_t>* data);
//
//    static int SafeCopyStringBufferToCpp(JNIEnv* jenv, std::stringstream* copyTo, jobject jdata);
//    static int SafeCopyByteBufferToCpp(JNIEnv* jenv, std::vector<uint8_t>* copyTo, jobject jdata);
//
    static int SafeCopyStringBufferToSwift(NSString** occopyTo, const std::stringstream* data);
    static int SafeCopyByteBufferToSwift(NSData** occopyTo, const std::vector<uint8_t>* data);

//    template <class T>
//    static int64_t SafeCastCrossObject(T* ocdata);
//    template <>
//    static int64_t SafeCastCrossObject(NSObject* ocdata) {
//      auto ptr = (__bridge class CrossBase *)(ocdata);
//      int64_t handle = reinterpret_cast<int64_t>(ptr);
//      return handle;
//    }
  
  static int64_t AddGlobalObject(NSObject* ocobj);
  static int64_t DelGlobalObject(NSObject* ocobj);

  static crosspl::native::CrossBase* SafeCastCrossObjectToNative(NSObject* ocdata);
  
    template <class T>
    static int64_t SafeCastCrossObjectToHandle(NSObject* ocdata) {
//        auto ptr = (__bridge class CrossBase *)(ocdata);
//        int64_t handle = reinterpret_cast<int64_t>(ptr);
        int64_t handle = reinterpret_cast<int64_t>(ocdata);
        return handle;
    }
  
    //template <class T> static T* SafeCastCrossObject(int64_t handle);
    template <class T>
    static T* SafeCastCrossObjectToSwift(int64_t handle) {
        void* ptr = reinterpret_cast<void*>(handle);
        auto ocdata = (__bridge T *)(ptr);
        return ocdata;
    }
    template <class T>
    static T* SafeCastCrossObjectToCpp(int64_t handle) {
        auto data = reinterpret_cast<T*>(handle);
        return data;
    }
  
//
//    template <class T>
//    static jobject SafeCastCrossObject(JNIEnv* jenv, const T* data) {
//      int64_t ret = data->getPlatformHandle();
//      return reinterpret_cast<jobject>(ret);
//    }

    /*** class function and variable ***/

private:
    /*** type define ***/

    /*** static function and variable ***/
    static void EnsureRunOnThread(std::thread::id threadId);

//    static void* SafeCastCrossObject(NSObject* ocdata);

//        static const char* JavaClassNameBoolean;
//        static const char* JavaClassNameInteger;
//        static const char* JavaClassNameLong;
//        static const char* JavaClassNameDouble;
//    static const char* JavaClassNameString;
//    static const char* JavaClassNameByteArray;
//    static const char* JavaClassNameRunnable;
//    static const char* JavaClassNameStringBuffer;
//    static const char* JavaClassNameByteArrayOutputStream;

//    static std::map<const char*, jclass> sJavaClassCache;
//    static JavaVM* sJVM;
    /*** class function and variable ***/

}; // class CrossPLUtils
} // namespace crosspl

#endif /* _CROSSPL_CrossPLUtils_HPP_ */

