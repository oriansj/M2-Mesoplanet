/* Copyright (C) 2016 Jeremiah Orians
 * Copyright (C) 2020 deesix <deesix@tuta.io>
 * This file is part of M2-Planet.
 *
 * M2-Planet is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * M2-Planet is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with M2-Planet.  If not, see <http://www.gnu.org/licenses/>.
 */

#include"cc.h"
#include <sys/utsname.h>

void init_macro_env(char* sym, char* value, char* source, int num);


void setup_env(char** envp)
{
	char* ARCH = NULL;
	struct utsname* unameData = calloc(1, sizeof(struct utsname));
	uname(unameData);
	if(match("i386", unameData->machine) ||
	   match("i486", unameData->machine) ||
	   match("i586", unameData->machine) ||
	   match("i686", unameData->machine) ||
	   match("i686-pae", unameData->machine)) ARCH = "x86";
	else if(match("x86_64", unameData->machine)) ARCH = "amd64";
	else ARCH = unameData->machine;


	/* TODO Check for override using envp */


	/* Set desired architecture */
	if(match("knight-native", ARCH)) Architecture = KNIGHT_NATIVE;
	else if(match("knight-posix", ARCH)) Architecture = KNIGHT_POSIX;
	else if(match("x86", ARCH))
	{
		Architecture = X86;
		init_macro_env("__i386__", "1", "--architecture", 0);
	}
	else if(match("amd64", ARCH))
	{
		Architecture = AMD64;
		init_macro_env("__x86_64__", "1", "--architecture", 0);
	}
	else if(match("armv7l", ARCH))
	{
		Architecture = ARMV7L;
		init_macro_env("__arm__", "1", "--architecture", 0);
	}
	else if(match("aarch64", ARCH))
	{
		Architecture = AARCH64;
		init_macro_env("__aarch64__", "1", "--architecture", 0);
	}
	else if(match("riscv64", ARCH))
	{
		Architecture = RISCV64;
		init_macro_env("__riscv", "1", "--architecture", 0);
		init_macro_env("__riscv_xlen", "64", "--architecture", 1);
	}
	else
	{
		fputs("Unknown architecture: ", stderr);
		fputs(ARCH, stderr);
		fputs(" know values are: knight-native, knight-posix, x86, amd64, armv7l, aarch64 and riscv64\n", stderr);
		exit(EXIT_FAILURE);
	}
}
