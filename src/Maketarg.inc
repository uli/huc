# Include file for Makefile for HuC
# Written for unified version, October 16, 2001 by Dave Shadoff 
#
# This file defines default targets
#

clean::
	$(RM) core
	$(RM) *.o
ifneq "$(EXE)" ""
	$(RM) $(EXE)
endif

%.o: %.c
	$(CC) $(CDEFS) $(CFLAGS) -c $< -o $@

