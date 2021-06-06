#!/bin/sh

echo "#include <stdio.h>" > vm_debug.h

echo "static const struct { int val; const char *name; } vm_instr_debug[] = {" \
   >> vm_debug.h
grep "^#define VM_INSTR_" vm.h | \
   grep -v MIN | \
   grep -v MAX | \
   sed -E 's/#define ([A-Z0-9_]*).*/{\1, "\1"},/' >> vm_debug.h
echo "{-1, NULL}};" >> vm_debug.h

echo "static const struct { int val; const char *name; } vm_sysc_debug[] = {" \
   >> vm_debug.h
grep "^#define VM_SYSC_" vm.h | \
   grep -v MIN | \
   grep -v MAX | \
   sed -E 's/#define ([A-Z0-9_]*).*/{\1, "\1"},/' >> vm_debug.h
echo "{-1, NULL}};" >> vm_debug.h
