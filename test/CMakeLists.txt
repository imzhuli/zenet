project(test_zenet)


include_directories(
	../zenet/include/
	../../zecore/zecore/include/
	)
link_directories(
	~/.local/lib/
)

if (CMAKE_BUILD_TYPE MATCHES "Release")
	set(ze zecore_r)
else()
	set(ze zecore)
endif()
set(libevent event_core event_extra event_pthreads)
set(test_dependency ${ze} zenet_dynamic ${cpp})

aux_source_directory(./libevent TEST_LIBEVENT_SRC)
add_executable(test_libevent ${TEST_LIBEVENT_SRC})
target_link_libraries(test_libevent ${libevent} )
target_link_libraries(test_libevent ${test_dependency})
add_test(test_libevent ${EXECUTABLE_OUTPUT_PATH}/test_libevent)
