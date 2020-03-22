project(libzenet)

include_directories(
	./include/
	../../zecore/zecore/include/
	)
link_directories(
	~/.local/lib/
	)

aux_source_directory(./src/ ZENET_SRC)

if (CMAKE_BUILD_TYPE MATCHES "Release")
	set(ze zecore_r)
else()
	set(ze zecore)
endif()

set(libevent event_core event_extra event_pthreads)
set(shared_lib_dependency_private ${libevent} ${libevent} ${ze} ${cpp})
set(shared_lib_dependency_public)

#static libs:
add_library(zenet STATIC ${ZENET_SRC})
if (CMAKE_BUILD_TYPE MATCHES "Release")
	set_target_properties(zenet PROPERTIES OUTPUT_NAME zenet_r)
else()
	set_target_properties(zenet PROPERTIES OUTPUT_NAME zenet)
endif()

add_library(zenet_dynamic SHARED ${ZENET_SRC})
target_link_libraries(zenet_dynamic PRIVATE ${shared_lib_dependency_private})
target_link_libraries(zenet_dynamic PUBLIC  ${shared_lib_dependency_public})
set_target_properties(zenet_dynamic PROPERTIES VERSION 1.0)
if (CMAKE_BUILD_TYPE MATCHES "Release")
	set_target_properties(zenet_dynamic PROPERTIES OUTPUT_NAME zenet_r)
else()
	set_target_properties(zenet_dynamic PROPERTIES OUTPUT_NAME zenet)
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
