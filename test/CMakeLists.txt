project(test_zenet)

include_directories(
	../zenet/include/
	../../zecore/zecore/include/
	${libevent_include_dir}
	)
link_directories(
	${CMAKE_INSTALL_PREFIX}/lib/
	${libevent_lib_dir}
)

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

set(test_dependency zenet_dynamic ${libevent} ${ze_dynamic} ${cpp})

aux_source_directory(./libevent TEST_LIBEVENT_SRC)
add_executable(test_libevent ${TEST_LIBEVENT_SRC})
target_link_libraries(test_libevent ${test_dependency})

add_test(test_libevent ${EXECUTABLE_OUTPUT_PATH}/test_libevent)
