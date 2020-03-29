pkg_search_module(pkg-Elastos.SDK.Keypair.C REQUIRED Elastos.SDK.Keypair.C)
message(STATUS "Elastos.SDK.Keypair.C include path: ${pkg-Elastos.SDK.Keypair.C_INCLUDE_DIRS}")
message(STATUS "Elastos.SDK.Keypair.C library path: ${pkg-Elastos.SDK.Keypair.C_STATIC_LDFLAGS}")

pkg_search_module(pkg-curl REQUIRED libcurl)
message(STATUS "curl include path: ${pkg-curl_INCLUDE_DIRS}")
message(STATUS "curl library path: ${pkg-curl_STATIC_LDFLAGS}")

pkg_search_module(pkg-oss REQUIRED PersonalStorage.SDK.OSS)
message(STATUS "PersonalStorage.SDK.OSS include path: ${pkg-oss_INCLUDE_DIRS}")
message(STATUS "PersonalStorage.SDK.OSS library path: ${pkg-oss_STATIC_LDFLAGS}")

set(pkg-Elastos.NET.Carrier_STATIC_LDFLAGS
    elacarrier
    elafiletrans
    elasession
    cjson
    flatccrt
    pjmedia
    pjnath
    pjlib-util
    pj
    crystal
    srtp
    toxcore
    sodium)

file( GLOB SRC_SOURCES "*.cpp" "*/*.cpp")
file( GLOB SRC_HEADERS "*.hpp" )

if(${CMAKE_VERSION} VERSION_LESS "3.12")
    add_library(src STATIC ${SRC_SOURCES})
else()
    add_library(src OBJECT ${SRC_SOURCES})
endif()
target_include_directories(src PRIVATE
    "${pkg-Elastos.SDK.Keypair.C_INCLUDE_DIRS}"
    "${pkg-curl_INCLUDE_DIRS}"
    "${pkg-oss_INCLUDE_DIRS}")
target_include_directories(src PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(src PRIVATE
    httpclient
    json)
target_link_libraries(src PUBLIC
    ${pkg-curl_STATIC_LDFLAGS}
    ${pkg-oss_STATIC_LDFLAGS}
    ${pkg-Elastos.SDK.Keypair.C_STATIC_LDFLAGS}
    ${pkg-Elastos.NET.Carrier_STATIC_LDFLAGS}
    crypto
    ssl
    common
    httpclient
    platform)
#if("${CFG_TARGET_PLATFORM}" STREQUAL "Android")
    #target_link_libraries(src PRIVATE log)
#elseif("${CFG_TARGET_PLATFORM}" STREQUAL "iOS")
    #target_link_libraries(src PRIVATE "-framework CFNetwork")
#endif()
