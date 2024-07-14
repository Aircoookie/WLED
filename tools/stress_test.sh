#!/bin/bash
# Some web server stress tests
#
# Perform a large number of parallel requests, stress testing the web server
# TODO: some kind of performance metrics


TARGET=$1

CURL_ARGS="--compressed --parallel --parallel-immediate --parallel-max 50"

JSON_TARGETS=('json/state' 'json/info' 'json/si', 'json/palettes' 'json/fxdata' 'settings/s.js?p=2')
FILE_TARGETS=('' 'iro.js' 'rangetouch.js' 'settings' 'settings/wifi')

# Replicate one target many times
function replicate() {
  printf "${1}?%d " {1..8}
}
read -a JSON_LARGE_TARGETS <<< $(replicate "json/si")
read -a JSON_LARGER_TARGETS <<< $(replicate "json/fxdata")

# TODO: argument parsing

# Test static file targets
#TARGETS=(${JSON_TARGETS[@]})
#TARGETS=(${FILE_TARGETS[@]})
TARGETS=(${JSON_LARGER_TARGETS[@]})

# Expand target URLS to full arguments for curl
FULL_OPTIONS=$(printf "http://${TARGET}/%s -o /dev/null " "${TARGETS[@]}")

#echo ${FULL_OPTIONS}
time curl ${CURL_ARGS} ${FULL_OPTIONS}
