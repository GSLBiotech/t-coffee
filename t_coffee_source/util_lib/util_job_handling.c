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
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>

#include "io_lib_header.h"
#include "util_lib_header.h"
#include "define_header.h"
#include "dp_lib_header.h"



Job_TC* print_lib_job ( Job_TC *job,char *string, ...)
{
  va_list ap;
  char **list;
  char **value;
  char **name;
  int a, np, n;
    
  char bname[100];
  char bval[100];
  
  list=string2list2(string, " =");
  n=atoi (list[0]);
  

  name =(char**)vcalloc ( (n-1)/2, sizeof (char*));
  value=(char**)vcalloc ( (n-1)/2, sizeof (char*));
  
  
  va_start (ap, string);
  for (a=1, np=0; a<n; a++)
    {
      if (list[a][0]=='%' && list[a][1]=='s')
	{
	  value[np++]=duplicate_string (va_arg (ap,char *));
	}
      else if (list[a][0]=='%')
	{
	  if ( list[a][1]=='d')sprintf (bval, "%d", va_arg (ap,int));
	  else if ( list[a][1]=='f')sprintf (bval, "%lf",(float) va_arg (ap,double));
	  else if ( list[a][1]=='p')sprintf (bval, "%ld", (long)va_arg (ap,void *));
	  value[np++]=duplicate_string (bval);
	}
      else
	{
	  sprintf (bname, "%s", list[a]);
	  name[np]=duplicate_string (bname);
	}
    }
  free_arrayN((void**)list, 2);
  
  va_end (ap);
  
  return print_lib_job2 ( job,np,name,value);
}

static int njobs;
Job_TC *print_lib_job2 ( Job_TC* job, int n, char **name, char **value)
{
  int a;
  
  if ( job==NULL)
    {
      job=(Job_TC*)vcalloc ( 1, sizeof (Job_TC));
      job->pl=(char**)vcalloc (100, sizeof (char*));job->pl[job->np++]=(char*)job->pl;
      job->jobid=njobs++;
    }
  
  for ( a=0; a< n; a++)
    {
      int string=0;
      if ( strstr(name[a], "control") && !job->control){job->control=(Job_control_TC*)vcalloc ( 1, sizeof (Job_control_TC));job->pl[job->np++]=(char*)job->control;}
      else if ( strstr(name[a], "io") && !job->io){job->io=(Job_io_TC*)vcalloc ( 1, sizeof (Job_io_TC));job->pl[job->np++]=(char*)job->io;}
      else if ( strstr(name[a], "param") && !job->param){job->param=(Job_param_TC*)vcalloc ( 1, sizeof (Job_param_TC));job->pl[job->np++]=(char*)job->param;}
      
      if (           strm (name[a], "control"))                {job->control=(struct Job_control_TC*)atol(value[a]);string=0;}
      else if (      strm (name[a], "control->submitF"))       {(job->control) ->submitF=(struct Job_TC *(*)(struct Job_TC *))atol(value[a]);string=0;}
      else if (      strm (name[a], "control->retrieveF"))     {(job->control) ->retrieveF=(struct Job_TC *(*)(struct Job_TC *))atol(value[a]);string=0;}
      else if (      strm (name[a], "control->mode"))   {(job->control)->mode=value[a];string=1;}
      
      else if ( strm (name[a], "param"))            {job->param=(struct Job_param_TC*)atol(value[a]);string=0;}
      else if ( strm (name[a], "param->method"))    {job->pl[job->np++]=((job->param)->method)=value[a];string=1;}
      else if ( strm (name[a], "param->TCM"))       {(job->param)->TCM= (TC_method *) atol(value[a]) ;string=0;}
      else if ( strm (name[a], "param->aln_c"))     {job->pl[job->np++]=(job->param)->aln_c=value[a] ;string=1;}
      else if ( strm (name[a], "param->seq_c"))     {job->pl[job->np++]=(job->param)->seq_c=value[a] ;string=1;}
                  	   
      
      else if (           strm (name[a], "io"))                {job->io=(struct Job_io_TC*)atol(value[a]);string=0;}
      else if ( strm (name[a], "io->out")) {job->pl[job->np++]=(job->io)->out=value[a] ;string=1;}
      else if ( strm (name[a], "io->in" )) {job->pl[job->np++]=(job->io)->in =value[a] ;string=1;}
      else if ( strm (name[a], "io->CL"))  {(job->io)->CL=(Constraint_list*)atol (value[a]); string=0;}
      else
	{
	  fprintf ( stderr, "ERROR: print_lib_job2: %s is unknown [FATAL:%s]", name[a], PROGRAM);
	  myexit (EXIT_FAILURE);
	}
      if ( string==0) vfree ( value[a]);
    }
  vfree ( value);
  free_arrayN ((void **)name, 2);
  return job;
}



