#!/usr/bin/env bash

RUN_UNIT_TEST=0
RUN_PERFORMANCE_TEST=0
MYOSTREAM=FALSE
while [ "$#" -gt 0 ]; do
  case $1 in
    -ru)
      RUN_UNIT_TEST=1
      shift
      ;;
    -rp)
      RUN_PERFORMANCE_TEST=1
      shift
      ;;
    -r)
      RUN_UNIT_TEST=1
      RUN_PERFORMANCE_TEST=1
      shift
      ;;
    -o)
      MYOSTREAM=TRUE
      shift
      ;;
    -h)
      echo "run_test.sh [-o] [-r | -ru | -rp]"
      exit 0
      ;;
    *)
      echo "Invalid option '$1'"
      echo
      exit 1
      ;;
  esac
done

mkdir -p build
cd build
cmake .. -DBUILD_TEST=TRUE -DENABLE_MYOSTREAM_WATCH=${MYOSTREAM}
make
make test

if [ ${RUN_UNIT_TEST} -eq 1 ]; then
  ./test/unit_test/unit_test
fi
if [ ${RUN_PERFORMANCE_TEST} -eq 1 ]; then
  ./test/performance_test/performance_test
fi
