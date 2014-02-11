EXE     = Bin/zipsample
SO      = Bin/libzip.so

#####################

#CC       = gcc
#CXX      = g++-4.8
CC       = clang
CXX      = clang++
CFLAGS   = -fPIC -Wno-enum-conversion -O3
CXXFLAGS = -fPIC -std=c++11 -O3
LDFLAGS  = -pthread
SRCSZL   = $(wildcard Source/ZipLib/extlibs/zlib/*.c)
SRCSLZ   = $(wildcard Source/ZipLib/extlibs/lzma/unix/*.c)
SRCSBZ   = $(wildcard Source/ZipLib/extlibs/bzip2/*.c)
SRCS     = $(wildcard Source/ZipLib/*.cpp)
SRCSDET  = $(wildcard Source/ZipLib/detail/*.cpp)
OBJS     = $(patsubst %.cpp,%.o,$(SRCS))
OBJSDET  = $(patsubst %.cpp,%.o,$(SRCSDET))
OBJSZL   = $(patsubst %.c,%.o,$(SRCSZL))
OBJSLZ   = $(patsubst %.c,%.o,$(SRCSLZ))
OBJSBZ   = $(patsubst %.c,%.o,$(SRCSBZ))

all: $(EXE) $(SO)

$(EXE): $(OBJSZL) $(OBJSLZ) $(OBJSBZ) $(OBJS) $(OBJSDET)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) Source/Sample/Main.cpp -o $@ $^

$(SO): $(OBJSZL) $(OBJSLZ) $(OBJSBZ) $(OBJS) $(OBJSDET)
	$(CXX) $(LDFLAGS) -shared -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf Source/ZipLib/extlibs/zlib/*.o Source/ZipLib/detail/*.o Source/ZipLib/extlibs/lzma/unix/*.o Source/ZipLib/extlibs/bzip2/*.o Source/ZipLib/*.o ziplib.tar.gz $(SO) $(EXE) Bin/*.zip Bin/out*

tarball:
	tar -zcvf ziplib.tar.gz *
	