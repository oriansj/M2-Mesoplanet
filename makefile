## Copyright (C) 2017 Jeremiah Orians
## Copyright (C) 2020 deesix <deesix@tuta.io>
## This file is part of M2-Mesoplanet.
##
## M2-Mesoplanet is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## M2-Mesoplanet is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with M2-Mesoplanet.  If not, see <http://www.gnu.org/licenses/>.

# Prevent rebuilding
VPATH = bin
PACKAGE = m2-mesoplanet

# C compiler settings
CC?=gcc
CFLAGS:=$(CFLAGS) -D_GNU_SOURCE -O0 -std=c99 -ggdb -Wall -Wextra -Wstrict-prototypes
ARCH:=$(shell get_machine)
BLOOD_FLAG:=$(shell get_machine --blood)
ENDIAN_FLAG:=$(shell get_machine --endian)
BASE_ADDRESS:=$(shell get_machine --hex2)

all: M2-Mesoplanet

M2-Mesoplanet: bin results cc.h cc_reader.c cc_core.c cc_macro.c cc_env.c cc_spawn.c cc.c cc_globals.c cc_globals.h
	$(CC) $(CFLAGS) \
	M2libc/bootstrappable.c \
	cc_reader.c \
	cc_core.c \
	cc_macro.c \
	cc_env.c \
	cc_spawn.c \
	cc.c \
	cc.h \
	cc_globals.c \
	gcc_req.h \
	-o bin/M2-Mesoplanet

M2-boot: bin results cc.h cc_reader.c cc_core.c cc_macro.c cc_env.c cc_spawn.c cc.c cc_globals.c cc_globals.h
	echo $(ARCH)
	echo $(BLOOD_FLAG)
	echo $(ENDIAN_FLAG)
	echo $(BASE_ADDRESS)
	M2-Planet --architecture ${ARCH} \
	-f M2libc/sys/types.h \
	-f M2libc/stddef.h \
	-f M2libc/${ARCH}/linux/fcntl.c \
	-f M2libc/fcntl.c \
	-f M2libc/${ARCH}/linux/unistd.c \
	-f M2libc/${ARCH}/linux/sys/stat.c \
	-f M2libc/sys/utsname.h \
	-f M2libc/ctype.c \
	-f M2libc/stdlib.c \
	-f M2libc/stdarg.h \
	-f M2libc/stdio.h \
	-f M2libc/stdio.c \
	-f M2libc/string.c \
	-f M2libc/bootstrappable.c \
	-f cc.h \
	-f cc_globals.c \
	-f cc_env.c \
	-f cc_reader.c \
	-f cc_spawn.c \
	-f cc_core.c \
	-f cc_macro.c \
	-f cc.c \
	--debug \
	-o ./bin/M2-Mesoplanet-1.M1
	blood-elf ${ENDIAN_FLAG} ${BLOOD_FLAG} -f ./bin/M2-Mesoplanet-1.M1 -o ./bin/M2-Mesoplanet-1-footer.M1
	M1 --architecture ${ARCH} \
	${ENDIAN_FLAG} \
	-f M2libc/${ARCH}/${ARCH}_defs.M1 \
	-f M2libc/${ARCH}/libc-full.M1 \
	-f ./bin/M2-Mesoplanet-1.M1 \
	-f ./bin/M2-Mesoplanet-1-footer.M1 \
	-o ./bin/M2-Mesoplanet-1.hex2
	hex2 --architecture ${ARCH} \
	${ENDIAN_FLAG} \
	--base-address ${BASE_ADDRESS} \
	-f ./M2libc/${ARCH}/ELF-${ARCH}-debug.hex2 \
	-f ./bin/M2-Mesoplanet-1.hex2 \
	-o ./bin/M2-Mesoplanet


# Clean up after ourselves
.PHONY: clean
clean:
	rm -rf bin/
	rm -rf test/test0000/tmp
	rm -rf test/test0001/tmp
	rm -rf test/test0002/tmp
	rm -rf test/test0003/tmp
	rm -rf test/test0004/tmp

.PHONY: clean-tmp
clean-tmp:
	rm -vf /tmp/M2-Mesoplanet-*
	rm -vf /tmp/M2-Planet-*
	rm -vf /tmp/M1-macro-*
	rm -vf /tmp/blood-elf-*

# Directories
bin:
	mkdir -p bin

results:
	mkdir -p test/results

# tests
test: M2-Mesoplanet
	./test/test0000/run_test.sh
	./test/test0001/run_test.sh
	./test/test0002/run_test.sh
	./test/test0003/run_test.sh
	./test/test0004/run_test.sh
#	sha256sum -c test/test.answers


# Generate test answers
.PHONY: Generate-test-answers
Generate-test-answers:
	sha256sum test/test0000/tmp/return.c >| test/test0000/proof.answer
	sha256sum test/test0001/tmp/return.c >| test/test0001/proof.answer
	sha256sum test/test0002/tmp/macro_functions.c >| test/test0002/proof.answer
	sha256sum test/test0003/tmp/include_paths1.c >| test/test0003/proof.answer

DESTDIR:=
PREFIX:=/usr/local
bindir:=$(DESTDIR)$(PREFIX)/bin
.PHONY: install
install: bin/M2-Mesoplanet
	mkdir -p $(bindir)
	cp $^ $(bindir)

###  dist
.PHONY: dist

COMMIT=$(shell git describe --dirty)
TARBALL_VERSION=$(COMMIT:Release_%=%)
TARBALL_DIR:=$(PACKAGE)-$(TARBALL_VERSION)
TARBALL=$(TARBALL_DIR).tar.gz
# Be friendly to Debian; avoid using EPOCH
MTIME=$(shell git show HEAD --format=%ct --no-patch)
# Reproducible tarball
TAR_FLAGS=--sort=name --mtime=@$(MTIME) --owner=0 --group=0 --numeric-owner --mode=go=rX,u+rw,a-s

$(TARBALL):
	(git ls-files --recurse-submodules		\
	    --exclude=$(TARBALL_DIR);			\
	    echo $^ | tr ' ' '\n')			\
	    | tar $(TAR_FLAGS)				\
	    --transform=s,^,$(TARBALL_DIR)/,S -T- -cf-	\
	    | gzip -c --no-name > $@

dist: $(TARBALL)
