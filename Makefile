ifeq ($(shell test -f /.dockerenv; echo $$?),1) # This is a shim
MAKEFLAGS += --no-builtin-rules
.SUFFIXES:
default:
	docker run -v $(PWD):$(PWD) -w $(PWD) --network=host --rm --user $$UID:$$GID "devkitpro/devkitarm" $(MAKE)
%:
	docker run -v $(PWD):$(PWD) -w $(PWD) --network=host --rm --user $$UID:$$GID "devkitpro/devkitarm" $(MAKE) $@
bash:
	docker run -v $(PWD):$(PWD) -w $(PWD) --network=host -it --user $$UID:$$GID --rm "devkitpro/devkitarm"

else


CC=/opt/devkitpro/devkitARM/bin/arm-none-eabi-gcc

ARCH:=-march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft

CFLAGS+=-I /opt/devkitpro/libctru/include/ -D__3DS__
CFLAGS+=-I /opt/devkitpro/portlibs/3ds/include/
CFLAGS+=$(shell PKG_CONFIG_PATH=/opt/devkitpro/portlibs/3ds/lib/pkgconfig/ pkg-config libcurl --cflags)
# CFLAGS+=-mword-relocations -ffunction-sections
CFLAGS+=$(ARCH)

# LDFLAGS+=-L/opt/devkitpro/devkitARM/arm-none-eabi/lib/armv6k/fpu/
# LDFLAGS+=-lm
LDFLAGS+=-L/opt/devkitpro/libctru/lib/ -lctru 
# LDFLAGS+=-L/opt/devkitpro/portlibs/3ds/lib/ -lcurl
LDFLAGS+=$(shell PKG_CONFIG_PATH=/opt/devkitpro/portlibs/3ds/lib/pkgconfig/ pkg-config libcurl --libs)


# LDFLAGS+=-L/usr/lib/gcc/arm-none-eabi/14/armv6k/fpu/
LDFLAGS+=-specs=3dsx.specs -g $(ARCH) 


%.o: %.c
	$(CC) $(CFLAGS) -o $@ $^ -c

OBJS=main.o immich/upload.o

PROJNAME=immich-3ds

%.elf: $(OBJS)
	# it's VERY VERY IMPORTANT THAT THE OBJECTS GO BEFORE THE LDFLAGS JFC
	$(CC) $^ $(LDFLAGS) -Wl,-Map,whatever.map -o $@

%.3dsx: %.elf
	3dsxtool $< $@ # --smdh=hello-world.smdh

.DEFAULT_GOAL := build
.PHONY: build
build: $(PROJNAME).3dsx

clean:
	rm $(OBJS) *.elf *.map *.3dsx

upload: $(PROJNAME).3dsx
	3dslink $<



endif

