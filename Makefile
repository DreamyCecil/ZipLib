EXE     = Bin/zipsample
SO      = Bin/libzip.so

#####################
CC       = gcc
CXX      = g++-4.7
CFLAGS   = -fPIC -O3
CXXFLAGS = -fPIC -std=c++11 -O3
LDFLAGS  = 
SRCSZL   = $(wildcard Source/ZipLib/extlibs/zlib/*.c)
SRCS     = $(wildcard Source/ZipLib/*.cpp)
OBJS     = $(patsubst %.cpp,%.o,$(SRCS))
OBJSZL   = $(patsubst %.c,%.o,$(SRCSZL))

all: $(EXE) $(SO)

$(EXE): $(OBJSZL) $(OBJS)
	$(CXX) $(CXXFLAGS) Source/Sample/Main.cpp -o $@ $^

$(SO): $(OBJSZL) $(OBJS)
	$(CXX) -shared -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf Source/ZipLib/extlibs/zlib/*.o Source/ZipLib/*.o ziplib.tar.gz $(SO) $(EXE) Bin/*.zip Bin/out*

tarball:
	tar -zcvf ziplib.tar.gz *
	