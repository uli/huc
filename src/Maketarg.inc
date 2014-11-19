# Include file for Makefile for HuC
# Written for unified version, October 16, 2001 by Dave Shadoff 
#
# This file defines default targets
#

clean::
	$(RM) core
	$(RM) *.o *.gcno *.gcda *.gcov
ifneq "$(EXE)" ""
	$(RM) $(EXE) $(BINDIR)/$(EXE)
endif

%.o: %.c
	$(CC) $(CDEFS) $(CFLAGS) -c $< -o $@

