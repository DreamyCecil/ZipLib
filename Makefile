EXE      = ziplib.so

#####################
CC       = gcc
CXX      = g++-4.7
CFLAGS   = -O3
CXXFLAGS = -std=c++11
LDFLAGS  = -03
SRCSZL   = $(wildcard extlibs/zlib/*.c)
SRCS     = $(wildcard *.cpp)
OBJS     = $(patsubst %.cpp,%.o,$(SRCS))
OBJSZL   = $(patsubst %.c,%.o,$(SRCSZL))

$(EXE): $(OBJSZL) $(OBJS)
	$(CXX) -shared -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf *.o $(EXE) ziplib.tar.gz
	rm -rf extlibs/zlib/*.o

tarball:
	tar -zcvf ziplib.tar.gz *
