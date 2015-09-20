#!/bin/sh
#/
#/ Wrapper script for whisper sub-commands
#/

set -e

test $# -eq 0 && {
  echo >&2 "usage: $(basename $0) SUBCOMMAND [args]"
  exit 1
}

# shift off process type
subcommand="$1"
shift

exec whisper-$subcommand "$@"
