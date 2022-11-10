# source files
set(sources
    Main.cpp
)
# executable
add_executable(ZipLib_Sample
    ${sources}
)
# properties
target_compile_features(ZipLib_Sample PRIVATE cxx_std_11)
target_compile_definitions(ZipLib_Sample PRIVATE ZIPLIB_BZIP2 ZIPLIB_LZMA ZIPLIB_ZLIB)
target_link_libraries(ZipLib_Sample
    PRIVATE
        ZipLib
)
# configuration
configure_file(in1.jpg ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(in2.png ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(in3.txt ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