/*Stack Manipulation*/
Job_TC *queue_cat  (Job_TC *P, Job_TC *C)
{
  if ( !P && !C) return NULL;
  else if (!P || P->jobid==-1)
    {
      vfree (P);
      C->p=NULL;
      return C;
    }
  else
    {
      P->c=C;
      if (C)C->p=P;
      return queue2last(P);
    }
  return NULL;
}
Job_TC *free_queue  (Job_TC *job)
{
  return NULL;
  if (!job) return job;
  else
    {
      job=queue2last(job);
      while ( job)
	{
	  job=free_job (job);
	}
      return job;
    }
}
Job_TC *free_job  (Job_TC *job)
  {
    int a;
    Job_TC *p;
    
    if ( !job ) return job;
    else
      {
	for ( a=job->np-1; a>=0; a--)
	  vfree ( job->pl[a]);
	p=job->p;
	job->p=job->c=NULL;
	vfree (job);
	return p;
      }
    return NULL;
  }
Job_TC * queue2heap (Job_TC*job)
{

  while (job && job->p)
    job=job->p;
  return job;
}
Job_TC * queue2last (Job_TC*job)
{

  while (job && job->c)
    {
      job=job->c;
    }
  return job;
}

int queue2n (Job_TC*job)
{
  int n=0;


  job=queue2last (job);
  while (job && job->p)
    {
      n++;
      job=job->p;

    }
  return n;
}

Job_TC * descend_queue (Job_TC*job)
{

  if (!job ||!job->c)return job;
  else
    {
      (job->c)->p=job;
      job=job->c;
    }
  return job;
}

Job_TC* delete_job (Job_TC *job)
{
  Job_TC *p, *c;
  
  p=job->p;
  c=job->c;
  free_job (job);
  
  return queue_cat (p, c);
}

Job_TC*** split_job_list (Job_TC *job, int ns)
{
  int a,u,n,nj,split;
  Job_TC*** jl;
  Job_TC *ljob;
  //retun a pointer to ns splits for joblist
  
  
  if (ns==0)return NULL;
  job=queue2heap(job);
  jl=(Job_TC***)vcalloc(ns+1, sizeof (Job_TC**));
  jl[0]=(Job_TC**)vcalloc (2, sizeof (Job_TC*));
  
  nj=queue2n(job);
  split=(nj/ns)+1;
     
  n=a=u=0;
  jl[a][0]=job;
  while (job)
    {
      ljob=job;
      if (n==split && a<ns)
	{
	  jl[a][1]=job;
	  if (a<ns-1)
	    {
	      jl[a+1]=(Job_TC**)vcalloc (2, sizeof (Job_TC*));
	      jl[a+1][0]=job;
	      u++;
	    }
	  a++;
	  n=0;
	}
      n++;
      job=job->c;
    }

  /*Display job details when debugging
  a=0;
  while (jl[a])
    {
      Job_TC *start,* end;
      int todo=0;
      
      start=job=jl[a][0];
      end=jl[a][1];
      while (job!=end){todo++;job=job->c;}
      job=start;
      while (job!=end)
	{
	  HERE ("--- %d %d %s",a, job, job->param->aln_c);
	  job=job->c;
	}
      a++;
    }
  
    a=0;
    while (jl[a]){HERE ("**** %d %d ", jl [a][0], jl[a][1]);a++;}
    myexit (0);
  */
  return jl;
}



  
/*Job Control*/
Job_TC* submit_job ( Job_TC *job)
{
  
  if (!(job->control)->mode ||!(job->control)->mode[0] || 1==1)
    {
      return (job->control)->submitF (job);
    }
  else
    {
      fprintf ( stderr, "\n%s is an unkown mode for posting jobs [FATAL:%s]",(job->control)->mode, PROGRAM);
      myexit (EXIT_FAILURE);
      return NULL;
    }
  
}

Job_TC* retrieve_job ( Job_TC *job)
{
  if (!(job->control)->mode ||!(job->control)->mode[0] || 1==1)
    {
      return (job->control)->retrieveF (job);
    }
  else
    {
      fprintf ( stderr, "\n%s is an unkown mode for posting jobs [FATAL:%s]",(job->control)->mode, PROGRAM);
      myexit (EXIT_FAILURE);
      return NULL;
    }
}
int **n2splits (int splits, int tot)
{
  int **l;
  int a,b,delta;
  
  if (splits==0)return NULL;
  else if ( tot==0)return NULL;
  else
    {
      
      l=declare_int (splits,2);
      delta=tot/splits;
      
      for (a=0,b=0; a<splits && b<tot; a++,b+=delta)
	{
	  l[a][0]=b;
	  l[a][1]=MIN((b+delta),tot);
	}
      l[splits-1][1]=MAX((l[splits-1][1]),tot);
      return l;
    }
}
  
