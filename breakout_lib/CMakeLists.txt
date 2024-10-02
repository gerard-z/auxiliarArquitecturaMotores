
# We are listing each file independently as recommended by CMake creators
# ref: https://stackoverflow.com/questions/1027247/is-it-better-to-specify-source-files-with-glob-or-each-file-individually-in-cmak
# ref2: https://cmake.org/cmake/help/latest/command/file.html#glob

# we don't need to specify headers, only source files
# headers are referenced from inside each cpp file
add_library(breakout_lib STATIC
    "private/Ball.cpp"
    "private/Block.cpp"
    "private/Paddle.cpp"
)

# We need to tell this library where to look for the headers referenced in the source files
target_include_directories(breakout_lib PRIVATE ${MONA_INCLUDE_DIRECTORY} ${THIRD_PARTY_INCLUDE_DIRECTORIES} "${CMAKE_CURRENT_SOURCE_DIR}/public")
target_link_libraries(breakout_lib  PRIVATE MonaEngine)

set_property(TARGET breakout_lib PROPERTY CXX_STANDARD 20)