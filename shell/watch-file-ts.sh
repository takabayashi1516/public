#!/bin/bash
#
# usage
#   $ sh ./watch-file-ts.sh --watch-path=. [ --time-threshold="2025-04-17 23:59:59" --direction=11 ]
#

w_path=.
t_thrshld=
drctn=11

options=$(getopt -l watch-path:,time-threshold:,direction: $0 $* | sed -e "s/'//g")
set -- $options
while [ $# -gt 0 ]; do
  case $1 in
    --watch-path)
      w_path=$2
      shift ;;
    --time-threshold)
      t_thrshld=$(date "+%s%3N" -d "$2")
      shift ;;
    --direction)
      drctn=$2
      shift ;;
    --) ;;
    *)  ;;
  esac
  shift
done

if ! [ -d ${w_path} ]; then
  echo "no such directory."
  exit 1
fi

if [ "${t_thrshld}" == "" ]; then
  t_thrshld=$(date "+%s%3N" -d "$(date '+%Y-%m-%d %H:%M:%S.%3N')")
fi

#
find ${w_path} -type f | while read f; do
  t=$(stat "${f}" | grep "Modify" | sed "s/Modify: //")
  ts=$(date "+%s%3N" -d "${t}")
  if [ ${drctn} -eq -11 ]; then
    if [ ${ts} -lt ${t_thrshld} ]; then
      echo ${t} ${f}
    fi
  elif [ ${drctn} -eq -1 ]; then
    if [ ${ts} -le ${t_thrshld} ]; then
      echo ${t} ${f}
    fi
  elif [ ${drctn} -eq 0 ]; then
    if [ ${ts} -eq ${t_thrshld} ]; then
      echo ${t} ${f}
    fi
  elif [ ${drctn} -eq 1 ]; then
    if [ ${ts} -ge ${t_thrshld} ]; then
      echo ${t} ${f}
    fi
  else #elif [ ${drctn} -eq 11 ]; then
    if [ ${ts} -gt ${t_thrshld} ]; then
      echo ${t} ${f}
    fi
  fi
done
