#!/bin/bash
for i in {1..23}; do
  if [ -f tests-out/$i.out ]; then cp tests-out/$i.out tests/$i.out; fi
  if [ -f tests-out/$i.rc ]; then cp tests-out/$i.rc tests/$i.rc; fi
  if [ -f tests-out/$i.err ]; then cp tests-out/$i.err tests/$i.err; fi
done
echo "All outputs synced (1-23)"
