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
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>

#include "io_lib_header.h"
#include "util_lib_header.h"
#include "define_header.h"
#include "dp_lib_header.h"


/*******************************************************************************/
/*                myers and Miller                                             */
/*                                                                             */
/*	makes DP between the the ns[0] sequences and the ns[1]                 */
/*                                                                             */
/*	for MODE, see the function  get_dp_cost                                */
/*******************************************************************************/

#define gap(k)  ((k) <= 0 ? 0 : g+h*(k))    /* k-symbol indel cost */
static int *sapp;                /* Current script append ptr */
static int  last;                /* Last script op appended */
                       /* Append "Delete k" op */
#define DEL(k)                \
{ if (last < 0)                \
    last = sapp[-1] -= (k);        \
  else                    \
    last = *sapp++ = -(k);        \
}
                        /* Append "Insert k" op */
#define INS(k)                \
{ if (last < 0)                \
    { sapp[-1] = (k); *sapp++ = last; }    \
  else                    \
    last = *sapp++ = (k);        \
}
 
#define REP { last = *sapp++ = 0; }        /* Append "Replace" op */
static int *sns;
static int **sls;
int myers_miller_pair_wise (Alignment *A,int *ns, int **ls,Constraint_list *CL )
	{
	int **pos;
	int a,b, i, j, l,l1, l2, len;
	int *S;
	char ** char_buf;
	int score;
		
	/********Prepare Penalties******/
	//ns2master_ns (ns,ls, &sns,&sls);
	sns=ns;
	sls=ls;

	/********************************/
	
	
	pos=aln2pos_simple ( A,-1, ns, ls);
	

	l1=strlen (A->seq_al[ls[0][0]]);
	l2=strlen (A->seq_al[ls[1][0]]);
	S=(int*)vcalloc (l1+l2+1, sizeof (int));
	last=0;
	sapp=S;

        score=diff (A,ns, ls, 0, l1, 0, l2, 0, 0, CL, pos);	
	diff (NULL,ns, ls, 0, l1, 0, l2, 0, 0, CL, pos);



	i=0;j=0;sapp=S; len=0;
	while (!(i==l1 && j==l2))
	      {
		  if (*sapp==0){i++; j++;len++;}
		  else if ( *sapp<0){i-=*sapp;len-=*sapp;}
		  else if ( *sapp>0){j+=*sapp;len+=*sapp;}		  
		  sapp++;
	      }
	


	A=realloc_aln2  ( A,A->max_n_seq,len+1);
	char_buf=declare_char (A->max_n_seq,len+1);
	
	i=0;j=0;sapp=S; len=0;
	while (!(i==l1 && j==l2))
	      { 

		   if (*sapp==0)
		      {
			  for (b=0; b< ns[0]; b++)
			      char_buf[ls[0][b]][len]=A->seq_al[ls[0][b]][i];
			  for (b=0; b< ns[1]; b++)
			      char_buf[ls[1][b]][len]=A->seq_al[ls[1][b]][j];
			  i++; j++;len++;
		      }
		  else if ( *sapp>0)
		        {
			    l=*sapp;
			    for ( a=0; a<l; a++, j++, len++)
			        {
				for (b=0; b< ns[0]; b++)
				    char_buf[ls[0][b]][len]='-';
				for (b=0; b< ns[1]; b++)
				    char_buf[ls[1][b]][len]=A->seq_al[ls[1][b]][j];
				}
			}
		  else if ( *sapp<0)
		        {
			    l=-*sapp;
			    for ( a=0; a<l; a++, i++, len++)
			        {
				for (b=0; b< ns[0]; b++)
				    char_buf[ls[0][b]][len]=A->seq_al[ls[0][b]][i];;
				for (b=0; b< ns[1]; b++)
				    char_buf[ls[1][b]][len]='-';
				}		        
			}
		   
		  sapp++;
	      }
	
	
	A->len_aln=len;
	A->nseq=ns[0]+ns[1];
	
	for ( a=0; a< ns[0]; a++){char_buf[ls[0][a]][len]='\0'; sprintf ( A->seq_al[ls[0][a]], "%s", char_buf[ls[0][a]]);}
	for ( a=0; a< ns[1]; a++){char_buf[ls[1][a]][len]='\0'; sprintf ( A->seq_al[ls[1][a]], "%s", char_buf[ls[1][a]]);}

	
	vfree (S);
	free_char ( char_buf, -1);
	l1=strlen (A->seq_al[ls[0][0]]);
	l2=strlen (A->seq_al[ls[1][0]]);
	if ( l1!=l2) exit(1);
		
	free_int (pos, -1);
	return score;
	}


