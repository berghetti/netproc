#!/bin/sh

# Example:
#   $ ./release 0.1.0

make_release()
{
  sed -i "s/PROG_VERSION[ \t]\+\"\(.\+\)\"/PROG_VERSION \"$1\"/" src/config.h
  git add src/config.h
  git commit -m "set version $1"
  git tag -s $1 -m "version $1"
}

if [ -z "$1" ]; then
  exit 1
fi

make_release $1
