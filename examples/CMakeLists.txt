include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${SVI_SRC_DIR}
)

add_executable(
	basic_example
	basic_example.cpp
)

target_link_libraries(
	basic_example
	SimpleVideoInput
)

add_executable(
	file_source
	file_source.cpp
)

target_link_libraries(
	file_source
	SimpleVideoInput
)

find_package(curl)
if (CURL_FOUND)
	include_directories(CURL_INCLUDE_DIRS)

	add_executable(
		web_source
		web_source.cpp
	)

	target_link_libraries(
		web_source
		SimpleVideoInput
		${CURL_LIBRARIES}
	)
endif (CURL_FOUND)