int diff (Alignment *A, int *ns, int **ls, int s1, int M,int s2, int N , int tb, int te, Constraint_list *CL, int **pos)
        {
	 static int *CC;
	 static int *DD;
	     /* Forward cost-only vectors */
	 static int *RR;
	 static int *SS;
             /* Reverse cost-only vectors */
         int   midi, midj, type;    /* Midpoint, type, and cost */
         int  midc;

/*TREATMENT OF THE TERMINAL GAP PENALTIES*/
/*TG_MODE=0---> gop and gep*/
/*TG_MODE=1---> ---     gep*/
/*TG_MODE=2---> ---     ---*/







	 if ( !CC)
	    {
	      int L;
	      L=M+N+1;

	      CC=(int*)vcalloc (L, sizeof (int));
	      DD=(int*)vcalloc (L, sizeof (int));
	      RR=(int*)vcalloc (L, sizeof (int));
	      SS=(int*)vcalloc (L, sizeof (int));
	    }

	 if ( A==NULL)
	    {
	      vfree(CC);
	      vfree(DD);
	      vfree(RR);
	      vfree(SS);
	      CC=DD=RR=SS=NULL;
	      return 0;
	    }
	 
	 {
	 int   i, j;
	 int   c, e, d, s,ma;
         int t, g,h,m;
	 
	 

	 g=CL->gop*SCORE_K;
	 h=CL->gep*SCORE_K;
	 m=g+h;
	
	 if (N <= 0){if (M > 0) DEL(M);return gap(M);}
	 if (M <= 1)
	       { 

	       if (M <= 0)
	          {INS(N);
		  return gap(N);
		  }

	       
	       if (tb > te) tb = te;
	       midc = (tb+h) + gap(N);
	       midj = 0;
    
	       for (j = 1; j <= N; j++)
	           { 
		     
		     c = gap(j-1) +(CL->get_dp_cost) (A, pos, sns[0], sls[0],s1, pos, sns[1], sls[1],j-1+s2,CL)+ gap(N-j);
		     
		     if (c > midc)
		       { midc = c;
			 midj = j;
		       }
		   }
	       if (midj == 0)
	          {DEL(1) INS(N)}
	       else
	          {if (midj > 1) INS(midj-1);
		  REP;
		  if (midj < N) INS(N-midj);
		  }
	       
	       return midc;
	       }
/* Divide: Find optimum midpoint (midi,midj) of cost midc */
 

	 midi = M/2;            /* Forward phase:                          */
	 CC[0] = 0;            /*   Compute C(M/2,k) & D(M/2,k) for all k */
	 t = tb;  
	 for (j = 1; j <= N; j++)
	     { CC[j] = t = t+h;
	       DD[j] = t+g;
	     }
	 t = tb;
	 for (i = 1; i <= midi; i++)
	     { 
	     s = CC[0];
	     CC[0] = c = t = t+h;
	     e = t+g;

	     for (j = 1; j <= N; j++)
	         {

		   
		 if ((c =   c   + m) > (e =   e   + h)) e = c;
		 if ((c = CC[j] + m) > (d = DD[j] + h)) d = c;
		 
		 ma=c = s + (CL->get_dp_cost) (A, pos, sns[0], sls[0],i-1+s1, pos, sns[1], sls[1],j-1+s2,CL);
		 
		 if (e > c) c = e;
		 if (d > c) c = d;
		 
		     
		 s = CC[j];
		 CC[j] = c;
		 DD[j] = d;
		 }
	     }
	 DD[0] = CC[0];
	 
	 RR[N] = 0;            /* Reverse phase:                          */
	 t = te;
	 
	 
	 for (j = N-1; j >= 0; j--)
	     { RR[j] = t = t+h;
	     SS[j] = t+g;
	     }
	 t = te;
	 for (i = M-1; i >= midi; i--)
	     { s = RR[N];
	     RR[N] = c = t = t+h;
	     e = t+g;
	     for (j = N-1; j >= 0; j--)
	          { 
		  if ((c =   c   + m) > (e =   e   + h)) e = c;
		  if ((c = RR[j] + m) > (d = SS[j] + h)) d = c;
		  
		  ma=c = s + (CL->get_dp_cost) (A, pos, sns[0], sls[0],i+s1, pos, sns[1], sls[1],j+s2,CL);
		  
		  if (e > c) c = e;
		  if (d > c) c = d;
		 
	
		  s = RR[j];
		  RR[j] = c;
		  SS[j] = d;
		
		  }
	     }
	 SS[N] = RR[N];	 
	 midc = CC[0]+RR[0];        /* Find optimal midpoint */
	 midj = 0;
	 type = 1;
	 for (j = 0; j <= N; j++)
	     if ((c = CC[j] + RR[j]) >= midc)
	       if (c > midc || (CC[j] != DD[j] && RR[j] == SS[j]))
		    { 
		    midc = c;
		    midj = j;
		    }
	 for (j = N; j >= 0; j--)
	     if ((c = DD[j] + SS[j] - g) > midc)
	         {midc = c;
		 midj = j;
		 type = 2;
		 }
	 }	    
/* Conquer: recursively around midpoint */

  if (type == 1)
    { 
	
    diff (A,ns, ls, s1,midi, s2, midj, tb, CL->gop*SCORE_K, CL, pos); 
    diff (A,ns, ls, s1+midi,M-midi, s2+midj, N-midj, CL->gop*SCORE_K,te, CL, pos); 
    }
  else
    { 
      diff (A,ns, ls, s1,midi-1, s2, midj, tb,0, CL, pos); 
      DEL(2);
      diff (A,ns, ls, s1+midi+1, M-midi-1,s2+midj, N-midj,0,te, CL, pos); 
    }
  return midc;
  }       















