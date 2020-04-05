project(libzenet)
set(libname zenet)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

include_directories(
	./include/
	${CMAKE_INSTALL_PREFIX}/include/
	)
link_directories(
	${CMAKE_INSTALL_PREFIX}/lib/
	)

aux_source_directory(./src/ SRC)

if (CMAKE_BUILD_TYPE MATCHES "Release")
	set(ze_dynamic zecore_shared_r)
else()
	set(ze_dynamic zecore_shared)
endif()

if (WIN32)
	set(libevent libevent_core libevent_extras ws2_32)
else()
	set(libevent event_core event_extra event_pthreads)
endif()

set(shared_lib_dependency_private ${libevent} ${ze} ${cpp})
set(shared_lib_dependency_public)

#static libs:
add_library(${libname} STATIC ${SRC})
if (CMAKE_BUILD_TYPE MATCHES "Release")
	set_target_properties(${libname} PROPERTIES OUTPUT_NAME ${libname}_r)
else()
	set_target_properties(${libname} PROPERTIES OUTPUT_NAME ${libname})
endif()

add_library(${libname}_dynamic SHARED ${SRC})
target_link_libraries(${libname}_dynamic PRIVATE ${shared_lib_dependency_private})
set_target_properties(${libname}_dynamic PROPERTIES VERSION 1.0)
if (CMAKE_BUILD_TYPE MATCHES "Release")
	set_target_properties(${libname}_dynamic PROPERTIES OUTPUT_NAME ${libname}_shared_r)
else()
	set_target_properties(${libname}_dynamic PROPERTIES OUTPUT_NAME ${libname}_shared)
endif()

install(
	DIRECTORY ${PROJECT_SOURCE_DIR}/./include/zenet
	DESTINATION include
	FILES_MATCHING PATTERN "*.h"
	)
install(TARGETS zenet zenet_dynamic
	RUNTIME DESTINATION bin
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
	)
