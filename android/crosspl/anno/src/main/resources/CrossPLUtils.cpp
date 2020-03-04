#include "CrossPLUtils.hpp"
#include <jni.h>

#include <android/log.h>
#include <vector>

namespace crosspl {

/***********************************************/
/***** static variables initialize *************/
/***********************************************/
//const char* CrossPLUtils::JavaClassNameBoolean      = "java/lang/Boolean";
//const char* CrossPLUtils::JavaClassNameInteger      = "java/lang/Integer";
//const char* CrossPLUtils::JavaClassNameLong         = "java/lang/Long";
//const char* CrossPLUtils::JavaClassNameDouble       = "java/lang/Double";
const char* CrossPLUtils::JavaClassNameString                  = "java/lang/String";
const char* CrossPLUtils::JavaClassNameByteArray               = "[B";
const char* CrossPLUtils::JavaClassNameRunnable                = "java/lang/Runnable";
const char* CrossPLUtils::JavaClassNameStringBuffer            = "java/lang/StringBuffer";
const char* CrossPLUtils::JavaClassNameByteBuffer              = "org/elastos/tools/crosspl/utils/DataBuffer";

std::map<const char*, jclass> CrossPLUtils::sJavaClassCache {};
JavaVM* CrossPLUtils::sJVM = nullptr;


/***********************************************/
/***** static function implement ***************/
/***********************************************/
extern "C" JavaVM* GetJavaVM()
{
    return crosspl::CrossPLUtils::GetJavaVM();
}

void CrossPLUtils::SetJavaVM(JavaVM* jvm)
{
    sJVM = jvm;
}

JavaVM* CrossPLUtils::GetJavaVM()
{
    return sJVM;
}

void CrossPLUtils::DetachCurrentThread()
{
//    JNIEnv *jenv;
//    jint jret = sJVM->GetEnv((void **) &jenv, JNI_VERSION_1_6);
//    if (jret != JNI_OK) {
        std::thread::id threadId = std::this_thread::get_id();
    //    //__android_log_print(ANDROID_LOG_WARN, "crosspl", "============= DetachCurrentThread tid=%d", threadId);
        //__android_log_print(ANDROID_LOG_WARN, "crosspl", "============= DetachCurrentThread");
        sJVM->DetachCurrentThread();
//    }
}

std::shared_ptr<JNIEnv> CrossPLUtils::SafeGetEnv()
{
    std::shared_ptr<JNIEnv> ret;
    bool needDetach = false;
    std::thread::id threadId = std::this_thread::get_id();

    auto creater = [&]() -> JNIEnv* {
        JNIEnv *jenv;
        jint jret = sJVM->GetEnv((void **) &jenv, JNI_VERSION_1_6);
        if (jret != JNI_OK) {
            jret = sJVM->AttachCurrentThread(&jenv, nullptr);
            if (jret != JNI_OK) {
                throw std::runtime_error("CrossPL: Failed to get jni env, AttachCurrentThread return error");
            }
//            __android_log_print(ANDROID_LOG_DEBUG, "crosspl", "CrossPLUtils::SafeGetEnv() AttachCurrentThread tid=%d", threadId);
            needDetach = true;
        }
        return jenv;
    }();

    auto deleter = [=](JNIEnv* jenv) -> void {
        EnsureRunOnThread(threadId);
        if (needDetach == true) {
            sJVM->DetachCurrentThread();
//            __android_log_print(ANDROID_LOG_DEBUG, "crosspl", "CrossPLUtils::SafeGetEnv() DetachCurrentThread tid=%d", threadId);
        }
    };

    ret = std::shared_ptr<JNIEnv>(creater, deleter);

    return ret;
}

jclass CrossPLUtils::FindJavaClass(JNIEnv* jenv, const char* className)
{
    // CRASH if not found
    auto found = sJavaClassCache.find(className);
    if(found == sJavaClassCache.end()) {
        jclass clazz = jenv->FindClass(className);
//        __android_log_print(ANDROID_LOG_WARN, "crosspl", "%s %p", __PRETTY_FUNCTION__, clazz);
        sJavaClassCache[className] = (jclass) jenv->NewGlobalRef(clazz);
        jenv->DeleteLocalRef(clazz);
    }


    return sJavaClassCache.at(className);
}

void CrossPLUtils::EnsureRunOnThread(std::thread::id threadId)
{
    std::thread::id currThreadId = std::this_thread::get_id();
    if(currThreadId != threadId) {
        //__android_log_print(ANDROID_LOG_FATAL, "crosspl", "Running on incorrect thread!!!");
        throw std::runtime_error("CrossPL: Running on incorrect thread");
    }
}

std::shared_ptr<const char> CrossPLUtils::SafeCastString(JNIEnv* jenv, jstring jdata)
{
    std::shared_ptr<const char> ret;
    std::thread::id threadId = std::this_thread::get_id();

    if(jdata == nullptr) {
        return ret; // nullptr
    }

    auto creater = [=]() -> const char* {
        EnsureRunOnThread(threadId);
        const char* ptr = jenv->GetStringUTFChars(jdata, nullptr);
        //__android_log_print(ANDROID_LOG_WARN, "crosspl", "%s %p", __PRETTY_FUNCTION__, ptr);
        return ptr;
    };
    auto deleter = [=](const char* ptr) -> void {
        EnsureRunOnThread(threadId);
        jenv->ReleaseStringUTFChars(jdata, ptr);
        //__android_log_print(ANDROID_LOG_WARN, "crosspl", "%s %p", __PRETTY_FUNCTION__, ptr);
    };

    ret = std::shared_ptr<const char>(creater(), deleter);

    return ret;
}

std::shared_ptr<std::function<void()>> CrossPLUtils::SafeCastFunction(JNIEnv* jenv, jobject jdata)
{
    std::shared_ptr<std::function<void()>> ret;
    std::thread::id threadId = std::this_thread::get_id();

    if(jdata == nullptr) {
        return ret; // nullptr
    }

    auto creater = [=]() -> std::function<void()>* {
        EnsureRunOnThread(threadId);

        auto ptr = new std::function<void()>([=]() {
            jclass jclazz = FindJavaClass(jenv, JavaClassNameRunnable);
            jmethodID jmethod = jenv->GetMethodID(jclazz, "run", "()V");
            jenv->CallVoidMethod(jdata, jmethod);
        });
        //__android_log_print(ANDROID_LOG_WARN, "crosspl", "%s %p", __PRETTY_FUNCTION__, ptr);
        return ptr;
    };
    auto deleter = [=](std::function<void()>* ptr) -> void {
        EnsureRunOnThread(threadId);
        //__android_log_print(ANDROID_LOG_WARN, "crosspl", "%s %p", __PRETTY_FUNCTION__, ptr);
        delete ptr;
    };

    ret = std::shared_ptr<std::function<void()>>(creater(), deleter);

    return ret;
}

std::shared_ptr<std::span<uint8_t>> CrossPLUtils::SafeCastByteArray(JNIEnv* jenv, jbyteArray jdata)
{
    std::shared_ptr<std::span<uint8_t>> ret;
    std::thread::id threadId = std::this_thread::get_id();

    if(jdata == nullptr) {
        return ret; // nullptr
    }

    auto creater = [=]() -> std::span<uint8_t>* {
        EnsureRunOnThread(threadId);
        jbyte* arrayPtr = jenv->GetByteArrayElements(jdata, nullptr);
        jsize arrayLen = jenv->GetArrayLength(jdata);

        auto ptr = new std::span<uint8_t>(reinterpret_cast<uint8_t*>(arrayPtr), arrayLen, true);
        jenv->ReleaseByteArrayElements(jdata, arrayPtr, JNI_ABORT);

//        __android_log_print(ANDROID_LOG_WARN, "crosspl", "%s %p", __PRETTY_FUNCTION__, retPtr->data());
        return ptr;
    };
    auto deleter = [=](std::span<uint8_t>* ptr) -> void {
        EnsureRunOnThread(threadId);
//        __android_log_print(ANDROID_LOG_WARN, "crosspl", "%s %p", __PRETTY_FUNCTION__, ptr->data());
        delete ptr;
    };

    ret = std::shared_ptr<std::span<uint8_t>>(creater(), deleter);

    return ret;
}

std::shared_ptr<std::stringstream> CrossPLUtils::SafeCastStringBuffer(JNIEnv* jenv, jobject jdata)
{
    std::shared_ptr<std::stringstream> ret;
    std::thread::id threadId = std::this_thread::get_id();

    if(jdata == nullptr) {
        return ret; // nullptr
    }

    auto creater = [=]() -> std::stringstream* {
        EnsureRunOnThread(threadId);

        jclass jclazz = FindJavaClass(jenv, JavaClassNameStringBuffer);
        jmethodID jmethod = jenv->GetMethodID(jclazz, "toString", "()Ljava/lang/String;");
        jstring jstr = static_cast<jstring>(jenv->CallObjectMethod(jdata, jmethod));
        auto str = SafeCastString(jenv, jstr);

        auto retPtr = new std::stringstream(str.get());

        str.reset();
        jenv->DeleteLocalRef(jstr);
//        __android_log_print(ANDROID_LOG_WARN, "crosspl", "%s %d %p", __PRETTY_FUNCTION__, __LINE__, retPtr);

        return retPtr;
    };
    auto deleter = [=](std::stringstream* ptr) -> void {
        EnsureRunOnThread(threadId);
//        __android_log_print(ANDROID_LOG_WARN, "crosspl", "%s %d %p", __PRETTY_FUNCTION__, __LINE__, ptr);
        delete ptr;
    };

    ret = std::shared_ptr<std::stringstream>(creater(), deleter);

    return ret;
}

std::shared_ptr<std::vector<uint8_t>> CrossPLUtils::SafeCastByteBuffer(JNIEnv* jenv, jobject jdata)
{
    std::shared_ptr<std::vector<uint8_t>> ret;
    std::thread::id threadId = std::this_thread::get_id();

    if(jdata == nullptr) {
        return ret; // nullptr
    }

    auto creater = [=]() -> std::vector<uint8_t>* {
        EnsureRunOnThread(threadId);

        jclass jclazz = jenv->GetObjectClass(jdata);
        jmethodID jmethod = jenv->GetMethodID(jclazz, "toBytes", "()[B");
        jbyteArray jbytes = static_cast<jbyteArray>(jenv->CallObjectMethod(jdata, jmethod));
        auto bytes = SafeCastByteArray(jenv, jbytes);
        if(bytes == nullptr) {
            jenv->DeleteLocalRef(jbytes);
            return nullptr;
        }

        auto ret = new std::vector<uint8_t>(bytes->data(), bytes->data() + bytes->size());

        bytes.reset();
        jenv->DeleteLocalRef(jbytes);

//        __android_log_print(ANDROID_LOG_WARN, "crosspl", "%s %p", __PRETTY_FUNCTION__, ret);
        return ret;
    };
    auto deleter = [=](std::vector<uint8_t>* ptr) -> void {
        EnsureRunOnThread(threadId);
//        __android_log_print(ANDROID_LOG_WARN, "crosspl", "%s %p", __PRETTY_FUNCTION__, ptr);
        if(ptr == nullptr) {
            return;
        }
        delete ptr;
    };

    ret = std::shared_ptr<std::vector<uint8_t>>(creater(), deleter);

    return ret;
}

std::shared_ptr<_jstring> CrossPLUtils::SafeCastString(JNIEnv* jenv, const char* data)
{
    std::shared_ptr<_jstring> ret;
    std::thread::id threadId = std::this_thread::get_id();

    if (data == nullptr) {
        return ret; // nullptr
    }

    auto creater = [=]() -> jstring {
        EnsureRunOnThread(threadId);
        jstring ptr = jenv->NewStringUTF(data);
        //__android_log_print(ANDROID_LOG_WARN, "crosspl", "%s %p", __PRETTY_FUNCTION__, ptr);
        return ptr;
    };

    auto deleter = [=](jstring ptr) -> void {
        EnsureRunOnThread(threadId);
        jenv->DeleteLocalRef(ptr);
        //__android_log_print(ANDROID_LOG_WARN, "crosspl", "%s %p", __PRETTY_FUNCTION__, ptr);
    };

    ret = std::shared_ptr<_jstring>(creater(), deleter);

    return ret;
}

std::shared_ptr<_jbyteArray> CrossPLUtils::SafeCastByteArray(JNIEnv* jenv, const std::span<uint8_t>* data)
{
    std::shared_ptr<_jbyteArray> ret;
    std::thread::id threadId = std::this_thread::get_id();

    if(data == nullptr) {
        return ret; // nullptr
    }

    auto creater = [=]() -> jbyteArray {
        EnsureRunOnThread(threadId);
        jbyteArray jdata = jenv->NewByteArray(data->size());
        jenv->SetByteArrayRegion(jdata, 0, data->size(), reinterpret_cast<jbyte*>(data->data()));
        //__android_log_print(ANDROID_LOG_WARN, "crosspl", "%s %p", __PRETTY_FUNCTION__, jdata);
        return jdata;
    };
    auto deleter = [=](jbyteArray ptr) -> void {
        EnsureRunOnThread(threadId);
        jenv->DeleteLocalRef(ptr);
        //__android_log_print(ANDROID_LOG_WARN, "crosspl", "%s %p", __PRETTY_FUNCTION__, ptr);
    };

    ret = std::shared_ptr<_jbyteArray>(creater(), deleter);

    return ret;
}

std::shared_ptr<_jobject> CrossPLUtils::SafeCastFunction(JNIEnv* jenv, const std::function<void()>* data)
{
    std::shared_ptr<_jobject> ret;
    std::thread::id threadId = std::this_thread::get_id();

    if(data == nullptr) {
        return ret; // nullptr
    }

    auto creater = [&]() -> jobject {
        EnsureRunOnThread(threadId);

        jclass jclazz = FindJavaClass(jenv, JavaClassNameRunnable);

//        jmethodID jconstructor  = jenv->GetMethodID(jclazz, "<init>", "()V");
//        jobject jobj = jenv->NewObject(jclazz, jconstructor);

 //       return jobj;
          return nullptr;
    };
    auto deleter = [=](jobject ptr) -> void {
        EnsureRunOnThread(threadId);
//        jenv->DeleteLocalRef(ptr);
    };

    ret = std::shared_ptr<_jobject>(creater(), deleter);

    return ret;
}

std::shared_ptr<_jobject> CrossPLUtils::SafeCastStringBuffer(JNIEnv* jenv, const std::stringstream* data)
{
    std::shared_ptr<_jobject> ret;
    std::thread::id threadId = std::this_thread::get_id();

    if(data == nullptr) {
        return ret; // nullptr
    }

    auto creater = [=]() -> jobject {
        EnsureRunOnThread(threadId);

        jclass jclazz = FindJavaClass(jenv, JavaClassNameStringBuffer);

        jmethodID jconstructor  = jenv->GetMethodID(jclazz, "<init>", "()V");
        jobject jobj = jenv->NewObject(jclazz, jconstructor);

        SafeCopyStringBufferToJava(jenv, jobj, data);

//        __android_log_print(ANDROID_LOG_WARN, "crosspl", "%s %p", __PRETTY_FUNCTION__, jobj);
        return jobj;
    };
    auto deleter = [=](jobject ptr) -> void {
        EnsureRunOnThread(threadId);
        jenv->DeleteLocalRef(ptr);
//        __android_log_print(ANDROID_LOG_WARN, "crosspl", "%s %p", __PRETTY_FUNCTION__, ptr);
    };

    ret = std::shared_ptr<_jobject>(creater(), deleter);

    return ret;
}

std::shared_ptr<_jobject> CrossPLUtils::SafeCastByteBuffer(JNIEnv* jenv, const std::vector<uint8_t>* data)
{
    std::shared_ptr<_jobject> ret;
    std::thread::id threadId = std::this_thread::get_id();

    if(data == nullptr) {
        return ret; // nullptr
    }

    auto creater = [=]() -> jobject {
        EnsureRunOnThread(threadId);

        jclass jclazz = FindJavaClass(jenv, JavaClassNameByteBuffer);

        jmethodID jconstructor  = jenv->GetMethodID(jclazz, "<init>", "()V");
        jobject jobj = jenv->NewObject(jclazz, jconstructor);

        SafeCopyByteBufferToJava(jenv, jobj, data);

        //__android_log_print(ANDROID_LOG_WARN, "crosspl", "%s %p", __PRETTY_FUNCTION__, jobj);
        return jobj;
    };
    auto deleter = [=](jobject ptr) -> void {
        EnsureRunOnThread(threadId);
        jenv->DeleteLocalRef(ptr);
        //__android_log_print(ANDROID_LOG_WARN, "crosspl", "%s %p", __PRETTY_FUNCTION__, ptr);
    };

    ret = std::shared_ptr<_jobject>(creater(), deleter);

    return ret;
}

int CrossPLUtils::SafeCopyStringBufferToCpp(JNIEnv* jenv, std::stringstream* copyTo, jobject jdata)
{
    if(copyTo == nullptr) {
        return 0; // nullptr
    }

    auto tmpPtr = SafeCastStringBuffer(jenv, jdata);
    copyTo->str(""); // clear stringstream
    copyTo->swap(*tmpPtr);

    return 0;
}

int CrossPLUtils::SafeCopyByteBufferToCpp(JNIEnv* jenv, std::vector<uint8_t>* copyTo, jobject jdata)
{
    if(copyTo == nullptr) {
        return 0; // nullptr
    }

    copyTo->clear();

    auto tmpPtr = SafeCastByteBuffer(jenv, jdata);
    if(tmpPtr.get() != nullptr) {
        copyTo->swap(*tmpPtr);
    }
    return 0;
}

int CrossPLUtils::SafeCopyStringBufferToJava(JNIEnv* jenv, jobject jcopyTo, const std::stringstream* data)
{
    if(jcopyTo == nullptr) {
        return 0; // nullptr
    }

    // clear exists content
    jclass jclazz = FindJavaClass(jenv, JavaClassNameStringBuffer);
    jmethodID jmethodSetLength  = jenv->GetMethodID(jclazz, "setLength", "(I)V");
    jenv->CallVoidMethod(jcopyTo, jmethodSetLength, 0);

    if(data == nullptr) {
        return 0; // nullptr
    }

    auto jstrPtr = SafeCastString(jenv, data->str().c_str());
    jmethodID jmethodAppend = jenv->GetMethodID(jclazz, "append", "(Ljava/lang/String;)Ljava/lang/StringBuffer;");
    auto jret = jenv->CallObjectMethod(jcopyTo, jmethodAppend, jstrPtr.get());
    jenv->DeleteLocalRef(jret);

    return 0;
}

int CrossPLUtils::SafeCopyByteBufferToJava(JNIEnv* jenv, jobject jcopyTo, const std::vector<uint8_t>* data)
{
    if(jcopyTo == nullptr) {
        return 0; // nullptr
    }

    // clear exists content
    jclass jclazz = FindJavaClass(jenv, JavaClassNameByteBuffer);
    jmethodID jmethodReset  = jenv->GetMethodID(jclazz, "clear", "()V");
    jenv->CallVoidMethod(jcopyTo, jmethodReset);

    if(data == nullptr) {
        return 0; // nullptr
    }

    auto spanData = std::span<uint8_t>(const_cast<uint8_t*>(data->data()), data->size());
    auto jbytesPtr = SafeCastByteArray(jenv, &spanData);
    jmethodID jmethodWrite  = jenv->GetMethodID(jclazz, "append", "([B)V");
    jenv->CallVoidMethod(jcopyTo, jmethodWrite, jbytesPtr.get());

    return 0;
}

int64_t CrossPLUtils::AddGlobalObject(JNIEnv* jenv, jobject jobj)
{
    if(jobj == nullptr) {
        return 0;
    }

    auto jGlobalObj = jenv->NewGlobalRef(jobj);
    auto obj = reinterpret_cast<int64_t>(jGlobalObj);

    return obj;
}

std::shared_ptr<_jobject> CrossPLUtils::SafeCastGlobalObject(JNIEnv* jenv, int64_t obj)
{
    std::shared_ptr<_jobject> ret;
    std::thread::id threadId = std::this_thread::get_id();

    if(obj == 0) {
        return nullptr;
    }

    auto creater = [=]() -> jobject {
        auto jobj = reinterpret_cast<jobject>(obj);

        EnsureRunOnThread(threadId);
        jobject ptr = jenv->NewLocalRef(jobj);
        //__android_log_print(ANDROID_LOG_WARN, "crosspl", "%s %p", __PRETTY_FUNCTION__, ptr);
        return ptr;
    };

    auto deleter = [=](jobject ptr) -> void {
        EnsureRunOnThread(threadId);
        jenv->DeleteLocalRef(ptr);
        //__android_log_print(ANDROID_LOG_WARN, "crosspl", "%s %p", __PRETTY_FUNCTION__, ptr);
    };

    ret = std::shared_ptr<_jobject>(creater(), deleter);

    return ret;
}

void CrossPLUtils::DelGlobalObject(JNIEnv* jenv, jobject jobj)
{
    if(jobj == nullptr) {
        return;
    }

    jenv->DeleteGlobalRef(jobj);
}

void* CrossPLUtils::SafeCastCrossObject(JNIEnv* jenv, jobject jdata)
{
    jclass jclazz = jenv->GetObjectClass(jdata);

    jfieldID jfield = jenv->GetFieldID(jclazz, "nativeHandle", "J");

    jlong jnativeHandle = jenv->GetLongField(jdata, jfield);

    return reinterpret_cast<void*>(jnativeHandle);
}

/***********************************************/
/***** class public function implement  ********/
/***********************************************/


/***********************************************/
/***** class protected function implement  *****/
/***********************************************/


/***********************************************/
/***** class private function implement  *******/
/***********************************************/


} // namespace crosspl
