#
# Makefile for HuC sources
#

SUBDIRS = src

all clean:
	@$(MAKE) $(SUBDIRS) "COMMAND=$@"

.PHONY: $(SUBDIRS)

$(SUBDIRS):
	@echo " "
	@echo " -----> make $(COMMAND) in $@"
	$(MAKE) --directory=$@ $(COMMAND)

package:
	$(MAKE) clean
	rm -f huc.zip
	zip -R huc \* -x \*CVS\*

