project(libzenet)
set(libname zenet)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

include_directories(
	./include/
	../../zecore/zecore/include/
	)
link_directories(
	~/.local/lib/
	)

aux_source_directory(./src/ SRC)

if (CMAKE_BUILD_TYPE MATCHES "Release")
	set(ze_dynamic zecore_shared_r)
else()
	set(ze_dynamic zecore_shared)
endif()
set(libevent event_core event_extra event_pthreads)
set(shared_lib_dependency_private ${libevent} ${libevent} ${ze_dynamic} ${cpp})
set(shared_lib_dependency_public)

#output libs:
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
