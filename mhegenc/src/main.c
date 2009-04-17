/* -*- c -*- ****************************************************************
 *
 *   Copyright 2003 Samsung Electronics (UK) Ltd.
 *   Copyright 2005 SpongeLava Ltd.
 *      
 ****************************************************************************
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 ****************************************************************************/
/*
  This is the main file for the MHEG-5 encoder/decoder.
 */

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <getopt.h>

#include <asn-incl.h>
#include "mh_snacc.h"
#include "diag.h"
#include "lexer.h"
#include "parser.h"

extern int yydebug;             /* parser */
extern InterchangedObject snacc_object; /* parser */
extern int include_comments;    /* decoder */
extern int testing_mode;

static const char *rcsid =
"@(#) $Id: main.c 1873 2008-08-24 13:36:54Z pthomas $";

#define ENCODE_BUFFER_SIZE (16 * 1024)

#ifndef VERSION_MAJOR
#error need to define version macros
#endif
#ifndef VERSION_MINOR
#error need to define version macros
#endif

static void
print_version()
{
    printf("MHEG-5 textual notation encoder version %d.%d\n",
           VERSION_MAJOR, VERSION_MINOR);
}

static void
usage()
{
    print_version();
    
    printf("Usage: mhegenc [options] file\n"
           "Options:\n"
           "  -h                  Display this message and exit\n"
           "  -o <file>           Save output in <file>\n"
           "  -v                  Display debug messages from the encoder\n"
           "  -V                  Display version and exit\n"
           "  -d                  decode ASN.1 and save textual notation\n"
           "  -c                  include useful comments in decode\n"
           "  -W                  Display warnings about input file\n"
           "  -O                  Prune defaults and unsupported features\n"
           "  -t                  Testing (allow items not in UK/FreeSat profile)\n");
}


