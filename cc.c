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
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include"cc.h"

/* The core functions */
void initialize_types();
struct token_list* read_all_tokens(FILE* a, struct token_list* current, char* filename);
struct token_list* reverse_list(struct token_list* head);

struct token_list* remove_line_comments(struct token_list* head);
struct token_list* remove_line_comment_tokens(struct token_list* head);
struct token_list* remove_preprocessor_directives(struct token_list* head);

void eat_newline_tokens();
void preprocess();
void program();
void recursive_output(struct token_list* i, FILE* out);
void output_tokens(struct token_list *i, FILE* out);
int numerate_string(char *a);

int main(int argc, char** argv)
{
	MAX_STRING = 4096;
	BOOTSTRAP_MODE = FALSE;
	PREPROCESSOR_MODE = FALSE;
	FILE* in = stdin;
	FILE* destination_file = stdout;
	Architecture = KNIGHT_NATIVE; /* Assume Knight-native */
	char* name;

	int i = 1;
	while(i <= argc)
	{
		if(NULL == argv[i])
		{
			i = i + 1;
		}
		else if(match(argv[i], "-f") || match(argv[i], "--file"))
		{
			if(NULL == hold_string)
			{
				hold_string = calloc(MAX_STRING, sizeof(char));
				require(NULL != hold_string, "Impossible Exhustion has occured\n");
			}

			name = argv[i + 1];
			in = fopen(name, "r");
			if(NULL == in)
			{
				fputs("Unable to open for reading file: ", stderr);
				fputs(name, stderr);
				fputs("\n Aborting to avoid problems\n", stderr);
				exit(EXIT_FAILURE);
			}
			global_token = read_all_tokens(in, global_token, name);
			fclose(in);
			i = i + 2;
		}
		else if(match(argv[i], "-o") || match(argv[i], "--output"))
		{
			destination_file = fopen(argv[i + 1], "w");
			if(NULL == destination_file)
			{
				fputs("Unable to open for writing file: ", stderr);
				fputs(argv[i + 1], stderr);
				fputs("\n Aborting to avoid problems\n", stderr);
				exit(EXIT_FAILURE);
			}
			i = i + 2;
		}
		else if(match(argv[i], "--max-string"))
		{
			MAX_STRING = numerate_string(argv[i+1]);
			require(0 < MAX_STRING, "Not a valid string size\nAbort and fix your --max-string\n");
			i = i + 2;
		}
		else if(match(argv[i], "-h") || match(argv[i], "--help"))
		{
			fputs(" -f input file\n -o output file\n --help for this message\n --version for file version\n", stdout);
			exit(EXIT_SUCCESS);
		}
		else if(match(argv[i], "-V") || match(argv[i], "--version"))
		{
			fputs("M2-Mesoplanet v1.7.0\n", stderr);
			exit(EXIT_SUCCESS);
		}
		else
		{
			fputs("UNKNOWN ARGUMENT\n", stdout);
			exit(EXIT_FAILURE);
		}
	}

	/* Deal with special case of wanting to read from standard input */
	if(stdin == in)
	{
		hold_string = calloc(MAX_STRING, sizeof(char));
		require(NULL != hold_string, "Impossible Exhustion has occured\n");
		global_token = read_all_tokens(in, global_token, "STDIN");
	}

	if(NULL == global_token)
	{
		fputs("Either no input files were given or they were empty\n", stderr);
		exit(EXIT_FAILURE);
	}
	global_token = reverse_list(global_token);

	global_token = remove_line_comments(global_token);
	preprocess();

	fputs("\n/* Preprocessed source */\n", destination_file);
	output_tokens(global_token, destination_file);

	if (destination_file != stdout)
	{
		fclose(destination_file);
	}
	return EXIT_SUCCESS;
}
