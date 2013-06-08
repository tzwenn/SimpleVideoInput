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
	SimpleVideoCapture
)
