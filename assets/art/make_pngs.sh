#!/bin/bash

# check for imagemagick
type convert >/dev/null 2>&1 || {
	echo >&2 "imagemagick is not installed ";
	exit 1;
}

# check for pngcrush
type pngcrush >/dev/null 2>&1 || {
	echo >&2 "pngcrush is not installed ";
	exit 1;
}

file=$1
scale=$2
if [ -z "$file" ]; then
	echo "No input .svg file"
	exit
fi

if [ -z "$scale" ]; then
	scale=1
fi
density=$((90 * $scale))

# get size of each image
size=`identify -format "%[fx:w/10]" "$file"`
size=$(($size * $scale))

# get list of objects
names=`grep id=\"layer $file -C 3 | grep label | grep =.* -o | tr -d '="<>' | tac`
names=`xmllint --xpath "//*[local-name()='svg']/*[local-name()='metadata']//*[local-name()='description']/text()" "$file"`

mkdir -p export

# export pngs
convert -density ${density} -background none ${file} -crop ${size}x${size} -depth 8 +repage PNG32:export/_out.png

# name files
i=0
for name in $names; do
	oldname="export/_out-${i}.png"
	newname="export/_${name}.png"
	optname="export/${name}.png"

	mv "$oldname" "${newname}"
	pngcrush -brute "${newname}" "${optname}"
	((i++))
done

rm export/_*

