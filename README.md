SimpleVideoInput
================

A simple class, that uses ffmpeg to create OpenCV images from local video files.

It's desgined to resemble cv::VideoCapture (which uses ffmpeg too) and just serves as a personal ffmpeg/C++11 exercise for me.

##Building##

Just copy the contents from the `src` directory to your project. Make sure you have OpenCV, libavcodec, libavformat and libswscale installed and linked. You'll also need C++11 support enabled (`-std=c++0x` with gcc <= 4.6 and `-std=c++11` with clang/gcc4.7+).

You can also build and link SimpleVideoInput as a shared library.

```bash
$ mkdir build
$ cd build
$ cmake .. -DWITH_EXAMPLES=true
$ make -j5
$ # For testing:
# bin/basic_example myVideoFile.mp4
```

Provide `-DBUILD_STATIC=true` to CMake if you wish a static library instead.

##Usage##

See `examples/basic_example.cpp` for how to use SimpleVideoInput.
