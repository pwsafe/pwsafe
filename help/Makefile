#
# Makefile to compile all versions of the pwsafe help
#
# We're not checking any dependencies, just invoking the 
# compiler whenver we're called.
# We're igonring the return status of hhc, since it's != 0 even
# when compiles OK.
#

HELP_DIRS := default pwsafeDE pwsafeFR pwsafeRU
# pwsafePL -- nothing there :-(
# pwsafeZH -- currently breaks my hhc :-(

HHC = /cygdrive/c/local/MSVS8/HTMLHelpWorkshop/hhc.exe

.PHONY: all

all:
	-$(foreach dir, $(HELP_DIRS), cd $(CURDIR)/$(dir); $(HHC) pwsafe.hhp;)
