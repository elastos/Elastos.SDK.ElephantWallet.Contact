cmake_minimum_required(VERSION 3.4.1)

find_package(PkgConfig REQUIRED)
set(ENV{PKG_CONFIG_PATH}  "${CFG_DEPENDS_DIR}/${ANDROID_ABI}/lib/pkgconfig")
include_directories("${CFG_DEPENDS_DIR}/${ANDROID_ABI}/include")
add_definitions(-DWITH_CROSSPL)

include( "${CROSSPL_CMAKEFILE}" )
include_directories(base/api)
include_directories(base/include)

add_subdirectory(base/api)
add_subdirectory(base/common)
add_subdirectory(base/src)
add_subdirectory(base/platform)
add_subdirectory(base/wrapper/httpclient)
add_subdirectory(base/wrapper/json)

file( GLOB Elastos.SDK.Contact.Jni-SOURCES "*.cpp")
add_library(Elastos.SDK.Contact.Jni SHARED
        ${Elastos.SDK.Contact.Jni-SOURCES}
        ${CROSSPL_PROXY_SOURCES} )

target_link_libraries( Elastos.SDK.Contact.Jni Elastos.SDK.Contact )
target_link_libraries( Elastos.SDK.Contact.Jni json )
target_link_libraries( Elastos.SDK.Contact.Jni log )

target_include_directories( Elastos.SDK.Contact.Jni PRIVATE "${CROSSPL_LIB_HEADERDIR}" )
target_link_libraries( Elastos.SDK.Contact.Jni ${CROSSPL_LIB_LIBS} )
