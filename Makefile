#
# Makefile for HuC sources
#

SUBDIRS = src tgemu

all clean:
	@$(MAKE) $(SUBDIRS) "COMMAND=$@"

.PHONY: $(SUBDIRS)

$(SUBDIRS):
	@echo " "
	@echo " -----> make $(COMMAND) in $@"
	$(MAKE) --directory=$@ $(COMMAND)

install:
	cp -p bin/* /usr/local/bin
	cp -p include/pce/* /usr/include/pce/

package:
	$(MAKE) clean
	rm -f huc.zip
	zip -R huc \* -x \*CVS\*

