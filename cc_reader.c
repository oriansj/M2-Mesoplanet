/* Copyright (C) 2016 Jeremiah Orians
 * Copyright (C) 2021 Andrius Štikonas <andrius@stikonas.eu>
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

#include "cc.h"
char* env_lookup(char* variable);
char* int2str(int x, int base, int signed_p);

struct visited
{
	struct visited* prev;
	char* name;
};

/* Globals */
FILE* input;
struct token_list* token;
int line;
char* file;
struct visited* vision;

int previously_seen(char* s)
{
	struct visited* v = vision;
	while(NULL != v)
	{
		if(match(v->name, s)) return TRUE;
		v = v->prev;
	}
	return FALSE;
}

void just_seen(char* s)
{
	struct visited* hold = calloc(1, sizeof(struct visited));
	hold->prev = vision;
	hold->name = s;
	vision = hold;
}

int grab_byte()
{
	int c = fgetc(input);
	if(10 == c) line = line + 1;
	if(9 == c) c = ' ';
	return c;
}

int consume_byte(int c)
{
	hold_string[string_index] = c;
	string_index = string_index + 1;
	require(MAX_STRING > string_index, "Token exceeded MAX_STRING char limit\nuse --max-string number to increase\n");
	return grab_byte();
}

int preserve_string(int c)
{
	int frequent = c;
	int escape = FALSE;
	do
	{
		if(!escape && '\\' == c ) escape = TRUE;
		else escape = FALSE;
		c = consume_byte(c);
		require(EOF != c, "Unterminated string\n");
	} while(escape || (c != frequent));
	return grab_byte();
}


void copy_string(char* target, char* source, int max)
{
	int i = 0;
	while(0 != source[i])
	{
		target[i] = source[i];
		i = i + 1;
		if(i == max) break;
	}
}


void fixup_label()
{
	int hold = ':';
	int prev;
	int i = 0;
	do
	{
		prev = hold;
		hold = hold_string[i];
		hold_string[i] = prev;
		i = i + 1;
	} while(0 != hold);
}

int preserve_keyword(int c, char* S)
{
	while(in_set(c, S))
	{
		c = consume_byte(c);
	}
	return c;
}

void reset_hold_string()
{
	int i = string_index + 2;
	while(0 <= i)
	{
		hold_string[i] = 0;
		i = i - 1;
	}
}

/* note if this is the first token in the list, head needs fixing up */
struct token_list* eat_token(struct token_list* token)
{
	if(NULL != token->prev)
	{
		token->prev->next = token->next;
	}

	/* update backlinks */
	if(NULL != token->next)
	{
		token->next->prev = token->prev;
	}

	return token->next;
}

struct token_list* eat_until_newline(struct token_list* head)
{
	while (NULL != head)
	{
		if('\n' == head->s[0])
		{
			return head;
		}
		else
		{
			head = eat_token(head);
		}
	}

	return NULL;
}

struct token_list* remove_line_comments(struct token_list* head)
{
	struct token_list* first = NULL;

	while (NULL != head)
	{
		if(match("//", head->s))
		{
			head = eat_until_newline(head);
		}
		else
		{
			if(NULL == first)
			{
				first = head;
			}
			head = head->next;
		}
	}

	return first;
}

struct token_list* remove_comments(struct token_list* head)
{
	struct token_list* first = NULL;

	while (NULL != head)
	{
		if(match("//", head->s))
		{
			head = eat_token(head);
		}
		else if('/' == head->s[0] && '*' == head->s[1])
		{
			head = eat_token(head);
		}
		else
		{
			if(NULL == first)
			{
				first = head;
			}
			head = head->next;
		}
	}

	return first;
}

struct token_list* remove_preprocessor_directives(struct token_list* head)
{
	struct token_list* first = NULL;

	while (NULL != head)
	{
		if('#' == head->s[0])
		{
			head = eat_until_newline(head);
		}
		else
		{
			if(NULL == first)
			{
				first = head;
			}
			head = head->next;
		}
	}

	return first;
}

void new_token(char* s, int size)
{
	struct token_list* current = calloc(1, sizeof(struct token_list));
	require(NULL != current, "Exhausted memory while getting token\n");

	/* More efficiently allocate memory for string */
	current->s = calloc(size, sizeof(char));
	require(NULL != current->s, "Exhausted memory while trying to copy a token\n");
	copy_string(current->s, s, MAX_STRING);

	current->prev = token;
	current->next = token;
	current->linenumber = line;
	current->filename = file;
	token = current;
}

