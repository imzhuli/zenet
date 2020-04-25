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

set(ze_dynamic zecore_shared)

if (WIN32)
	set(libevent libevent_core libevent_extras ws2_32)
else()
	set(libevent event_core event_extra event_pthreads)
endif()

set(shared_lib_dependency_private ${libevent} ${ze} ${cpp})
set(shared_lib_dependency_public)

#static lib:
add_library(${libname} STATIC ${SRC})

#dynamic lib
add_library(${libname}_dynamic SHARED ${SRC})
target_link_libraries(${libname}_dynamic PRIVATE ${shared_lib_dependency_private})
set_target_properties(${libname}_dynamic PROPERTIES VERSION 1.0)
set_target_properties(${libname}_dynamic PROPERTIES OUTPUT_NAME ${libname}_shared)

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
