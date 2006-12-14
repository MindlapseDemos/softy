csrc = $(wildcard src/*.c src/vmath/*.c)
ccsrc = $(wildcard src/*.cc src/vmath/*.cc)
obj = $(csrc:.c=.o) $(ccsrc:.cc=.o) src/libfixgl/libGL.a
bin = softy

dbg = -g
opt = -O3 -ffast-math

inc = -Isrc -Isrc/vmath -Isrc/libfixgl/src

CFLAGS = -Wall $(dbg) $(opt) $(inc) `sdl-config --cflags`
CXXFLAGS = -Wall $(dbg) $(opt) $(inc) `sdl-config --cflags`
LDFLAGS = `sdl-config --libs`

$(bin): $(obj)
	$(CXX) -o $@ $(obj) $(LDFLAGS)

src/libfixgl/libGL.a:
	cd src/libfixgl; make

.PHONY: clean
clean:
	rm -f $(obj) $(bin)
