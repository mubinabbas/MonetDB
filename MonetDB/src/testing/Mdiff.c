/*
 * The contents of this file are subject to the MonetDB Public
 * License Version 1.0 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at
 * http://monetdb.cwi.nl/Legal/MonetDBLicense-1.0.html
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is the Monet Database System.
 *
 * The Initial Developer of the Original Code is CWI.
 * Portions created by CWI are Copyright (C) 1997-2005 CWI.
 * All Rights Reserved.
 */

#ifdef HAVE_CONFIG_H
#include <monetdb_config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include "difflib.h"
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_IO_H
# include <io.h>
#endif
#include <string.h>

#include "monet_options.h"

void showUsage(char *name)
{
    printf("Usage:  %s [-I<exp>] [-C<num>] [-A<num>] [-t<text>] [-r<rev>] <oldfile> <newfile> [<outfile>]\n",name);
    printf("\n");
    printf(" -I<exp>   : ignore lines matching <exp> during diff (optional, default: -I'^#')\n");
    printf(" -C<num>   : use <num> lines of context during diff (optional, default: -C1)\n");
    printf(" -A<num>   : accuracy for diff: 0=lines, 1=words, 2=chars (optional, default: -A1)\n");
    printf(" -d        : change the algorithm to perhaps find a smaller set of changes;\n");
    printf("             this makes diff slower (sometimes much slower)\n");
    printf(" -t<text>  : text for caption (optional, default: empty)\n");
    printf(" -r<rev>   : revision of old file (optinal, default: empty)\n");
    printf(" <oldfile> : first file for diff\n");
    printf(" <newfile> : second file for diff\n");
    printf(" <outfile> : file for HTML output (optional, default: stdout)\n");
}

int main(int argc, char** argv)
{
  char EMPTY[]="";
  char DEFAULT[]="-I'^#'";
  char ignoreWHITE[]=" -b -B";
  char *old_fn,*new_fn,*html_fn,*caption=EMPTY,*revision=EMPTY,*ignoreEXP=DEFAULT,*ignore;
  int LWC=1,context=1,option,mindiff=0;

  while((option=getopt(argc,argv,"hdA:C:I:t:r:"))!=EOF)
   switch (option)
     {
       case 'd': mindiff=1; break;
       case 'A': LWC=atoi(optarg); break;
       case 'C': context=atoi(optarg); break;
       case 'I': ignoreEXP=(char*)malloc(strlen(optarg)+6);
#ifdef NATIVE_WIN32
		 strcpy(ignoreEXP,"-I");
#else
		 strcpy(ignoreEXP,"'-I");
#endif
		 strcat(ignoreEXP,optarg);
#ifndef NATIVE_WIN32
		 strcat(ignoreEXP,"'");
#endif
		 break;
       case 't': caption=optarg; break;
       case 'r': revision=optarg; break;
       case 'h':
       default:  showUsage(argv[0]); exit(1);
     }
     
  ignore=(char*)malloc(strlen(ignoreEXP)+strlen(ignoreWHITE)+2);
  strcpy(ignore,ignoreEXP);
  strcat(ignore,ignoreWHITE);

  optind--;
  old_fn=((argc>(++optind))?argv[optind]:"-");
  new_fn=((argc>(++optind))?argv[optind]:"-");
  html_fn=((argc>(++optind))?argv[optind]:"-");

  TRACE(fprintf(STDERR,"%s %s -A %i -C %i %s -t %s -r %s  %s %s %s\n",
                 argv[0],mindiff?"-d":"",LWC,context,ignore,caption,revision,old_fn,new_fn,html_fn));

  switch ( oldnew2html (mindiff,LWC,context,ignore,old_fn,new_fn,html_fn,caption,revision) )
    {
      case 0: fprintf(STDERR,"%s and %s are equal.\n",old_fn,new_fn); break;
      case 1: fprintf(STDERR,"%s and %s differ slightly.\n",old_fn,new_fn); break;
      case 2: fprintf(STDERR,"%s and %s differ SIGNIFICANTLY!\n",old_fn,new_fn); break;
    }

  TRACE(fprintf(STDERR,"done.\n"));
  return 0;
}

