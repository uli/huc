# DEVELO LIBRARY MAKEFILE [MSDOS]
#

#
# Defines
#
BASEDIR=..d ..d ..d ..
include ../../../Make_src.inc

OBJS     = init.o xfer.o error.o command.o mx.o slave.o
LIB      = libdevel.a
EXE      = $(LIB)

#
#

all: $(EXE)


# RULES
#
$(OBJS) : develo.h
xfer.o  : crc16.h 

# EXE
#
$(LIB): $(OBJS)
	ar rs $(LIB) $(OBJS)

#
# Targets
#
include $(MAKETARG)

