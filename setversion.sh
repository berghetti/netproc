if [ -z "$1" ]; then
cat <<@@
Usage: $(basename $0) version

Example:
  $ ./$(basename $0) 0.1.0
@@

  exit 1
fi

sed -i "s/PROG_VERSION[ \t]\+\"\(.\+\)\"/PROG_VERSION \"$1\"/"  src/config.h
