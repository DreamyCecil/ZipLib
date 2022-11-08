# source files
set(sources
    adler32.c
    compress.c
    crc32.c
    crc32.h
    deflate.c
    deflate.h
    gzguts.h
    infback.c
    inffast.c
    inffast.h
    inffixed.h
    inflate.c
    inflate.h
    inftrees.c
    inftrees.h
    trees.c
    trees.h
    uncompr.c
    zconf.h
    zlib.h
    zutil.c
    zutil.h
)
# library
add_library(zlib STATIC
    ${sources}
)
# properties
target_include_directories(zlib
    PUBLIC
        .
)
set_target_properties(zlib
    PROPERTIES
        FOLDER "External libraries"
)
