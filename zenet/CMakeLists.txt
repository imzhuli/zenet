project(libzenet)
set(libname zenet)

# enable specific macors by system type:
if (CMAKE_SYSTEM_NAME MATCHES "MSYS")
	set(libevent_LIBRARIES event_core event_extra event_pthreads)
elseif (CMAKE_SYSTEM_NAME MATCHES "Windows")
	find_package(Libevent CONFIG REQUIRED)
	set(libevent_LIBRARIES event_shared event_core_shared event_extra_shared)
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
	set(libevent_LIBRARIES event_core event_extra event_pthreads)
elseif (CMAKE_SYSTEM_NAME MATCHES "Darwin")
	set(libevent_LIBRARIES event_core event_extra event_pthreads)
endif()

include_directories(
	./include/
	${CMAKE_INSTALL_PREFIX}/include/
	${LIBEVENT_INCLUDE_DIRS}
	)
link_directories(
	${CMAKE_INSTALL_PREFIX}/lib/
	)

# source files:
aux_source_directory(./src/ SRC)

# dependencies:
set(shared_lib_dependency_private ${libevent_LIBRARIES} zecore_shared ${cpp})
set(shared_lib_dependency_public)

#static lib:
add_library(${libname} STATIC ${SRC})
target_compile_definitions(${libname} PRIVATE -D zecore_EXPORT)

#dynamic lib:
add_library(${libname}_shared SHARED ${SRC})
target_link_libraries(${libname}_shared PRIVATE ${shared_lib_dependency_private})
set_target_properties(${libname}_shared PROPERTIES VERSION 1.0)
target_compile_definitions(${libname}_shared PRIVATE -D zecore_EXPORT)

#install
install(
	DIRECTORY ${PROJECT_SOURCE_DIR}/./include/${libname}
	DESTINATION include
	FILES_MATCHING PATTERN "*.h"
	)
install(TARGETS ${libname} ${libname}_shared
	RUNTIME DESTINATION bin
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
	)