int
main(int argc, char *argv[])
{
    int opt;
    const char arglist[] = "fthvVdcWo:OD:";

    char *infilename = "";
    char *outfilename = "";
    char name_buffer[FILENAME_MAX + 1];
    int debug_mode = 0;
    int warn_mode = 0;
    int optimise_mode = 0;
    int decode_mode = 0;
    testing_mode = 0;

    FILE *outp;
    
    while( (opt = getopt(argc, argv, arglist)) != -1 )
    {
        switch(opt)
        {
        case 'h':
            usage(); exit(0);
        case 'o':
            outfilename = optarg; break;
        case 'v':
            debug_mode = 1; break;
        case 'V':
            print_version();
            exit(0);
        case 'W':
            warn_mode = 1; break;
        case 'O':
            optimise_mode = 1; break;
        case 'd':
            decode_mode = 1; break;
        case 'c':
            include_comments = 1; break;
        case 't':
            testing_mode = 1; break;
        case ':':
            fprintf(stderr, "Need an argument for %c option\n", optopt);
            usage();
            exit (-1);
        case '?':
            fprintf(stderr, "Unknown option %c\n", optopt);
            usage();
            exit (-1);
        }
    }

    if( optind >= argc ){
        fprintf(stderr, "No input file specified\n");
        usage();
        exit(-1);
    }
    
    infilename = argv[optind];
    diag_set_filename(infilename);
    
    InitNibbleMem(1024, 1024);

    if( decode_mode )
    {
        FILE *infp;
        int length;
        int val;
        ENV_TYPE env;
        ExpBuf  snacc_buf;            // buffer used by snacc routines.
        ExpBuf* buf = &snacc_buf;
        char* decode_buffer;
        
        if( !outfilename[0] ) {
            /* No outfile specified, put .mhg extension on infile. */
            char *idx;
            int len;
            len = strlen(infilename);
            if( len > FILENAME_MAX - 4 ){
                fprintf(stderr, "Ludicrous filename for this platform.");
                exit (-1);
            }
            strcpy(name_buffer, infilename);
            outfilename = name_buffer;
            idx = strrchr(outfilename, '.');
            if( !idx )
                /* just append */
                strcat(name_buffer, ".mhg");
            else
                strcpy(idx, ".mhg");

            printf("Generated output filename: %s\n\n", outfilename);
        }

        /* Read the input contents to the local buffer. */
        infp = fopen(infilename, "rb");
        if( !infp ){
            perror("Can't open input file");
            return -1;
        }

        fseek(infp, 0, SEEK_END);
        length = ftell(infp);
        fseek(infp, 0, SEEK_SET);
        decode_buffer = malloc(length);
        if( !decode_buffer )
        {
            fprintf(stderr, "Sorry, input file is to big (%d bytes).\n",
                    length);
            return -1;
        }
            
        fread(decode_buffer, length, 1, infp);
        fclose(infp);
               
        /* Set up the snacc environment, and do the decode. */
        ExpBufInstallDataInBuf( &snacc_buf, ((char *)decode_buffer), length );
        
        if ((val = setjmp (env)) == 0)
        {
            AsnTag tag;
            AsnLen len, declen;
            InterchangedObject decoded;
            
            tag = BDecTag(&buf, &declen, env);
            len = BDecLen(&buf, &declen, env);

            declen = 0;
            BDecInterchangedObjectContent(&buf, tag, len, &decoded, &declen,
                                          env);

            infp = fopen(outfilename, "wb");
            if( !infp ){
                perror("Can't open output file");
                return -1;
            }
            
            if( include_comments ){
                char date_time_str[256];
                time_t curtime;
                time(&curtime);
                strftime(date_time_str, 256, "%d %B %Y @ %X",
                         localtime(&curtime));
                fprintf(infp,
                        "// -*- mheg -*- \n"
                        "// Decoded from ASN.1 file '%s'\n"
                        "// %s\n"
                        "//   by 'mhegenc'\n"
                        "//   bugs/suggestions to mhegenc@seri.co.uk please!\n"
                        "//\n",
                        infilename, date_time_str
                        );
            }
    

            PrintInterchangedObject(infp, &decoded, 0);
            fclose(infp);
            
        }
        else
        {
            fprintf (stderr, "ERROR - Decode routines returned %d\n",val);
            return -2;
        }
        
        
    }
    else
    {   
        AsnLen length;
        ExpBuf *buf;
        
        if( !outfilename[0] ) {
            /* No outfile specified, put .asn extension on infile. */
            char *idx;
            int len;
            len = strlen(infilename);
            if( len > FILENAME_MAX - 4 ){
                fprintf(stderr, "Ludicrous filename for this platform.");
                exit (-1);
            }
            strcpy(name_buffer, infilename);
            outfilename = name_buffer;
            idx = strrchr(outfilename, '.');
            if( !idx )
                /* just append */
                strcat(name_buffer, ".asn");
            else
                strcpy(idx, ".asn");

            printf("Generated output filename: %s\n\n", outfilename);
        }

        if( debug_mode )        
            yydebug = 1;

        /* Using a dynamic buffer to do the encoding. */
        ExpBufInit(ENCODE_BUFFER_SIZE);
        buf = ExpBufAllocBufAndData();
        
        /* Open the files. */
        yyin = fopen(infilename, "rb");
        if( !yyin ){
            perror("Can't open input file");
            return -1;
        }

        /* Do the parse. */
        if( yyparse() )
        {
            fclose(yyin);
            return -2;
        }

        fclose(yyin);

        /* Do the analysis */
//        analyse(&snacc_object);

        /* Do the encode. */
        length = BEncInterchangedObjectContent(&buf, &snacc_object);

        /* Check for an error (write past end of buffer) */
        if( ExpBufWriteError(&buf) )
        {
            fprintf (stderr, "ERROR - unexpected encoding error\n");
            return -1;
        }

        /* Write the output to file. */
        outp = fopen(outfilename, "wb");
        if( !outp ){
            perror("Can't open output file");
            return -1;
        }

        /* The ASN.1 message is now in a chain of buffers that must be
         * written out in order.
         */
        {
            ExpBuf *tmpBuf;
            buf->curr = buf->dataStart;
            for ( tmpBuf = buf; tmpBuf != NULL; tmpBuf = tmpBuf->next)
            {
                fwrite(tmpBuf->dataStart,
                       tmpBuf->dataEnd - tmpBuf->dataStart, 1,
                       outp);
            }
        }
        

        fclose(outp);        
    }

    /* Clean up (a bit!) */
    ShutdownNibbleMem();
    return 0;
}


/*
 * Local variables:
 * mode: c
 * compile-command: "make -C ../"
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
