# ZipLib

ZipLib is a lightweight C++11 library for working with ZIP archives with ease. The very useful feature of ZipLib is that **it is built around STL streams, no additional dependencies (boost) are required**.

It is a fork of https://bitbucket.org/wbenny/ziplib with own adjustments and unmerged fixes from issues and pull requests from the original repository.

## Features

- Compression/decompresion using the DEFLATE algorithm
- Storing/fetching files without compression
- Adding, editing and removing files and directories in the archive
- Support of the data descriptor
- Data flow is driven via STL streams, therefore there is no need to load huge amount of data into the memory
- Support of PKWare encryption
- Support of entry comments and archive comments
- Contains very usable stream classes in `ZipLib/streams` like memory stream, substream or teestream
- Contains functions for easy (de)serialization (`ZipLib/streams/serialization.h`)
- Project is using only C++11 standard libraries, therefore there IS NO dependency on 3rd party libraries (except of included zlib, which uses C standard library)
- Built around STL smart pointers
- Support of Windows and Linux

## License

Original software belongs to Petr Benes (see `Licence.txt` file).
