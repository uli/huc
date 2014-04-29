
TARGET	= tgemu

OBJS	= $(RESOBJS) game.o \
  src/fileio.o \
  src/pce.o \
  src/psg.o \
  src/render.o \
  src/system.o \
  src/unzip.o \
  src/vce.o \
  src/vdc.o \
  src/cpu/h6280.o \

LIBS	= -lz

ARCH = $(shell uname -p)

ifneq ($(ARCH), ppc)
ifneq ($(ARCH), powerpc)
ENDIAN = -DLSB_FIRST
endif
endif

CFLAGS = -Wall -W -Isrc -Isrc/cpu -Isrc/unix -fno-strict-aliasing -D_GNU_SOURCE $(ENDIAN) -DFAST_MEM -O2 -g

all: $(TARGET)
$(TARGET):	$(OBJS)
	$(CC) -o tgemu $(OBJS) $(LIBS)
clean:
	rm -f $(OBJS) tgemu
