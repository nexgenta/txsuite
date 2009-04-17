#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "dertest-mheg.h"

int
main(int argc, char *argv[])
{
	char *dername;
	FILE *derfile;
	int len;
	InterchangedObject obj;

	if(argc != 2)
	{
		printf("Syntax: %s <DER-file>\n", argv[0]);
		exit(1);
	}

	dername = argv[1];
	if((derfile = fopen(dername, "r")) == NULL)
	{
		printf("fopen: %s\n", strerror(errno));
		exit(1);
	}

	fseek(derfile, 0, SEEK_END);
	len = ftell(derfile);
	rewind(derfile);
	if(der_decode_InterchangedObject(derfile, &obj, len) < 0)
		printf("failed\n");

	free_InterchangedObject(&obj);

	fclose(derfile);

	return 0;
}

