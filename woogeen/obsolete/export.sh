#!/bin/sh

if test "$1" = "-r"; then
    backup=$2.orig
    source=$2
    if test -d "$backup"; then
        rm -rf "$source"
        mv "$backup" "$source"
    else
	echo "backup not found"
        exit 1
    fi
    exit
fi

test $# -ge 1 -a -d "$1" || exit 1

source=$1
destination=$2
backup=$source.orig

if test -z "$destination" -o "$destination" = "$source"; then
    test -d "$backup" || mv "$source" "$backup"
    destination=$source
    source=$backup
fi

find "$source" -name "*.h" | \
while read input; do
    echo "Processing $input ..."
    output=$destination/${input#$source/}
    dir=${output%/*}
    test -d "$dir" || mkdir -p "$dir"
    sed "s/^ *class \([A-Z][[:alnum:]_]* .*[{:].*\)$/class __attribute__((visibility(\"default\"))) \1/g;\
	s/^ *struct \([A-Z][[:alnum:]_]* .*[{:].*\)$/struct __attribute__((visibility(\"default\"))) \1/g" $input > "$output"
done
