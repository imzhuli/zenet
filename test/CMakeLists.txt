project(test_zenet)

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
	../zenet/include/
	${CMAKE_INSTALL_PREFIX}/include/
	${LIBEVENT_INCLUDE_DIRS}
	)
link_directories(
	${CMAKE_INSTALL_PREFIX}/lib/
	)

message(STATE ${CMAKE_INSTALL_PREFIX}/lib/)
set(test_dependency zenet_shared zecore_shared ${cpp})

aux_source_directory(./libevent TEST_LIBEVENT_SRC)
add_executable(test_libevent ${TEST_LIBEVENT_SRC})
target_link_libraries(test_libevent ${test_dependency})

add_test(test_libevent ${EXECUTABLE_OUTPUT_PATH}/test_libevent)
