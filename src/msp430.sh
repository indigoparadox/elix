#!/bin/bash

case $1 in
   "flash")
      mspdebug rf2500 "prog qd430.bin"
      ;;

   "debug")
      mspdebug rf2500 gdb &
      msp430-gdb --command=mspgdb qd430.bin
      ;;

   *)
      echo "usage: ${0} flash|debug"
      ;;
esac

