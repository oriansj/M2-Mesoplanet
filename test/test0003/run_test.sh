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

TMPDIR="test/test0003/tmp"

mkdir -p ${TMPDIR}

# Build the test
bin/M2-Mesoplanet \
	-E \
	-f test/test0003/include_paths1.c \
	-o ${TMPDIR}/include_paths1.c \
	|| exit 1

bin/M2-Mesoplanet \
	-E \
	test/test0003/include_paths2.c \
	-o ${TMPDIR}/include_paths2.c \
	|| exit 3

bin/M2-Mesoplanet \
	-E \
	-I test/test0003/directory \
	test/test0003/include_paths3.c \
	-o ${TMPDIR}/include_paths3.c \
	|| exit 4

bin/M2-Mesoplanet \
	-E \
	-I test/test0003 \
	-I test/test0003/directory \
	test/test0003/include_paths3.c \
	-o ${TMPDIR}/include_paths3.c \
	|| exit 5

bin/M2-Mesoplanet \
	-E \
	-I test/test0003 \
	-I test/test0003/directory \
	test/test0003/include_paths4.c \
	-o ${TMPDIR}/include_paths4.c \
	|| exit 6

bin/M2-Mesoplanet \
	-E \
	-I test/test0003 \
	-I test/test0003/directory \
	test/test0003/include_paths4.c \
	-o ${TMPDIR}/include_paths5.c \
	|| exit 7

bin/M2-Mesoplanet \
	-I M2libc \
	test/test0003/include_paths5.c \
	-o ${TMPDIR}/include_paths6 \
	|| exit 7

${TMPDIR}/include_paths6 || exit 8

sha256sum -c test/test0003/proof.answer || exit 2
exit 0
