#!/usr/bin/env bash

RETURN=0
CLANG_FORMAT=$(which clang-format)
if [ $? -ne 0 ]; then
    echo "[!] clang-format not installed. Unable to check source file format policy." >&2
    exit 1
fi

FILES=`git diff --cached --name-only --diff-filter=ACMR | grep -E "\.(c|cpp|h)$"`
for FILE in $FILES; do
    nf=`git checkout-index --temp $FILE | cut -f 1`
    tempdir=`mktemp -d` || exit 1
    newfile=`mktemp ${tempdir}/${nf}.XXXXXX` || exit 1
    basename=`basename $FILE`

    source="${tempdir}/${basename}"
    mv $nf $source
    cp .clang-format $tempdir
    $CLANG_FORMAT $source > $newfile 2>> /dev/null
    r=$?
    rm -rf "${tempdir}"
    if [ $r != 0 ] ; then
        $CLANG_FORMAT -i $FILE
    fi
done

