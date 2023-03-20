#!/usr/bin/env bash
# Copyright (c) 2021-2022 Shuangquan Li. All Rights Reserved.
#
# Licensed under the MIT License (the "License"); you may not use this file
# except in compliance with the License. You may obtain a copy of the License
# at
#
#   http://opensource.org/licenses/MIT
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.

cd $(dirname "$0") || exit

readonly SUBDIRS="include include/* include/*/* test test/*"
readonly FILE_TYPES="*.h *.hpp *.c *.cc *.cpp"
readonly FILE_FILTER_RE="\.(h|hpp|c|cc|cpp)$"

readonly FS_ALL="ALL"
readonly FS_DIFF="DIFF"
readonly FS_UDF="USER_DEFINED"

function show_usage() {
  echo "USAGE: fmt.sh [-a|-d|--file <file>] [--use <clang-format>] [--git-add] [-h]"
  echo "OPTIONS:"
  echo "  -a                    format all C++ source files"
  echo "  -d                    format modified C++ source files"
  echo "  --file <file>         format user defined file <file>"
  echo "  --git-add             add formatted files to git"
  echo "  --use <clang-format>  use user defined format tool <clang-format>"
  echo "  -h                    show usage"
}

FILE_SECTION=${FS_DIFF}
UDF_FILE=""
CLANG_FORMAT=""
GIT_ADD=0

while [ "$#" -gt 0 ]; do
  case $1 in
    -a)
      FILE_SECTION=${FS_ALL}
      shift
      ;;
    -d)
      FILE_SECTION=${FS_DIFF}
      shift
      ;;
    --file)
      FILE_SECTION=${FS_UDF}
      UDF_FILE=$2
      shift
      shift
      ;;
    --git-add)
      GIT_ADD=1
      shift
      ;;
    --use)
      CLANG_FORMAT=$2
      shift
      shift
      ;;
    -h)
      show_usage
      exit 0
      ;;
    *)
      echo "Invalid option '$1'"
      echo
      show_usage
      exit 1
      ;;
  esac
done

function which_clang_format() {
  if [ -n "${CLANG_FORMAT}" ]; then
    return
  elif [ -x "$(command -v clang-format)" ]; then
    CLANG_FORMAT="clang-format"
    return
  fi

  for ((i = 20; i >= 0; i--)); do
    for cmd in clang-format-${i}{,.{9,8,7,6,5,4,3,2,1,0}}; do
      if [ -x "$(command -v ${cmd})" ]; then
        CLANG_FORMAT=${cmd}
        return
      fi
    done
  done

  echo "ERROR: clang-format not available"
  exit 1
}

which_clang_format
echo "Start formatting using '${CLANG_FORMAT}' for '${FILE_SECTION}' files."

function fmt_one() {
  ${CLANG_FORMAT} --style=file -i "$1"
  added="no-git-add"
  if [ ${GIT_ADD} == 1 ] && [ "$3" == "cached" ]; then
    git add "$1"
    added="git-added"
  fi
  echo "formatting [$2][$3][${added}]: $1"
}

if [ ${FILE_SECTION} == ${FS_DIFF} ]; then
  files=$(echo $(git diff --name-only) | tr -s " " "\n" | grep -E ${FILE_FILTER_RE})
  cached_files=$(echo $(git diff --name-only --cached) | tr -s " " "\n" | grep -E ${FILE_FILTER_RE})
  for file in ${files}; do
    fmt_one "${file}" "${FS_DIFF}" "not-cached"
  done
  for file in ${cached_files}; do
    fmt_one "${file}" "${FS_DIFF}" "cached"
  done
elif [ ${FILE_SECTION} == ${FS_ALL} ]; then
  for d in ${SUBDIRS}; do
    for t in ${FILE_TYPES}; do
      for file in $d/$t; do
        if test -f "${file}"; then
          fmt_one "${file}" "${FS_ALL}"
        fi
      done
    done
  done
elif [ ${FILE_SECTION} == ${FS_UDF} ]; then
  fmt_one "${UDF_FILE}" "${FS_UDF}"
fi

echo "FORMAT DONE!"

if [ ! -f ".git/hooks/pre-commit" ]; then
  echo "Install pre-commit automatically."
  echo "#!/bin/bash" >> .git/hooks/pre-commit
  echo "./fmt.sh --git-add" >> .git/hooks/pre-commit
  chmod +x .git/hooks/pre-commit
fi

