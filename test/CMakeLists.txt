project(test_zenet)

include_directories(
	./include/
	../zenet/include/
	${CMAKE_INSTALL_PREFIX}/include/
	)
link_directories(
	${CMAKE_INSTALL_PREFIX}/lib/
	)

set(ze_dynamic zecore_shared)
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
