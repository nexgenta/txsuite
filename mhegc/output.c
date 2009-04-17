/*
 * output.c
 */

/*
 * Copyright (C) 2007, Simon Kilvington
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "utils.h"

void print_newline(FILE *);
void print_token(FILE *, char *);

int indent = 0;
bool newline = true;

/* keep track of () blocks that should be indented */
#define PAREN_LEVEL_MAX		8192
int paren_level = 0;
bool indent_paren[PAREN_LEVEL_MAX];

void
output_init(void)
{
	bzero(indent_paren, sizeof(indent_paren));

	return;
}

void
output_token(FILE *out, char *tok)
{
	/* assert */
	if(tok[0] == '\0')
		fatal("output_token: 0 length token");

	if(tok[0] == '(' && tok[1] == '\0')
	{
		/* assert */
		if(paren_level >= (PAREN_LEVEL_MAX - 1))
			fatal("output_token: max nested parenthesis reached");
		print_token(out, tok);
		paren_level ++;
		if(indent_paren[paren_level])
		{
			print_newline(out);
			indent ++;
		}
	}
	else if(tok[0] == ')' && tok[1] == '\0')
	{
		/* assert */
		if(paren_level <= 0)
			fatal("output_token: unexpected ')'");
		if(indent_paren[paren_level])
		{
			indent --;
			print_newline(out);
		}
		indent_paren[paren_level] = false;
		paren_level --;
		print_token(out, tok);
	}
	else if(tok[0] == '{' && tok[1] == ':')
	{
		print_newline(out);
		print_token(out, tok);
		indent ++;
	}
	else if(tok[0] == '}')
	{
		/* assert */
		if(indent == 0)
			fatal("output_token: unexpected '}'");
		indent --;
		print_newline(out);
		print_token(out, tok);
	}
	else if(strcmp(tok, ":ActionSlots") == 0
	     || strcmp(tok, ":Items") == 0
	     || strcmp(tok, ":LinkEffect") == 0
	     || strcmp(tok, ":MovementTable") == 0
	     || strcmp(tok, ":Multiplex") == 0
	     || strcmp(tok, ":NextScenes") == 0
	     || strcmp(tok, ":NoTokenActionSlots") == 0
	     || strcmp(tok, ":OnCloseDown") == 0
	     || strcmp(tok, ":OnRestart") == 0
	     || strcmp(tok, ":OnSpawnCloseDown") == 0
	     || strcmp(tok, ":OnStartUp") == 0
	     || strcmp(tok, ":Positions") == 0
	     || strcmp(tok, ":TokenGroupItems") == 0)
	{
		print_newline(out);
		print_token(out, tok);
		/* next () block should be indented */
		indent_paren[paren_level + 1] = true;
	}
	else if(tok[0] == ':'
	     && strcmp(tok, ":CCPriority") != 0
	     && strcmp(tok, ":ContentRef") != 0
	     && strcmp(tok, ":IndirectRef") != 0
	     && strcmp(tok, ":GBoolean") != 0
	     && strcmp(tok, ":GContentRef") != 0
	     && strcmp(tok, ":GInteger") != 0
	     && strcmp(tok, ":GObjectRef") != 0
	     && strcmp(tok, ":GOctetString") != 0
	     && strcmp(tok, ":NewCCPriority") != 0
	     && strcmp(tok, ":NewRefContent") != 0)
	{
		print_newline(out);
		print_token(out, tok);
	}
	else
	{
		print_token(out, tok);
	}

	return;
}

void
print_newline(FILE *out)
{
	if(!newline)
		fprintf(out, "\n");

	newline = true;

	return;
}

void
print_token(FILE *out, char *tok)
{
	int i = indent;

	if(newline)
	{
		while(i > 0)
		{
			fprintf(out, "\t");
			i --;
		}
		newline = false;
	}
	else
	{
		fprintf(out, " ");
	}

	fprintf(out, "%s", tok);

	return;
}
