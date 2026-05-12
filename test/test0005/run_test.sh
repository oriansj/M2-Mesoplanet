#! /bin/sh
## Copyright (C) 2026
## This file is part of M2-Mesoplanet.

set -x

TMPDIR="test/test0005/tmp"

mkdir -p ${TMPDIR}

bin/M2-Mesoplanet \
	-E \
	--no-includes \
	-f test/test0005/comment_suffix.c \
	-o ${TMPDIR}/comment_suffix.c \
	|| exit 1

if grep -q '\\*/\\.c' ${TMPDIR}/comment_suffix.c
then
	cat ${TMPDIR}/comment_suffix.c
	exit 2
fi

exit 0
