#!/bin/sh

path=xmake/confs.3/third_party

root=${1:-$(pwd)}
while test -n "$root"; do
    test -d $root/$path && break
    root=${root%/*}
done
test ! -d "$root/$path" && echo "yunos root not found" && exit 1
path=$path/webrtc

dir=${0%/*}
test $dir = $0 && dir="."
for i in $dir/*; do
    xmake=$i/xmake.conf
    test -f "$xmake" && install -D "$xmake" "$root/$path/${i##*/}/xmake.conf"
done

IFS=""
st=0
xmake=$root/xmake/platform/tablet/product/yunhal/xmake.product
test -f "$xmake.orig" || mv -f "$xmake" "$xmake.orig"
while read -r line; do
    case $st in
        0)  test "$line" = "# thirdparty" && st=1;;
        1)  if test "$line" = "${line%\\}"; then
                st=0
                echo "$line \\"
                echo "    woogeen \\"
                line="    woogeen_jsni"
            fi;;
    esac
    echo "$line"
done < "$xmake.orig" > "$xmake"

ln -sf "$dir" "$root/third-party/webrtc"
