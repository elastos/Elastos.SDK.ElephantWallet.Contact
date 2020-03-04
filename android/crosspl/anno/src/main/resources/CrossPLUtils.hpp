#ifndef _CROSSPL_CrossPLUtils_HPP_
#define _CROSSPL_CrossPLUtils_HPP_

#include <jni.h>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <sstream>
#include <thread>
#include <vector>

#include <experimental-span.hpp>

namespace crosspl {

class CrossPLUtils {
public:
    /*** type define ***/

    /*** static function and variable ***/
    static void SetJavaVM(JavaVM* jvm);
    static JavaVM* GetJavaVM();
    static void DetachCurrentThread();
    static std::shared_ptr<JNIEnv> SafeGetEnv();
    static jclass FindJavaClass(JNIEnv* jenv, const char* className);

    static std::shared_ptr<const char> SafeCastString(JNIEnv* jenv, jstring jdata);
    static std::shared_ptr<std::span<uint8_t>> SafeCastByteArray(JNIEnv* jenv, jbyteArray jdata);
    static std::shared_ptr<std::function<void()>> SafeCastFunction(JNIEnv* jenv, jobject jdata);
    static std::shared_ptr<std::stringstream> SafeCastStringBuffer(JNIEnv* jenv, jobject jdata);
    static std::shared_ptr<std::vector<uint8_t>> SafeCastByteBuffer(JNIEnv* jenv, jobject jdata);

    static std::shared_ptr<_jstring> SafeCastString(JNIEnv* jenv, const char* data);
    static std::shared_ptr<_jbyteArray> SafeCastByteArray(JNIEnv* jenv, const std::span<uint8_t>* data);
    static std::shared_ptr<_jobject> SafeCastFunction(JNIEnv* jenv, const std::function<void()>* data);
    static std::shared_ptr<_jobject> SafeCastStringBuffer(JNIEnv* jenv, const std::stringstream* data);
    static std::shared_ptr<_jobject> SafeCastByteBuffer(JNIEnv* jenv, const std::vector<uint8_t>* data);

    static int SafeCopyStringBufferToCpp(JNIEnv* jenv, std::stringstream* copyTo, jobject jdata);
    static int SafeCopyByteBufferToCpp(JNIEnv* jenv, std::vector<uint8_t>* copyTo, jobject jdata);

    static int SafeCopyStringBufferToJava(JNIEnv* jenv, jobject jcopyTo, const std::stringstream* data);
    static int SafeCopyByteBufferToJava(JNIEnv* jenv, jobject jcopyTo, const std::vector<uint8_t>* data);

    static int64_t AddGlobalObject(JNIEnv* jenv, jobject jobj);
    static std::shared_ptr<_jobject> SafeCastGlobalObject(JNIEnv* jenv, int64_t obj);
    static void DelGlobalObject(JNIEnv* jenv, jobject jobj);

    template <class T>
    static T* SafeCastCrossObject(JNIEnv* jenv, jobject jdata) {
      void* ret = SafeCastCrossObject(jenv, jdata);
      return reinterpret_cast<T*>(ret);
    }

    template <class T>
    static jobject SafeCastCrossObject(JNIEnv* jenv, const T* data) {
      int64_t ret = data->getPlatformHandle();
      return reinterpret_cast<jobject>(ret);
    }

    /*** class function and variable ***/

private:
    /*** type define ***/

    /*** static function and variable ***/
    static void EnsureRunOnThread(std::thread::id threadId);

    static void* SafeCastCrossObject(JNIEnv* jenv, jobject jdata);

//        static const char* JavaClassNameBoolean;
//        static const char* JavaClassNameInteger;
//        static const char* JavaClassNameLong;
//        static const char* JavaClassNameDouble;
    static const char* JavaClassNameString;
    static const char* JavaClassNameByteArray;
    static const char* JavaClassNameRunnable;
    static const char* JavaClassNameStringBuffer;
    static const char* JavaClassNameByteBuffer;

    static std::map<const char*, jclass> sJavaClassCache;
    static JavaVM* sJVM;
    /*** class function and variable ***/

}; // class CrossPLUtils
} // namespace crosspl

#endif /* _CROSSPL_CrossPLUtils_HPP_ */

