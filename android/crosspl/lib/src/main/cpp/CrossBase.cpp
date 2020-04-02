//
//  CrossBase.cpp
//
//  Created by mengxk on 19/03/16.
//  Copyright © 2016 mengxk. All rights reserved.
//

#include "CrossBase.hpp"

#include <android/log.h>
#include <CrossPLFactory.hpp>
#include <list>
#include <string>


/***********************************************/
/***** static variables initialize *************/
/***********************************************/
#define FORMAT_METHOD GetFormatMethod(__PRETTY_FUNCTION__).c_str()

/*** static function and variable ***/
static std::string GetFormatMethod(const std::string& prettyFunction) {
    size_t colons = prettyFunction.find("::");
    size_t begin = prettyFunction.substr(0,colons).rfind(" ") + 1;
    size_t end = prettyFunction.rfind("(") - begin;
    std::string method = prettyFunction.substr(begin,end) + "()";
    return method;
}


/***********************************************/
/***** static function implement ***************/
/***********************************************/
static auto gCreateCppObjFuncList = std::list<int64_t(*)(const char*)>();
static auto gDestroyCppObjFuncList = std::list<int(*)(const char*,int64_t)>();
static auto gCreateJavaObjFuncList = std::list<int64_t(*)(const char*,int64_t)>();
static auto gDestroyJavaObjFuncList = std::list<int(*)(const char*,int64_t)>();

void RegCreateCppObjFunc(int64_t(*func)(const char*))
{
    __android_log_print(ANDROID_LOG_DEBUG, "crosspl", "%s", FORMAT_METHOD);
    gCreateCppObjFuncList.push_back(func);
}
void RegDestroyCppObjFunc(int(*func)(const char*,int64_t))
{
    __android_log_print(ANDROID_LOG_DEBUG, "crosspl", "%s", FORMAT_METHOD);
    gDestroyCppObjFuncList.push_back(func);
}

void RegCreateJavaObjFunc(int64_t(*func)(const char*,int64_t))
{
    __android_log_print(ANDROID_LOG_DEBUG, "crosspl", "%s", FORMAT_METHOD);
    gCreateJavaObjFuncList.push_back(func);
}
void RegDestroyJavaObjFunc(int(*func)(const char*,int64_t))
{
    __android_log_print(ANDROID_LOG_DEBUG, "crosspl", "%s", FORMAT_METHOD);
    gDestroyJavaObjFuncList.push_back(func);
}

namespace crosspl {
namespace native {

int64_t CrossBase::CreateNativeObject(const char* javaClassName)
{
    __android_log_print(ANDROID_LOG_DEBUG, "crosspl", "%s", FORMAT_METHOD);

    for(auto func: gCreateCppObjFuncList) {
        auto cppHandle = func(javaClassName);
        if(cppHandle != 0) { // success
            return cppHandle;
        }
    }

    // need return before here.
    auto ex = std::string("CrossPL: Failed to create cpp object.") + javaClassName;
    throw std::runtime_error(ex);
}

void CrossBase::DestroyNativeObject(const char* javaClassName, int64_t nativeHandle)
{
    __android_log_print(ANDROID_LOG_DEBUG, "crosspl", "%s", FORMAT_METHOD);

    for(auto func: gDestroyCppObjFuncList) {
        auto cppHandle = func(javaClassName, nativeHandle);
        if(cppHandle == 0) { // success
            return;
        }
    }

    // need return before here.
    auto ex = std::string("CrossPL: Failed to destroy cpp object.") + javaClassName;
    throw std::runtime_error(ex);
}

int64_t CrossBase::CreatePlatformObject(const char* cppClassName, int64_t nativeHandle)
{
    __android_log_print(ANDROID_LOG_DEBUG, "crosspl", "%s", FORMAT_METHOD);

    for(auto func: gCreateJavaObjFuncList) {
        auto javaHandle = func(cppClassName, nativeHandle);
        if(javaHandle != 0) { // success
            return javaHandle;
        }
    }

    // need return before here.
    auto ex = std::string("CrossPL: Failed to create java object.") + cppClassName;
    throw std::runtime_error(ex);
}

void CrossBase::DestroyPlatformObject(const char* cppClassName, int64_t platformHandle)
{
    __android_log_print(ANDROID_LOG_DEBUG, "crosspl", "%s", FORMAT_METHOD);

    for(auto func: gDestroyJavaObjFuncList) {
        auto javaHandle = func(cppClassName, platformHandle);
        if(javaHandle == 0) { // success
            return;
        }
    }

    // need return before here.
    auto ex = std::string("CrossPL: Failed to destroy java object.") + cppClassName;
    throw std::runtime_error(ex);
}

/***********************************************/
/***** class public function implement  ********/
/***********************************************/
//CrossBase::CrossBase(int64_t platformHandle)
//    : mPlatformHandle(platformHandle)
//{
//    if(platformHandle == 0) {
//        platformHandle = CreatePlatformObject(typeid(this).name(), reinterpret_cast<int64_t>(this));
//    }
//}

CrossBase::CrossBase()
//    : CrossBase(0)
    : mPlatformHandle(0)
{
}

CrossBase::~CrossBase()
{
//    if(mPlatformHandle == 0) {
//        for(auto func: gDestroyJavaObjFuncList) {
//            int ret = func(typeid(this).name(), mPlatformHandle);
//            if(ret == 0) { // success
//                break;
//            }
//        }
//    }
}

void CrossBase::bindPlatformHandle(int64_t platformHandle)
{
    mPlatformHandle = platformHandle;
}

void CrossBase::unbindPlatformHandle(int64_t platformHandle)
{
    mPlatformHandle = 0;
}

int64_t CrossBase::getPlatformHandle() const
{
    return mPlatformHandle;
}


/***********************************************/
/***** class protected function implement  *****/
/***********************************************/


/***********************************************/
/***** class private function implement  *******/
/***********************************************/

} //namespace native
} //namespace crosspl