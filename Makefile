csrc = $(wildcard src/*.c src/vmath/*.c)
ccsrc = $(wildcard src/*.cc src/vmath/*.cc)
obj = $(csrc:.c=.o) $(ccsrc:.cc=.o)
bin = softy

dbg = -g
#opt = -O3 -ffast-math

inc = -Isrc -Ivmath

CFLAGS = -std=c89 -pedantic -Wall $(dbg) $(opt) $(inc) `sdl-config --cflags`
CXXFLAGS = -ansi -pedantic -Wall $(dbg) $(opt) $(inc) `sdl-config --cflags`
LDFLAGS = `sdl-config --libs`

$(bin): $(obj)
	$(CXX) -o $@ $(obj) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) $(bin)
