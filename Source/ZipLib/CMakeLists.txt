add_subdirectory(extlibs)
# source files
set(sources_compression
    compression/bzip2/bzip2_decoder_properties.h
    compression/bzip2/bzip2_decoder.h
    compression/bzip2/bzip2_encoder_properties.h
    compression/bzip2/bzip2_encoder.h
    compression/deflate/deflate_decoder_properties.h
    compression/deflate/deflate_decoder.h
    compression/deflate/deflate_encoder_properties.h
    compression/deflate/deflate_encoder.h
    compression/lzma/detail/lzma_alloc.h
    compression/lzma/detail/lzma_handle.h
    compression/lzma/detail/lzma_header.h
    compression/lzma/detail/lzma_in_stream.h
    compression/lzma/detail/lzma_out_stream.h
    compression/lzma/lzma_decoder_properties.h
    compression/lzma/lzma_decoder.h
    compression/lzma/lzma_encoder_properties.h
    compression/lzma/lzma_encoder.h
    compression/store/store_decoder_properties.h
    compression/store/store_decoder.h
    compression/store/store_encoder_properties.h
    compression/store/store_encoder.h
    compression/compression_interface.h    
)
set(sources_detail
    detail/EndOfCentralDirectoryBlock.cpp
    detail/EndOfCentralDirectoryBlock.h
    detail/ZipCentralDirectoryFileHeader.cpp
    detail/ZipCentralDirectoryFileHeader.h
    detail/ZipGenericExtraField.cpp
    detail/ZipGenericExtraField.h
    detail/ZipLocalFileHeader.cpp
    detail/ZipLocalFileHeader.h
)
set(sources_methods
    methods/Bzip2Method.h
    methods/DeflateMethod.h
    methods/ICompressionMethod.h
    methods/LzmaMethod.h
    methods/StoreMethod.h
    methods/ZipMethodResolver.h
)
set(sources_streams
    streams/streambuffs/compression_decoder_streambuf.h
    streams/streambuffs/compression_encoder_streambuf.h
    streams/streambuffs/crc32_streambuf.h
    streams/streambuffs/mem_streambuf.h
    streams/streambuffs/null_streambuf.h
    streams/streambuffs/sub_streambuf.h
    streams/streambuffs/tee_streambuff.h
    streams/streambuffs/zip_crypto_streambuf.h
    streams/compression_decoder_stream.h
    streams/compression_encoder_stream.h
    streams/crc32stream.h
    streams/memstream.h
    streams/nullstream.h
    streams/serialization.h
    streams/substream.h
    streams/teestream.h
    streams/zip_cryptostream.h
)
set(sources_utils
    utils/enum_utils.h
    utils/stream_utils.h
    utils/time_utils.h
)
set(sources
    ZipArchive.cpp
    ZipArchive.h
    ZipArchiveEntry.cpp
    ZipArchiveEntry.h
    ZipFile.cpp
    ZipFile.h
)
source_group("Sources/Compression" FILES ${sources_compression})
source_group("Sources/Detail" FILES ${sources_detail})
source_group("Sources/Methods" FILES ${sources_methods})
source_group("Sources/Streams" FILES ${sources_streams})
source_group("Sources/Utils" FILES ${sources_utils})
source_group("Sources" FILES ${sources})
# library
add_library(ZipLib STATIC
    ${sources_compression} ${sources_detail} ${sources_methods} ${sources_streams} ${sources_utils} ${sources}
)
# properties
target_include_directories(ZipLib
    PUBLIC
        .
)
target_compile_features(ZipLib_Sample PRIVATE cxx_std_11)
target_link_libraries(ZipLib
    PRIVATE
        bzip2
        lzma
        zlib
)
