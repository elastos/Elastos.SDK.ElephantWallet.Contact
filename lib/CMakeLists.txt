cmake_minimum_required(VERSION 3.12)

include(../config/cmake/CMakeLists.txt)

#if(("${CFG_TARGET_PLATFORM}" STREQUAL "Android") OR ("${CFG_TARGET_PLATFORM}" STREQUAL "iOS"))
#add_definitions(-DWITH_CROSSPL)
#endif()

include_directories("${CMAKE_INSTALL_PREFIX}/include")

project(Elastos.SDK.Contact)
add_subdirectory(api)
add_subdirectory(common)
add_subdirectory(src)
add_subdirectory(platform)
add_subdirectory(wrapper/httpclient)
add_subdirectory(wrapper/json)

if(DEFINED CFG_WITH_TEST)
	add_subdirectory(test)
endif()

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/Elastos.SDK.Contact.pc.in
	${CMAKE_CURRENT_BINARY_DIR}/Elastos.SDK.Contact.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Elastos.SDK.Contact.pc DESTINATION lib/pkgconfig)
