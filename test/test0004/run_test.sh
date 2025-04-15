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

cd ${TMPDIR} && "${BINDIR}/M2-Mesoplanet" \
	-c \
	-f ../f.c \
	|| exit 2

cd -

if [ ! -f ${TMPDIR}/f.o ]; then
	echo "FAILURE: ${TMPDIR}/f.o not found!"
	exit 3
fi

if [ ! -f ${TMPDIR}/main.o ]; then
	echo "FAILURE: ${TMPDIR}/main.o not found!"
	exit 4
fi

cd ${TMPDIR} && "${BINDIR}/M2-Mesoplanet" \
	../f.c ../main.c \
	|| exit 5

cd -

if [ ! -f ${TMPDIR}/a.out ]; then
	echo "FAILURE: ${TMPDIR}/a.out not found!"
	exit 6
fi

exit 0
