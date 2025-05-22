#! /bin/sh
## Copyright (C) 2017 Jeremiah Orians
## Copyright (C) 2020-2021 deesix <deesix@tuta.io>
## This file is part of M2-Planet.
##
## M2-Planet is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## M2-Planet is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with M2-Planet.  If not, see <http://www.gnu.org/licenses/>.

set -x

TMPDIR="test/test0004/tmp"
BINDIR="$PWD/bin"

mkdir -p ${TMPDIR}

"${BINDIR}/M2-Mesoplanet" \
	-c \
	-f test/test0004/main.c \
	-o ${TMPDIR}/main.o \
	|| exit 1

export M2LIBC_PATH=../../../M2libc
cd ${TMPDIR} && "${BINDIR}/M2-Mesoplanet" \
	-c \
	-I ../../../M2libc \
	-f ../f.c \
	|| exit 2

unset M2LIBC_PATH

cd -

if [ ! -f ${TMPDIR}/f.o ]; then
	echo "FAILURE: ${TMPDIR}/f.o not found!"
	exit 3
fi

if [ ! -f ${TMPDIR}/main.o ]; then
	echo "FAILURE: ${TMPDIR}/main.o not found!"
	exit 4
fi

export M2LIBC_PATH=../../../M2libc
cd ${TMPDIR} && "${BINDIR}/M2-Mesoplanet" \
	../f.c ../main.c \
	|| exit 5
unset M2LIBC_PATH
cd -

if [ ! -f ${TMPDIR}/a.out ]; then
	echo "FAILURE: ${TMPDIR}/a.out not found!"
	exit 6
fi

# Two object files with one stdio.h

"${BINDIR}/M2-Mesoplanet" \
	test/test0004/main.c \
	test/test0004/f.c \
	-o ${TMPDIR}/both_source \
	|| exit 7

OUTPUT=$("${TMPDIR}/both_source") || exit 8
if [ "$OUTPUT" != "Hello world" ]; then
	exit 9
fi

"${BINDIR}/M2-Mesoplanet" \
	test/test0004/main.c \
	test/test0004/tmp/f.o \
	-o ${TMPDIR}/main_source \
	|| exit 10

OUTPUT=$("${TMPDIR}/main_source") || exit 11
if [ "$OUTPUT" != "Hello world" ]; then
	exit 12
fi

"${BINDIR}/M2-Mesoplanet" \
	test/test0004/tmp/main.o \
	test/test0004/f.c \
	-o ${TMPDIR}/f_source \
	|| exit 13

OUTPUT=$("${TMPDIR}/f_source") || exit 14
if [ "$OUTPUT" != "Hello world" ]; then
	exit 15
fi

"${BINDIR}/M2-Mesoplanet" \
	test/test0004/tmp/main.o \
	test/test0004/tmp/f.o \
	--dirty-mode \
	-o ${TMPDIR}/double_object_file \
	|| exit 16

OUTPUT=$("${TMPDIR}/double_object_file") || exit 17
if [ "$OUTPUT" != "Hello world" ]; then
	exit 18
fi

# Two object files without any stdio.h

"${BINDIR}/M2-Mesoplanet" \
	-c \
	"test/test0004/f_no_print.c" \
	-o "${TMPDIR}/f_no_print.o" \
	|| exit 19

if [ ! -f ${TMPDIR}/f_no_print.o ]; then
	echo "FAILURE: ${TMPDIR}/f_no_print.o not found!"
	exit 20
fi

"${BINDIR}/M2-Mesoplanet" \
	test/test0004/main.c \
	test/test0004/f_no_print.c \
	-o ${TMPDIR}/both_source_no_print \
	|| exit 21

"${TMPDIR}/both_source_no_print" || exit 22

"${BINDIR}/M2-Mesoplanet" \
	test/test0004/main.c \
	test/test0004/tmp/f_no_print.o \
	-o ${TMPDIR}/main_source_no_print \
	|| exit 23

"${TMPDIR}/main_source_no_print" || exit 24

"${BINDIR}/M2-Mesoplanet" \
	test/test0004/tmp/main.o \
	test/test0004/f_no_print.c \
	-o ${TMPDIR}/f_source_no_print \
	|| exit 25

"${TMPDIR}/f_source_no_print" || exit 26

"${BINDIR}/M2-Mesoplanet" \
	test/test0004/tmp/main.o \
	test/test0004/tmp/f_no_print.o \
	--dirty-mode \
	-o ${TMPDIR}/double_object_file_no_print \
	|| exit 27

"${TMPDIR}/double_object_file_no_print" || exit 28

exit 0
