SUBDIRS=# 1-make

.PHONY: all check clean
all check clean: $(SUBDIRS)

all: TARGET=all
check: TARGET=check
clean: TARGET=clean

# No, you can't do TARGET=$@, or at least I don't know how to.

# recursive call to make
$(SUBDIRS): force
	$(MAKE) -s -C $@ $(TARGET)

clean:
	rm -f *~ *.bak

test: mem-op.S test.c 2-wait-copy.c
	arm-none-eabi-gcc -O2 -march=armv6 -c mem-op.S
	arm-none-eabi-gcc -O2 -march=armv6 -c test.c
	arm-none-eabi-gcc -O2 -march=armv6 -c 2-wait-copy.c
.PHONY: force
	force :;
