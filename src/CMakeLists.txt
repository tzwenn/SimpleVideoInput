set(MODULE_NAME SimpleVideoCapture)

file(GLOB sources "*.cpp")
file(GLOB headers "*.h")

include_directories(
	${OPENCV_INCLUDE_DIR}
	${FFMPEG_INCLUDE_DIR}
)

add_library(
	${MODULE_NAME} SHARED
	${sources}
	${headers}
)

target_link_libraries(
	${MODULE_NAME}
	${OpenCV_LIBS}
	${FFMPEG_LIBRARIES}
)
