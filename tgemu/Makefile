
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

CFLAGS = -Wall -W -Isrc -Isrc/cpu -Izlib-1.2.7 -Isrc/unix -fno-strict-aliasing -DLSB_FIRST -DFAST_MEM -O2 -g

all: $(TARGET)
$(TARGET):	$(OBJS)
	$(CC) -o tgemu $(OBJS) $(LIBS)
clean:
	rm -f $(OBJS) tgemu
