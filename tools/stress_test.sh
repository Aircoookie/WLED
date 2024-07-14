#!/bin/bash
# Some web server stress tests
#
# Perform a large number of parallel requests, stress testing the web server
# TODO: some kind of performance metrics

# Accepts two command line arguments:
# - first argument - mandatory - IP or hostname of target server
# - second argument - targert type
HOST=$1
declare -n TARGET_STR="${2:-JSON_LARGER}_TARGETS"

CURL_ARGS="--compressed --parallel --parallel-immediate --parallel-max 50"

JSON_TARGETS=('json/state' 'json/info' 'json/si', 'json/palettes' 'json/fxdata' 'settings/s.js?p=2')
FILE_TARGETS=('' 'iro.js' 'rangetouch.js' 'settings' 'settings/wifi')
# Replicate one target many times
function replicate() {
  printf "${1}?%d " {1..8}
}
read -a JSON_LARGE_TARGETS <<< $(replicate "json/si")
read -a JSON_LARGER_TARGETS <<< $(replicate "json/fxdata")

# Expand target URLS to full arguments for curl
TARGETS=(${TARGET_STR[@]})
#echo "${TARGETS[@]}"
FULL_TGT_OPTIONS=$(printf "http://${HOST}/%s -o /dev/null " "${TARGETS[@]}")
#echo ${FULL_TGT_OPTIONS}

time curl ${CURL_ARGS} ${FULL_TGT_OPTIONS}
