#!/usr/bin/env bash
# Copyright (c) 2021-2022 Shuangquan Li. All Rights Reserved.
#
# Licensed under the MIT License (the "License"); you may not use this file
# except in compliance with the License. You may obtain a copy of the License
# at
#
#   http:#opensource.org/licenses/MIT
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.

RUN_UNIT_TEST=0
RUN_PERFORMANCE_TEST=0
MYOSTREAM=FALSE
REBUILD=0
CLEAR=0
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
    --rebuild)
      REBUILD=1
      shift
      ;;
    --clear)
      CLEAR=1
      shift
      ;;
    -h)
      echo "run_test.sh [-o] [-r|-ru|-rp] [--rebuild] [--clear]"
      exit 0
      ;;
    *)
      echo "Invalid option '$1'"
      echo
      exit 1
      ;;
  esac
done

if [ ${REBUILD} -eq 1 ]; then
  rm -rf build
fi
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

if [ ${CLEAR} -eq 1 ]; then
  cd ..
  rm -rf build
fi
