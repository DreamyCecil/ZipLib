# source files
set(sources
    blocksort.c
    bzerror.c
    bzlib_private.h
    bzlib.c
    bzlib.h
    compress.c
    crctable.c
    decompress.c
    huffman.c
    randtable.c
)
# library
add_library(bzip2 STATIC
    ${sources}
)
#properties
target_include_directories(bzip2
    PUBLIC
        .
)
set_target_properties(bzip2
    PROPERTIES
        FOLDER "External libraries"
)
