#!/bin/bash

case $1 in
   "flash")
      mspdebug rf2500 "prog qd430.bin"
      ;;

   "debugs")
      mspdebug rf2500 gdb
      ;;

   "debugc")
      msp430-gdb --command=mspgdb qd430.bin
      ;;

   *)
      echo "usage: ${0} flash|debug"
      ;;
esac

