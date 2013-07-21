set(MODULE_NAME SimpleVideoCapture)

file(GLOB sources "*.cpp")
file(GLOB headers "*.h")

if (BUILD_STATIC)
	set(SVC_LIB_TYPE STATIC)
else (BUILD_STATIC)
	set(SVC_LIB_TYPE SHARED)
endif(BUILD_STATIC)

include_directories(
	${OPENCV_INCLUDE_DIR}
	${FFMPEG_INCLUDE_DIR}
)

add_library(
	${MODULE_NAME} ${SVC_LIB_TYPE}
	${sources}
	${headers}
)

target_link_libraries(
	${MODULE_NAME}
	${OpenCV_LIBS}
	${FFMPEG_LIBRARIES}
)
