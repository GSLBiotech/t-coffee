/******************************COPYRIGHT NOTICE*******************************/
/*  (c) Centro de Regulacio Genomica                                                        */
/*  and                                                                                     */
/*  Cedric Notredame                                                                        */
/*  12 Aug 2014 - 22:07.                                                                    */
/*All rights reserved.                                                                      */
/*This file is part of T-COFFEE.                                                            */
/*                                                                                          */
/*    T-COFFEE is free software; you can redistribute it and/or modify                      */
/*    it under the terms of the GNU General Public License as published by                  */
/*    the Free Software Foundation; either version 2 of the License, or                     */
/*    (at your option) any later version.                                                   */
/*                                                                                          */
/*    T-COFFEE is distributed in the hope that it will be useful,                           */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of                        */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                         */
/*    GNU General Public License for more details.                                          */
/*                                                                                          */
/*    You should have received a copy of the GNU General Public License                     */
/*    along with Foobar; if not, write to the Free Software                                 */
/*    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA             */
/*...............................................                                           */
/*  If you need some more information                                                       */
/*  cedric.notredame@europe.com                                                             */
/*...............................................                                           */
/******************************COPYRIGHT NOTICE*******************************/
// #include "km_util.h"
#include "platform_lib/platform_lib.h"
#include "km_coffee_header.h"
FILE *
my_fopen(char *name_f, char *mode)
{
	FILE *name_F = fopen(name_f, mode);
	if (name_F == NULL)
	{
		fprintf(stderr, "ERROR: Cannot open file %s!\n", name_f);
		exit(1);
	}
	else
		return name_F;
}


void *
my_malloc(size_t size)
{
	void *p = malloc(size);
	if (p == NULL)
	{
    fprintf(stderr, "ERROR: Could not allocate space of size %llu\n", size);
		exit(1);
	}
	return p;
}

void *
my_calloc ( size_t num, size_t size )
{
	void *p = calloc(num, size);
	if (p == NULL)
	{
    fprintf(stderr, "ERROR: Could not allocate space of size %llu\n", size*num);
		exit(1);
	}
	return p;
}


void *
my_realloc(void *p, size_t size)
{
	p = realloc(p, size);
	if (p == NULL)
	{
    fprintf(stderr, "ERROR: Could not allocate space of size %llu\n", size);
		exit(1);
	}
	return p;
}




char *
my_make_temp_dir(char *templatee, char *function)
{
#ifdef _MSC_VER
  int failed = 1;
  char *temp_dir_name = (char*) malloc( sizeof(char) * MAX_PATH );

  int retry = 20;
  while( --retry > 0 )
  {
    sprintf( temp_dir_name, "%s", templatee );
    if( NULL == mktemp( temp_dir_name ) )
      continue;

    if( CreateDirectory( temp_dir_name, NULL ) )
    {
      failed = 0;
      break;
    }
  }

  if( failed )
#else
  char *temp_dir_name = (char*)malloc(256);
	sprintf(temp_dir_name, "%s", templatee);
// 	char hostname[50];
// 	tc_gethostname(hostname, 50);
// 	printf("%s %s\n",hostname, temp_dir_name);
	if ((temp_dir_name = mkdtemp(temp_dir_name))==NULL)
#endif
  {
		int errsv = errno;
		fprintf(stderr, "ERROR! A temporary directory could not be created: %s\n",strerror(errsv));
    fprintf(stderr, "This error was caused by '%s'\n", function);

		exit(-1);
	}
	return temp_dir_name;
}









