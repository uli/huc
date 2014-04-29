#
# Makefile for HuC sources
#

SUBDIRS = src tgemu

all clean: bin
	@$(MAKE) $(SUBDIRS) "COMMAND=$@"

bin:
	mkdir -p bin

.PHONY: $(SUBDIRS)

$(SUBDIRS):
	@echo " "
	@echo " -----> make $(COMMAND) in $@"
	$(MAKE) --directory=$@ $(COMMAND)

install:
	cp -p bin/* /usr/local/bin
	mkdir -p /usr/include/pce
	cp -pr include/pce/* /usr/include/pce/

package:
	$(MAKE) clean
	rm -f huc.zip
	zip -R huc \* -x \*CVS\*

