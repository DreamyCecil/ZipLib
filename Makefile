EXE      = Bin/ziplib.so

#####################
CC       = gcc
CXX      = g++-4.7
CFLAGS   = -O3
CXXFLAGS = -std=c++11 -03
LDFLAGS  = 
SRCSZL   = $(wildcard Source/ZipLib/extlibs/zlib/*.c)
SRCS     = $(wildcard Source/ZipLib/*.cpp)
OBJS     = $(patsubst %.cpp,%.o,$(SRCS))
OBJSZL   = $(patsubst %.c,%.o,$(SRCSZL))

$(EXE): $(OBJSZL) $(OBJS)
	$(CXX) -shared -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf Source/ZipLib/extlibs/zlib/*.o Source/ZipLib/*.o ziplib.tar.gz $(EXE)

tarball:
	tar -zcvf ziplib.tar.gz *
