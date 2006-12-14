obj :=
lib := libGL.a

#dbg := -g
opt := -O3 -ffast-math

CC := gcc
CFLAGS := -Wall $(dbg) $(opt) -DHAVE_INLINE
#-DDBG_USE_FLOAT

## if you get undefined symbols like uint32_t etc, and you know your
## compiler/C library has the stdint.h file, uncomment this line.
# CFLAGS += -DHAVE_STDINT

include src/makefile.part

$(lib): $(obj)
	$(AR) rcs $@ $(obj)

.PHONY: example
example:
	cd example; make

-include $(obj:.o=.d)

%.d: %.c
	@$(RM) $@; $(CC) -MM $(CFLAGS) $< > $@

.PHONY: clean
clean:
	$(RM) $(obj)

.PHONY: cleandep
cleandep:
	$(RM) $(obj:.o=.d)
