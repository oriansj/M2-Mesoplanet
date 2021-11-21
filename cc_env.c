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
char* env_lookup(char* variable);

void setup_env()
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


	/* Check for override */
	char* hold = env_lookup("ARCHITECTURE_OVERRIDE");
	if(NULL != hold) ARCH = hold;

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

struct Token
{
	/*
	 * For the token linked-list, this stores the token; for the env linked-list
	 * this stores the value of the variable.
	 */
	char* value;
	/*
	 * Used only for the env linked-list. It holds a string containing the
	 * name of the var.
	 */
	char* var;
	/*
	 * This struct stores a node of a singly linked list, store the pointer to
	 * the next node.
	 */
	struct Token* next;
};

struct Token* env;

int array_length(char** array)
{
	int length = 0;

	while(array[length] != NULL)
	{
		length = length + 1;
	}

	return length;
}

/* Search for a variable in the token linked-list */
char* token_lookup(char* variable, struct Token* token)
{
	/* Start at the head */
	struct Token* n = token;

	/* Loop over the linked-list */
	while(n != NULL)
	{
		if(match(variable, n->var))
		{
			/* We have found the correct node */
			return n->value; /* Done */
		}

		/* Nope, try the next */
		n = n->next;
	}

	/* We didn't find anything! */
	return NULL;
}

/* Search for a variable in the env linked-list */
char* env_lookup(char* variable)
{
	return token_lookup(variable, env);
}

void populate_env(char** envp)
{
	/* You can't populate a NULL environment */
	if(NULL == envp)
	{
		return;
	}

	/* avoid empty arrays */
	int max = array_length(envp);

	if(0 == max)
	{
		return;
	}

	/* Initialize env and n */
	env = calloc(1, sizeof(struct Token));
	require(env != NULL, "Memory initialization of env failed\n");
	struct Token* n;
	n = env;
	int i;
	int j;
	int k;
	char* envp_line;

	for(i = 0; i < max; i = i + 1)
	{
		n->var = calloc(MAX_STRING, sizeof(char));
		require(n->var != NULL, "Memory initialization of n->var in population of env failed\n");
		n->value = calloc(MAX_STRING, sizeof(char));
		require(n->value != NULL, "Memory initialization of n->var in population of env failed\n");
		j = 0;
		/*
		 * envp is weird.
		 * When referencing envp[i]'s characters directly, they were all jumbled.
		 * So just copy envp[i] to envp_line, and work with that - that seems
		 * to fix it.
		 */
		envp_line = calloc(MAX_STRING, sizeof(char));
		require(envp_line != NULL, "Memory initialization of envp_line in population of env failed\n");
		strcpy(envp_line, envp[i]);

		while(envp_line[j] != '=')
		{
			/* Copy over everything up to = to var */
			n->var[j] = envp_line[j];
			j = j + 1;
		}

		/* If we get strange input, we need to ignore it */
		if(n->var == NULL)
		{
			continue;
		}

		j = j + 1; /* Skip over = */
		k = 0; /* As envp[i] will continue as j but n->value begins at 0 */

		while(envp_line[j] != 0)
		{
			/* Copy everything else to value */
			n->value[k] = envp_line[j];
			j = j + 1;
			k = k + 1;
		}

		/* Sometimes, we get lines like VAR=, indicating nothing is in the variable */
		if(n->value == NULL)
		{
			n->value = "";
		}

		/* Advance to next part of linked list */
		n->next = calloc(1, sizeof(struct Token));
		require(n->next != NULL, "Memory initialization of n->next in population of env failed\n");
		n = n->next;
	}

	/* Get rid of node on the end */
	n = NULL;
	/* Also destroy the n->next reference */
	n = env;

	while(n->next->var != NULL)
	{
		n = n->next;
	}

	n->next = NULL;
}