int get_token(int c)
{
	reset_hold_string();
	string_index = 0;

	if(c == EOF)
	{
		return c;
	}
	else if((32 == c) || (9 == c) || (c == '\n'))
	{
		c = consume_byte(c);
	}
	else if('#' == c)
	{
		c = consume_byte(c);
		c = preserve_keyword(c, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_");
	}
	else if(in_set(c, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_"))
	{
		c = preserve_keyword(c, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_");
		if(':' == c)
		{
			fixup_label();
			c = ' ';
		}
	}
	else if(in_set(c, "<=>|&!^%"))
	{
		c = preserve_keyword(c, "<=>|&!^%");
	}
	else if(in_set(c, "'\""))
	{
		c = preserve_string(c);
	}
	else if(c == '/')
	{
		c = consume_byte(c);
		if(c == '*')
		{
			c = consume_byte(c);
			while(c != '/')
			{
				while(c != '*')
				{
					c = consume_byte(c);
					require(EOF != c, "Hit EOF inside of block comment\n");
				}
				c = consume_byte(c);
				require(EOF != c, "Hit EOF inside of block comment\n");
			}
			c = consume_byte(c);
		}
		else if(c == '/')
		{
			c = consume_byte(c);
		}
		else if(c == '=')
		{
			c = consume_byte(c);
		}
	}
	else if(c == '*')
	{
		c = consume_byte(c);
		if(c == '=')
		{
			c = consume_byte(c);
		}
	}
	else if(c == '+')
	{
		c = consume_byte(c);
		if(c == '=')
		{
			c = consume_byte(c);
		}
		if(c == '+')
		{
			c = consume_byte(c);
		}
	}
	else if(c == '-')
	{
		c = consume_byte(c);
		if(c == '=')
		{
			c = consume_byte(c);
		}
		if(c == '>')
		{
			c = consume_byte(c);
		}
		if(c == '-')
		{
			c = consume_byte(c);
		}
	}
	else
	{
		c = consume_byte(c);
	}

	new_token(hold_string, string_index + 2);
	return c;
}

struct token_list* reverse_list(struct token_list* head)
{
	struct token_list* root = NULL;
	struct token_list* next;
	while(NULL != head)
	{
		next = head->next;
		head->next = root;
		root = head;
		head = next;
	}
	return root;
}

int read_include(int c)
{
	reset_hold_string();
	string_index = 0;
	int done = FALSE;
	int ch;

	while(!done)
	{
		if(c == EOF)
		{
			fputs("we don't support EOF as a filename in #include statements\n", stderr);
			exit(EXIT_FAILURE);
		}
		else if((32 == c) || (9 == c) || (c == '\n'))
		{
			c = grab_byte();
		}
		else if(('"' == c) || ('<' == c))
		{
			if('<' == c) c = '>';
			ch = c;
			do
			{
				c = consume_byte(c);
				require(EOF != c, "Unterminated filename in #include\n");
			} while(c != ch);
			if('>' == ch) hold_string[0] = '<';
			done = TRUE;
		}
	}

	/* with just a little extra to put in the matching at the end */
	new_token(hold_string, string_index + 3);
	return c;
}

struct token_list* read_all_tokens(FILE* a, struct token_list* current, char* filename);
int include_file(int ch)
{
	/* The old state to restore to */
	char* hold_filename = file;
	FILE* hold_input = input;
	char* hold_line;
	int hold_number;

	/* The new file to load */
	char* new_filename;
	FILE* new_file;

	require(EOF != ch, "#include failed to receive filename\n");
	/* Remove the #include */
	token = token->next;

	/* Get new filename */
	read_include(ch);
	ch = ' ';
	new_filename = token->s;
	/* Remove name from stream */
	token = token->next;

	/* Try to open the file */
	if('<' == new_filename[0])
	{
		char* path = env_lookup("M2LIBC_PATH");
		if(NULL == path) path = "./M2libc";
		reset_hold_string();
		strcat(hold_string, path);
		strcat(hold_string, "/");
		strcat(hold_string, new_filename + 1);
		strcat(new_filename, ">");
		new_file = fopen(hold_string, "r");
	}
	else
	{
		new_file = fopen(new_filename+1, "r");
		strcat(new_filename, "\"");
	}

	/* prevent multiple visits */
	if(previously_seen(new_filename)) return ch;
	just_seen(new_filename);

	/* special case this compatibility crap */
	if(match("\"../gcc_req.h\"", new_filename) || match("\"gcc_req.h\"", new_filename)) return ch;

	fputs("reading file: ", stderr);
	fputs(new_filename, stderr);
	fputc('\n', stderr);

	/* catch garbage input */
	if(NULL == new_file)
	{
		fputs("unable to read file: ", stderr);
		fputs(new_filename, stderr);
		fputs("\nAborting hard!\n", stderr);
		exit(EXIT_FAILURE);
	}


	/* Replace token */
	new_token("//", 4);
	new_token(" // #FILENAME", 11);
	new_token(new_filename, strlen(new_filename) + 2);
	new_token("1", 3);
	new_token("\n", 3);
	/* make sure to store return line number right after include */
	hold_line = int2str(line + 1, 10, FALSE);
	hold_number = line + 1;
	read_all_tokens(new_file, token, new_filename);

	/* put back old file info */
	new_token("//", 4);
	new_token(" // #FILENAME", 11);
	new_token(hold_filename, strlen(hold_filename)+2);
	new_token(hold_line, strlen(hold_line)+2);
	new_token("\n", 3);

	/* resume reading old file */
	input = hold_input;
	line = hold_number;
	file = hold_filename;
	return ch;
}

struct token_list* read_all_tokens(FILE* a, struct token_list* current, char* filename)
{
	input  = a;
	line = 1;
	file = filename;
	token = current;
	int ch = grab_byte();
	while(EOF != ch)
	{
		ch = get_token(ch);
		if(match("#include", token->s)) ch = include_file(ch);
	}

	return token;
}
