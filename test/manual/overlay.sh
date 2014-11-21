#!/bin/sh
# ATM this test has to be (painfully) done manually because TGEmu does not
# support CDs.

# Tests all variants of overlay loading:
# - CD and SCD
# - with and without data initialization
# - with and without heap initialization
# - with and without -msmall
# - with and without -fno-recursive

set -e

export PCE_INCLUDE="../../include/pce"
export PCE_PCEAS="../../bin/pceas"

HUC=../../src/huc/huc
ISOLINK=../../bin/isolink

for sys in cd scd
do
  for eins in "" "-lmalloc"
  do
    for zwei in "" "-lmalloc"
    do
      for init_eins in "" "-DINIT_GLOBAL1"
      do
        for init_zwei in "" "-DINIT_GLOBAL2"
        do
        for small in "" "-msmall"
        do
        for rec in "" "-fno-recursive"
        do
          set -x
          $HUC $init_eins $small $rec -over -${sys} overlay1.c $eins >/dev/null
          $HUC $init_zwei $small $rec -over -${sys} overlay2.c $zwei >/dev/null
          $ISOLINK overlay.iso overlay1.ovl overlay2.ovl >/dev/null
          set +x
          mednafen overlay.cue >/dev/null
        done
        done
        done
      done
    done
  done
done

