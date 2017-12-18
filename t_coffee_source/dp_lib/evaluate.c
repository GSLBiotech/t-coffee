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
#include <ctype.h>
#include "io_lib_header.h"
#include "util_lib_header.h"
#include "define_header.h"
#include "dp_lib_header.h"
               
float compute_lambda (int **matrix,char *alphabet);
 
/**
 * \file evaluate.c
 * Functions to evaluate consistency between CL and Alignment
 *
 * \note This comment was already in the code, so I leave it here:
 *    - Fast:         score= extended_res/max_extended_residue_for the whole aln
 *    - slow:         score= extended_res/sum all extended score for that residue
 *    - non_extended  score= non_ext     /sum all non extended  score for that residue
 *    - heuristic     score= extended    /sum of extended score of all pairs in the library
 *                                    (i.e. Not ALL the possible pairs)
 */
Alignment * main_coffee_evaluate_output2 ( Alignment *IN,Constraint_list *CL, const char *mode );

int sub_aln2ecl_raw_score (Alignment *A, Constraint_list *CL, int ns, int *ls)
{
  int **pos;
  int p1,r1, r2, s1, s2;
  int score=0;

  if ( !A) return 0;
  A=reorder_aln (A, (CL->S)->name,(CL->S)->nseq);
  pos=aln2pos_simple ( A,A->nseq);
  
  
  
  for (p1=0; p1<A->len_aln; p1++)
    {
      for (s1=0; s1<ns-1; s1++)
	{
	  for (s2=s1+1; s2<ns; s2++)
	    {
	      
	      r1=pos[ls[s1]][p1];
	      r2=pos[ls[s2]][p1];
	      if (r1>0 && r2>0)
		{
		  score+= residue_pair_extended_list_pc (CL,ls[s1], r1, ls[s2], r2)*SCORE_K;
		}
	    }
	}
    }
  free_int (pos, -1);
  return score;
}
int aln2ecl_raw_score (Alignment *A, Constraint_list *CL)
{
  int **pos;
  int p1,r1, r2, s1, s2;
  int score=0;
  if ( !A) return 0;
  A=reorder_aln (A, (CL->S)->name,(CL->S)->nseq);
  pos=aln2pos_simple ( A,A->nseq);
  

  
  for (p1=0; p1<A->len_aln; p1++)
    {
      for (s1=0; s1<A->nseq-1; s1++)
	{
	  for (s2=s1+1; s2<A->nseq; s2++)
	    {
	      
	      r1=pos[s1][p1];
	      r2=pos[s2][p1];
	      if (r1>0 && r2>0)score+= residue_pair_extended_list_pc (CL,s1, r1, s2, r2);
	    }
	}
    }
  free_int (pos, -1);
  return score;
}
int node2sub_aln_score    (Alignment *A,Constraint_list *CL, char *mode, NT_node T)
{
  if ( !T || !T->right ||!T->left)return 0;
  else 
    {
      int *ns;
      int **ls;
      ns=(int*)vcalloc (2, sizeof (int));
      ls=(int**)vcalloc (2, sizeof (int*));
      ns[0]= (T->left)->nseq;
      ns[1]=(T->right)->nseq;
      ls[0]= (T->left)->lseq;
      ls[1]=(T->right)->lseq;
      
      return sub_aln2sub_aln_score (A, CL, mode, ns, ls);
    }
}
int sub_aln2sub_aln_score ( Alignment *A,Constraint_list *CL, const char *mode, int *ns, int **ls)
{
  /*Warning: Does Not Take Gaps into account*/

  int **pos;
  int a;
  float score=0;
  
  /*Make sure evaluation functions update their cache if needed*/
  A=update_aln_random_tag (A);
  pos=aln2pos_simple ( A, -1, ns, ls);
  for (a=0; a< A->len_aln; a++)
    score+=CL->get_dp_cost (A, pos, ns[0], ls[0], a, pos, ns[1],ls[1], a, CL);
  free_int (pos, -1);
   
  score=(int)(((float)score)/(A->len_aln*SCORE_K));
  score=(int)(CL->residue_index && CL->normalise)?((score*MAXID)/(CL->normalise)):(score);
  return (int)score;
}
int sub_aln2sub_aln_raw_score ( Alignment *A,Constraint_list *CL, const char *mode, int *ns, int **ls)
{
  /*Warning: Does Not Take Gaps into account*/

  int **pos;
  int a;
  float score=0;
  
  /*Make sure evaluation functions update their cache if needed*/
  A=update_aln_random_tag (A);
  pos=aln2pos_simple ( A, -1, ns, ls);
  for (a=0; a< A->len_aln; a++)
    score+=CL->get_dp_cost (A, pos, ns[0], ls[0], a, pos, ns[1],ls[1], a, CL);
  free_int (pos, -1);
  return (int) score;
}

Alignment* main_coffee_evaluate_output_sub_aln ( Alignment *A,Constraint_list *CL, const char *mode, int *n_s, int **l_s)
{
  Alignment *SUB1, *SUB2, *SUB3;
  int a, b, c,*list_seq;
  
  
  if (strm ( CL->evaluate_mode, "no"))return NULL;
  else 
    {
        list_seq=(int*)vcalloc (n_s[0]+n_s[1], sizeof (int));
	for (b=0, a=0; a< 2; a++){for (c=0;c< n_s[a]; c++)list_seq[b++]=l_s[a][c];}
	

	SUB1=copy_aln (A, NULL);	  
	SUB2=extract_sub_aln (SUB1,n_s[0]+n_s[1],list_seq);
	SUB3=main_coffee_evaluate_output (SUB2,CL,CL->evaluate_mode);
	free_aln (SUB1);
	free_aln (SUB2);
	vfree (list_seq);
	
	return SUB3;
    }
}
Alignment * overlay_alignment_evaluation     ( Alignment *I, Alignment *O)
{
  int a, b, r;
  int *buf;
  
  if ( !I || !O) return O;
  if ( I->len_aln!=O->len_aln)printf_exit (EXIT_FAILURE, stderr, "ERROR: Incompatible alignments in overlay_alignment_evaluation");
  
  buf=(int*)vcalloc ( MAX(I->len_aln, O->len_aln), sizeof (int));
 
  for (a=0; a<O->nseq; a++)
    {
      if (!strm (I->name[a], O->name[a]))printf_exit (EXIT_FAILURE, stderr, "ERROR: Incompatible alignments in overlay_alignment_evaluation");
      for (b=0; b<O->len_aln; b++)
	{
	  r=I->seq_al[a][b];
	  if ( islower(r))O->seq_al[a][b]=0;
	  else if (r<=9 || (r>='0' && r<='9'))O->seq_al[a][b]=I->seq_al[a][b];
	}
    }
  return O;
}

Alignment * main_coffee_evaluate_output ( Alignment *IN,Constraint_list *CL, const char *mode )
{
  
  Alignment *TopS=NULL, *LastS=NULL, *CurrentS=NULL;
  

  if ( IN->A){IN=IN->A;}
  while (IN)
    {

      CurrentS= main_coffee_evaluate_output2(IN, CL, mode);
      if (!TopS)LastS=TopS=CurrentS;
      else
	{
	  LastS->A=CurrentS;
	  LastS=CurrentS;
	}
      IN=IN->A;
    }
  return TopS;
}

Alignment * main_coffee_evaluate_output2 ( Alignment *IN,Constraint_list *CL, const char *mode )
{
  
  /*Make sure evaluation functions update their cache if needed*/
 
  IN=update_aln_random_tag (IN);
  
  if ( CL->evaluate_residue_pair==evaluate_matrix_score || CL->ne==0 ||strm ( mode , "categories") || strm ( mode , "matrix")|| strm(mode, "sar")|| strstr (mode, "boxshade") )
     {
      
       if ( strm ( mode , "categories")) return categories_evaluate_output (IN, CL);
       else if ( strm ( mode , "matrix"))return matrix_evaluate_output (IN, CL);
       else if ( strm ( mode, "sar"))return sar_evaluate_output (IN, CL);
       else if ( strstr ( mode, "boxshade"))return boxshade_evaluate_output (IN, CL, atoi (strstr(mode, "_")+1));
       
       else if ( CL->evaluate_residue_pair==evaluate_matrix_score) return matrix_evaluate_output (IN, CL);
       else if ( CL->ne==0) return matrix_evaluate_output (IN, CL);
     }
   else if ( strm (mode, "no"))return NULL;
   else if ( strstr( mode, "triplet") || strstr (mode, "tcs"))
     {
       return triplet_coffee_evaluate_output ( IN,CL);
     }
   else if ( strstr ( mode, "fast"))
     {
      
       return fast_coffee_evaluate_output ( IN,CL);
     }
   else if ( strstr ( mode, "slow"))
     {
       return slow_coffee_evaluate_output ( IN,CL);
     }
   else if ( strstr (mode, "rna"))
     {
       Alignment *OUT;
       int a, b,res;

       OUT=struc_evaluate4tcoffee (IN, CL, "strike",0,3,NULL);
       for (a=0; a<OUT->nseq; a++)
	 for (b=0; b<OUT->len_aln; b++)
	   {
	     res=OUT->seq_al[a][b];
	     if (res>9 && !(res>='0' && res<='9') )OUT->seq_al[a][b]=NO_COLOR_RESIDUE;
	   }
       sprintf (OUT->name[OUT->nseq], "Cons");
       return OUT;
     }
   else if ( strstr ( mode, "non_extended"))
     {
       return non_extended_t_coffee_evaluate_output ( IN,CL);
     }
   
   else if ( strm (mode, "sequences"))
     {
       return coffee_seq_evaluate_output ( IN,CL);
     }
   else 
     {
       fprintf ( stderr, "\nUNKNOWN MODE FOR ALIGNMENT EVALUATION: *%s* [FATAL:%s]",mode, PROGRAM);
       crash ("");
       return NULL;
     }
  return IN;
}



Alignment * coffee_evaluate_output ( Alignment *IN,Constraint_list *CL)
    {
    fprintf ( stderr, "\n[WARNING:%s]THE FUNCTION coffee_evaluate_output IS NOT ANYMORE SUPPORTED\n", PROGRAM);
    fprintf ( stderr, "\n[WARNING]fast_coffee_evaluate_output WILL BE USED INSTEAD\n");
    
    return fast_coffee_evaluate_output (IN,CL);
    }
Alignment * matrix_evaluate_output ( Alignment *IN,Constraint_list *CL)
    {
    int a,b, c,r, s, r1, r2;	
    Alignment *OUT=NULL;
  
    double **tot_res;
    double **max_res;
    
    double **tot_seq;
    double **max_seq;
    
    double **tot_col;
    double **max_col;

    double max_aln=0;
    double tot_aln=0;
    

    /*
      Residue x: sum of observed extended X.. /sum of possible X..
    */

    
    if ( !CL->M)CL->M=read_matrice ("blosum62mt");
    
    OUT=copy_aln (IN, OUT);
    
    
    tot_res=declare_double ( IN->nseq, IN->len_aln);
    max_res=declare_double ( IN->nseq, IN->len_aln);
    
    tot_seq=declare_double ( IN->nseq, 1);
    max_seq=declare_double ( IN->nseq, 1);
    tot_col=declare_double ( IN->len_aln,1);
    max_col=declare_double ( IN->len_aln,1);
    
    max_aln=tot_aln=0;
    
    for (a=0; a< IN->len_aln; a++)
        {
	  double r, s;
	  for ( b=0; b< IN->nseq; b++)
	    {
	      r1=tolower(IN->seq_al[b][a]);
	      if ( is_gap(r1))continue;
	      r= CL->M[r1-'A'][r1-'A'];
	      r=1;
	      for ( c=0; c<IN->nseq; c++)
		{
		  r2=tolower(IN->seq_al[c][a]);
		  if (b==c || is_gap (r2))continue;
		  
		  s=(double)CL->M[r2-'A'][r1-'A'];
		  s=(s>0)?1:0;
		  
		  tot_res[b][a]+=s;
		  max_res[b][a]+=r;
		  
		  tot_col[a][0]+=s;
		  max_col[a][0]+=r;
		  
		  tot_seq[b][0]+=s;
		  max_seq[b][0]+=r;
		  
		  tot_aln+=s;
		  max_aln+=r;
		}
	    }
	}
    
    
    for ( a=0; a< IN->nseq; a++)
      {
	if ( !max_seq[a][0])continue;
       
	OUT->score_seq[a]=(tot_seq[a][0]*100)/max_seq[a][0];
	for (b=0; b< IN->len_aln; b++)
	  {
	    r1=IN->seq_al[a][b];
	    if (is_gap (OUT->seq_al[a][b]));
	    else if (!max_res[a][b])(OUT)->seq_al[a][b]=NO_COLOR_RESIDUE;
	    else
	      {
		r1=(tot_res[a][b]*10)/max_res[a][b];
		r1=(r1==10)?9:r1;
		(OUT)->seq_al[a][b]=((r1==10)?9:r1)+'0';
	      }
	  }
      }
    sprintf (OUT->name[OUT->nseq], "cons");
    for ( a=0; a< IN->len_aln; a++)
      {
	
	r1=((int)(max_col[a][0]==0)?0:(tot_col[a][0]*(double)10)/max_col[a][0]);
	(OUT)->seq_al[IN->nseq][a]=((r1>=10)?9:r1)+'0';
      }

    if (max_aln)
      {
	OUT->score_seq[OUT->nseq]=OUT->score_aln=(100*tot_aln)/max_aln;
	OUT->score=OUT->score_aln=(1000*tot_aln)/max_aln;
      }
    
	
   
    free_double (tot_res,-1);
    free_double (max_res,-1);
    
    free_double (tot_seq,-1);
    free_double (max_seq,-1);
    
    return OUT;
    }

Alignment * sar_evaluate_output ( Alignment *IN,Constraint_list *CL)
    {
      int a,b, c,r, s, r1, r2;
      Alignment *OUT=NULL;
      
      double **tot_res;
      double **max_res;
      
      double **tot_seq;
      double **max_seq;
      
      double **tot_col;
      double **max_col;
      
      double max_aln=0;
      double tot_aln=0;
      
      
    /*
      Residue x: sum of observed extended X.. /sum of possible X..
    */

    
    if ( !CL->M)CL->M=read_matrice ("blosum62mt");
    
    OUT=copy_aln (IN, OUT);
    
    
    tot_res=declare_double ( IN->nseq, IN->len_aln);
    max_res=declare_double ( IN->nseq, IN->len_aln);
    
    tot_seq=declare_double ( IN->nseq, 1);
    max_seq=declare_double ( IN->nseq, 1);
    tot_col=declare_double ( IN->len_aln,1);
    max_col=declare_double ( IN->len_aln,1);
    
    max_aln=tot_aln=0;
    
    for (a=0; a< IN->len_aln; a++)
        {
	  for (b=0; b< IN->nseq; b++)
		{
		  r1=tolower(IN->seq_al[b][a]);
		  for (c=0; c<IN->nseq; c++)
		    {
		      r2=tolower(IN->seq_al[c][a]);
		      if (b==c)continue;
		      
		      if ( is_gap(r1) && is_gap(r2))s=0;
		      else s=(r1==r2)?1:0;
		      
		      r=1;
		      

		      tot_res[b][a]+=s;
		      max_res[b][a]+=r;
		      
		      tot_col[a][0]+=s;
		      max_col[a][0]+=r;
		      
		      tot_seq[b][0]+=s;
		      max_seq[b][0]+=r;
		      
		      tot_aln+=s;
		      max_aln+=r;
		    }
		}
	}
     
    for ( a=0; a< IN->nseq; a++)
      {
	if ( !max_seq[a][0])continue;
	OUT->score_seq[a]=(max_seq[a][0]*100)/max_seq[a][0];
	for (b=0; b< IN->len_aln; b++)
	  {
	    r1=IN->seq_al[a][b];
	    if ( is_gap(r1) || !max_res[a][b])continue;
	    r1=(tot_res[a][b]*10)/max_res[a][b];
	    r1=(r1>=10)?9:r1;
	    r1=r1<0?0:r1;
	    (OUT)->seq_al[a][b]=r1+'0';
	  }
      }
    
    for ( a=0; a< IN->len_aln; a++)
      {
	r1=(max_col[a][0]==0)?0:((tot_col[a][0]*10)/max_col[a][0]);
	r1=(r1>=10)?9:r1;
	(OUT)->seq_al[OUT->nseq][a]=r1+'0';
      }
    sprintf ( OUT->name[IN->nseq], "cons");
    if (max_aln)OUT->score_aln=(100*tot_aln)/max_aln;

   
    free_double (tot_res,-1);
    free_double (max_res,-1);
    
    free_double (tot_seq,-1);
    free_double (max_seq,-1);
    
    return OUT;
    }
Alignment * boxshade_evaluate_output ( Alignment *IN,Constraint_list *CL, int T)
    {
      Alignment *OUT=NULL;
      int **aa;
      int r,br, bs, a, b;
      float f;
     
      
    /*
      Residue x: sum of observed extended X.. /sum of possible X..
    */

        
    OUT=copy_aln (IN, OUT);
    aa=declare_int (26, 2);
        
    for ( a=0; a< OUT->len_aln; a++)
      {
	for ( b=0; b< 26; b++){aa[b][1]=0;aa[b][0]='a'+b;}
	for ( b=0; b< OUT->nseq; b++)
	  {
	    r=tolower(OUT->seq_al[b][a]);
	    if ( !is_gap(r))aa[r-'a'][1]++;
	  }
	sort_int ( aa, 2, 1, 0,25);
	f=(aa[25][1]*100)/OUT->nseq;
	
	if (f<T);
	else
	  {
	    bs=((f-T)*10)/(100-T);
	    br=aa[25][0];
	
	    if (bs==10)bs--;bs+='0';
	    for ( b=0; b< OUT->nseq; b++)
	      {
		r=tolower(OUT->seq_al[b][a]);
		if (r==br && bs>'1')OUT->seq_al[b][a]=bs;
	      }	
	    OUT->seq_al[b][a]=bs;
	  }
      }
    sprintf ( OUT->name[IN->nseq], "cons");
    
    return OUT;
    }

Alignment * categories_evaluate_output ( Alignment *IN,Constraint_list *CL)
    {
    
    Alignment *OUT=NULL;
    int a, b, r;
    int *aa;
    float score, nseq2, tot_aln;
    float n;
    /*
      Residue x: sum of observed extended X.. /sum of possible X..
    */
    OUT=copy_aln (IN, OUT);
    aa=(int*)vcalloc ( 26, sizeof (int));
    nseq2=IN->nseq*IN->nseq;
    
    for (tot_aln=0, a=0; a< IN->len_aln; a++)
      {
	for (n=0,b=0; b< IN->nseq; b++)
	  {
	    r=IN->seq_al[b][a];
	    
	    if ( is_gap(r))n++;
	    else 
	      {
		aa[tolower(r)-'a']++;
		n++;
	      }
	  }
	n=n*n;
	for ( score=0,b=0; b< 26; b++){score+=aa[b]*aa[b];aa[b]=0;}
	/*score/=nseq2;*/
	score=(n==0)?0:score/n;
	tot_aln+=score;
	r=score*10;
	r=(r>=10)?9:r;
	(OUT)->seq_al[OUT->nseq][a]='0'+r;
      }
    OUT->score_aln=(tot_aln/OUT->len_aln)*100;
    sprintf ( OUT->name[IN->nseq], "cons");
    vfree(aa);
    return OUT;
    }

Alignment * categories_evaluate_output_old ( Alignment *IN,Constraint_list *CL)
    {
    
    Alignment *OUT=NULL;
    int nc,a, b, r;
    int *aa, ng;
    float score, nseq2, tot_aln, min=0;
    
    /*
      Residue x: sum of observed extended X.. /sum of possible X..
    */
    OUT=copy_aln (IN, OUT);
    aa=(int*)vcalloc ( 26, sizeof (int));
    nseq2=IN->nseq*IN->nseq;
    
    for (tot_aln=0, a=0; a< IN->len_aln; a++)
      {
	for (ng=0,b=0; b< IN->nseq; b++)
	  {
	    r=IN->seq_al[b][a];
	    
	    if ( is_gap(r))ng++;
	    else 
	      {
		aa[tolower(r)-'a']++;
	      }
	  }
	for (nc=0, b=0; b<26; b++)
	  {
	    if ( aa[b])nc++;
	    aa[b]=0;
	  }
	if (nc>9)score=0;
	else score=9-nc;
	
	score=(2*min)/IN->nseq;
	
	tot_aln+=score;
	r=score*10;
	r=(r>=10)?9:r;
	(OUT)->seq_al[OUT->nseq][a]='0'+r;
      }

    OUT->score_aln=(tot_aln/OUT->len_aln)*100;
    sprintf ( OUT->name[IN->nseq], "cons");
    vfree(aa);
    return OUT;
    }





Alignment * fork_triplet_coffee_evaluate_output ( Alignment *IN,Constraint_list *CL,int nproc);
Alignment * nfork_triplet_coffee_evaluate_output ( Alignment *IN,Constraint_list *CL);
Alignment * triplet_coffee_evaluate_output ( Alignment *IN,Constraint_list *CL)  
{
  
  if (!IN || !CL || !CL->residue_index) return IN;
  
  if (get_nproc()==1)return  nfork_triplet_coffee_evaluate_output (IN,CL);
  else if (strstr ( CL->multi_thread, "evaluate"))return  fork_triplet_coffee_evaluate_output (IN,CL,get_nproc());
  else return fork_triplet_coffee_evaluate_output (IN,CL,1);
}
Alignment * fork_triplet_coffee_evaluate_output ( Alignment *IN,Constraint_list *CL,int nproc)
    {
      Alignment *OUT=NULL;
      int **pos;
      
      double score_col=0, score_aln=0, score_res=0, score=0;
      double max_col=0, max_aln=0, max_res=0;
      double *max_seq, *score_seq;
      
      int a,b, x, y,res;
      int s1,r1,s2,r2,w2,s3,r3,w3;
      int **lu;
      
      //multi-threading
      FILE *fp;
      char **pid_tmpfile;
      int sjobs, njobs;
      int **sl;
      int j;
      
      OUT=copy_aln (IN, OUT);
      pos=aln2pos_simple(IN, IN->nseq);
      sprintf ( OUT->name[IN->nseq], "cons");
      
      max_seq=(double*)vcalloc ( IN->nseq+1, sizeof (double));
      score_seq=(double*)vcalloc ( IN->nseq+1, sizeof (double));
      lu=declare_int (IN->nseq, IN->len_aln+1);
      
      //multi Threading stuff
      njobs=nproc;
      sl=n2splits (njobs,IN->len_aln);
      pid_tmpfile=(char**)vcalloc (njobs, sizeof (char*));
      
      for (sjobs=0,j=0; sjobs<njobs; j++)
	{
	  pid_tmpfile[j]=vtmpnam(NULL);
	  if (vvfork (NULL)==0)
	    {
	      initiate_vtmpnam(NULL);
	      fp=vfopen (pid_tmpfile[j], "w");
	      score_aln=max_aln=0;
	      for (a=sl[j][0]; a<sl[j][1]; a++)
		{
		  if (j==0)output_completion (CL->local_stderr,a,sl[0][1],1, "Final Evaluation");
		  score_col=max_col=0;
		  for (b=0; b<IN->nseq; b++)
		    {
		      s1=IN->order[b][0];
		      r1=pos[b][a];
		      if (r1>=0)lu[s1][r1]=1;
		    }
		  for (b=0; b<IN->nseq; b++)
		    {
		      score_res=max_res=NORM_F;
		      res=NO_COLOR_RESIDUE;
		      s1=IN->order[b][0];
		      r1=pos[b][a];
		      if (r1<=0)
			{
			  fprintf (fp, "%d ", res);
			  continue;
			}
		      
		      for (x=1;x<CL->residue_index[s1][r1][0];x+=ICHUNK)
			{
			  s2=CL->residue_index[s1][r1][x+SEQ2];
			  r2=CL->residue_index[s1][r1][x+R2];
			  w2=CL->residue_index[s1][r1][x+WE];
			  for (y=1; y<CL->residue_index[s2][r2][0];y+=ICHUNK)
			    {
			      s3=CL->residue_index[s2][r2][y+SEQ2];
			      r3=CL->residue_index[s2][r2][y+R2];
			      w3=CL->residue_index[s2][r2][y+WE];
			      
			      score=MIN(w2,w3);
			      
			      max_res+=score;
			      max_col+=score;
			      max_seq[b]+=score;
			      max_aln+=score;
			      
			      if (lu[s3][r3])
				{
				  score_res+=score;
				  score_col+=score;
				  score_seq[b]+=score;
				  score_aln+=score;
				}
			    }
			}
		      res=(max_res==0)?NO_COLOR_RESIDUE:((score_res*10)/max_res);
		      //res=(res==NO_COLOR_RESIDUE)?res:(MIN(res,9));
		      res=MIN(res,9);
		      fprintf ( fp, "%d ", res);
		    }
		  for (b=0; b<IN->nseq; b++)
		    {
		      s1=IN->order[b][0];
		      r1=pos[b][a];
		      if (r1>0)lu[s1][r1]=0;
		    }
		  
		  res=(max_col==0)?NO_COLOR_RESIDUE:((score_col*10)/max_col);	
		  //res=(res==NO_COLOR_RESIDUE)?res:(MIN(res,9));
		  res=MIN(res,9);
		  
		  fprintf (fp, "%d ", res);
		  
		  for (b=0; b<IN->nseq; b++)
		    fprintf (fp, "%f %f ", score_seq[b], max_seq[b]);
		}
	      fprintf (fp, "%f %f ", score_aln, max_aln);
	      vfclose (fp);
	      myexit (EXIT_SUCCESS);
	    }
	  else
	    {
	      sjobs++;
	    }
	}
    
      while (sjobs>=0){vwait(NULL); sjobs--;}//wait for all jobs to complete
      
      for (j=0; j<njobs; j++)
	{
	  float sseq, mseq;
	  fp=vfopen (pid_tmpfile[j], "r");
	  for (a=sl[j][0];a<sl[j][1]; a++)
	    {
	      for (b=0; b<=IN->nseq; b++)//don't forget the consensus
		{

		  fscanf (fp, "%d ", &res);
		  OUT->seq_al[b][a]=res;
		}
	      for (b=0; b<IN->nseq; b++)
		{
		  fscanf (fp, "%f %f", &sseq, &mseq);
		  score_seq[b]+=sseq;
		  max_seq[b]+=mseq;
		}
	    }
	  fscanf (fp, "%f %f", &sseq, &mseq);
	  score_aln+=sseq;
	  max_aln+=mseq;
	  vfclose (fp);
	  remove (pid_tmpfile[j]);
	}
      fprintf ( stderr, "\n");
      
      IN->score_aln=OUT->score_aln=(max_aln==0)?0:((score_aln*1000)/max_aln);
      for ( a=0; a<= OUT->nseq; a++)
	{
	  OUT->score_seq[a]=(max_seq[a]==0)?0:((score_seq[a]*100)/max_seq[a]);
	}
      
      free_int (lu,-1);
      free_int (pos , -1);
      vfree (pid_tmpfile);
      free_int (sl, -1);
      vfree ( score_seq);
      vfree ( max_seq);
      return OUT;
    }  

Alignment * nfork_triplet_coffee_evaluate_output ( Alignment *IN,Constraint_list *CL)
    {
      Alignment *OUT=NULL;
      int **pos;
      
      double score_col=0, score_aln=0, score_res=0, score=0;
      double max_col=0, max_aln=0, max_res=0;
      double *max_seq, *score_seq;
      
      int a,b, x, y,res;
      int s1,r1,s2,r2,w2,s3,r3,w3;
      int **lu;
      

      
      OUT=copy_aln (IN, OUT);
      pos=aln2pos_simple(IN, IN->nseq);
      sprintf ( OUT->name[IN->nseq], "cons");
      
      max_seq=(double*)vcalloc ( IN->nseq, sizeof (double));
      score_seq=(double*)vcalloc ( IN->nseq, sizeof (double));
      lu=declare_int (IN->nseq, IN->len_aln+1);
      
     
          
      score_aln=max_aln=0;
      for (a=0; a<IN->len_aln; a++)
	{
	  output_completion (stderr,a,IN->len_aln,1, "Final Evaluation");
	  score_col=max_col=0;
	  for (b=0; b<IN->nseq; b++)
	    {
	      s1=IN->order[b][0];
	      r1=pos[b][a];
	      if (r1>=0)lu[s1][r1]=1;
	    }
	  
	  for (b=0; b<IN->nseq; b++)
	    {
	      score_res=max_res=NORM_F;
	      OUT->seq_al[b][a]=NO_COLOR_RESIDUE;
	      s1=IN->order[b][0];
	      r1=pos[b][a];
	      if (r1<=0)continue;
	      for (x=1;x<CL->residue_index[s1][r1][0];x+=ICHUNK)
		{
		  s2=CL->residue_index[s1][r1][x+SEQ2];
		  r2=CL->residue_index[s1][r1][x+R2];
		  w2=CL->residue_index[s1][r1][x+WE];
		  for (y=1; y<CL->residue_index[s2][r2][0];y+=ICHUNK)
		    {
		      s3=CL->residue_index[s2][r2][y+SEQ2];
		      r3=CL->residue_index[s2][r2][y+R2];
		      w3=CL->residue_index[s2][r2][y+WE];
		      
		      score=MIN(w2,w3);
		      
		      max_res+=score;
		      max_col+=score;
		      max_seq[b]+=score;
		      max_aln+=score;
		      
		      if (lu[s3][r3])
			{
			  score_res+=score;
			  score_col+=score;
			  score_seq[b]+=score;
			  score_aln+=score;
			}
		    }
		}
	      res=(max_res==0)?NO_COLOR_RESIDUE:((score_res*10)/max_res);
	      //res=(res==NO_COLOR_RESIDUE)?res:(MIN(res,9));
	      res=MIN(res,9);
	      OUT->seq_al[b][a]=res;
	    }
	  for (b=0; b<IN->nseq; b++)
	    {
	      s1=IN->order[b][0];
	      r1=pos[b][a];
	      if (r1>0)lu[s1][r1]=0;
	    }
	  
	  res=(max_col==0)?NO_COLOR_RESIDUE:((score_col*10)/max_col);	
	  //res=(res==NO_COLOR_RESIDUE)?res:(MIN(res,9));
	  res=MIN(res,9);
	  OUT->seq_al[IN->nseq][a]=res;
	}
      fprintf ( stderr, "\n");
      
      IN->score_aln=OUT->score_aln=(max_aln==0)?0:((score_aln*100)/max_aln);
      for ( a=0; a< OUT->nseq; a++)
	{
	  OUT->score_seq[a]=(max_seq[a]==0)?0:((score_seq[a]*100)/max_seq[a]);
	}
      
      free_int (lu,-1);
      free_int (pos , -1);
      vfree ( score_seq);
      vfree ( max_seq);
      return OUT;
    }   



int  sp_triplet_coffee_evaluate_output ( Alignment *IN,Constraint_list *CL, char *fname)
    {
      int **pos;
      int s1, s2, s3, s4;
      int r1, r2, r3, r4;
      int a, b,c, x, y;
      float w3,w, w4, pw, tw;
      FILE *fp;
     
      fp=vfopen (fname, "w");
      pos=aln2pos_simple(IN, IN->nseq);
      
      fprintf ( fp, "! SP_ASCII_FORMAT_O1\n");
      fprintf ( fp, "! <column> <seq1> <seq2> <tcs score>\n");
      for (a=0; a<IN->len_aln; a++)
	{
	  output_completion (stderr,a,IN->len_aln,1, "Final SP Evaluation");
	  for (b=0; b<IN->nseq-1; b++)
	    {
	      s1=IN->order[b][0];
	      r1=pos[b][a];
	      if (r1<=0)continue;
	      
	      for (c=b+1; c<IN->nseq; c++)
		{
		  pw=tw=0;
		  s2=IN->order[c][0];
		  r2=pos[c][a];
		  if (r2<=0)continue;
		  
		  //compute r1->x
		  for ( x=1;x<CL->residue_index[s1][r1][0];x+=ICHUNK)
		    {
		      s3=CL->residue_index[s1][r1][x+SEQ2];
		      r3=CL->residue_index[s1][r1][x+R2];
		      w3=CL->residue_index[s1][r1][x+WE];
		      if (s3==s1 && r3==r1);
		      else if ( s3==s2 && r3==r2){pw+=w3, tw+=w3;}
		      else
			{
			  for (y=1; y<CL->residue_index[s3][r3][0];y+=ICHUNK)
			    {
			      s4=CL->residue_index[s3][r3][y+SEQ2];
			      r4=CL->residue_index[s3][r3][y+R2];
			      w4=CL->residue_index[s3][r3][y+WE];
			      w=MIN(w4,w3);
			      
			      if (s4==s1 && r4==r1);
			      else if ( s4==s2 && r4==r2){pw+=w; tw+=w;}
			      else if ( s4==s2)tw+=w;
			    }
			}
		    }
		  //compute r2->y 
		  for ( x=1;x<CL->residue_index[s2][r2][0];x+=ICHUNK)
		    {
		      s3=CL->residue_index[s2][r2][x+SEQ2];
		      r3=CL->residue_index[s2][r2][x+R2];
		      w3=CL->residue_index[s2][r2][x+WE];
		      if (s3==s2 && r3==r2)continue;
		      else if ( s3==s1 && r3==r1){pw+=w3;tw+=w3;}
		      else
			{
			  for (y=1; y<CL->residue_index[s3][r3][0];y+=ICHUNK)
			    {
			      s4=CL->residue_index[s3][r3][y+SEQ2];
			      r4=CL->residue_index[s3][r3][y+R2];
			      w4=CL->residue_index[s3][r3][y+WE];
			      w=MIN(w4,w3);
			      
			      if (s4==s2 && r4==r2);
			      else if ( s4==s1 && r4==r1){pw+=w; tw+=w;}
			      else if ( s4==s1)tw+=w;
			    }
			}  
		    }
		  fprintf ( fp, "%d %d %d %.3f\n", a+1, b+1, c+1, (tw>0)?pw/tw:0); 
		}
	    }
	}
      vfclose (fp);
      return 1;
    }  



int  sp_triplet_coffee_evaluate_output2 ( Alignment *IN,Constraint_list *CL, char *fname)
    {
      int **pos;
      int s1, s2, s3, s4;
      int r1, r2, r3, r4;
      float w1, w2, w3,w, w4, pw, tw;
      int a, b,c, x, y;
      FILE *fp;
     
      fp=vfopen (fname, "w");
      save_list_header( fp, CL);
      pos=aln2pos_simple(IN, IN->nseq);
      
	for (b=0; b<IN->nseq-1; b++)		//---> for every seq
	 {
	    for (c=b+1; c<IN->nseq; c++)
	     {
		fprintf ( fp, "#%d %d \n", b+1, c+1); 
		
		for (a=0; a<IN->len_aln; a++)	//---> for every aa in a seq
		 {
		  output_completion (stderr,a,IN->len_aln,1, "Final SP Evaluation");
		  
		  s1=IN->order[b][0];
		  r1=pos[b][a];
		  if (r1<=0)continue;
	      
		  pw=tw=0;
		  s2=IN->order[c][0];
		  r2=pos[c][a];
		  if (r2<=0)continue;
		  
		  //compute r1->x
		  for ( x=1;x<CL->residue_index[s1][r1][0];x+=ICHUNK)
		    {
		      s3=CL->residue_index[s1][r1][x+SEQ2];
		      r3=CL->residue_index[s1][r1][x+R2];
		      w3=CL->residue_index[s1][r1][x+WE];
		      if (s3==s1 && r3==r1);
		      else if ( s3==s2 && r3==r2){pw+=w3, tw+=w3;}
		      else
			{
			  for (y=1; y<CL->residue_index[s3][r3][0];y+=ICHUNK)
			    {
			      s4=CL->residue_index[s3][r3][y+SEQ2];
			      r4=CL->residue_index[s3][r3][y+R2];
			      w4=CL->residue_index[s3][r3][y+WE];
			      w=MIN(w4,w3);
			      
			      if (s4==s1 && r4==r1);
			      else if ( s4==s2 && r4==r2){pw+=w; tw+=w;}
			      else if ( s4==s2)tw+=w;
			    }
			}
		    }
		  //compute r2->y 
		  for ( x=1;x<CL->residue_index[s2][r2][0];x+=ICHUNK)
		    {
		      s3=CL->residue_index[s2][r2][x+SEQ2];
		      r3=CL->residue_index[s2][r2][x+R2];
		      w3=CL->residue_index[s2][r2][x+WE];
		      if (s3==s2 && r3==r2)continue;
		      else if ( s3==s1 && r3==r1){pw+=w3;tw+=w3;}
		      else
			{
			  for (y=1; y<CL->residue_index[s3][r3][0];y+=ICHUNK)
			    {
			      s4=CL->residue_index[s3][r3][y+SEQ2];
			      r4=CL->residue_index[s3][r3][y+R2];
			      w4=CL->residue_index[s3][r3][y+WE];
			      w=MIN(w4,w3);
			      
			      if (s4==s2 && r4==r2);
			      else if ( s4==s1 && r4==r1){pw+=w; tw+=w;}
			      else if ( s4==s1)tw+=w;
			    }
			}  
		    }
		  fprintf ( fp, "%5d %5d     %.0f\n", r1, r2, ((tw>0)?pw/tw:0)*1000 ); 
		}
	    }
	}
      vfclose (fp);
      return 1;
    }  

Alignment *struc_evaluate4tcoffee (Alignment *A, Constraint_list *CL, char *mode, float imaxD, int enb,char *in_matrix_name)
{
  double **max_pw_sc;
  double **tot_pw_sc;
  double **max_res_sc;
  double **tot_res_sc;
  double *max_seq_sc;
  double *tot_seq_sc;
  double *gscore;
 
  double *max_col_sc;
  double *tot_col_sc;
  
  double tot_sc=0;
  double max_sc=0;
  Alignment *OUT, *T;
  Sequence *S=NULL;
  int replicates;
  
  int **dm1,**dm2,**pos,i,s, **lu,*rseq, a, b, c,d,s1,s2,c1,ic1,c2,r1,r2,p1,p2,we, rep, nM, it;
  int **matrix;
  int *max;
  char *matrix_name;
  int strikeM, contactsM,distancesM, modeM;
  int tree;
  float gap, max_gap;
  int nlen_aln, nlen_aln1;
  int *used_col, *col_lu;
  char *tree_mode=NULL;
  int scan_maxD=0;
  float highD, lowD, maxD;
  int nd, ntrees;
  long tot_inf=0;
  int use_columns=0;
  
  if (!A) return A;
  
  //Get arguments passed via environement
  if (getenv ("TREE_GAP_4_TCOFFEE")) max_gap  =atofgetenv("TREE_GAP_4_TCOFFEE");
  else max_gap=0.5;
  
  if (getenv ("TREE_MODE_4_TCOFFEE"))tree_mode=getenv("TREE_MODE_4_TCOFFEE");
  else tree_mode="nj";
    
  if (getenv ("REPLICATES_4_TCOFFEE"))
    {
      if ( strm (getenv ("REPLICATES_4_TCOFFEE"), "columns"))
	{
	  use_columns=1;
	  replicates=A->len_aln;
	}
      else 
	replicates=atoigetenv ("REPLICATES_4_TCOFFEE");
    }
  else replicates=0;
  
  S=CL->S;
  OUT=copy_aln (A, NULL);
  
  
  nM=0;strikeM=++nM;contactsM=++nM;distancesM=++nM;
  if (!mode || strm (mode, "strike"))modeM=strikeM;
  else if (strm (mode, "distances"))modeM=distancesM;
  else if (strm (mode, "contacts"))modeM=contactsM;
  else 	
    {
      printf_exit ( EXIT_FAILURE,stderr, "\nERROR: struc_evaluate4tcoffe::mode %s is unknown", (mode)?mode:"unset");
    }
  


  //identify sequences with contact information
  rseq=(int*)vcalloc (A->nseq, sizeof (int));
  for (a=0,s1=0; s1<A->nseq; s1++)
    {
      rseq[s1]=name_is_in_list (A->name[s1], S->name, S->nseq, 100);
      if (rseq[s1]==-1);
      else
	{
	  int ls1=rseq[s1];
	  rseq[s1]=-1;
	  for (r1=1;r1<=S->len[ls1]; r1++)
	    {
	      if (CL->residue_index[ls1][r1][0]>1){rseq[s1]=ls1;r1=S->len[ls1]+1;}
	    }
	  if (rseq[s1]!=-1)a++;
	}      
    }
  
  //Trim out sequences without contact information or exit if no a single contact information
  if (!a)
    printf_exit ( EXIT_FAILURE,stderr, "\nERROR: No contact information could be gathered for any sequence  [FATAL]");
  else if (a!=A->nseq && modeM!=strikeM)
    {
      int ns=0;
      for (s1=0; s1<A->nseq; s1++)
	{
	  if (rseq[s1]==-1);
	  else
	    {
	      sprintf (A->name[ns], "%s", A->name[s1]);
	      sprintf (A->seq_al[ns], "%s", A->seq_al[s1]);
	      rseq[ns]=rseq[s1];
	      ns++;
	    }
	}
      A->nseq=ns;
    }
  

  //Prepare the evlauation information
  
  
  max_res_sc=declare_double (A->nseq, A->len_aln);
  tot_res_sc=declare_double (A->nseq, A->len_aln);
  
  max_seq_sc=(double*)vcalloc (A->nseq, sizeof (double));
  tot_seq_sc=(double*)vcalloc (A->nseq, sizeof (double));
  
  max_col_sc=(double*)vcalloc (A->len_aln, sizeof (double));
  tot_col_sc=(double*)vcalloc (A->len_aln, sizeof (double));
  
  max_sc=0;
  tot_sc=0;
 
  //Set the right evlaution matrix for proteins or RNA
  if (modeM==strikeM)
    {
      
      matrix_name=(char*)vcalloc ((strlen(in_matrix_name)+2), sizeof (char));
      if (strm (in_matrix_name, "strike"))
	{
	  S=fast_get_sequence_type(S);
	  if (strm (S->type, "RNA"))sprintf(matrix_name, "strikeR");
	  else sprintf(matrix_name, "strikeP");
	}
      else
	sprintf(matrix_name, "%s", in_matrix_name);
      matrix=read_matrice (matrix_name);
      max=(int*)vcalloc (256, sizeof (int));
      for (a=0; a<256; a++)
	{
	  for (max[a]=-1000,b=0; b<256; b++)
	    {
	      if (matrix[a][b]>max[a]){max[a]=matrix[a][b];}
	    }
	}
    }
  else
    {
      matrix_name=NULL;
      matrix=NULL;
    }
 
  dm1=declare_int (A->len_aln, A->len_aln);
  dm2=declare_int (A->len_aln, A->len_aln);
  
  pos=aln2pos_simple (A, A->nseq);
  lu=declare_int (A->nseq, A->len_aln);
  for (s1=0; s1<A->nseq; s1++)
    {
      for (c=0,c1=0; c1<A->len_aln; c1++)
	{
	  if (!is_gap(A->seq_al[s1][c1]))lu[s1][c++]=c1;
	}
      if (rseq[s1]!=-1 && c!=S->len[rseq[s1]])
	{printf_exit ( EXIT_FAILURE,stderr, "\nERROR: %s differs in MSA/contact-lib[FATAL]", A->name[s2]);}
    }
 
  vsrand (0);
  
  highD=imaxD*100; //set in picometers
  lowD=1500;
  if (modeM==distancesM && replicates>0 && scan_maxD)lowD=500;
  else lowD=highD;
  for (nd=0,maxD=lowD;maxD<=highD; maxD+=100, nd++);
  
  

  //Prepare the gap cache
  nlen_aln=0;
  used_col=(int*)vcalloc (A->len_aln, sizeof (int));
  col_lu=(int*)vcalloc (A->len_aln, sizeof (int));
  
  for (c1=0; c1<A->len_aln; c1++)
    {
      for (gap=0,r1=0; r1<A->nseq; r1++)
	if (is_gap(A->seq_al[r1][c1]))gap++;
      gap/=(float)A->nseq;
      if (gap<=max_gap){used_col[nlen_aln++]=c1;col_lu[c1]=1;}
    }
  if (nlen_aln==0)
    {printf_exit ( EXIT_FAILURE,stderr, "\nERROR: No column with less than %.2f %% gaps[FATAL]", max_gap*100);}
  

  if (replicates>=1)
    {
      tree=1;
      max_pw_sc=declare_double (A->nseq, A->nseq);
      tot_pw_sc=declare_double (A->nseq, A->nseq);
      if (use_columns)replicates=nlen_aln;
      
    }
  else 
    {
      tree=0;
      replicates=1;
      max_pw_sc=tot_pw_sc=NULL;
    }
  ntrees=nd*replicates;
  if (!A->Tree)T=A->Tree=declare_aln2(ntrees,0);
  T->RepColList=declare_int  (ntrees, ((use_columns)?1:nlen_aln)+1);
  

  
  
  for (maxD=lowD; maxD<=highD;maxD+=100) 
    {
      for (rep=0; rep<replicates; rep++)
	{
	  //prepare the replicates drawing from the proper positions
	  if (use_columns)
	    {
	      nlen_aln1=1;
	      T->RepColList[rep][0]=used_col[rep];
	      T->RepColList[rep][1]=-1;
	    }
	  else
	    {
	      nlen_aln1=nlen_aln;
	      for (c1=0; c1<nlen_aln; c1++)
		{
		  if (rep>0)T->RepColList[rep][c1]=used_col[(int)rand()%nlen_aln];
		  else T->RepColList[rep][c1]=used_col[c1];
		}
	      T->RepColList[rep][c1]=-1;
	    }
	  for (s1=0; s1<A->nseq; s1++)
	    {
	    int ls1=rseq[s1];
	    int s2_has_contacts=0;
	    
	    if (ls1==-1)continue;
	    
	    //Get contacts from top sequence
	    for (r1=1;r1<=S->len[ls1]; r1++)
	      {
		for (b=1; b<CL->residue_index[ls1][r1][0]; b+=ICHUNK)
		  {
		    int ss2;
		    r2 =CL->residue_index[ls1][r1][b+R2];
		    we =CL->residue_index[ls1][r1][b+WE];
		    ss2=CL->residue_index[ls1][r1][b+SEQ2];
		    
		    if (ss2!=ls1){HERE ("Warning: Contact library contains inter-sequence data contacts: %d %d", a, ss2);}
		    p1=lu[s1][r1-1];
		    p2=lu[s1][r2-1];
		    dm1[p1][p2]=dm1[p2][p1]=we;
		  }
	      }
	    //Scan bottom sequences
	    for (s2=0; s2<A->nseq; s2++)
	      {
		int ls2=rseq[s2];
		if (s1==s2) continue;
		else if (ls2!=-1)
		  {
		    s2_has_contacts=1;
		    
		    for (r1=1;r1<=S->len[ls2]; r1++)
		      {
			for (b=1; b<CL->residue_index[ls2][r1][0]; b+=ICHUNK)
			  {
			    int ss2;
			    r2 =CL->residue_index[ls2][r1][b+R2];
			    we =CL->residue_index[ls2][r1][b+WE];
			    ss2=CL->residue_index[ls2][r1][b+SEQ2];
			    
			    if (ss2!=ls2){HERE ("Warning: Contact library contains inter-sequence data contacts: %d %d", a,ss2);}
			    p1=lu[s2][r1-1];
			    p2=lu[s2][r2-1];
			    dm2[p1][p2]=dm2[p2][p1]=we;
			  }
		      }
		  }
		else
		  {
		    s2_has_contacts=0;
		  }
		//Scan the pairs of contact for S1 vs S2
		
		if (modeM!=strikeM && !s2_has_contacts)continue;//ignore sequences w/o contacts 
		
		for (ic1=0; ic1<nlen_aln1; ic1++)
		  {
		    c1=T->RepColList[rep][ic1];
		    
		    if (A->seq_al[s1][c1]=='-')continue;
		    for (c2=0; c2<A->len_aln; c2++)
		      {
			double sc=0;
			double in=0;
			double w1=(double)dm1[c1][c2];
			
			
			if (A->seq_al[s1][c2]=='-')continue;
			else if (!col_lu[c2])continue; 
			else if (FABS((pos[s1][c1]-pos[s1][c2]))<enb)continue;
			else if (!dm1[c1][c2])continue;
			else if (modeM==distancesM && w1>maxD)continue;
			
			if (modeM==strikeM && !s2_has_contacts)
			  {
			    if (A->seq_al[s2][c1]=='-' || A->seq_al[s2][c2]=='-'){sc=0;}
			    else 
			      {
				
				sc=matrix[tolower(A->seq_al[s2][c1])][tolower(A->seq_al [s2][c2])]*2;
				sc/=max[tolower(A->seq_al[s2][c1])]+max[tolower(A->seq_al[s2][c1])];
			      }
			    in=1;
			  }
			else
			  {
			    if (A->seq_al[s2][c1]=='-' || A->seq_al[s2][c2]=='-'){sc=0;in=1;}
			    else
			      {
				double w2=(double)dm2[c1][c2];
				if (modeM==distancesM && dm2[c1][c2])
				  {
				    double we=w1;
				    sc=((MIN((w1/w2),(w2/w1))));
				    sc=sc*sc*sc*we;
				    in=we;
				  }
				else if (modeM==contactsM && w2>0){sc=1;in=1;}
				else if (modeM==strikeM   && w2>0){sc=1;in=1;}
				else {sc=0;in=1;}
			      }
			  }

			if (tree)
			  {
			    tot_pw_sc[s2][s1]+=sc;
			    max_pw_sc[s1][s2]+=in;;
			  }
			
			
			//residues
			max_res_sc[s1][c1]+=in;
			tot_res_sc[s1][c1]+=sc;
			
			max_res_sc[s2][c1]+=in;
			tot_res_sc[s2][c1]+=sc;
			
			//seq
			tot_seq_sc[s1]+=sc;
			max_seq_sc[s1]+=in;
			
			tot_seq_sc[s2]+=sc;
			max_seq_sc[s2]+=in;
			
			//column
			max_col_sc[c1]+=in;;
			tot_col_sc[c1]+=sc;
			
			//aln
			max_sc+=in;
			tot_sc+=sc;
			
		      }
		  }
		for (c1=0; c1<A->len_aln; c1++)for (c2=0; c2<A->len_aln; c2++)dm2[c1][c2]=0;
	      }
	    for (c1=0; c1<A->len_aln; c1++)for (c2=0; c2<A->len_aln; c2++)dm1[c1][c2]=0;
	    }
	  
	 
	  //Estimate Phylogenetic Tree Replicates
	  if (tree)
	    {
	    static char *treeF=vtmpnam (NULL);
	    
	    output_completion (stderr,T->nseq,ntrees,1, "Distance Tree Replicates");
	    	    
	    for (s1=0; s1<A->nseq; s1++)
	      for (s2=0; s2<A->nseq; s2++)
		if (max_pw_sc[s1][s2])
		  {tot_pw_sc[s1][s2]/=max_pw_sc[s1][s2];tot_pw_sc[s1][s2]=(double)100*(1-tot_pw_sc[s1][s2]);}
	    
	    
	    if (strm (tree_mode, "nj"))
	      dist2nj_tree (tot_pw_sc, A->name, A->nseq, treeF);
	    else if ( strm (tree_mode, "upgma"))
	      dist2upgma_tree (tot_pw_sc, A->name, A->nseq, treeF);
	    else
	      printf_exit ( EXIT_FAILURE,stderr, "\nERROR: %s is not a known tree_mode[FATAL]",tree_mode);
	    
	    T->seq_al[T->nseq]=file2string(treeF);
	    sprintf (T->name[T->nseq], "%d",T->nseq+1); 
	    T->nseq++;
	    
	    if (T->nseq<ntrees)
	      {
		for (s1=0; s1<A->nseq; s1++)
		  {
		    max_seq_sc[s1]=tot_seq_sc[s1]=0;
		    for (c1=0; c1<A->len_aln; c1++)
		      max_res_sc[s1][c1]=tot_res_sc[s1][c1]=0;
		    
		    for (s2=0; s2<A->nseq; s2++)
		      max_pw_sc[s1][s2]=tot_pw_sc[s1][s2]=0;
		  }
		for (c1=0; c1<A->len_aln; c1++)max_col_sc[c1]=tot_col_sc[c1]=0;
	      }
	    else 
	      {
		OUT->dm=tot_pw_sc;
	      }
	    }
	}
    }
  
  //compute bs_score
  
  if (tree)fprintf (stderr, "\n");
  
  for (a=0; a<A->nseq; a++)
    {
      for (c=0; c<A->len_aln; c++)
	{
	 if (pos[a][c]>0 && max_res_sc[a][c]>0 )
	    {
	      int r1=(max_res_sc[a][c]==0)?0:(tot_res_sc[a][c]*(double)10/max_res_sc[a][c]);
	      r1=(r1>=10)?9:r1;
	      OUT->seq_al[a][c]=r1+'0';
	    }
	  else if (pos[a][c]>0)
	    {
	      OUT->seq_al[a][c]=NO_COLOR_RESIDUE;
	    }
	}

      A->score_seq[a]=OUT->score_seq[a]=(max_seq_sc[a]==0)?0:(tot_seq_sc[a]*(double)100)/max_seq_sc[a];
      
    }
  sprintf (OUT->name[OUT->nseq], "cons");
  
  for (c=0; c<A->len_aln; c++)
    {
      int r1=(max_col_sc[c]==0)?0:(tot_col_sc[c]*(double)10)/max_col_sc[c];
      OUT->seq_al[A->nseq][c]=((r1>=10)?9:r1)+'0';
    }

  A->score=A->score_aln=OUT->score=OUT->score_aln=(int)(max_sc==0)?0:(tot_sc*1000)/max_sc;
  
  
  free_int (lu,  -1);
  free_int (dm1, -1);
  free_int (dm2, -1);
  free_double (tot_res_sc, -1);
  free_double (max_res_sc, -1);
  if (max_pw_sc)free_double(max_pw_sc, -1);
  vfree (tot_seq_sc);vfree (max_seq_sc);
  vfree (tot_col_sc);vfree (max_col_sc);
  
  return OUT;
}   	
Alignment *treealn_evaluate4tcoffee (Alignment *A, Sequence *G)
{
  int a,b,c;
  int *count;
  double  max, min;
  double *tot_seq_sc, *gscore;
  
  Alignment *OUT;
  Alignment *T;

  
  
  if (!A) return A;
  else if (!G)printf_exit (EXIT_FAILURE,stderr, "ERROR: treealn_evaluate requires a group");
  else if (!A->Tree)printf_exit (EXIT_FAILURE,stderr, "ERROR: treealn_evaluate requires pre-computed trees");

  T=A->Tree;
  
  OUT=copy_aln (A, NULL);
  gscore =(double*)vcalloc (A->len_aln, sizeof (double));
  tot_seq_sc=(double*)vcalloc (A->nseq, sizeof (double));
  count  =(int*)   vcalloc (A->len_aln, sizeof (int)); 
  
  for (a=0; a<T->nseq; a++)
    {
      double sc=(double)group2tree_score (G, T->seq_al[a]);
      sprintf (T->seq_comment[a], "GSCORE: %.2f",sc);
      if (sc>=0)
	{
	  T->score_seq[a]=(int)((double)100*sc);
	  
	  b=0;
	  while((c=T->RepColList[a][b++])!=-1)
	    {
	      
	      gscore[c]+=sc;
	      count[c]++;
	    }
	}
    }
  
  for (a=0; a<A->len_aln; a++)
    {
      gscore[a]=(count[a])?(gscore[a]/(double)count[a]):-1;
      if (gscore[a]<0);
      else 
	{
	  if (a==0)min=max=gscore[a];
	  else
	    {
	      if (gscore[a]>max)max=gscore[a];
	      if (gscore[a]<min)min=gscore[a];
	    }
	}
    }
  
  for (a=0; a<A->len_aln; a++)
    {
      gscore[a]=((max-min)==0)?max:(gscore[a]-min)/(max-min);
    }
  
  for (a=0; a<A->nseq; a++)
    {
      for (c=0; c<A->len_aln; c++)
	{
	  if (gscore[c]<0)OUT->seq_al[a][c]=NO_COLOR_RESIDUE;
	  else 
	    {
	      int r1=(float)(gscore[c]*10);
	      r1=(r1>=10)?9:r1;
	      OUT->seq_al[a][c]=r1+'0';
	      tot_seq_sc[a]=(gscore[c]>tot_seq_sc[a])?gscore[c]:tot_seq_sc[a];
	    }
	}
      A->score_seq[a]=OUT->score_seq[a]=tot_seq_sc[a]*100;
    }
  sprintf (OUT->name[OUT->nseq], "cons");
  
  for (c=0; c<A->len_aln; c++)
    {
      if (gscore[c]<0)OUT->seq_al[a][c]='0';
      else 
	{
	  int r1=(int)(gscore[c]*(float)10);
	  r1=(r1>=10)?9:r1;
	  OUT->seq_al[A->nseq][c]=r1+'0';
	}
    }
  max*=1000;
 
  A->score=A->score_aln=OUT->score=OUT->score_aln=(int)max;
  vfree (count);
  vfree (gscore);
  vfree (tot_seq_sc);
  return OUT;
}
  
Alignment *evaluate_tree_group (Alignment *T, Sequence *G)
{
  int a;
  double s;
  if (T->Tree)return evaluate_tree_group (T->Tree, G);
  
  for (a=0; a<T->nseq; a++)
    {
      s=group2tree_score (G, T->seq_al[a]);
      sprintf (T->seq_comment[a], "GSCORE: %.2f", (float)s);
      T->score_seq[a]=(int)((double)100*s);
    }
  return T;
}
Alignment * fast_coffee_evaluate_output ( Alignment *IN,Constraint_list *CL)
    {
    int a,b, c, m,res, s, s1, s2, r1, r2;	
    Alignment *OUT=NULL;
    int **pos, **pos2;

    double score_col=0, score_aln=0, score_res=0;
    double max_col, max_aln;
    double *max_seq, *score_seq;
    int local_m;
    int local_nseq;
    


    /*NORMALIZE: with the highest scoring pair found in the multiple alignment*/
   
    
    if ( !CL->evaluate_residue_pair){fprintf ( stderr, "\nWARNING: CL->evaluate_residue_pair Not set\nSet to: extend_residue_pair\n");CL->evaluate_residue_pair= extend_residue_pair; }
	
    OUT=copy_aln (IN, OUT);
    pos=aln2pos_simple(IN, IN->nseq);
    pos2=aln2defined_residues (IN, CL);
    
    max_seq=(double*)vcalloc ( IN->nseq, sizeof (double));
    score_seq=(double*)vcalloc ( IN->nseq, sizeof (double));
    
    
    
    /*1: Identify the highest scoring pair within the alignment*/
 
    for ( m=0, a=0; a< IN->len_aln; a++)
        {
	    for ( b=0; b< IN->nseq; b++)
		{
		s1=IN->order[b][0];
		r1=pos[b][a];
	

		for ( c=0; c< IN->nseq; c++)
		    {
		    s2=IN->order[c][0];
		    r2=pos[c][a];
		    if ( s1==s2 && !CL->do_self)continue;
	
		    if ( s1< s2)s=(CL->evaluate_residue_pair)( CL, s1, r1, s2, r2);
		    else        s=(CL->evaluate_residue_pair)( CL, s2, r2, s1, r1);
		    
		    s=(s!=UNDEFINED)?s:0;
		    m=MAX(m, s);
		    }
		}
	}
    
    local_m=m;

    sprintf ( OUT->name[IN->nseq], "cons");
    for ( max_aln=0,score_aln=0,a=0; a< IN->len_aln; a++)
	{
	OUT->seq_al[IN->nseq][a]=NO_COLOR_RESIDUE;
	for ( local_nseq=0,b=0; b<IN->nseq; b++){local_nseq+=(pos[b][a]>0 && pos2[b][a])?1:0;}
	local_m=m*(local_nseq-1);

	for ( max_col=0, score_col=0,b=0; b< IN->nseq; b++)
	    {
	    OUT->seq_al[b][a]=NO_COLOR_RESIDUE;
	    s1=IN->order[b][0];
	    r1=pos[b][a];
	    
	    if (r1<=0 || !pos2[b][a])
	      {
		continue;
	      }
	    
	    for ( score_res=0,c=0; c< IN->nseq; c++)
	        {
		    s2=IN->order[c][0];
		    r2=pos[c][a];		    
		    
		    if ((s1==s2 && !CL->do_self) || r2<=0 || !pos2[c][a]){continue;}	
		    max_col   +=m;
		    max_seq[b]+=m;
		    max_aln   +=m;
		   
		    if ( s1< s2)s=(CL->evaluate_residue_pair)( CL, s1, r1, s2, r2);
		    else        s=(CL->evaluate_residue_pair)( CL, s2, r2, s1, r1);
		    s=(s!=UNDEFINED)?s:0;
		    
		    score_res+=s;	
		    score_col+=s;		    
		    score_seq[b]+=s;
		    score_aln+=s;		    
		}
	    
	    res=(local_m==0)?NO_COLOR_RESIDUE:((score_res*10)/local_m);
	    (OUT)->seq_al[b][a]=(res==NO_COLOR_RESIDUE)?res:(MIN(res,9));	
	   
	    
	    }
	
	res=(max_col==0)?NO_COLOR_RESIDUE:((score_col*10)/max_col);	
	OUT->seq_al[IN->nseq][a]=(res==NO_COLOR_RESIDUE)?res:(MIN(res,9));
	
	}
    
    IN->score_aln=OUT->score_aln=(max_aln==0)?0:((score_aln*100)/max_aln);
    for ( a=0; a< OUT->nseq; a++)
	{
	OUT->score_seq[a]=(max_seq[a]==0)?0:((score_seq[a]*100)/max_seq[a]);
	}
    
    free_int (pos , -1);
    free_int (pos2, -1);
    
    vfree ( score_seq);
    vfree ( max_seq);
    return OUT;
    }      
  
Alignment * slow_coffee_evaluate_output ( Alignment *IN,Constraint_list *CL)
    {
    int a,b, c,res, s, s1, s2, r1, r2;	
    Alignment *OUT=NULL;
    int **pos, **pos2;
    double max_score_r, score_r, max;
    double score_col=0, score_aln=0;
    double max_score_col, max_score_aln;
    double *max_score_seq, *score_seq;
    int ***res_extended_weight;
    int n_res_in_col;
    
    
    /*
      Residue x: sum of observed extended X.. /sum of possible X..
    */

    


    if ( !CL->evaluate_residue_pair){fprintf ( stderr, "\nWARNING: CL->evaluate_residue_pair Not set\nSet to: extend_residue_pair\n");CL->evaluate_residue_pair= extend_residue_pair; }
	
    
    OUT=copy_aln (IN, OUT);
    pos=aln2pos_simple(IN, IN->nseq);
    pos2=aln2defined_residues (IN, CL);
    
    max_score_seq=(double*)vcalloc ( IN->nseq, sizeof (double));
    score_seq=(double*)vcalloc ( IN->nseq, sizeof (double));
    res_extended_weight=(int***)declare_arrayN(3,sizeof(int), (CL->S)->nseq, (CL->S)->max_len+1, 2);
    max=(CL->normalise)?(100*CL->normalise)*SCORE_K:100;
    
    for (a=0; a< IN->len_aln; a++)
        {
	  for ( b=0; b< IN->nseq-1; b++)
	    {
	      s1=IN->order[b][0];
	      r1=pos[b][a];
	      for ( c=b+1; c< IN->nseq; c++)
		{
		  s2=IN->order[c][0];
		  r2=pos[c][a];	
		  if ( s1==s2 && !CL->do_self)continue;
		  else if ( r1<=0 || r2<=0)   continue;		    
		  else 
		    {
		      s=(CL->evaluate_residue_pair)( CL, s1, r1, s2, r2);
		      res_extended_weight[s1][r1][0]+=s*100;
		      res_extended_weight[s2][r2][0]+=s*100;
		      res_extended_weight[s1][r1][1]+=max;
		      res_extended_weight[s2][r2][1]+=max;
		    }
		}
	    }
	}

    
    sprintf ( OUT->name[IN->nseq], "cons");
    for ( max_score_aln=0,score_aln=0,a=0; a< IN->len_aln; a++)
      {
	OUT->seq_al[IN->nseq][a]=NO_COLOR_RESIDUE;
	for ( n_res_in_col=0,b=0; b<IN->nseq; b++){n_res_in_col+=(pos[b][a]>0 && pos2[b][a]>0)?1:0;}
	for ( max_score_col=0, score_col=0,b=0; b< IN->nseq; b++)
	    {
	    OUT->seq_al[b][a]=NO_COLOR_RESIDUE;
	    s1=IN->order[b][0];
	    r1=pos[b][a];
	    if (r1<=0 || pos2[b][a]<1)continue;
	    else
	      {
		max_score_r  =res_extended_weight[s1][r1][1];
		score_r      =res_extended_weight[s1][r1][0];
		if      ( max_score_r==0 && n_res_in_col>1)res=0;
		else if ( n_res_in_col==1)res=NO_COLOR_RESIDUE;
		else res=((score_r*10)/max_score_r);

		
		(OUT)->seq_al[b][a]=(res==NO_COLOR_RESIDUE)?res:(MIN(res, 9));
		max_score_col+=max_score_r;
		    score_col+=score_r;
		max_score_seq[b]+=max_score_r;
		    score_seq[b]+=score_r;
		max_score_aln+=max_score_r;
		    score_aln+=score_r;
	      }
	    if      ( max_score_col==0 && n_res_in_col>1)res=0;
	    else if ( n_res_in_col<2)res=NO_COLOR_RESIDUE;
	    else res=((score_col*10)/max_score_col);

	    OUT->seq_al[IN->nseq][a]=(res==NO_COLOR_RESIDUE)?res:(MIN(res,9));
	    }
	}
    IN->score_aln=OUT->score_aln=(max_score_aln==0)?0:((score_aln*100)/max_score_aln);
    for ( a=0; a< OUT->nseq; a++)
      {
	OUT->score_seq[a]=(max_score_seq[a]==0)?0:((score_seq[a]*100)/max_score_seq[a]);
      }

    
    vfree ( score_seq);
    vfree ( max_score_seq);
    free_arrayN((void*)res_extended_weight, 3);


    free_int (pos, -1);
    free_int (pos2, -1);
    return OUT;
    }

double genepred2sd    (Sequence *S);
double genepred2avg   (Sequence *S);
double genepred2zsum2 (Sequence *S);
int *  genepred2orf_len (Sequence *S);
double genepred2acc (Sequence *S, Sequence *PS);
double seq_genepred2acc (Sequence *S, Sequence *PS, char *name);
Alignment *coffee_seq_evaluate_output ( Alignment *IN, Constraint_list *CL)
{
  int **w, a, best_cycle, ncycle=20, min_ne;
  char **pred_list,**weight_list; 
  float nsd, best_score,score;
  Sequence *S, *PS;

  min_ne=CL->ne/10; //avoid side effects due to very relaxed libraries
  w=NULL;
  pred_list=declare_char (ncycle, 100);
  weight_list=declare_char(ncycle,100);
  S=CL->S;
  fprintf ( stderr, "\nSCORE_MODE: %s\n", CL->genepred_score);
  
  for (a=0; a<ncycle && CL->ne>min_ne; a++)
    {
      if (w);free_int (w, -1);
      w=list2residue_total_weight (CL);
      PS=dnaseq2geneseq (S, w);
      nsd=(float)(genepred2sd(PS)/genepred2avg(PS));
      if   ( strm (CL->genepred_score, "nsd"))score=nsd;
      else score=1-seq_genepred2acc (S, PS, CL->genepred_score);
      fprintf ( stderr, "Cycle: %2d LIB: %6d AVG LENGTH: %6.2f NSD: %8.2f SCORE: %.2f ",a+1,CL->ne,(float) genepred2avg(PS),nsd,score);
	
      vfree (display_accuracy (genepred_seq2accuracy_counts (S, PS, NULL),stderr));
      pred_list[a]=vtmpnam(NULL);
      weight_list[a]=vtmpnam(NULL);
      output_fasta_seqS(pred_list[a],PS);
      output_array_int (weight_list[a],w);
      free_sequence (PS, -1);
      
      if (a==0 || score<best_score)
	{
	  best_score=score;
	  best_cycle=a;
	}
      CL=relax_constraint_list_4gp(CL);
    }
  if (w)free_int(w,-1);
  S=get_fasta_sequence (pred_list[best_cycle], NULL);

  fprintf ( stderr, "\nSelected Cycle: %d (SCORE=%.4f)----> ", best_cycle+1, best_score);
  vfree (display_accuracy (genepred_seq2accuracy_counts ((CL->S), S, NULL),stderr));

  genepred_seq2accuracy_counts4all((CL->S), S);
  
  IN=seq2aln (S,NULL, 1);
  IN->score_res=input_array_int (weight_list[best_cycle]);
  return IN;
}

double seq_genepred2acc (Sequence *S, Sequence *PS, char *name)
{ 
  float *count;
  float *acc;
  float r;
  int ref, target;

  if ( strm (name, "best"))return genepred2acc (S, PS);
  
  ref=name_is_in_list (name, S->name, S->nseq, 100);
  target=name_is_in_list (name, PS->name, PS->nseq, 100);

  if ( target==-1 || ref==-1)
    {
      printf_exit (EXIT_FAILURE,stderr, "\nERROR: %s is not a valid sequence", name);
    }
  count=genepred2accuracy_counts      (S->seq[ref],PS->seq[target],NULL);
  acc=counts2accuracy (count);
  
  r=acc[3];
  vfree (acc);
  vfree (count);
  return r;
}

  
    
double genepred2acc (Sequence *S, Sequence *PS)
{
  float *count;
  float *acc;
  float r;
  count=genepred_seq2accuracy_counts (S, PS, NULL);
  acc=counts2accuracy (count);
  
  r=acc[3];
  vfree (acc);
  vfree (count);
  return r;
}
  
double genepred2sd (Sequence *S)
{
  double sum=0, sum2=0;
  int a, *len;
  len=genepred2orf_len (S);
  
  for (a=0; a<S->nseq; a++)
    {
      sum+=(double)len[a];
      sum2+=(double)len[a]*(double)len[a];
    }
  sum/=(double)S->nseq;
  sum2/=S->nseq;
  vfree (len);
  return sqrt (sum2-(sum*sum));
}
double genepred2avg (Sequence *S)
{
  int a, *len;
  double avg=0;

  len=genepred2orf_len (S);

  for (a=0; a<S->nseq; a++)avg+=len[a];
  vfree (len);
  return avg/(double)S->nseq;
}
double genepred2zsum2 (Sequence *S)
{
  double zscore=0, zsum2=0, sd, avg;
  int a, *len;
  
  sd=genepred2sd(S);
  avg=genepred2avg (S);
  
  len=genepred2orf_len (S);
  for (a=0; a<S->nseq; a++)
    {
      zscore=((double)len[a]-avg)/sd;
      zsum2+=FABS(zscore);
    }
  zsum2/=(float)S->nseq;
  vfree (len);
  return zsum2; 
}
int *genepred2orf_len (Sequence *S)
{
  int a,b, *len;
  len=(int*)vcalloc (S->nseq, sizeof (int));
   for (a=0; a<S->nseq; a++)
    for (b=0; b<S->len[a]; b++)
      len[a]+=(isupper(S->seq[a][b]));
   return len;
}
  
Alignment *coffee_seq_evaluate_output_old2 ( Alignment *IN, Constraint_list *CL)
{
  int **w, a,b,c;
  int avg, min_avg, best_cycle, ncycle=100;;
  char **pred_list; 
  Sequence *S, *PS;

  w=NULL;
  pred_list=declare_char (ncycle, 100);
  S=CL->S;
  
  //CL=expand_constraint_list_4gp(CL);
  min_avg=constraint_list2avg(CL);
  
  for (a=1; a<ncycle && CL->ne>0; a++)
    {
      if (w);free_int (w, -1);
      w=list2residue_total_weight (CL);
      CL=relax_constraint_list_4gp (CL);
      fprintf (stderr,"\nRELAX CYCLE: %2d AVG: %5d [%10d] ", a, avg=constraint_list2avg(CL), CL->ne);
      for (b=0; b<S->nseq; b++)for (c=0; c<S->len[b]; c++)w[b][c]-=1;
	
      //rescale nuclotide coding weights
  
      PS=dnaseq2geneseq (S, w);
      vfree (display_accuracy (genepred_seq2accuracy_counts (S, PS, NULL),stderr));
     
     
      free_sequence (PS, PS->nseq);
      if ( avg<min_avg)
	{
	  min_avg=avg;
	  best_cycle=a;
	}
    }
  
  S=get_fasta_sequence (pred_list[best_cycle], NULL);
  vfree (display_accuracy (genepred_seq2accuracy_counts ((CL->S), S, NULL),stderr));myexit (0);
  for (a=0; a<S->nseq; a++)
    HERE (">%s\n%s", S->name[a], S->seq[a]);
  myexit (0);
  return seq2aln (S,NULL, 1);
}



Alignment * non_extended_t_coffee_evaluate_output ( Alignment *IN,Constraint_list *CL)
    {
    int a,b, c,res, s1, s2, r1, r2;	
    Alignment *OUT=NULL;
    int **pos;
    int max_score_r, score_r;
    double score_col=0, score_aln=0;
    int max_score_col, max_score_aln;
    double *max_score_seq, *score_seq;
    int local_nseq;
    int **tot_non_extended_weight;
    int **res_non_extended_weight;
    int *l;
    CLIST_TYPE  *entry=NULL;
    int p;
    int max_score=0;

    entry=(int*)vcalloc (CL->entry_len+1, CL->el_size);
    if ( !CL->evaluate_residue_pair){fprintf ( stderr, "\nWARNING: CL->evaluate_residue_pair Not set\nSet to: extend_residue_pair\n");CL->evaluate_residue_pair= extend_residue_pair; }
	
    OUT=copy_aln (IN, OUT);
    pos=aln2pos_simple(IN, IN->nseq);


    max_score_seq=(double*)vcalloc ( IN->nseq, sizeof (double));
    score_seq=(double*)vcalloc ( IN->nseq, sizeof (double));
    
    tot_non_extended_weight=list2residue_total_weight(CL);
    res_non_extended_weight=declare_int ((CL->S)->nseq, (CL->S)->max_len+1);
         
    for (a=0; a< IN->len_aln; a++)
        {
	    for ( b=0; b< IN->nseq-1; b++)
		{
		s1=IN->order[b][0];
		r1=pos[b][a];
		for ( c=b+1; c< IN->nseq; c++)
		    {
		    s2=IN->order[c][0];
		    r2=pos[c][a];	
		    if ( s1==s2 && !CL->do_self)continue;
		    else if ( r1<=0 || r2<=0)   continue;		    
		    else 
		      {
			entry[SEQ1]=s1;
			entry[SEQ2]=s2;
			entry[R1]=r1;
			entry[R2]=r2;
			if ((l=main_search_in_list_constraint (entry,&p,4,CL))!=NULL)
			  {
			    res_non_extended_weight[s1][r1]+=l[WE];
			    res_non_extended_weight[s2][r2]+=l[WE];
			  }
			entry[SEQ1]=s2;
			entry[SEQ2]=s1;
			entry[R1]=r2;
			entry[R2]=r1;
			if ((l=main_search_in_list_constraint (entry,&p,4,CL))!=NULL)
			    {
			      res_non_extended_weight[s1][r1]+=l[WE];
			      res_non_extended_weight[s2][r2]+=l[WE];
			    }
			max_score=MAX(max_score,res_non_extended_weight[s1][r1]);
			max_score=MAX(max_score,res_non_extended_weight[s2][r2]);
						
		      }
		    }
		}
	}
  
    sprintf ( OUT->name[IN->nseq], "cons");
    for ( max_score_aln=0,score_aln=0,a=0; a< IN->len_aln; a++)
	{
	OUT->seq_al[IN->nseq][a]=NO_COLOR_RESIDUE;
	for ( local_nseq=0,b=0; b<IN->nseq; b++){local_nseq+=(pos[b][a]>0)?1:0;}
	
       	for ( max_score_col=0, score_col=0,b=0; b< IN->nseq; b++)
	    {
	    OUT->seq_al[b][a]=NO_COLOR_RESIDUE;
	    s1=IN->order[b][0];
	    r1=pos[b][a];
	    if (r1<=0)continue;
	    else
	      {
		max_score_r  =max_score;/*tot_non_extended_weight[s1][r1];*/
		score_r=res_non_extended_weight[s1][r1];
		res=(max_score_r==0 || local_nseq<2 )?NO_COLOR_RESIDUE:((score_r*10)/max_score_r);
	
		(OUT)->seq_al[b][a]=(res==NO_COLOR_RESIDUE)?res:(MIN(res, 9));
		max_score_col+=max_score_r;
		    score_col+=score_r;
		max_score_seq[b]+=max_score_r;
		    score_seq[b]+=score_r;
		max_score_aln+=max_score_r;
		    score_aln+=score_r;
	      }
	    res=(max_score_col==0 || local_nseq<2)?NO_COLOR_RESIDUE:((score_col*10)/max_score_col);	
	    OUT->seq_al[IN->nseq][a]=(res==NO_COLOR_RESIDUE)?res:(MIN(res,9));
	    }
	}
    IN->score_aln=OUT->score_aln=(max_score_aln==0)?0:((score_aln*100)/max_score_aln);
    for ( a=0; a< OUT->nseq; a++)
      {
	OUT->score_seq[a]=(max_score_seq[a]==0)?0:((score_seq[a]*100)/max_score_seq[a]);
	OUT->score_seq[a]=(OUT->score_seq[a]>100)?100:OUT->score_seq[a];
      }
    OUT->score_aln=(OUT->score_aln>100)?100:OUT->score_aln;
    
    vfree ( score_seq);
    vfree ( max_score_seq);
    
    free_int (tot_non_extended_weight, -1);
    free_int (res_non_extended_weight, -1);
    vfree(entry);
    free_int (pos, -1);

    return OUT;
    }


/*********************************************************************************************/
/*                                                                                           */
/*        PROFILE/PRofile Functions                                                          */
/*                                                                                           */
/*********************************************************************************************/
int channel_profile_profile (int *prf1, int *prf2, Constraint_list *CL);

Profile_cost_func get_profile_mode_function (char *name, Profile_cost_func func)
{
  int a;
  static int nfunc;
  static Profile_cost_func *flist;
  static char **nlist;
 
  
 
  /*The first time: initialize the list of pairwse functions*/
  /*If func==NULL:REturns a pointer to the function associated with a name*/
  /*If name is empty:Prints the name of the function  associated with name*/
  
    if ( nfunc==0)
      {
	flist=(int (**)(int*, int*, Constraint_list*) )vcalloc ( 100, sizeof (Pwfunc));
	nlist=declare_char (100, 100);
	
	flist[nfunc]=cw_profile_profile;
	sprintf (nlist[nfunc], "cw_profile_profile");
	nfunc++;	
			
	flist[nfunc]=muscle_profile_profile;
	sprintf (nlist[nfunc], "muscle_profile_profile");
	nfunc++;
	
	flist[nfunc]=channel_profile_profile;
	sprintf (nlist[nfunc], "channel_profile_profile");
	nfunc++;
      }
  
   for ( a=0; a<nfunc; a++)
      {
	if ( (name && strm (nlist[a],name)) || flist[a]==func)
	     {
	       if (name)sprintf (name,"%s", nlist[a]);
	       return flist[a];
	     }
      }
    fprintf ( stderr, "\n[%s] is an unknown profile_profile function[FATAL:%s]\n",name, PROGRAM);
    crash ( "\n");
    return NULL;
  } 

int generic_evaluate_profile_score     (Constraint_list *CL,Alignment *Profile1,int s1, int r1, Alignment *Profile2,int s2, int r2, Profile_cost_func prf_prf)
    {
      int *prf1, *prf2;
      static int *dummy;
      int score;
      
   
      /*Generic profile function*/
      if( !dummy)
	{
	 dummy=(int*)vcalloc (10, sizeof(int));
	 dummy[0]=1;/*Number of Amino acid types on colum*/
	 dummy[1]=5;/*Length of Dummy*/
	 dummy[3]='\0';/*Amino acid*/
	 dummy[4]=1; /*Number of occurences*/
	 dummy[5]=100; /*Frequency in the MSA column*/

	}
      
      if (r1>0 && r2>0)
	    {
	    r1--;
	    r2--;
	    
	    prf1=(Profile1)?(Profile1->P)->count2[r1]:NULL;
	    prf2=(Profile2)?(Profile2->P)->count2[r2]:NULL;
	    
	    if (!prf1)     {prf1=dummy; prf1[3]=(CL->S)->seq[s1][r1];}
	    else if (!prf2){prf2=dummy; prf2[3]=(CL->S)->seq[s2][r2];}
	    
	    score=((prf_prf==NULL)?cw_profile_profile:prf_prf) (prf1, prf2, CL);
	    return score;
	    }
      else
	return 0;
    }

int cw_profile_profile_count    (int *prf1, int *prf2, Constraint_list *CL)
    {
      /*see function aln2count2 for prf structure*/
      int a, b, n;
      int res1, res2;
      double score=0;
      

      for ( n=0,a=3; a<prf1[1]; a+=3)
	for ( b=3; b<prf2[1]; b+=3)
	  {
	    
	    res1=prf1[a];
	    res2=prf2[b];
	
	    score+=prf1[a+1]*prf2[b+1]*CL->M[res1-'A'][res2-'A'];
	    n+=prf1[a+1]*prf2[b+1];
	  }
      

      score=(score*SCORE_K)/n;
      return score;
    }
int muscle_profile_profile    (int *prf1, int *prf2, Constraint_list *CL)
    {
      /*see function aln2count2 for prf structure*/
      int a, b;
      int res1, res2;
      double score=0, fg1, fg2, fi, fj, m;
      static double *exp_lu;
      if (exp_lu==NULL)
	{
	  exp_lu=(double*)vcalloc ( 10000, sizeof (double));
	  exp_lu+=2000;
	  for ( a=-1000; a<1000; a++)
	    exp_lu[a]=exp((double)a);
	}
	  
      

      for (a=3; a<prf1[1]; a+=3)
	{
	  res1=prf1[a];
	  fi=(double)prf1[a+2]/100;
	  
	  for ( b=3; b<prf2[1]; b+=3)
	    {
	      res2=prf2[b];
	      fj=(double)prf2[b+2]/100;
	      /*m=exp((double)CL->M[res1-'A'][res2-'A']);*/
	      m=exp_lu[CL->M[res1-'A'][res2-'A']];
	      score+=m*fi*fj;
	    }
	}
      
      fg1=(double)prf1[2]/100;
      fg2=(double)prf2[2]/100;
      score=(score==0)?0:log(score)*(1-fg1)*(1-fg2);
      score=(score*SCORE_K);
      /*if ( score<-100)fprintf ( stderr, "\nSCORE %d %d", (int)score, cw_profile_profile(prf1, prf2, CL));*/
       
      return (int)score;
    }

int cw_profile_profile   (int *prf1, int *prf2, Constraint_list *CL)
    {
      /*see function aln2count2 for prf structure*/
      int a, b, n,p;
      int res1, res2;
      double score=0;


      for ( n=0,a=3; a<prf1[1]; a+=3)
	for ( b=3; b<prf2[1]; b+=3)
	  {
	    
	    res1=prf1[a];
	    res2=prf2[b];
	    p=prf1[a+1]*prf2[b+1];
	
	    n+=p;
	    score+=p*CL->M[res1-'A'][res2-'A'];
	  }
      
      score=(score*SCORE_K)/((double)(n==0)?1:n);
      return score;
    }
int cw_profile_profile_ref    (int *prf1, int *prf2, Constraint_list *CL)
    {
      /*see function aln2count2 for prf structure*/
      int a, b, n,p;
      int res1, res2;
      double score=0;


      for ( n=0,a=3; a<prf1[1]; a+=3)
	for ( b=3; b<prf2[1]; b+=3)
	  {
	    
	    res1=prf1[a];
	    res2=prf2[b];
	    p=prf1[a+1]*prf2[b+1];
	
	    n+=p;
	    score+=p*CL->M[res1-'A'][res2-'A'];
	  }
      score=(score*SCORE_K)/((double)(n==0)?1:n);
      return score;
    }
int cw_profile_profile_old    (int *prf1, int *prf2, Constraint_list *CL)
    {
      /*see function aln2count2 for prf structure*/
      int a, b, n,p;
      int res1, res2;
      double score=0;
      

      
      for ( n=0,a=3; a<prf1[1]; a+=3)
	for ( b=3; b<prf2[1]; b+=3)
	  {
	    
	    res1=prf1[a];
	    res2=prf2[b];
	    p=prf1[a+1]*prf2[b+1];
	
	    n+=p;
	    score+=p*CL->M[res1-'A'][res2-'A'];
	  }
      score=(score*SCORE_K)/((double)(n==0)?1:n);
      return score;
    }
int channel_profile_profile ( int *prf1, int *prf2, Constraint_list *CL)
{

  int score=0;
  
  prf1+=prf1[1];
  prf2+=prf2[1];

  
  if (prf1[0]!=prf1[0]){fprintf ( stderr, "\nERROR: Inconsistent number of channels [channel_profile_profile::FATAL%s]", PROGRAM);}
  else
    {
      int a, n;
      for (a=1, n=0; a<=prf1[0]; a++)
	{
	  if (prf1[a]>0 && prf2[a]>0)
	    {
	      n++;score+=CL->M[prf1[a]-'A'][prf2[a]-'A'];

	    }
	}

      if ( n==0)return 0;
      
      score=(n==0)?0:(score*SCORE_K)/n;
     
    }
  return score;
}
  
/*********************************************************************************************/
/*                                                                                           */
/*         FUNCTIONS FOR GETING THE COST : (Sequences) ->evaluate_residue_pair               */
/*                                                                                           */
/*********************************************************************************************/
int initialize_scoring_scheme (Constraint_list *CL)
{
  if (!CL) return 0;
  else if (!CL->evaluate_residue_pair)return 0;
  else if ( !CL->S) return 0;
  else if ( (CL->S)->nseq<2) return 0;
  else if ( strlen ((CL->S)->seq[0])==0)return 0;
  else if ( strlen ((CL->S)->seq[1])==0)return 0;
  else
    {
      //CL->evaluate_residue_pair (CL,13,1,16,1);
      CL->evaluate_residue_pair (CL,0,1,1,1);
      
    }
  return 1;
}

int evaluate_blast_profile_score (Constraint_list *CL, int s1, int r1, int s2, int r2)
{
  Alignment *PRF1;
  Alignment *PRF2;


  PRF1=(Alignment*)atop(seq2T_value (CL->S, s1, "A", "_RB_"));
  PRF2=(Alignment*)atop(seq2T_value (CL->S, s2, "A", "_RB_"));
  
  return generic_evaluate_profile_score     (CL,PRF1,s1, r1, PRF2,s2, r2, CL->profile_mode);
}

int evaluate_aln_profile_score (Constraint_list *CL, int s1, int r1, int s2, int r2)
{
  
  return generic_evaluate_profile_score   (CL,seq2R_template_profile((CL->S),s1),s1, r1, seq2R_template_profile(CL->S,s2),s2, r2, CL->profile_mode); 
}


int evaluate_profile_score     (Constraint_list *CL,Alignment *Prf1, int s1, int r1, Alignment *Prf2,int s2, int r2)
{
  return generic_evaluate_profile_score (CL, Prf1, s1,r1,Prf2, s2,r2,CL->profile_mode);
}

int evaluate_cdna_matrix_score (Constraint_list *CL, int s1, int r1, int s2, int r2)
    {
      char a1, a2;
      
      if (r1>0 && r2>0) 
       {
	 r1--;
	 r2--;
	 
	 a1=translate_dna_codon((CL->S)->seq[s1]+r1,'x');
	 a2=translate_dna_codon((CL->S)->seq[s2]+r2,'x');
	 
	 
	 
	 if (a1=='x' || a2=='x')return 0;
	 else return CL->M[a1-'A'][a2-'A']*SCORE_K;
       }
      else
	{
	  return 0;
	}
    }
int evaluate_physico_score ( Constraint_list *CL, int s1, int r1, int s2, int r2)
{
  int a, b, p;
  double tot;
  static float **prop_table;
  static int n_prop;
  static double max;
  if (r1<0 || r2<0)return 0;
  if ( !prop_table)
    {
      prop_table= initialise_aa_physico_chemical_property_table(&n_prop);
      for (p=0; p< n_prop; p++)max+=100;
      max=sqrt(max);
    }
  a=tolower (( CL->S)->seq[s1][r1]);
  b=tolower (( CL->S)->seq[s2][r2]);
  
  for (tot=0,p=0; p< n_prop; p++)
    {
      tot+=(double)(prop_table[p][a]-prop_table[p][b])*(prop_table[p][a]-prop_table[p][b]);
    }
 
  tot=(sqrt(tot)/max)*10;
 
  return (int) tot*SCORE_K;
}



int evaluate_diaa_matrix_score ( Constraint_list *CL, int s1, int r1, int s2, int r2)
    {

      static int ****m;
      static int *alp;
      
      if (m==NULL)
	{
	  FILE *fp;
	  char k1[2], k2[2];
	  int v1, v2, c;
	  char *buf=NULL;
	  int a;
	  
	  m=(int****)declare_arrayN(4, sizeof (int), 26, 26, 26, 26);
	  fp=vfopen ("diaa_mat.mat", "r");
	  while ((c=fgetc (fp))!=EOF)
	    {
	      
	      ungetc (c, fp);
	      buf=vfgets(buf, fp);
	     
	      if (c=='#');
	      else 
		{
		  sscanf (buf, "%s %s %d %d", k1, k2, &v1, &v2);
		  
		  m[k1[0]-'a'][k1[1]-'a'][k2[0]-'a'][k2[1]-'a']=v1;
		  m[k2[0]-'a'][k2[1]-'a'][k1[0]-'a'][k1[1]-'a']=v1;
		}
	    }
	  vfclose (fp);
	  alp=(int*)vcalloc (256, sizeof (int));
	  for (a=0; a<26; a++)alp[a+'a']=1;
	  alp['b']=0;
	  alp['j']=0;
	  alp['o']=0;
	  alp['u']=0;
	  alp['x']=0;
	  alp['z']=0;
	}
		    
          
      if (r1>0 && r2>0)
	  {
	    int s=0, n=0;
	    char aa1, aa2, aa3, aa4, u;
	      
	    r1--;
	    r2--;
	    
	    if (r1>0 && r2>0)
	      {
		aa1=tolower((CL->S)->seq[s1][r1-1]);
		aa2=tolower((CL->S)->seq[s1][r1]);
		aa3=tolower((CL->S)->seq[s2][r2-1]);
		aa4=tolower((CL->S)->seq[s2][r2]);
		u=alp[(int)aa1];u+=alp[(int)aa2];u+=alp[(int)aa3];u+=alp[(int)aa4];
		if (u==4)
		  {
		    s+=m[aa1-'a'][aa2-'a'][aa3-'a'][aa4-'a'];
		    n++;
		  }
	      }
	    
	    aa1=tolower((CL->S)->seq[s1][r1]);
	    aa2=tolower((CL->S)->seq[s1][r1+1]);
	    aa3=tolower((CL->S)->seq[s2][r2]);
	    aa4=tolower((CL->S)->seq[s2][r2+1]);
	    u=alp[(int)aa1];u+=alp[(int)aa2];u+=alp[(int)aa3];u+=alp[(int)aa4];
	    if (u==4)
	      {
		s+=m[aa1-'a'][aa2-'a'][aa3-'a'][aa4-'a'];
		n++;
	      }
	    if (n)return (s*SCORE_K)/n;
	    else return 0;
	    }
      return 0;}
int evaluate_monoaa_matrix_score ( Constraint_list *CL, int s1, int r1, int s2, int r2)
    {

      static int **m;
      static int *alp;
      
      if (m==NULL)
	{
	  FILE *fp;
	  char k1[2], k2[2];
	  int v1, v2, c;
	  char *buf=NULL;
	  int a;
	  
	  m=(int**)declare_arrayN(2, sizeof (int), 26, 26);
	  fp=vfopen ("monoaa_mat.mat", "r");
	  while ((c=fgetc (fp))!=EOF)
	    {
	      
	      ungetc (c, fp);
	      buf=vfgets(buf, fp);
	     
	      if (c=='#');
	      else 
		{
		  sscanf (buf, "%s %s %d %d", k1, k2, &v1, &v2);
		  
		  m[k1[0]-'a'][k2[0]-'a']=v1;
		  m[k2[0]-'a'][k1[0]-'a']=v1;
		}
	    }
	  vfclose (fp);
	  alp=(int*)vcalloc (256, sizeof (int));
	  for (a=0; a<26; a++)alp[a+'a']=1;
	  alp['b']=0;
	  alp['j']=0;
	  alp['o']=0;
	  alp['u']=0;
	  alp['x']=0;
	  alp['z']=0;
	}
		    
          
      if (r1>0 && r2>0)
	  {
	    int s=0, n=0;
	    char aa1, aa3, u;
	      
	    r1--;
	    r2--;
	    
	    if (r1>0 && r2>0)
	      {
		aa1=tolower((CL->S)->seq[s1][r1]);
		aa3=tolower((CL->S)->seq[s2][r2]);
		u=alp[(int)aa1];u+=alp[(int)aa3];
		if (u==2)
		  {
		    s+=m[aa1-'a'][aa3-'a'];
		    n++;
		  }
	      }
	    
	    if (n)return (s*SCORE_K)/n;
	    else return 0;
	    }
      return 0;}
int evaluate_matrix_score ( Constraint_list *CL, int s1, int r1, int s2, int r2)
    {

      if ( seq2R_template_profile (CL->S,s1) ||seq2R_template_profile (CL->S,s2))
	{
	  return evaluate_aln_profile_score ( CL, s1,r1, s2, r2);
	}
      
      if (r1>0 && r2>0)
	    {
	    r1--;
	    r2--;
	    return CL->M[(CL->S)->seq[s1][r1]-'A'][(CL->S)->seq[s2][r2]-'A']*SCORE_K;
	    }
	else
	    return 0;
    }
int *get_curvature ( int s1, Constraint_list *CL);
int evaluate_curvature_score( Constraint_list *CL, int s1, int r1, int s2, int r2)
{
  static int **st;
  int score;
  CL->gop=0;
  CL->gep=0;
  
  if (!st) st=(int**)vcalloc ((CL->S)->nseq, sizeof (char*));
  if (!st[s1])
    {
      st[s1]=get_curvature (s1, CL);
    }
  if (!st[s2])
    {
      st[s2]=get_curvature (s2, CL);
    }
  
  if (r1>0 && r2>0)
    {
      char p1, p2;
      
      r1--;
      r2--;
  
      p1=st[s1][r1];
      p2=st[s2][r2];
      
      score=p1-p2;
      score=FABS(score);
      score=20-score;
      return score;
    }
  else
    {
      return 0;
    }
  
}
int *get_curvature ( int s1, Constraint_list *CL)
{
  int *array, n=0, a;
  char c, name [1000], b1[100];
  float f;
  FILE *fp;
  
  sprintf ( name, "%s.curvature", (CL->S)->name[s1]);
  array=(int*)vcalloc (strlen ((CL->S)->seq[s1]), sizeof (int));
  fp=vfopen ( name, "r");
  while ( fscanf (fp, "%s %d %c %f\n",b1, &a, &c,&f )==4)
    {
      if ( c!=(CL->S)->seq[s1][n]){HERE ("ERROR: %c %c", c,(CL->S)->seq[s1][n] );myexit (0);}
      else array[n++]=(int)(float)100*(float)f;
    }
  vfclose (fp);
  return array;
}
int evaluate_tm_matrix_score ( Constraint_list *CL, int s1, int r1, int s2, int r2)
{
  static char **st;
  float F, RF;
  
  if (!st) 
    {
      st=(char**)vcalloc ((CL->S)->nseq, sizeof (char*));
      RF=atoigetenv ("TM_FACTOR_4_TCOFFEE");
      if ( !RF)RF=10;
    }
  
  if (!st[s1])st[s1]=seq2T_template_string((CL->S),s1);
  if (!st[s2])st[s2]=seq2T_template_string((CL->S),s2);
  
  if (r1<=0 || r2<=0)return 0;
  else if (st[s1] && st[s2] && (st[s1][r1-1]==st[s2][r2-1]))F=RF;
  else if (st[s1] && st[s2] && (st[s1][r1-1]!=st[s2][r2-1]))F=0;
  else F=0;

  return (int)(evaluate_matrix_score (CL, s1, r1, s2, r2))+F;
}
int evaluate_ssp_matrix_score ( Constraint_list *CL, int s1, int r1, int s2, int r2)
{
  static char **st;
  float F, RF;
  
  if (!st) 
    {
      st=(char**) vcalloc ((CL->S)->nseq, sizeof (char*));
      RF=atoigetenv ("SSP_FACTOR_4_TCOFFEE");
      if ( !RF)RF=10;
    }
  
  if (!st[s1])st[s1]=seq2T_template_string((CL->S),s1);
  if (!st[s2])st[s2]=seq2T_template_string((CL->S),s2);
  
  if (r1<=0 || r2<=0)return 0;
  else if (st[s1] && st[s2] && (st[s1][r1-1]==st[s2][r2-1]))F=RF;
  else if (st[s1] && st[s2] && (st[s1][r1-1]!=st[s2][r2-1]))F=0;
  else F=0;

  return (int)(evaluate_matrix_score (CL, s1, r1, s2, r2))+F;
}      
      
  
int evaluate_combined_matrix_score ( Constraint_list *CL, int s1, int r1, int s2, int r2)
    {
      /*
	function documentation: start
	
	int evaluate_matrix_score ( Constraint_list *CL, int s1, int s2, int r1, int r2)
	
	this function evaluates the score for matching residue S1(r1) wit residue S2(r2)
	using Matrix CL->M;
	
	function documentation: end
      */

      if ( seq2R_template_profile (CL->S,s1) ||seq2R_template_profile (CL->S,s2))
	{
	  return evaluate_aln_profile_score ( CL, s1,r1, s2, r2);
	}
    
      if (r1>0 && r2>0)
	    {
	    r1--;
	    r2--;
	    if (r1==0 || r2==0)return CL->M[(CL->S)->seq[s1][r1]-'A'][(CL->S)->seq[s2][r2]-'A']*SCORE_K;
	    else
	      {
		int A1, A2, B1, B2, a2, b2;
		int score;
		
		A1=toupper((CL->S)->seq[s1][r1-1]);
		A2=toupper((CL->S)->seq[s1][r1]);
		B1=toupper((CL->S)->seq[s2][r2-1]);
		B2=toupper((CL->S)->seq[s2][r2]);
		
		a2=tolower(A2);
		b2=tolower(B2);
		A1-='A';A2-='A';B1-='A'; B2-='A';a2-='A';b2-='A';
		
		score=CL->M[a2][b2]-FABS((CL->M[A1][A2])-(CL->M[B1][B2]));
		score*=SCORE_K;
		return score;
	      }
	    }
      else
	return 0;
    }	    

int residue_pair_non_extended_list ( Constraint_list *CL, int s1, int r1, int s2, int r2 )
        {
		
	/*
	  This is the generic Function->works with everything
	  	  
	  int residue_pair_non_extended_list ( Constraint_list *CL, int s1, int r1, int s2, int r2, int field );
	  
	  Computes the non extended score for aligning residue seq1(r1) Vs seq2(r2)
	  
	  This function can compare a sequence with itself.
	  
	  Associated functions: See util constraint list, list extention functions.
	  function documentation: end
	*/

	int p;


	static int *entry;
	int *r;
	int field;

	field = CL->weight_field;


	if ( r1<=0 || r2<=0)return 0;
	else if ( !CL->extend_jit)
	   {
	    if ( !entry) entry=(int*)vcalloc (LIST_N_FIELDS , sizeof (int));
	    entry[SEQ1]=s1;
	    entry[SEQ2]=s2;
	    entry[R1]=r1;
	    entry[R2]=r2;
	    if ( r1==r2 && s1==s2) return UNDEFINED;
	    r=main_search_in_list_constraint( entry,&p,4,CL);
	    if (r==NULL)return 0;
	    else return r[field]*SCORE_K;
	   }
	else
	  return UNDEFINED;/*ERROR*/

	
	}



int residue_pair_extended_list_mixt (Constraint_list *CL, int s1, int r1, int s2, int r2 )
        {
	  int score=0;
	  
	  score+= residue_pair_extended_list_quadruplet(CL, s1, r1, s2, r2);
	  score+= residue_pair_extended_list (CL, s1, r1, s2, r2);
	  
	  return score*SCORE_K;
	}

int residue_pair_extended_list_quadruplet (Constraint_list *CL, int s1, int r1, int s2, int r2 )
        {	
	  double score=0;

	  int t_s, t_r, t_w, q_s, q_r, q_w;
	  int a, b;
	  static int **hasch;
	  
	  int field;
	  /* This measure the quadruplets cost on a pair of residues*/
	  
	  
	  
	  field=CL->weight_field;
	  
	  if ( r1<=0 || r2<=0)return 0;
	  if ( !hasch)
	    {
	      hasch=(int**)vcalloc ( (CL->S)->nseq, sizeof (int*));
	      for ( a=0; a< (CL->S)->nseq; a++)hasch[a]=(int*)vcalloc ( (CL->S)->len[a]+1, sizeof (int));
	    }
	  

	  hasch[s1][r1]=100000;
	  for (a=1; a< CL->residue_index[s1][r1][0]; a+=ICHUNK)
	    {
	      t_s=CL->residue_index[s1][r1][a+SEQ2];
	      t_r=CL->residue_index[s1][r1][a+R2];
	      t_w=CL->residue_index[s1][r1][a+WE];
	      if ( CL->seq_for_quadruplet[t_s])
		{
		  for ( b=1; b<CL->residue_index[t_s][t_r][0]; b+=ICHUNK)
		    {
		      q_s=CL->residue_index[t_s][t_r][b+SEQ2];		  
		      q_r=CL->residue_index[t_s][t_r][b+R2];
		      q_w=CL->residue_index[t_s][t_r][b+WE];
		      if (CL-> seq_for_quadruplet[q_s])
			hasch[q_s][q_r]=MIN(q_w,t_w);
		      
		    }
		}
	    }
	  
	  
	  for (a=1; a< CL->residue_index[s2][r2][0]; a+=ICHUNK) 
	    {
	      t_s=CL->residue_index[s2][r2][a+SEQ2];
	      t_r=CL->residue_index[s2][r2][a+R2];
	      t_w=CL->residue_index[s2][r2][a+WE];
	      if ( CL->seq_for_quadruplet[t_s])
		{
		  for ( b=1; b<CL->residue_index[t_s][t_r][0]; b+=ICHUNK)
		    {
		      q_s=CL->residue_index[t_s][t_r][b+SEQ2];		  
		      q_r=CL->residue_index[t_s][t_r][b+R2];	
		      q_w=CL->residue_index[t_s][t_r][b+WE];	
		      if (hasch[q_s][q_r] && CL->seq_for_quadruplet[q_s])
			score+=MIN(hasch[q_s][q_r],MIN(q_w,t_w));
		    }
		}
	    }
	  
	  score=(CL->normalise)?((score*CL->normalise)/CL->max_ext_value):score;
	  
	  for (a=1; a< CL->residue_index[s1][r1][0]; a+=ICHUNK)
	    {
	      t_s=CL->residue_index[s1][r1][a+SEQ2];
	      t_r=CL->residue_index[s1][r1][a+R2];
	      t_w=CL->residue_index[s1][r1][a+WE];
	      if ( CL->seq_for_quadruplet[t_s])
		{
		  for ( b=1; b<CL->residue_index[t_s][t_r][0]; b+=ICHUNK)
		    {
		      q_s=CL->residue_index[t_s][t_r][b+SEQ2];		  
		      q_r=CL->residue_index[t_s][t_r][b+R2];		 
		      hasch[q_s][q_r]=0;
		    }
		}
	    }
	  
	  return (int)(score*SCORE_K);
	}  


Constraint_list * R_extension ( Constraint_list *CL, Constraint_list *R);
int residue_pair_extended_list4rna4 (Constraint_list *CL,int s1, int r1, int s2, int r2 )
{
  static int rna_lib;

  if (!rna_lib)
    {
      sprintf ( CL->rna_lib, "%s", seq2rna_lib (CL->S, NULL));
      rna_lib=1;
    }
  return residue_pair_extended_list4rna2 (CL, s1, r1, s2,r2);
}
int residue_pair_extended_list4rna1 (Constraint_list *CL, int s1, int r1, int s2, int r2 )
{
  static Constraint_list *R;
  if (!R)R=read_contact_lib (CL->S, CL->rna_lib,NULL);
  return residue_pair_extended_list4rna (CL, R, s1, r1, s2, r2);
}

int residue_pair_extended_list4rna3 (Constraint_list *CL,int s1, int r1, int s2, int r2 )
{
  static Constraint_list *R;
  if (!R)
    {
      R=read_contact_lib (CL->S, CL->rna_lib,NULL);
      rna_lib_extension (CL,R);
    }
  return residue_pair_extended_list (CL, s1,r1, s2,r2);
}

int residue_pair_extended_list4rna2 (Constraint_list *CL,int s1, int r1, int s2, int r2 )
{
  static Constraint_list *R;

  if (!R)
    {
      R=read_contact_lib (CL->S, CL->rna_lib,NULL );
      rna_lib_extension (CL,R);

    }
  
  return residue_pair_extended_list4rna (CL, R, s1, r1, s2, r2);
}
int residue_pair_extended_list4rna ( Constraint_list *CL,Constraint_list *R, int s1, int r1, int s2, int r2 )
{
  
  int a, b, n1, n2;
  int list1[100];
  int list2[100];
  int score=0, score2;
  
  

  if ( r1<0 || r2<0)return 0;
  n1=n2=0;
  
  list1[n1++]=r1;
  for (a=1; a<R->residue_index[s1][r1][0]; a+=ICHUNK)
    {
      list1[n1++]=R->residue_index[s1][r1][a+R2];
    }
  

  list2[n2++]=r2;
  for (a=1; a<R->residue_index[s2][r2][0]; a+=ICHUNK)
    {
      list2[n2++]=R->residue_index[s2][r2][a+R2];
    }
  
  
  score=residue_pair_extended_list ( CL, s1,list1[0], s2,list2[0]);
  
  for (score2=0,a=1; a<n1; a++)
    for ( b=1; b<n2; b++)
      score2=MAX(residue_pair_extended_list ( CL, s1,list1[a], s2,list2[b]), score2);
  
  if ( n1==1 || n2==1);
  else score=MAX(score,score2);
  
  return score;
}


int residue_pair_extended_list4rna_ref ( Constraint_list *CL, int s1, int r1, int s2, int r2 )
{
  static Constraint_list *R;
  int a, b, n1, n2;
  int list1[100];
  int list2[100];
  int score=0, score2;
  
  if (R==NULL)
    {
      char **list;
      int n,a;
      list=read_lib_list ( CL->rna_lib, &n);
      
      R=declare_constraint_list ( CL->S,NULL, NULL, 0,NULL, NULL); 
      
      for (a=0; a< n; a++)
	{
	  R=read_constraint_list_file (R, list[a]);
	}
     
    }

  if ( r1<0 || r2<0)return 0;
  n1=n2=0;

  list1[n1++]=r1;
  for (a=1; a<R->residue_index[s1][r1][0]; a+=ICHUNK)
    {
      list1[n1++]=R->residue_index[s1][r1][a+R2];
    }
  

  list2[n2++]=r2;
  for (a=1; a<R->residue_index[s2][r2][0]; a+=ICHUNK)
    {
      list2[n2++]=R->residue_index[s2][r2][a+R2];
    }
  
  
  score=residue_pair_extended_list ( CL, s1,list1[0], s2,list2[0]);
  
  for (score2=0,a=1; a<n1; a++)
    for ( b=1; b<n2; b++)
      score2=MAX(residue_pair_extended_list ( CL, s1,list1[a], s2,list2[b]), score2);

  if ( n1==1 || n2==1);
  else score=MAX(score,score2);
  
  return score;
}

static int ** clean_residue_pair_hasch (int s1, int r1, int s2, int r2,int **hasch, Constraint_list *CL);
int residue_pair_extended_list_raw ( Constraint_list *CL, int s1, int r1, int s2, int r2 )
        {
	double score=0;  



	int a, t_s, t_r;
	static int **hasch;
	static int max_len;
	int field;
	double delta;
	/*
	  
	  function documentation: start

	  int residue_pair_extended_list ( Constraint_list *CL, int s1, int r1, int s2, int r2);
	  
	  Computes the extended score for aligning residue seq1(r1) Vs seq2(r2)
	  Computes: matrix_score
	            non extended score
		    extended score

	  The extended score depends on the function index_res_constraint_list.
	  This function can compare a sequence with itself.
	  
	  Associated functions: See util constraint list, list extention functions.
	  
	  function documentation: end
	*/
	
	field=CL->weight_field;
	field=WE;

	if ( r1<=0 || r2<=0)return 0;
	if ( !hasch || max_len!=(CL->S)->max_len)
	       {
	       max_len=(CL->S)->max_len;
	       if ( hasch) free_int ( hasch, -1);
	       hasch=declare_int ( (CL->S)->nseq, (CL->S)->max_len+1);
	       }
	


	/* Check matches for R1 in the indexed lib*/
	hasch[s1][r1]=FORBIDEN;
	for (a=1; a< CL->residue_index[s1][r1][0]; a+=ICHUNK)
	  {
	    t_s=CL->residue_index[s1][r1][a+SEQ2];
	    t_r=CL->residue_index[s1][r1][a+R2];
	    hasch[t_s][t_r]=CL->residue_index[s1][r1][a+field];
	  }

	/*Check Matches for r1 <-> r2 in the indexed lib */
	for (a=1; a< CL->residue_index[s2][r2][0]; a+=ICHUNK) 
	  {
	    t_s=CL->residue_index[s2][r2][a+SEQ2];
	    t_r=CL->residue_index[s2][r2][a+R2];
	    
	    
	    if (hasch[t_s][t_r])
	      {
		if (hasch[t_s][t_r]==FORBIDEN)
		  {
		    score+=CL->residue_index[s2][r2][a+field];
		  }
		else 
		  {
		    delta=MIN(hasch[t_s][t_r],CL->residue_index[s2][r2][a+field]);
		    score+=delta;
		  }
	      }
	  }

	clean_residue_pair_hasch ( s1, r1,s2, r2, hasch, CL);		
	return score;
	}
int residue_pair_extended_list_4gp ( Constraint_list *CL, int s1, int r1, int s2, int r2 )
        {
	  double score=0;  



	int a, t_s, t_r;
	static int **hasch;
	static int max_len;
	int field;
	double delta;
	
	/*
	  
	  function documentation: start

	  int residue_pair_extended_list ( Constraint_list *CL, int s1, int r1, int s2, int r2);
	  
	  Computes the extended score for aligning residue seq1(r1) Vs seq2(r2)
	  Computes: matrix_score
	            non extended score
		    extended score

	  The extended score depends on the function index_res_constraint_list.
	  This function can compare a sequence with itself.
	  
	  Associated functions: See util constraint list, list extention functions.
	  
	  function documentation: end
	*/
	
	field=CL->weight_field;
	field=WE;
	
	if ( r1<=0 || r2<=0)return 0;
	if ( !hasch || max_len!=(CL->S)->max_len)
	       {
	       max_len=(CL->S)->max_len;
	       if ( hasch) free_int ( hasch, -1);
	       hasch=declare_int ( (CL->S)->nseq, (CL->S)->max_len+1);
	       }
	


	/* Check matches for R1 in the indexed lib*/
	hasch[s1][r1]=FORBIDEN;
	for (a=1; a< CL->residue_index[s1][r1][0]; a+=ICHUNK)
	  {
	    t_s=CL->residue_index[s1][r1][a+SEQ2];
	    t_r=CL->residue_index[s1][r1][a+R2];
	    hasch[t_s][t_r]=CL->residue_index[s1][r1][a+field];
	  }

	/*Check Matches for r1 <-> r2 in the indexed lib */
	for (a=1; a< CL->residue_index[s2][r2][0]; a+=ICHUNK) 
	  {
	    t_s=CL->residue_index[s2][r2][a+SEQ2];
	    t_r=CL->residue_index[s2][r2][a+R2];
	    
	    
	    if (hasch[t_s][t_r])
	      {
		if (hasch[t_s][t_r]==FORBIDEN)
		  {
		    score+=((float)CL->residue_index[s2][r2][a+2]/NORM_F);
		  }
		else 
		  {
		    delta=MAX((((float)hasch[t_s][t_r]/NORM_F)),(((float)CL->residue_index[s2][r2][a+field]/NORM_F)));
		    score+=delta;
		  }
	      }
	  }

	clean_residue_pair_hasch ( s1, r1,s2, r2, hasch, CL);	
	score/=(CL->S)->nseq;
	score *=NORM_F;
	
	return score;
	}

int residue_pair_extended_list_pc ( Constraint_list *CL, int s1, int r1, int s2, int r2 )
        {
	  //old norm does not take into account the number of effective intermediate sequences
	  
	  double score=0;  
	  int a, t_s, t_r;
	  static int **hasch;
	  static int max_len;
	  int field;
	  double delta;
	  float norm1=0;
	  float norm2=0;
	  
	  /*
	    
	  function documentation: start
	  
	  int residue_pair_extended_list ( Constraint_list *CL, int s1, int r1, int s2, int r2);
	  
	  Computes the extended score for aligning residue seq1(r1) Vs seq2(r2)
	  Computes: matrix_score
	            non extended score
		    extended score

	  The extended score depends on the function index_res_constraint_list.
	  This function can compare a sequence with itself.
	  
	  Associated functions: See util constraint list, list extention functions.
	  
	  function documentation: end
	*/
	
	field=CL->weight_field;
	field=WE;
	
	if ( r1<=0 || r2<=0)return 0;
	if ( !hasch || max_len!=(CL->S)->max_len)
	       {
	       max_len=(CL->S)->max_len;
	       if ( hasch) free_int ( hasch, -1);
	       hasch=declare_int ( (CL->S)->nseq, (CL->S)->max_len+1);
	       
	       }


	/* Check matches for R1 in the indexed lib*/
	hasch[s1][r1]=FORBIDEN;
	for (a=1; a< CL->residue_index[s1][r1][0]; a+=ICHUNK)
	  {
	    t_s=CL->residue_index[s1][r1][a+SEQ2];
	    t_r=CL->residue_index[s1][r1][a+R2];
	    hasch[t_s][t_r]=CL->residue_index[s1][r1][a+field];
	    norm1++;
	    
	  }
	
	
	/*Check Matches for r1 <-> r2 in the indexed lib */
	for (a=1; a< CL->residue_index[s2][r2][0]; a+=ICHUNK) 
	  {
	    t_s=CL->residue_index[s2][r2][a+SEQ2];
	    t_r=CL->residue_index[s2][r2][a+R2];
	    norm2++;
	    
	    if (hasch[t_s][t_r])
	      {
		if (hasch[t_s][t_r]==FORBIDEN)
		  {
		    score+=((float)CL->residue_index[s2][r2][a+field]/NORM_F);
		  }
		else 
		  {
		    delta=MIN((((float)hasch[t_s][t_r]/NORM_F)),(((float)CL->residue_index[s2][r2][a+field]/NORM_F)));
		    score+=delta;
		  }
	      }
	  }

	clean_residue_pair_hasch ( s1, r1,s2, r2, hasch, CL);	
	
	//New Normalization
	norm1=MIN(norm1,norm2);
		
	//Old Normalization: on the number of sequences, useless when not doing an all against all
	//Old NORM seems to work better after all (23/03/2011)
	//norm2=(CL->S)->nseq;//OLD NORM (seems to work better: 23/03/2011)

	score=(norm1)?score/norm1:0;
	

	return score*NORM_F;
	}

/**
 * Documentation inaccurate.
 * 
 * These lines of documentation are in the code:
 * 
 * Computes the extended score for aligning residue seq1(r1) Vs seq2(r2)
 * Computes: 
 *     - matrix_score
 *     - non extended score
 *     - extended score
 *     
 * The extended score depends on the function index_res_constraint_list.
 * This function can compare a sequence with itself.  
 *
 */
int residue_pair_extended_list ( Constraint_list *CL, int s1, int r1, int s2, int r2 )
        {
	double score=0;  
	double max_score=0;
	double max_val=0;

	int a, t_s, t_r;
	static int **hasch;
	static int max_len;
	int field;
	
	
	field=CL->weight_field;
	field=WE;
	
	if ( r1<=0 || r2<=0)return 0;
	if ( !hasch || max_len!=(CL->S)->max_len)
	       {
	       max_len=(CL->S)->max_len;
	       if ( hasch) free_int ( hasch, -1);
	       hasch=declare_int ( (CL->S)->nseq, (CL->S)->max_len+1);
	       }
	


	/* Check matches for R1 in the indexed lib*/
	hasch[s1][r1]=FORBIDEN;
	for (a=1; a< CL->residue_index[s1][r1][0]; a+=ICHUNK)
	  {
	    t_s=CL->residue_index[s1][r1][a+SEQ2];
	    t_r=CL->residue_index[s1][r1][a+R2];
	    hasch[t_s][t_r]=CL->residue_index[s1][r1][a+field];
	    max_score+=CL->residue_index[s1][r1][a+field];
	  }

	/*Check Matches for r1 <-> r2 in the indexed lib */
	for (a=1; a< CL->residue_index[s2][r2][0]; a+=ICHUNK) 
	  {
	    t_s=CL->residue_index[s2][r2][a+SEQ2];
	    t_r=CL->residue_index[s2][r2][a+R2];
	    
	    
	    if (hasch[t_s][t_r])
	      {
		if (hasch[t_s][t_r]==FORBIDEN)
		  {
		    score+=CL->residue_index[s2][r2][a+field];
		    max_score+=CL->residue_index[s2][r2][a+field];
		  }
		else 
		  {
		    double delta;
		    delta=MIN(hasch[t_s][t_r],CL->residue_index[s2][r2][a+field]);
		    
		    score+=delta;
		    max_score-=hasch[t_s][t_r];
		    max_score+=delta;
		    max_val=MAX(max_val,delta);
		  }
	      }
	    else
	      {
		max_score+=CL->residue_index[s2][r2][a+field];
	      }
	  }

	max_score-=hasch[s2][r2];
	clean_residue_pair_hasch ( s1, r1,s2, r2, hasch, CL);		


	if ( max_score==0)score=0;
	else if ( CL->normalise) 
	  {
	    score=((score*CL->normalise)/max_score)*SCORE_K;
	    if (max_val> CL->normalise)
	      {
		score*=max_val/(double)CL->normalise;
	      }
	  }
	return (int) score;
	}
int ** clean_residue_pair_hasch (int s1, int r1, int s2, int r2,int **hasch, Constraint_list *CL)
  {
    int a, t_s, t_r;
    if ( !hasch) return hasch;
    
    for (a=1; a< CL->residue_index[s1][r1][0]; a+=ICHUNK)
      {
	t_s=CL->residue_index[s1][r1][a+SEQ2];
	t_r=CL->residue_index[s1][r1][a+R2];
	hasch[t_s][t_r]=0;	      
      }
    hasch[s1][r1]=hasch[s2][r2]=0;
    return hasch;
  }

int residue_pair_test_function ( Constraint_list *CL, int s1, int r1, int s2, int r2 )
        {
	  double score=0;

	  int a, t_s, t_r;
	  static int **hasch;
	  static int max_len;
	  int cons1;
	  int cons2;
	  
	  int field;
	/*
	  function documentation: start

	  int residue_pair_extended_list ( Constraint_list *CL, int s1, int r1, int s2, int r2);
	  
	  Computes the extended score for aligning residue seq1(r1) Vs seq2(r2)
	  Computes: matrix_score
	            non extended score
		    extended score

	  The extended score depends on the function index_res_constraint_list.
	  This function can compare a sequence with itself.
	  
	  Associated functions: See util constraint list, list extention functions.
	  
	  function documentation: end
	*/


	CL->weight_field=WE;
	field=CL->weight_field;

	
	if ( r1<=0 || r2<=0)return 0;
	if ( !hasch || max_len!=(CL->S)->max_len)
	       {
	       max_len=(CL->S)->max_len;
	       if ( hasch) free_int ( hasch, -1);
	       hasch=declare_int ( (CL->S)->nseq, (CL->S)->max_len+1);
	       }
	

	
	hasch[s1][r1]=1000;
	for (a=1; a< CL->residue_index[s1][r1][0]; a+=ICHUNK)
	  {
	    t_s=CL->residue_index[s1][r1][a+SEQ2];
	    t_r=CL->residue_index[s1][r1][a+R2];
	    hasch[t_s][t_r]=CL->residue_index[s1][r1][a+field];		
	  }
	
	
	for (a=1; a< CL->residue_index[s2][r2][0]; a+=ICHUNK) 
	  {
	    t_s=CL->residue_index[s2][r2][a+SEQ2];
	    t_r=CL->residue_index[s2][r2][a+R2];
	    if (hasch[t_s][t_r])
	      {
		cons1=hasch[t_s][t_r];
		cons2=CL->residue_index[s2][r2][a+field];
		score +=MIN(cons1,cons2);
	      }
	  }
	
	
	score=(CL->normalise)?((score*CL->normalise)/CL->max_ext_value):score;
	
	for (a=1; a< CL->residue_index[s1][r1][0]; a+=ICHUNK)
	  {
	    t_s=CL->residue_index[s1][r1][a+SEQ2];
	    t_r=CL->residue_index[s1][r1][a+R2];
	    hasch[t_s][t_r]=0;	      
	  }
	hasch[s1][r1]=hasch[s2][r2]=0;

	
	return (int)(score*SCORE_K);
	}

int residue_pair_relative_extended_list ( Constraint_list *CL, int s1, int r1, int s2, int r2 )
        {
	int a, t_s, t_r;
	static int **hasch;
	static int max_len;
	int score=0;
	int total_score=0;
	int field;
	/*
	  function documentation: start

	  int residue_pair_extended_list ( Constraint_list *CL, int s1, int r1, int s2, int r2);
	  
	  Computes the extended score for aligning residue seq1(r1) Vs seq2(r2)
	  Computes: matrix_score
	            non extended score
		    extended score

	  The extended score depends on the function index_res_constraint_list.
	  This function can compare a sequence with itself.
	  
	  Associated functions: See util constraint list, list extention functions.
	  
	  function documentation: end
	*/



	field=CL->weight_field;
	field=WE;
	
	if ( r1<=0 || r2<=0)return 0;
	if ( !hasch || max_len!=(CL->S)->max_len)
	       {
	       max_len=(CL->S)->max_len;
	       if ( hasch) free_int ( hasch, -1);
	       hasch=declare_int ( (CL->S)->nseq, (CL->S)->max_len+1);
	       }
	

	
	hasch[s1][r1]=100000;
	for (a=1; a< CL->residue_index[s1][r1][0]; a+=ICHUNK)
	  {
	    t_s=CL->residue_index[s1][r1][a+SEQ2];
	    t_r=CL->residue_index[s1][r1][a+R2];
	    hasch[t_s][t_r]=CL->residue_index[s1][r1][a+field];		
	    total_score+=CL->residue_index[s1][r1][a+field];
	  }
	
	
	for (a=1; a< CL->residue_index[s2][r2][0]; a+=ICHUNK) 
	  {
	    t_s=CL->residue_index[s2][r2][a+SEQ2];
	    t_r=CL->residue_index[s2][r2][a+R2];
	    total_score+=CL->residue_index[s1][r1][a+field];
	    if (hasch[t_s][t_r])
	      {
		if (field==WE){score+=2*MIN(hasch[t_s][t_r],CL->residue_index[s2][r2][a+field]);}
	      }
	  }
	
	score=((CL->normalise*score)/total_score);
	
	
	for (a=1; a< CL->residue_index[s1][r1][0]; a+=ICHUNK)
	  {
	    t_s=CL->residue_index[s1][r1][a+SEQ2];
	    t_r=CL->residue_index[s1][r1][a+R2];
	    hasch[t_s][t_r]=0;	      
	  }
	hasch[s1][r1]=hasch[s2][r2]=0;

	return score*SCORE_K;
	}
int residue_pair_extended_list_g_coffee_quadruplet ( Constraint_list *CL, int s1, int r1, int s2, int r2 )
{
   int t_s, t_r, t_w, q_s, q_r, q_w;
	  int a, b;
	  static int **hasch;
	  int score=0, s=0;
	  
	  int field;
	  /* This measure the quadruplets cost on a pair of residues*/
	  
	  field=CL->weight_field;
	  
	  if ( r1<=0 || r2<=0)return 0;
	  if ( !hasch)
	    {
	      hasch=(int**)vcalloc ( (CL->S)->nseq, sizeof (int*));
	      for ( a=0; a< (CL->S)->nseq; a++)hasch[a]=(int*)vcalloc ( (CL->S)->len[a]+1, sizeof (int));
	    }
	  

	  hasch[s1][r1]=100000;
	  for (a=1; a< CL->residue_index[s1][r1][0]; a+=ICHUNK)
	    {
	      t_s=CL->residue_index[s1][r1][a+SEQ2];
	      t_r=CL->residue_index[s1][r1][a+R2];
	      t_w=CL->residue_index[s1][r1][a+field];
	      if ( CL->seq_for_quadruplet[t_s])
		{
		  for ( b=1; b<CL->residue_index[t_s][t_r][0]; b+=ICHUNK)
		    {
		      q_s=CL->residue_index[t_s][t_r][b+SEQ2];		  
		      q_r=CL->residue_index[t_s][t_r][b+R2];		 
		      if (CL-> seq_for_quadruplet[q_s])
			{
			  
			  hasch[q_s][q_r]=MIN(CL->residue_index[t_s][t_r][b+2],t_w);
			}
		    }
		}
	    }
	  
	  
	  for (s=0,score=0,a=1; a< CL->residue_index[s2][r2][0]; a+=ICHUNK) 
	    {
	      t_s=CL->residue_index[s2][r2][a+SEQ2];
	      t_r=CL->residue_index[s2][r2][a+R2];
	      t_w=CL->residue_index[s2][r2][a+field];
	      if ( CL->seq_for_quadruplet[t_s])
		{
		  for ( b=1; b<CL->residue_index[t_s][t_r][0]; b+=ICHUNK)
		    {
		      q_s=CL->residue_index[t_s][t_r][b+SEQ2];		  
		      q_r=CL->residue_index[t_s][t_r][b+R2];	
		      q_w=CL->residue_index[t_s][t_r][b+field];	
		      if (hasch[q_s][q_r] && CL->seq_for_quadruplet[q_s])
			s=MIN(hasch[q_s][q_r],MIN(CL->residue_index[t_s][t_r][b+2],q_w));
		      score=MAX(score, s);
		    }
		}
	    }
	  
	  for (a=1; a< CL->residue_index[s1][r1][0]; a+=ICHUNK)
	    {
	      t_s=CL->residue_index[s1][r1][a+SEQ2];
	      t_r=CL->residue_index[s1][r1][a+R2];
	      t_w=CL->residue_index[s1][r1][a+field];
	      if ( CL->seq_for_quadruplet[t_s])
		{
		  for ( b=1; b<CL->residue_index[t_s][t_r][0]; b+=ICHUNK)
		    {
		      q_s=CL->residue_index[t_s][t_r][b+SEQ2];		  
		      q_r=CL->residue_index[t_s][t_r][b+R2];		 
		      hasch[q_s][q_r]=0;
		    }
		}
	    }

	  return score*SCORE_K;
	}  
int residue_pair_extended_list_g_coffee ( Constraint_list *CL, int s1, int r1, int s2, int r2 )
        {
	int a, t_s, t_r;
	static int **hasch;
	int score=0,s;

	int field;
	/*
	  function documentation: start

	  int residue_pair_extended_list ( Constraint_list *CL, int s1, int r1, int s2, int r2);
	  
	  Computes the extended score for aligning residue seq1(r1) Vs seq2(r2)
	  Computes: matrix_score
	            non extended score
		    extended score

	  The extended score depends on the function index_res_constraint_list.
	  This function can compare a sequence with itself.
	  
	  Associated functions: See util constraint list, list extention functions.
	  
	  function documentation: end
	*/

	field=CL->weight_field;

	if ( r1<=0 || r2<=0)return 0;
	if ( !hasch)
	       {
	       hasch=(int**)vcalloc ( (CL->S)->nseq, sizeof (int*));
	       for ( a=0; a< (CL->S)->nseq; a++)hasch[a]=(int*)vcalloc ( (CL->S)->len[a]+1, sizeof (int));
	       }
	

	
	hasch[s1][r1]=100000;
	for (a=1; a< CL->residue_index[s1][r1][0]; a+=ICHUNK)
	  {
	    t_s=CL->residue_index[s1][r1][a+SEQ2];
	    t_r=CL->residue_index[s1][r1][a+R2];
	    hasch[t_s][t_r]=CL->residue_index[s1][r1][a+field];		
	  }
	
	
	for (s=0, score=0,a=1; a< CL->residue_index[s2][r2][0]; a+=ICHUNK) 
	  {
	    t_s=CL->residue_index[s2][r2][a+SEQ2];
	    t_r=CL->residue_index[s2][r2][a+R2];

	    if (hasch[t_s][t_r])
	      {
		if (field==WE)
		  {s=MIN(hasch[t_s][t_r],CL->residue_index[s2][r2][a+WE]);
		   score=MAX(s,score);
		  }
	      }
	  }
	
	for (a=1; a< CL->residue_index[s1][r1][0]; a+=ICHUNK)
	  {
	    t_s=CL->residue_index[s1][r1][a+SEQ2];
	    t_r=CL->residue_index[s1][r1][a+R2];
	    hasch[t_s][t_r]=0;	      
	  }
	hasch[s1][r1]=hasch[s2][r2]=0;

	return score*SCORE_K;
	} 

int extend_residue_pair ( Constraint_list *CL, int s1, int r1, int s2, int r2)
        {
	double score=0; 
	
	int a, t_s, t_r, p;
	static int **hasch;
	
	static int *entry;
	int *r;
	int field;


	
	/*
	  This is the generic Function->works with everything
	  should be gradually phased out

	  
	  int extend_residue_pair ( Constraint_list *CL, int s1, int r1, int s2, int r2, int field )
	  
	  Computes the extended score for aligning residue seq1(r1) Vs seq2(r2)
	  Computes: matrix_score
	            non extended score
		    extended score

	  The extended score depends on the function index_res_constraint_list.
	  This function can compare a sequence with itself.
	  
	  Associated functions: See util constraint list, list extention functions.
	  function documentation: end
	*/


	field=CL->weight_field;

	if ( r1<=0 || r2<=0)return 0;
	else if ( !CL->residue_index && CL->M)
	   {
	    return evaluate_matrix_score (CL, s1,r1, s2, r2);	
	   }
	
	else if ( !CL->extend_jit)
	   {
	    if ( !entry) entry=(int*)vcalloc (LIST_N_FIELDS , sizeof (int));
	    entry[SEQ1]=s1;
	    entry[SEQ2]=s2;
	    entry[R1]=r1;
	    entry[R2]=r2;
	    r=main_search_in_list_constraint( entry,&p,4,CL);
	    if (r==NULL)return 0;
	    else return r[field];
	   }
	else
	   {
	   if ( !hasch)
	       {
	       hasch=(int**)vcalloc ( (CL->S)->nseq, sizeof (int*));
	       for ( a=0; a< (CL->S)->nseq; a++)hasch[a]=(int*)vcalloc ( (CL->S)->len[a]+1, sizeof (int));
	       }
	

	
	   hasch[s1][r1]=100000;
	   for (a=1; a< CL->residue_index[s1][r1][0]; a+=ICHUNK)
	        {
		t_s=CL->residue_index[s1][r1][a+SEQ2];
		t_r=CL->residue_index[s1][r1][a+R2];
		hasch[t_s][t_r]=CL->residue_index[s1][r1][a+WE];		
		}
	
	
	   for (a=1; a< CL->residue_index[s2][r2][0]; a+=ICHUNK) 
	       {
	       t_s=CL->residue_index[s2][r2][a+SEQ2];
	       t_r=CL->residue_index[s2][r2][a+R2];
	       if (hasch[t_s][t_r])
		   {
		   if (field==WE)score+=MIN(hasch[t_s][t_r],CL->residue_index[s2][r2][a+WE] );

		   }
	       }
	   score=(CL->normalise)?((score*CL->normalise)/CL->max_ext_value):score;
	   for (a=1; a< CL->residue_index[s1][r1][0]; a+=ICHUNK)
	       {
	       t_s=CL->residue_index[s1][r1][a+SEQ2];
	       t_r=CL->residue_index[s1][r1][a+R2];
	       hasch[t_s][t_r]=0;	      
	       }
	   hasch[s1][r1]=hasch[s2][r2]=0;

	   return (int)(score*SCORE_K);
	   }
	}
/*********************************************************************************************/
/*                                                                                           */
/*         FUNCTIONS FOR GETTING THE PW COST :  CL->get_dp_cost                              */
/*                                                                                           */
/*********************************************************************************************/
int get_dp_cost_blosum_matrix (Alignment *A, int**pos1, int ns1, int*list1, int col1, int**pos2, int ns2, int*list2, int col2, Constraint_list *CL)
{
  int s1, r1, s2, r2;
  static int **matrix;

  if (!matrix) matrix=read_matrice ("blosum62mt");
  s1=A->order[list1[0]][0];
  s2=A->order[list2[0]][0];
  r1=pos1[list1[0]][col1];
  r2=pos2[list2[0]][col2];

  /*dp cost function: works only with two sequences*/
  
  if ( seq2R_template_profile (CL->S,s1) ||seq2R_template_profile (CL->S,s2))
    return evaluate_aln_profile_score ( CL, s1,r1, s2, r2) -CL->nomatch*SCORE_K;
  else if (r1>0 && r2>0)
	    {
	    r1--;
	    r2--;
	    
	    
	    return matrix [(CL->S)->seq[s1][r1]-'A'][(CL->S)->seq[s2][r2]-'A']*SCORE_K -CL->nomatch*SCORE_K;
	    
	    }
  else
    return -CL->nomatch*SCORE_K ;
}
int get_dp_cost_pam_matrix (Alignment *A, int**pos1, int ns1, int*list1, int col1, int**pos2, int ns2, int*list2, int col2, Constraint_list *CL)
{
  int s1, r1, s2, r2;
  static int **matrix;
  
  if (!matrix) matrix=read_matrice ("pam250mt");
  s1=A->order[list1[0]][0];
  s2=A->order[list2[0]][0];
  r1=pos1[list1[0]][col1];
  r2=pos2[list2[0]][col2];

  /*dp cost function: works only with two sequences*/
  

  if ( seq2R_template_profile (CL->S,s1) ||seq2R_template_profile (CL->S,s2))
    return evaluate_aln_profile_score ( CL, s1,r1, s2, r2) -CL->nomatch*SCORE_K;
  else if (r1>0 && r2>0)
	    {
	    r1--;
	    r2--;
	    
	    
	    return matrix [(CL->S)->seq[s1][r1]-'A'][(CL->S)->seq[s2][r2]-'A']*SCORE_K -CL->nomatch*SCORE_K;
	    
	    }
  else
    return -CL->nomatch*SCORE_K ;
}

int get_dp_cost_pw_matrix (Alignment *A, int**pos1, int ns1, int*list1, int col1, int**pos2, int ns2, int*list2, int col2, Constraint_list *CL)
{
  int s1, r1, s2, r2;
   
  s1=A->order[list1[0]][0];
  s2=A->order[list2[0]][0];
  r1=pos1[list1[0]][col1];
  r2=pos2[list2[0]][col2];

  /*dp cost function: works only with two sequences*/
  if ( seq2R_template_profile (CL->S,s1) ||seq2R_template_profile (CL->S,s2))
    return evaluate_aln_profile_score ( CL, s1,r1, s2, r2) -CL->nomatch*SCORE_K;
  else if (r1>0 && r2>0)
	    {
	    r1--;
	    r2--;
	    
	    
	    return CL->M[(CL->S)->seq[s1][r1]-'A'][(CL->S)->seq[s2][r2]-'A']*SCORE_K -CL->nomatch*SCORE_K;
	    
	    }
  else
    return -CL->nomatch*SCORE_K ;
}

/*********************************************************************************************/
/*                                                                                           */
/*         FUNCTIONS FOR GETTING THE COST :  CL->get_dp_cost                                     */
/*                                                                                           */
/*********************************************************************************************/



int get_cdna_best_frame_dp_cost (Alignment *A, int**pos1, int ns1, int*list1, int col1, int**pos2, int ns2, int*list2, int col2, Constraint_list *CL)
    {
	int a, b;
	int n=4;
	int s;
	char a1, a2;
	static int l1, l2;
	static Alignment *B;
	static int **score;
	
	if ( !score)score=declare_int(3, 2);
	
	if (!A)
	   {
	       free_aln(B);
	       B=NULL;
	       return UNDEFINED;
	   }
	if (!B)
	   {
	       if (ns1+ns2>2){fprintf ( stderr, "\nERROR: get_cdna_dp_cost mode is only for pair-wise ALN [FATAL]\n");crash("");} 
	       free_aln (B);
	       B=copy_aln (A, NULL);
	       
	       l1=(int)strlen ( A->seq_al[list1[0]]);
	       for ( b=0; b<l1; b++)
		       B->seq_al[list1[0]][b]=translate_dna_codon (A->seq_al[list1[0]]+b, 'x');
	       l2=(int)strlen ( A->seq_al[list2[0]]);
	       for ( b=0; b<l2; b++)
		       B->seq_al[list2[0]][b]=translate_dna_codon (A->seq_al[list2[0]]+b, 'x');
	   }
	
/*Set the frame*/

	for ( a=0; a< 3; a++)score[a][0]=score[a][1]=0;
	for ( a=col1-(n*3),b=col2-(n*3); a<col1+(n*3) ; a++, b++)
	        {
		    if ( a<0 || b<0 || a>=l1 || b>=l2)continue;
		    
		    a1=tolower(B->seq_al[list1[0]][a]);
		    a2=tolower(B->seq_al[list2[0]][b]);
		    
		    score[a%3][0]+=(a1=='x' || a2=='x')?0:CL->M[a1-'A'][a2-'A'];
		    score[a%3][1]++;
		 }
	
	for ( a=0; a< 3; a++)score[a][0]=(score[a][1]>0)?(score[a][0]/score[a][1]):0;
	if ( score[0][0]>score[1][0] &&  score[0][0]>score[2][0])
	    s=score[0][0];
	else if ( score[1][0]>score[0][0] &&  score[1][0]>score[2][0])
	    s=score[1][0];
	else s=score[2][0];
	
	return s*SCORE_K;
	
	} 

int get_dp_cost_quadruplet ( Alignment *A, int**pos1, int ns1, int*list1, int col1, int**pos2, int ns2, int*list2, int col2, Constraint_list *CL)
        {
	  int score;
	  
	 
	  if ( ns1==1 || ns2==1)
	     score=slow_get_dp_cost ( A, pos1, ns1, list1,col1, pos2, ns2, list2, col2, CL);
	  else
	    score=fast_get_dp_cost_quadruplet ( A, pos1, ns1, list1,col1, pos2, ns2, list2, col2, CL);
	 
	  return (score==UNDEFINED)?UNDEFINED:(score-SCORE_K*CL->nomatch);
	}  
int get_dp_cost ( Alignment *A, int**pos1, int ns1, int*list1, int col1, int**pos2, int ns2, int*list2, int col2, Constraint_list *CL)
        {
	int MODE=0;
	int score;
	


	if (A==NULL)return 0;
	
	if (MODE!=2 || MODE==0  || (!CL->residue_index && CL->M) || (!CL->residue_index && CL->T)|| ns1==1 || ns2==1)
	  score=slow_get_dp_cost ( A, pos1, ns1, list1,col1, pos2, ns2, list2, col2, CL);
	else if (MODE==1 || MODE==2)
	  score=fast_get_dp_cost ( A, pos1, ns1, list1,col1, pos2, ns2, list2, col2, CL);
	else
	  score=0;

	

	return (score==UNDEFINED)?UNDEFINED:(score-SCORE_K*CL->nomatch);
	}
int ***make_cw_lu (int **cons, int l, Constraint_list *CL);
int ***make_cw_lu (int **cons, int l, Constraint_list *CL)
{
  int ***lu;
  int p, a,r;
  
  lu=(int***)declare_arrayN(3, sizeof (int),l,103, 2);
  for ( p=0; p<l ; p++)
    {
      for (r=0; r<26; r++)
	{
	  for (a=3; a<cons[p][1]; a+=3)
	    {
	      lu[p][r][0]+=cons[p][a+1]*CL->M[r][cons[p][a]-'A'];
	      lu[p][r][1]+=cons[p][a+1];
	    }
	}
    }
  return lu;
}
int id2_profile_get_dp_cost ( Alignment *A, int**pos1, int ns1, int*list1, int col1, int**pos2, int ns2, int*list2, int col2, Constraint_list *CL);    
int cw_profile_get_dp_cost ( Alignment *A, int**pos1, int ns1, int*list1, int col1, int**pos2, int ns2, int*list2, int col2, Constraint_list *CL)
{
	  static int last_tag;
	  static int *pr, ***lu;
	  int score;
	  static int *list[2], ns[2], **cons[2], ref;
	  int  eva_col,ref_col, a, p, r;
	  float t1, t2;
	  
	
	  
	  
	  
	  if (last_tag!=A->random_tag)
	    {
	      int n1, n2;
	      
	      last_tag=A->random_tag;
	      list[0]=list1;ns[0]=ns1;
	      list[1]=list2;ns[1]=ns2;
	      free_int (cons[0],-1);free_int (cons[1],-1);free_arrayN((void*)lu,3);
	      cons[0]=NULL;cons[1]=NULL;lu=NULL;
	      
	      n1=sub_aln2nseq_prf (A, ns[0], list[0]);
	      n2=sub_aln2nseq_prf (A, ns[1], list[1]);
	      if ( n1>1 || n2>1)
		{
		  cons[0]=sub_aln2count_mat2 (A, ns[0], list[0]);
		  cons[1]=sub_aln2count_mat2 (A, ns[1], list[1]);
		  ref=(ns[0]>ns[1])?0:1;
		  lu=make_cw_lu(cons[ref],(int)strlen(A->seq_al[list[ref][0]]), CL);
		}
	    }

	  if (!lu)
	    {
	      char r1, r2;
	      r1=A->seq_al[list1[0]][col1];
	      r2=A->seq_al[list2[0]][col2];
	      if ( r1!='-' && r2!='-')
		return CL->M[r1-'A'][r2-'A']*SCORE_K -SCORE_K*CL->nomatch;
	      else
		return -SCORE_K*CL->nomatch;
	    }
	  else
	    {
	      eva_col= (ref==0)?col2:col1;
	      ref_col= (ref==0)?col1:col2;
	      pr=cons[1-ref][eva_col];
	      t1=t2=0;
	      for (a=3; a< pr[1]; a+=3)
		{
		  r=tolower(pr[a]);
		  p=  pr[a+1];
		  
		  t1+=lu[ref_col][r-'a'][0]*p;
		  t2+=lu[ref_col][r-'a'][1]*p;
		}
	      
	      
	      
	      score=(t2==0)?0:(t1*SCORE_K)/t2;
	      score -=SCORE_K*CL->nomatch;
	      return score;
	    }
}
int cw_profile_get_dp_cost_old ( Alignment *A, int**pos1, int ns1, int*list1, int col1, int**pos2, int ns2, int*list2, int col2, Constraint_list *CL)
{
	  static int last_tag;
	  static int **cons1, **cons2;
	  int score;
	  

	  if (last_tag!=A->random_tag)
	    {
	      last_tag=A->random_tag;
	      free_int (cons1,-1);free_int (cons2,-1);
	      cons1=sub_aln2count_mat2 (A, ns1, list1);
	      cons2=sub_aln2count_mat2 (A, ns2, list2);
	    }
	  score=cw_profile_profile (cons1[col1], cons2[col2], CL)-SCORE_K*CL->nomatch;
	  return score;
}

int id2_profile_get_dp_cost ( Alignment *A, int**pos1, int ns1, int*list1, int col1, int**pos2, int ns2, int*list2, int col2, Constraint_list *CL)
{
	  static int last_tag;
	  static int **cons1, **cons2;
	  int score=0;
	  static int *aa;
	  int a, b, r, *pr[2];
	  
	  if (!aa)aa=(int*)vcalloc (100, sizeof (int));
	  
	  if (last_tag!=A->random_tag)
	    {
	      last_tag=A->random_tag;
	      free_int (cons1,-1);free_int (cons2,-1);
	      cons1=sub_aln2count_mat2 (A, ns1, list1);
	      cons2=sub_aln2count_mat2 (A, ns2, list2);
	    }
	  pr[0]=cons1[col1];
	  pr[1]=cons2[col2];
	  
	  for (a=0; a<2; a++)
	    {
	      for (b=3; b<pr[a][1]; b+=3)
		{
		  r=tolower (pr[a][b])-'a';
		  aa[r]+=pr[a][b+1];
		  if (a==2){aa[r]*=aa[r];}
		  if (aa[r]>score)score=aa[r];
		}
	    }
	  for (a=0; a<2; a++)
	    {
	      for (b=3; b<pr[a][1]; b+=3)
		{
		  aa[b-'a']=0;
		}
	    }
	      
	  return score;
}

int cw_profile_get_dp_cost_window ( Alignment *A, int**pos1, int ns1, int*list1, int col1, int**pos2, int ns2, int*list2, int col2, Constraint_list *CL)
{
	  static int last_tag;
	  static int **cons1, **cons2;
	  int a, score, window_size=5, n, p1, p2;
	  

	if (last_tag!=A->random_tag)
	  {
	    last_tag=A->random_tag;
	    free_int (cons1,-1);free_int (cons2,-1);
	    cons1=sub_aln2count_mat2 (A, ns1, list1);
	    cons2=sub_aln2count_mat2 (A, ns2, list2);
	  }
	
	for (n=0,score=0,a=0; a<window_size; a++)
	  {
	    p1=col1-a;
	    p2=col2-a;
	    if ( p1<0 || cons1[p1][0]==END_ARRAY)continue;
	    if ( p2<0 || cons2[p2][0]==END_ARRAY)continue;
	    score+=cw_profile_profile (cons1[col1], cons2[col2], CL)-SCORE_K*CL->nomatch;
	    n++;
	  }
	if (n>0)score/=n;
	
	return score;
	}

int consensus_get_dp_cost ( Alignment *A, int**pos1, int ns1, int*list1, int col1, int**pos2, int ns2, int*list2, int col2, Constraint_list *CL)
	{
	  static int last_tag;
	  static char *seq1, *seq2;


	/*Works only with matrix*/
	  if (last_tag !=A->random_tag)
	    {
	      int ls1, ls2, lenls1, lenls2;
	   
	      last_tag=A->random_tag;
	      vfree (seq1);vfree (seq2);
	      seq1=sub_aln2cons_seq_mat (A, ns1, list1, "blosum62mt");
	      seq2=sub_aln2cons_seq_mat (A, ns2, list2, "blosum62mt");
	      ls1=list1[ns1-1];ls2=list2[ns2-1];
	      lenls1=(int)strlen (A->seq_al[ls1]); lenls2=(int)strlen (A->seq_al[ls2]);
	  }

	return (CL->M[seq1[col1]-'A'][seq2[col2]-'A']*SCORE_K)-SCORE_K*CL->nomatch;
	} 

int fast_get_dp_cost_quadruplet ( Alignment *A, int**pos1, int ns1, int*list1, int col1, int**pos2, int ns2, int*list2, int col2, Constraint_list *CL)
	{
	/*WARNING: WORKS ONLY WITH List to Extend*/
	  /*That function does a quruple extension beween two columns by pooling the residues together*/
	  
	  double score=0;
	  
	  int a,b, c;
	  int n_gap1=0;
	  int n_gap2=0;
	  
	  int s1, rs1, r1, t_r, t_s,t_w, q_r, q_s, q_w, s2, rs2, r2;
	  int **buf_pos, buf_ns, *buf_list, buf_col; 

	  static int **hasch1;
	  static int **hasch2;
	  
	  static int **n_hasch1;
	  static int **n_hasch2;
	  
	  static int **is_in_col1;
	  static int **is_in_col2;
	  

	  if (ns2>ns1)
	    {
	      buf_pos=pos1;
	      buf_ns=ns1;
	      buf_list=list1;
	      buf_col=col1;
	      
	      pos1=pos2;
	      ns1=ns2;
	      list1=list2;
	      col1=col2;
	      
	      pos2=buf_pos;
	      ns2=buf_ns;
	      list2=buf_list;
	      col2=buf_col;
	    }
	  

	if ( !hasch1)
	    {
	    
	    hasch1=declare_int( (CL->S)->nseq, (CL->S)->max_len+1);
            hasch2=declare_int( (CL->S)->nseq, (CL->S)->max_len+1);
	    n_hasch1=declare_int ( (CL->S)->nseq, (CL->S)->max_len+1);
	    n_hasch2=declare_int( (CL->S)->nseq, (CL->S)->max_len+1);
	    is_in_col1=declare_int( (CL->S)->nseq, (CL->S)->max_len+1);
	    is_in_col2=declare_int( (CL->S)->nseq, (CL->S)->max_len+1);
	    }
	
	for ( a=0; a< ns1; a++)
	    {
		rs1= list1[a];
		s1=A->order[rs1][0];
		r1=pos1[rs1][col1];
		
		if (r1<0)n_gap1++;		
		else
		   {	
		   is_in_col1[s1][r1]=1;
		   for (b=1; b< CL->residue_index[s1][r1][0]; b+=ICHUNK)
		           {
			   t_s=CL->residue_index[s1][r1][b+SEQ2];
			   t_r=CL->residue_index[s1][r1][b+R2];
			   t_w=CL->residue_index[s1][r1][b+WE];
			   for ( c=1; c< CL->residue_index[t_s][t_r][0]; c+=ICHUNK)
			     {
			       q_s=CL->residue_index[t_s][t_r][c+SEQ2];
			       q_r=CL->residue_index[t_s][t_r][c+R2];
			       q_w=CL->residue_index[t_s][t_r][c+WE];
			       hasch1[q_s][q_r]+=MIN(q_w, t_w);
			       n_hasch1[q_s][q_r]++;
			     }
			   }
		   }
	    }
	
	for ( a=0; a< ns2; a++)
	    {
		rs2=list2[a];
		s2=A->order[rs2][0];
		r2=pos2[rs2][col2];
	
		if (r2<0)n_gap2++;
		else
		   {
		   is_in_col2[s2][r2]=1;
		   for (b=1; b< CL->residue_index[s2][r2][0]; b+=ICHUNK)
		           {
			   t_s=CL->residue_index[s2][r2][b+SEQ2];
			   t_r=CL->residue_index[s2][r2][b+R2];
			   t_w=CL->residue_index[s2][r2][b+WE];
			   for ( c=1; c< CL->residue_index[t_s][t_r][0]; c+=ICHUNK)
			     {
			       q_s=CL->residue_index[t_s][t_r][c+SEQ2];
			       q_r=CL->residue_index[t_s][t_r][c+R2];
			       q_w=CL->residue_index[t_s][t_r][c+WE];
			       hasch2[q_s][q_r]+=MIN(t_w, q_w);
			       n_hasch2[q_s][q_r]++;
			     }
			   }
		   }
	    }
	

	for ( a=0; a< ns2; a++)
	  {
	    rs2=list2[a];
	    s2=A->order[rs2][0];
	    r2=pos1[rs2][col2];
	    
	    if (r2<0);
	    else
	      {
		for (b=1; b< CL->residue_index[s2][r2][0]; b+=ICHUNK)
		  {
		    t_s=CL->residue_index[s2][r2][b+SEQ2];
		    t_r=CL->residue_index[s2][r2][b+R2];

		    for ( c=1; c< CL->residue_index[t_s][t_r][0]; c+=ICHUNK)
		      {
			q_s=CL->residue_index[t_s][t_r][c+SEQ2];
			q_r=CL->residue_index[t_s][t_r][c+R2];
			if ( hasch2[q_s][q_r] && hasch1[q_s][q_r]&& !(is_in_col1[q_s][q_r] || is_in_col2[q_s][q_r]))
			  {
			    score+=MIN(hasch2[q_s][q_r]*(n_hasch1[q_s][q_r]),hasch1[q_s][q_r]*(n_hasch2[q_s][q_r]));
			  }
			else if ( hasch2[q_s][q_r] && is_in_col1[q_s][q_r])
			  {
			    score+=hasch2[q_s][q_r]*(n_hasch1[q_s][q_r]+1);
			  }
			else if (hasch1[q_s][q_r] && is_in_col2[q_s][q_r])
			  {
			    score+=hasch1[q_s][q_r]*(n_hasch2[q_s][q_r]+1);
			  }
			hasch2[q_s][q_r]=0;
			n_hasch2[q_s][q_r]=0;
		      }
		  }
		hasch2[s2][r2]=0;
		is_in_col2[s2][r2]=0;
	      }
	  }
	
	
	for ( a=0; a< ns1; a++)
	  {
	    rs1= list1[a];
	    s1=A->order[rs1][0];
	    r1=pos1[rs1][col1];
	    
	    if (r1<0);
	    else
	      {
		is_in_col1[s1][r1]=0;
		hasch1[s1][r1]=0;
		for (b=1; b< CL->residue_index[s1][r1][0]; b+=ICHUNK)
		  {
		    t_s=CL->residue_index[s1][r1][b+SEQ2];
		    t_r=CL->residue_index[s1][r1][b+R2];
		    for ( c=1; c< CL->residue_index[t_s][t_r][0]; c+=ICHUNK)
		      {
			q_s=CL->residue_index[t_s][t_r][c+SEQ2];
			q_r=CL->residue_index[t_s][t_r][c+R2];
			hasch1[q_s][q_r]=0;
			n_hasch1[q_s][q_r]=0;
		      }
		  }
	      }
	  }
	

	score=(score*SCORE_K)/((ns1-n_gap1)*(ns2-n_gap2));
	score=(CL->normalise)?((score*CL->normalise)/CL->max_ext_value):score;

	return (int)(score-SCORE_K*CL->nomatch);
	}

	    
int fast_get_dp_cost ( Alignment *A, int**pos1, int ns1, int*list1, int col1, int**pos2, int ns2, int*list2, int col2, Constraint_list *CL)
	{
	/*WARNING: WORKS ONLY WITH List to Extend*/
	  
	  double score=0;
	  
	  int a,b;
	  int n_gap1=0;
	  int n_gap2=0;
	  
	  int s1, rs1, r1, t_r, t_s, s2, rs2, r2;
	  static int **hasch1;
	  static int **hasch2;
	  
	  static int **n_hasch1;
	  static int **n_hasch2;
	  
	  static int **is_in_col1;
	  static int **is_in_col2;


	    

	if ( !hasch1)
	    {
	    
	    hasch1=declare_int( (CL->S)->nseq, (CL->S)->max_len+1);
            hasch2=declare_int( (CL->S)->nseq, (CL->S)->max_len+1);
	    n_hasch1=declare_int ( (CL->S)->nseq, (CL->S)->max_len+1);
	    n_hasch2=declare_int( (CL->S)->nseq, (CL->S)->max_len+1);
	    is_in_col1=declare_int( (CL->S)->nseq, (CL->S)->max_len+1);
	    is_in_col2=declare_int( (CL->S)->nseq, (CL->S)->max_len+1);
	    }
	
	for ( a=0; a< ns1; a++)
	    {
		rs1= list1[a];
		s1=A->order[rs1][0];
		r1=pos1[rs1][col1];
		
		if (r1<0)n_gap1++;		
		else
		   {	
		   is_in_col1[s1][r1]=1;
		   for (b=1; b< CL->residue_index[s1][r1][0]; b+=ICHUNK)
		           {
			   t_s=CL->residue_index[s1][r1][b+SEQ2];
			   t_r=CL->residue_index[s1][r1][b+R2];			   
			   hasch1[t_s][t_r]+=CL->residue_index[s1][r1][b+WE];
			   n_hasch1[t_s][t_r]++;
			   }
		   }
	    }


	for ( a=0; a< ns2; a++)
	    {
		rs2=list2[a];
		s2=A->order[rs2][0];
		r2=pos2[rs2][col2];
	
		if (r2<0)n_gap2++;
		else
		   {
		   is_in_col2[s2][r2]=1;
		   for (b=1; b< CL->residue_index[s2][r2][0]; b+=ICHUNK)
		           {
			   t_s=CL->residue_index[s2][r2][b+SEQ2];
			   t_r=CL->residue_index[s2][r2][b+R2];

			   hasch2[t_s][t_r]+=CL->residue_index[s2][r2][b+WE];
			   n_hasch2[t_s][t_r]++;
			   }
		   }
	    }
	/*return 2;*/

	if ( ns2<ns1)
	    {
		for ( a=0; a< ns2; a++)
		    {
		    rs2=list2[a];
		    s2=A->order[rs2][0];
		    r2=pos1[rs2][col2];
		    
		    if (r2<0);
		    else
		        {
			for (b=1; b< CL->residue_index[s2][r2][0]; b+=ICHUNK)
			    {
			    t_s=CL->residue_index[s2][r2][b+SEQ2];
			    t_r=CL->residue_index[s2][r2][b+R2];
			    
			    if ( hasch2[t_s][t_r] && hasch1[t_s][t_r]&& !(is_in_col1[t_s][t_r] || is_in_col2[t_s][t_r]))
			        {
				score+=MIN(hasch2[t_s][t_r]*(n_hasch1[t_s][t_r]),hasch1[t_s][t_r]*(n_hasch2[t_s][t_r]));
				}
			    else if ( hasch2[t_s][t_r] && is_in_col1[t_s][t_r])
			        {
				score+=hasch2[t_s][t_r]*(n_hasch1[t_s][t_r]+1);
				}
			    else if (hasch1[t_s][t_r] && is_in_col2[t_s][t_r])
			        {
				score+=hasch1[t_s][t_r]*(n_hasch2[t_s][t_r]+1);
				}
			    hasch2[t_s][t_r]=0;
			    n_hasch2[t_s][t_r]=0;
			    }
			hasch2[s2][r2]=0;
			is_in_col2[s2][r2]=0;
			}
		    }

	
		for ( a=0; a< ns1; a++)
		    {
		    rs1= list1[a];
		    s1=A->order[rs1][0];
		    r1=pos1[rs1][col1];
		    
		    if (r1<0);
		    else
		        {
			is_in_col1[s1][r1]=0;
			hasch1[s1][r1]=0;
			for (b=1; b< CL->residue_index[s1][r1][0]; b+=ICHUNK)
			    {
			    t_s=CL->residue_index[s1][r1][b+SEQ2];
			    t_r=CL->residue_index[s1][r1][b+R2];
			    
			    hasch1[t_s][t_r]=0;
			    n_hasch1[t_s][t_r]=0;
			    }
			}
		    }
	    }
	else
	   {
		for ( a=0; a< ns1; a++)
		    {
		    rs1=list1[a];
		    s1=A->order[rs1][0];
		    r1=pos1[rs1][col1];
		    
		    if (r1<0);
		    else
		        {
			for (b=1; b< CL->residue_index[s1][r1][0]; b+=ICHUNK)
			    {
			    t_s=CL->residue_index[s1][r1][b+SEQ2];
			    t_r=CL->residue_index[s1][r1][b+R2];
			    
			    if ( hasch1[t_s][t_r] && hasch2[t_s][t_r]&& !(is_in_col2[t_s][t_r] || is_in_col1[t_s][t_r]))
			        {
				score+=MIN(hasch1[t_s][t_r]*(n_hasch2[t_s][t_r]),hasch2[t_s][t_r]*(n_hasch1[t_s][t_r]));
			        }
			    else if ( hasch1[t_s][t_r] && is_in_col2[t_s][t_r])
			        {
				score+=hasch1[t_s][t_r]*(n_hasch2[t_s][t_r]+1);
			        }
			    else if (hasch2[t_s][t_r] && is_in_col1[t_s][t_r])
			        {
				score+=hasch2[t_s][t_r]*(n_hasch1[t_s][t_r]+1);
			        }
			    hasch1[t_s][t_r]=0;
			    n_hasch1[t_s][t_r]=0;
			    }
			hasch1[s1][r1]=0;
			is_in_col1[s1][r1]=0;
		        }
		    }

	
		for ( a=0; a< ns2; a++)
		    {
		    rs2= list2[a];
		    s2=A->order[rs2][0];
		    r2=pos1[rs2][col2];
		    
		    if (r2<0);
		    else
		        {
			is_in_col2[s2][r2]=0;
			hasch1[s2][r2]=0;
			for (b=1; b< CL->residue_index[s2][r2][0]; b+=ICHUNK)
			    {
			    t_s=CL->residue_index[s2][r2][b+SEQ2];
			    t_r=CL->residue_index[s2][r2][b+R2];
			    
			    hasch2[t_s][t_r]=0;
			    n_hasch2[t_s][t_r]=0;
			    }
		        }
		    }
	    }
	score=(score*SCORE_K)/((ns1-n_gap1)*(ns2-n_gap2));
	score=(CL->normalise)?((score*CL->normalise)/CL->max_ext_value):score;

	return (int)(score-SCORE_K*CL->nomatch);
	}

int fast_get_dp_cost_2 ( Alignment *A, int**pos1, int ns1, int*list1, int col1, int**pos2, int ns2, int*list2, int col2, Constraint_list *CL)
	{
	double score=0;
	
	int a, b, s1, s2,r1, r2;
	static int n_pair;

	int s;
	int res_res=0;
	int rs1, rs2;
	static int last_ns1;
	static int last_ns2;
	static int last_top1;
	static int last_top2;
	static int **check_list;
	

	/*New heuristic get dp_cost on a limited number of pairs*/
	/*This is the current default*/

	
	if ( last_ns1==ns1 && last_top1==list1[0] && last_ns2==ns2 && last_top2==list2[0]);
	else
	  {
	    
	    
	    last_ns1=ns1;
	    last_ns2=ns2;
	    last_top1=list1[0];
	    last_top2=list2[0];
	    if ( check_list) free_int (check_list, -1);
	    check_list=declare_int ( (CL->S)->nseq*(CL->S)->nseq, 3);
	    
	    for ( n_pair=0,a=0; a< ns1; a++)
	      {
		s1 =list1[a];
		rs1=A->order[s1][0];
		for ( b=0; b< ns2; b++, n_pair++)
		  {
		    s2 =list2[b];
		    rs2=A->order[s2][0]; 
		    check_list[n_pair][0]=s1;
		    check_list[n_pair][1]=s2;
		    check_list[n_pair][2]=(!CL->DM)?0:(CL->DM)->similarity_matrix[rs1][rs2];
		  }
		sort_int ( check_list, 3, 2, 0, n_pair-1);
	      }
	  }

	if ( !CL->evaluate_residue_pair){fprintf ( stderr, "\nWARNING: CL->evaluate_residue_pair Not set\nSet to: extend_residue_pair\n");CL->evaluate_residue_pair= extend_residue_pair;}
	
	
	for ( a=n_pair-1; a>=0; a--)
	  {
	    s1= check_list[a][0];
	    rs1=A->order[s1][0];
	    r1 =pos1[s1][col1]; 
	  
	    s2= check_list[a][1];
	    rs2=A->order[s2][0];
	    r2 =pos2[s2][col2];
	    
	    if ( r1>0 && r2 >0) 
	      {res_res++;}
	    if ( rs1>rs2)
	      {			    
		SWAP (rs1, rs2);
		SWAP (r1, r2);
	      }
	    
	    if ((s=(CL->evaluate_residue_pair)(CL, rs1, r1, rs2, r2))!=UNDEFINED) score+=s;				    
	    else 
	      {
		
		return UNDEFINED;
	      }
	    if ( res_res>=CL->max_n_pair && CL->max_n_pair!=0)a=0;
	  }

	score=(res_res==0)?0:( (score)/res_res);
	score=score-SCORE_K*CL->nomatch;

	return (int)score;
	} 




int slow_get_dp_cost ( Alignment *A, int**pos1, int ns1, int*list1, int col1, int**pos2, int ns2, int*list2, int col2, Constraint_list *CL)
	{
	double score=0;

	int a, b, s1, s2,r1, r2;
	int s;
	int gap_gap=0;
	int gap_res=0;
	int res_res=0;
	int rs1, rs2;
	static int last_tag;
	static int *dummy;
	
	if (col1<0 || col2<0) return 0;
	if ( last_tag !=A->random_tag)
	  {
	    last_tag=A->random_tag;
	    if (!dummy)
	      {
		dummy=(int*)vcalloc (10, sizeof(int));
		dummy[0]=1;/*Number of Amino acid types on colum*/
		dummy[1]=5;/*Length of Dummy*/
		dummy[3]='\0';/*Amino acid*/
		dummy[4]=1; /*Number of occurences*/
		dummy[5]=100; /*Frequency in the MSA column*/
	      }
	  }
	
	if ( !CL->evaluate_residue_pair){fprintf ( stderr, "\nWARNING: CL->evaluate_residue_pair Not set\nSet to: extend_residue_pair\n");CL->evaluate_residue_pair= extend_residue_pair;}
		
	for ( a=0; a< ns1; a++)
	  {
	    for ( b=0; b<ns2; b++)
	      {
		
		s1 =list1[a];
		rs1=A->order[s1][0];
		r1 =pos1[s1][col1];
		
		s2 =list2[b];
		rs2=A->order[s2][0];
		r2 =pos2[s2][col2];
		
		if ( rs1>rs2)
		  {			    
		    SWAP (rs1, rs2);
		    SWAP (r1, r2);
		  }
		
		if (r1==0 && r2==0)gap_gap++;			     
		else if ( r1<0 || r2<0) gap_res++;			
		else 
		  {					
		    res_res++;

		    if ((s=(CL->evaluate_residue_pair)(CL, rs1, r1, rs2, r2))!=UNDEFINED) score+=s;				    
		    else 
		      {
			
			return UNDEFINED;
		      }
		  }
		
	      }
	  }
	
	
	score=(res_res==0)?0:( (score)/res_res);
	
	return score-SCORE_K*CL->nomatch;	
	
	}
int slow_get_dp_cost_pc ( Alignment *A, int**pos1, int ns1, int*list1, int col1, int**pos2, int ns2, int*list2, int col2, Constraint_list *CL)
	{
	double score=0;

	int a, b, s1, s2,r1, r2;
	int s;
	int gap_gap=0;
	int gap_res=0;
	int res_res=0;
	int rs1, rs2;
	static int last_tag;
	static int *dummy;
	
	if (col1<0 || col2<0) return 0;
	if ( last_tag !=A->random_tag)
	  {
	    last_tag=A->random_tag;
	    if (!dummy)
	      {
		dummy=(int*)vcalloc (10, sizeof(int));
		dummy[0]=1;/*Number of Amino acid types on colum*/
		dummy[1]=5;/*Length of Dummy*/
		dummy[3]='\0';/*Amino acid*/
		dummy[4]=1; /*Number of occurences*/
		dummy[5]=100; /*Frequency in the MSA column*/
	      }
	  }
	
	if ( !CL->evaluate_residue_pair){fprintf ( stderr, "\nWARNING: CL->evaluate_residue_pair Not set\nSet to: extend_residue_pair\n");CL->evaluate_residue_pair= extend_residue_pair;}
		
	for ( a=0; a< ns1; a++)
	  {
	    for ( b=0; b<ns2; b++)
	      {
		
		s1 =list1[a];
		rs1=A->order[s1][0];
		r1 =pos1[s1][col1];
		
		s2 =list2[b];
		rs2=A->order[s2][0];
		r2 =pos2[s2][col2];
		
		if ( rs1>rs2)
		  {			    
		    SWAP (rs1, rs2);
		    SWAP (r1, r2);
		  }
		
		if (r1==0 && r2==0)gap_gap++;			     
		else if ( r1<0 || r2<0) gap_res++;			
		else 
		  {					
		    res_res++;
		    if ((s=residue_pair_extended_list_pc(CL, rs1, r1, rs2, r2))!=UNDEFINED) score+=s;				    
		    else 
		      {
			
			return UNDEFINED;
		      }
		  }
		
	      }
	  }
	
	
	score=(res_res==0)?0:( (score)/res_res);
	
	return score;
	
	}
int slow_get_dp_cost_test ( Alignment *A, int**pos1, int ns1, int*list1, int col1, int**pos2, int ns2, int*list2, int col2, Constraint_list *CL)
	{
	double score=0;
	
	int a, b, s1, s2,r1, r2;
	int gap_gap=0, gap_res=0, res_res=0, rs1, rs2;

	for ( a=0; a< ns1; a++)
		{
		for ( b=0; b<ns2; b++)
			{
			
			s1 =list1[a];
			rs1=A->order[s1][0];
			r1 =pos1[s1][col1];
			
			s2 =list2[b];
			rs2=A->order[s2][0];
			r2 =pos2[s2][col2];
				
	        	if ( rs1>rs2)
			   {			    
			   SWAP (rs1, rs2);
			   SWAP (r1, r2);
			   }
			
			if (r1==0 && r2==0)gap_gap++;			     
			else if ( r1<0 || r2<0) gap_res++;			
			else 
			    {					
			    res_res++;
			    score+=residue_pair_extended_list_raw (CL, rs1, r1, rs2, r2);
			    }
			}
		}

	return (int)(score*10)/(ns1*ns2);	
	}

int sw_get_dp_cost     ( Alignment *A, int**pos1, int ns1, int*list1, int col1, int**pos2, int ns2, int*list2, int col2, Constraint_list *CL)
	{
	  int a, b;
	  int s1,r1,rs1;
	  int s2,r2,rs2;
	  
	  
	  

	  for ( a=0; a< ns1; a++)
	    {	
	      s1 =list1[a];
	      rs1=A->order[s1][0];
	      r1 =pos1[s1][col1];
	      if ( r1<=0)continue;
	      for ( b=0; b< ns2; b++)
		{
		  
		  
		  s2 =list2[b];
		  rs2=A->order[s2][0];
		  r2 =pos2[s2][col2];
		  
		  if (r2<=0)continue;
		  
		  
		  if (sw_pair_is_defined (CL, rs1, r1, rs2, r2)==UNDEFINED)return UNDEFINED;	       
		}
	    }	  
			
	  return slow_get_dp_cost ( A, pos1, ns1, list1, col1, pos2, ns2, list2,col2, CL);
	  	  
	}





  


int get_domain_dp_cost ( Alignment *A, int**pos1, int ns1, int*list1, int col1, int**pos2, int ns2, int*list2, int col2,Constraint_list *CL , int scale , int gop, int gep)

	{
	int a, b, s1, s2,r1, r2;
	static int *entry;
	int *r;
	int score=0;
	int gap_gap=0;
	int gap_res=0;
	int res_res=0;
	int rs1, rs2;
	int flag_list_is_aa_sub_mat=0;
	int p;

/*Needs to be cleanned After Usage*/
	


	if ( entry==NULL) entry=(int*)vcalloc (LIST_N_FIELDS , sizeof (int));
	
	for (a=0; a< ns1; a++)
		{
		s1=list1[a];
		rs1=A->order[s1][0];
		for ( b=0; b<ns2; b++)
			{
			s2 =list2[b];
			rs2=A->order[s2][0];
			
			entry[SEQ1]=rs1;
			entry[SEQ2]=rs2;
			r1=entry[R1]=pos1[s1][col1];
			r2=entry[R2]=pos2[s2][col2];
			
			if ( !flag_list_is_aa_sub_mat)
			    {
			    if ( r1==r2 && rs1==rs2)
			       {
				 
				 return UNDEFINED;
			       }
			    else if (r1==0 && r2==0)
			       {			    
			       gap_gap++;		
			       }
			    else if ( r1<=0 || r2<=0)
			       {
			       gap_res++;
			       }
			    else if ((r=main_search_in_list_constraint ( entry,&p,4,CL))!=NULL)
				{				
				res_res++; 
				
				if (r[WE]!=UNDEFINED) 
				    {
				    score+=(r[WE]*SCORE_K)+scale;
				    }
				else 
				  {
				    fprintf ( stderr, "**");
				    return UNDEFINED;
				  }
				}
			    }			    			  
			}
		}
	return score;
	} 

/*********************************************************************************************/
/*                                                                                           */
/*         FUNCTIONS FOR ANALYSING AL OR MATRIX                                              */
/*                                                                                           */
/*********************************************************************************************/

int aln2n_res ( Alignment *A, int start, int end)
    {
    int a, b;
    int score=0;

    for ( a=start; a<end; a++)for ( b=0; b< A->nseq; b++)score+=!is_gap(A->seq_al[b][a]);
    return score;
    }

float get_gop_scaling_factor ( int **matrix,float id, int l1, int l2)
    {
	return id* get_avg_matrix_mm(matrix, AA_ALPHABET);
    }

float get_avg_matrix_mm ( int **matrix, char *alphabet)
    {
	int a, b;
	float naa;
	float gop;
	int l;

	
	l=MIN(20,(int)strlen (alphabet));
	for (naa=0, gop=0,a=0; a<l; a++)
	    for ( b=0; b<l; b++)
	        {
		    if ( a!=b)
		        {
			gop+=matrix[alphabet[a]-'A'][alphabet[b]-'A'];
			naa++;
			}
		}
	
	gop=gop/naa;
	return gop;
    }
float get_avg_matrix_match ( int **matrix, char *alphabet)
    {
	int a;
	float gop;
	int l;

	

	
	l=MIN(20,(int)strlen (alphabet));
	for (gop=0,a=0; a<l; a++)
	  gop+=matrix[alphabet[a]-'A'][alphabet[a]-'A'];
	
	gop=gop/l;
	return gop;
    }	
		      
float get_avg_matrix_diff ( int **matrix1,int **matrix2, char *alphabet)
    {
	int a, b;
	float naa;
	float gop;
	int l,v1;

	

	
	l=MIN(20,(int)strlen (alphabet));
	for (naa=0, gop=0,a=0; a<l; a++)
	  for (b=0; b<l; b++)
	    {
	      v1=matrix1[alphabet[a]-'A'][alphabet[a]-'A']-matrix2[alphabet[b]-'A'][alphabet[b]-'A'];
	      gop+=v1*v1;
	      naa++;
	    }
	
	gop=gop/l;
	return gop;
    }	

float* set_aa_frequencies ()
   {
     
     float *frequency;
     /*frequencies tqken from psw*/
     
     frequency=(float*)vcalloc (100, sizeof (float));
     frequency ['x'-'A']=0.0013;
     frequency ['a'-'A']=0.0076;
     frequency ['c'-'A']=0.0176;
     frequency ['d'-'A']=0.0529;
     frequency ['e'-'A']=0.0628;
     frequency ['f'-'A']=0.0401;
     frequency ['g'-'A']=0.0695;
     frequency ['h'-'A']=0.0224;
     frequency ['i'-'A']=0.0561;
     frequency ['k'-'A']=0.0584;
     frequency ['l'-'A']=0.0922;
     frequency ['m'-'A']=0.0236;
     frequency ['n'-'A']=0.0448;
     frequency ['p'-'A']=0.0500;
     frequency ['q'-'A']=0.0403;
     frequency ['r'-'A']=0.0523;
     frequency ['s'-'A']=0.0715;
     frequency ['t'-'A']=0.0581;
     frequency ['v'-'A']=0.0652;
     frequency ['w'-'A']=0.0128;
     frequency ['y'-'A']=0.0321; 
     frequency ['X'-'A']=0.0013;
     frequency ['A'-'A']=0.0076;
     frequency ['C'-'A']=0.0176;
     frequency ['D'-'A']=0.0529;
     frequency ['E'-'A']=0.0628;
     frequency ['F'-'A']=0.0401;
     frequency ['G'-'A']=0.0695;
     frequency ['H'-'A']=0.0224;
     frequency ['I'-'A']=0.0561;
     frequency ['J'-'A']=0.0584;
     frequency ['L'-'A']=0.0922;
     frequency ['M'-'A']=0.0236;
     frequency ['N'-'A']=0.0448;
     frequency ['P'-'A']=0.0500;
     frequency ['Q'-'A']=0.0403;
     frequency ['R'-'A']=0.0523;
     frequency ['S'-'A']=0.0715;
     frequency ['T'-'A']=0.0581;
     frequency ['V'-'A']=0.0652;
     frequency ['W'-'A']=0.0128;
     frequency ['Y'-'A']=0.0321; 
     return frequency;
   }

float measure_matrix_pos_avg (int **matrix,char *alphabet)
{
	float naa=0, tot=0;
	int a, b;
	
	
	for ( tot=0,a=0; a< 20; a++)
	   for ( b=0; b<20; b++)
	     {
	       if (matrix[alphabet[a]-'A'][alphabet[b]-'A']>=0){naa++;tot+=matrix[alphabet[a]-'A'][alphabet[b]-'A'];}

	     }
	return tot/naa;
}
  
float measure_matrix_enthropy (int **matrix,char *alphabet)
   {
     
     int a, b;
     double s, p, q, h=0, tq=0;
     float lambda;
     float *frequency;
     /*frequencies tqken from psw*/
     
     frequency=set_aa_frequencies ();

     
     lambda=compute_lambda(matrix,alphabet);
     fprintf ( stderr, "\nLambda=%f", (float)lambda);
	   
     for ( a=0; a< 20; a++)
       for ( b=0; b<=a; b++)
	 {
	   s=matrix[alphabet[a]-'A'][alphabet[b]-'A'];
	   
	   
	   p=frequency[alphabet[a]-'A']*frequency[alphabet[b]-'A'];
	   
	   if ( p==0)continue;
	   
	   q=exp(lambda*s+log(p));
	   
	   tq+=q;
	   h+=q*log(q/p)*log(2);
	  
	 }
	   
     fprintf ( stderr,"\ntq=%f\n", (float)tq);
   
     return (float) h;
    }	
float compute_lambda (int **matrix,char *alphabet)
{

  int a, b;
  double lambda, best_lambda=0, delta, best_delta=0, p, tq,s;
  static float *frequency;
  
  if ( !frequency)frequency=set_aa_frequencies ();

  for ( lambda=0.001; lambda<1; lambda+=0.005)
     {
       tq=0;
       for ( a=0; a< 20; a++)
	 for ( b=0; b<20; b++)
	   {	     
	     p=frequency[alphabet[a]-'A']*frequency[alphabet[b]-'A'];
	     s=matrix[alphabet[a]-'A'][alphabet[b]-'A'];
	     tq+=exp(lambda*s+log(p));
	   }
       delta=fabs(1-tq);
       if (lambda==0.001)
	 {
	   best_delta=delta;
	   best_lambda=lambda;
	 }
       else
	 {
	   if (delta<best_delta)
	     {
	       best_delta=delta;
	       best_lambda=lambda;
	     }
	 }   
       
       fprintf ( stderr, "\n%f %f ", lambda, tq);
       if ( tq>1)break;
     }
   fprintf ( stderr, "\nRESULT: %f %f ", best_lambda, best_delta);
   return (float) best_lambda;
}



float evaluate_random_match (char  *mat, int n, int len,char *alp)
{
  int **matrix;
  matrix=read_matrice ( mat); 
  fprintf ( stderr, "Matrix=%15s ", mat);
  return evaluate_random_match2 (matrix, n,len,alp);
  
}

float evaluate_random_match2 (int **matrix, int n, int len,char *alp)
{
  int a, b, c, d, c1, c2, tot;
  static int *list;
  static float *freq;
  float score_random=0;
  float score_id=0;
  float score_good=0;
  float tot_len=0;
  float tot_try=0;


  if ( !list)
    {
      vsrand(0);
      freq=set_aa_frequencies ();
      list=(int*)vcalloc ( 10000, sizeof (char));
    }
  
  for (tot=0,c=0,a=0;a<20; a++)
    {
      b=freq[alp[a]-'A']*1000;
      tot+=b;
      for (d=0; d<b; d++, c++)
	{
	  list[c]=alp[a];
	}
    }
  
  
  for (a=0; a< len*n; a++)
    {
      c1=rand()%tot;
      c2=rand()%tot;
      score_random+=matrix[list[c1]-'A'][list[c2]-'A'];
      score_id+=matrix[list[c1]-'A'][list[c1]-'A'];
    }
  while (tot_len< len*n)
    {
      tot_try++;
      c1=rand()%tot;
      c2=rand()%tot;
      if ( matrix[list[c1]-'A'][list[c2]-'A']>=0){score_good+=matrix[list[c1]-'A'][list[c2]-'A']; tot_len++;}
    }
      

  score_random=score_random/tot_len;
  score_id=score_id/tot_len;
  score_good=score_good/tot_len;
  
  fprintf ( stderr, "Random=%8.3f Id=%8.3f Good=%8.3f [%7.2f]\n",score_random, score_id, score_good, tot_len/tot_try);
  
  return score_random;
}
float compare_two_mat (char  *mat1,char*mat2, int n, int len,char *alp)
{
  int **matrix1, **matrix2;
  
 evaluate_random_match (mat1, n, len,alp);
 evaluate_random_match (mat2, n, len,alp);
 matrix1=read_matrice ( mat1);
 matrix2=read_matrice ( mat2);
 matrix1=rescale_matrix(matrix1, 10, alp);
 matrix2=rescale_matrix(matrix2, 10, alp);
 compare_two_mat_array(matrix1,matrix2, n, len,alp);
 return 0;
} 


int ** rescale_two_mat (char  *mat1,char*mat2, int n, int len,char *alp)
{
  float lambda;
  int **matrix1, **matrix2;

  lambda=measure_lambda2 (mat1, mat2, n, len, alp)*10;

  fprintf ( stderr, "\nLambda=%.2f", lambda);
  matrix2=read_matrice(mat2);
  matrix2=neg_matrix2pos_matrix(matrix2);
  matrix2=rescale_matrix( matrix2, lambda,"abcdefghiklmnpqrstvwxyz");
  
  matrix1=read_matrice(mat1);
  matrix1=neg_matrix2pos_matrix(matrix1);
  matrix1=rescale_matrix( matrix1,10,"abcdefghiklmnpqrstvwxyz");
  
  output_matrix_header ( "stdout", matrix2, alp);
  evaluate_random_match2(matrix1, 1000, 100, alp);
  evaluate_random_match2(matrix2, 1000, 100, alp);
  compare_two_mat_array(matrix1,matrix2, n, len,alp);

  return matrix2;
}  
float measure_lambda2(char  *mat1,char*mat2, int n, int len,char *alp) 
{
  int **m1, **m2;
  float f1, f2;

  m1=read_matrice (mat1);
  m2=read_matrice (mat2);

  m1=neg_matrix2pos_matrix(m1);
  m2=neg_matrix2pos_matrix(m2);
  
  f1=measure_matrix_pos_avg( m1, alp);
  f2=measure_matrix_pos_avg( m2, alp);
  
  return f1/f2;
}
  

float measure_lambda (char  *mat1,char*mat2, int n, int len,char *alp) 
{
  int c;
  int **matrix1, **matrix2, **mat;
  float a;
  float best_quality=0, quality=0, best_lambda=0;
  
  matrix1=read_matrice ( mat1);
  matrix2=read_matrice ( mat2);
  matrix1=rescale_matrix(matrix1, 10, alp);
  matrix2=rescale_matrix(matrix2, 10, alp);

  for (c=0, a=0.1; a< 2; a+=0.05)
    {
      fprintf ( stderr, "Lambda=%.2f\n", a);
      mat=duplicate_int (matrix2,-1,-1);
      mat=rescale_matrix(mat, a, alp);
      quality=compare_two_mat_array(matrix1,mat, n, len,alp);
      quality=MAX((-quality),quality); 
      
      if (c==0 || (best_quality>quality))
	{
	  c=1;
	  fprintf ( stderr, "*");
	  best_quality=quality;
	  best_lambda=a;
	}
      
      
      evaluate_random_match2(mat, 1000, 100, alp);
      evaluate_random_match2(matrix1, 1000, 100, alp); 
      free_int (mat, -1);
    }
  
  return best_lambda;
  
}

float compare_two_mat_array (int  **matrix1,int **matrix2, int n, int len,char *alp)
{
  int a, b, c, d, c1, c2, tot;
  static int *list;
  static float *freq;
  float delta_random=0;
  float delta2_random=0;

  float delta_id=0;
  float delta2_id=0;

  float delta_good=0;
  float delta2_good=0;

  float delta;

  float tot_len=0;
  float tot_try=0;

 

  if ( !list)
    {
      vsrand(0);
      freq=set_aa_frequencies ();
      list=(int*)vcalloc ( 10000, sizeof (char));
    }
  
  for (tot=0,c=0,a=0;a<20; a++)
    {
      b=freq[alp[a]-'A']*1000;
      tot+=b;
      for (d=0; d<b; d++, c++)
	{
	  list[c]=alp[a];
	}
    }
  
  
  for (a=0; a< len*n; a++)
    {
      c1=rand()%tot;
      c2=rand()%tot;
      delta=matrix1[list[c1]-'A'][list[c2]-'A']-matrix2[list[c1]-'A'][list[c2]-'A'];
      delta_random+=delta;
      delta2_random+=MAX(delta,(-delta));
      
      delta=matrix1[list[c1]-'A'][list[c1]-'A']-matrix2[list[c1]-'A'][list[c1]-'A'];
      delta_id+=delta;
      delta2_id+=MAX(delta,(-delta));
    }
  while (tot_len< len*n)
    {
      tot_try++;
      c1=rand()%tot;
      c2=rand()%tot;
      if ( matrix1[list[c1]-'A'][list[c2]-'A']>=0 || matrix2[list[c1]-'A'][list[c2]-'A'] )
	{
	  delta=matrix1[list[c1]-'A'][list[c2]-'A']-matrix2[list[c1]-'A'][list[c2]-'A']; 
	  delta_good+=delta;
	  delta2_good+=MAX(delta,(-delta));
	  tot_len++;
	}
    }
      

  delta_random=delta_random/tot_len;
  delta2_random=delta2_random/tot_len;

  
  delta_id=delta_id/tot_len;
  delta2_id=delta2_id/tot_len;

  delta_good=delta_good/tot_len;
  delta2_good=delta2_good/tot_len;
  
    
  fprintf ( stderr, "\tRand=%8.3f %8.3f\n\tId  =%8.3f %8.3f\n\tGood=%8.3f %8.3f\n",delta_random, delta2_random, delta_id,delta2_id, delta_good,delta2_good);
  
  return delta_good;
}



int ** rescale_matrix ( int **matrix, float lambda, char *alp)
{
  int a, b;


  for ( a=0; a< 20; a++)
    for ( b=0; b< 20; b++)
      {
	matrix[alp[a]-'A'][alp[b]-'A']=	matrix[alp[a]-'A'][alp[b]-'A']*lambda;
      }
  return matrix;
}
int **mat2inverted_mat (int **matrix, char *alp)
{
  int a, b, min, max, v,l;
  int c1,c2, C1, C2;
  
  l=(int)strlen (alp);
  min=max=matrix[alp[0]-'A'][alp[0]-'A'];
  for ( a=0; a<l; a++)
    for ( b=0; b< l; b++)
      {
	v=matrix[alp[a]-'A'][alp[b]-'A'];
	min=MIN(min,v);
	max=MAX(max,v);
      }
  for ( a=0; a<l; a++)
    for ( b=0; b< l; b++)
      {
	v=matrix[alp[a]-'A'][alp[b]-'A'];
	v=max-v;
	
	c1=tolower(alp[a])-'A';
	c2=tolower(alp[b])-'A';
	
	C1=toupper(alp[a])-'A';
	C2=toupper(alp[b])-'A';
	matrix[C1][C2]=matrix[c1][c2]=matrix[C1][c2]=matrix[c1][C2]=v;
      }
  return matrix;
}
void output_matrix_header ( char *name, int **matrix, char *alp)
{
  int a, b;
  FILE *fp;
  char *nalp;
  int l;
  nalp=(char*)vcalloc ( 1000, sizeof (char));
  

  
  sprintf ( nalp, "ABCDEFGHIKLMNPQRSTVWXYZ");
  l=(int)strlen (nalp);

  fp=vfopen ( name, "w");
  fprintf (fp, "\nint []={\n");
  
  for (a=0; a<l; a++)
    {
      for ( b=0; b<=a; b++)
	fprintf ( fp, "%3d, ",matrix[nalp[a]-'A'][nalp[b]-'A']);
      fprintf ( fp, "\n");
    }
  fprintf (fp, "};\n");
  
  vfclose (fp);
}
  

float ** initialise_aa_physico_chemical_property_table (int *n)
{
  FILE *fp;
  int a, b,c;
  float **p;
  char c1;
  float v1, max, min;
  int in=0;

  n[0]=0;
  fp=vfopen ("properties.txt", "r");
  while ((c=fgetc(fp))!=EOF)n[0]+=(c=='#');
  vfclose (fp);
  
  
   
  p=declare_float ( n[0], 'z'+1);
  fp=vfopen ("properties.txt", "r");
  n[0]=0;
  while ((c=fgetc(fp))!=EOF)
	 {
	   if (c=='#'){in=0;while (fgetc(fp)!='\n');}
	   else
	     {
	       if ( !in){in=1; ++n[0];}
	       fscanf (fp, "%f %c %*s",&v1, &c1 );
	       p[n[0]-1][tolower(c1)]=v1;
	       while ( (c=fgetc(fp))!='\n');
	     }
	 }
  vfclose (fp);
 
  /*rescale so that Delta max=10*/
  for (a=0; a<n[0]; a++)
    {
      min=max=p[a]['a'];
      for (b='a'; b<='z'; b++)
	{
	  min=(p[a][b]<min)?p[a][b]:min;
	  max=(p[a][b]>max)?p[a][b]:max;
	}
      for (b='a'; b<='z'; b++)
	{
	  p[a][b]=((p[a][b]-min)/(max-min))*10;
	 
	}
    }

  return p;
}

/**
 * Set the extension mode. Default seems to be \c very_fast_triplet (??)
 *
 * Depending on the \c extend_mode, this functions specifies how the evaluation
 * step has to be performed. I assignes functions to the values
 * Constraint_list::evaluate_residue_pair and Constraint_list::get_dp_cost.
 * In many cases, the former value is set to the function ::residue_pair_extended_list,
 * except for several RNA modes (for example \b mode \b rcoffee uses the keyword \c rna2) and in case a matrix is specified.
 *
 * \param[in] extend_mode String specifying the extension mode
 * \param[in,out] CL global Constraint_list object
 *
 */
Constraint_list * choose_extension_mode ( char *extend_mode, Constraint_list *CL)
{
  //evaluation_functions: residues start at 1, sequences at 0;
  
  if ( !CL)
    {
      fprintf ( stderr, "\nWarning: CL was not set");
      return CL;
    }
  else if ( strm ( extend_mode, "rna0"))
    {
      CL->evaluate_residue_pair=residue_pair_extended_list;
      CL->get_dp_cost      =slow_get_dp_cost;
    }
  else if ( strm ( extend_mode, "rna1") || strm (extend_mode, "rna"))
    {
      CL->evaluate_residue_pair=residue_pair_extended_list4rna1;
      CL->get_dp_cost      =slow_get_dp_cost;
    }
  else if ( strm ( extend_mode, "rna2"))
    {
      CL->evaluate_residue_pair=residue_pair_extended_list4rna2;
      CL->get_dp_cost      =slow_get_dp_cost;
    }
  else if ( strm ( extend_mode, "rna3"))
    {
      CL->evaluate_residue_pair=residue_pair_extended_list4rna3;
      CL->get_dp_cost      =slow_get_dp_cost;
    }
  else if ( strm ( extend_mode, "rna4"))
    {
      CL->evaluate_residue_pair=residue_pair_extended_list4rna4;
      CL->get_dp_cost      =slow_get_dp_cost;
    }
  else if ( strm ( extend_mode, "pc") && !CL->M)
    {
      CL->evaluate_residue_pair=residue_pair_extended_list_pc;
      CL->get_dp_cost      =slow_get_dp_cost;
    }
  else if ( strm ( extend_mode, "triplet") && !CL->M)
    {
      CL->evaluate_residue_pair=residue_pair_extended_list;
      CL->get_dp_cost      =get_dp_cost;
    }
  else if ( strm ( extend_mode, "relative_triplet") && !CL->M)
    {
      CL->evaluate_residue_pair=residue_pair_relative_extended_list;
      CL->get_dp_cost      =fast_get_dp_cost_2;
    }
  else if ( strm ( extend_mode, "g_coffee") && !CL->M)
    {
      CL->evaluate_residue_pair=residue_pair_extended_list_g_coffee;
      CL->get_dp_cost      =slow_get_dp_cost;
    }
  else if ( strm ( extend_mode, "g_coffee_quadruplets") && !CL->M)
    {
      CL->evaluate_residue_pair=residue_pair_extended_list_g_coffee_quadruplet;
      CL->get_dp_cost      =slow_get_dp_cost;
    }
  else if ( strm ( extend_mode, "fast_triplet") && !CL->M)
    {
      CL->evaluate_residue_pair=residue_pair_extended_list;
      CL->get_dp_cost      =fast_get_dp_cost;
    }
 
  else if ( strm ( extend_mode, "very_fast_triplet") && !CL->M)
    {
      CL->evaluate_residue_pair=residue_pair_extended_list;
      CL->get_dp_cost      =fast_get_dp_cost_2;
    }
  else if ( strm ( extend_mode, "slow_triplet") && !CL->M)
    {
      CL->evaluate_residue_pair=residue_pair_extended_list;
      CL->get_dp_cost      =slow_get_dp_cost;
    }
  else if (  strm ( extend_mode, "mixt") && !CL->M)
    {
      CL->evaluate_residue_pair=residue_pair_extended_list_mixt;
      CL->get_dp_cost=slow_get_dp_cost;
    }
  else if (  strm ( extend_mode, "quadruplet") && !CL->M)
    {
      CL->evaluate_residue_pair=residue_pair_extended_list_quadruplet;
      CL->get_dp_cost      =get_dp_cost_quadruplet;
    }
  else if (  strm ( extend_mode, "test") && !CL->M)
    {
      CL->evaluate_residue_pair=residue_pair_test_function;
      CL->get_dp_cost      =slow_get_dp_cost_test;
    }
  else if (  strm ( extend_mode, "ssp"))
    {
      
      CL->evaluate_residue_pair=evaluate_ssp_matrix_score;
      CL->get_dp_cost=slow_get_dp_cost;
      CL->normalise=1;
    }
  else if (  strm ( extend_mode, "tm"))
    {

      CL->evaluate_residue_pair=evaluate_tm_matrix_score;
      CL->get_dp_cost=slow_get_dp_cost;
      CL->normalise=1;
    }
  else if (  strm ( extend_mode, "matrix"))
    {
      
      CL->evaluate_residue_pair=evaluate_matrix_score;
      CL->get_dp_cost=cw_profile_get_dp_cost;
      CL->normalise=1;
    }
  else if ( strm ( extend_mode, "curvature"))
    {
       CL->evaluate_residue_pair=evaluate_curvature_score;
       CL->get_dp_cost=slow_get_dp_cost;
       CL->normalise=1;
    }
  else if ( CL->M)
    {
      CL->evaluate_residue_pair=evaluate_matrix_score;
      CL->get_dp_cost=cw_profile_get_dp_cost;
      CL->normalise=1;
    } 
  else 
    {
      fprintf ( stderr, "\nERROR: %s is an unknown extend_mode[FATAL:%s]\n", extend_mode, PROGRAM);
      myexit (EXIT_FAILURE);
    }
  
  return CL;
}
int ** combine_two_matrices ( int **mat1, int **mat2)
{
  int naa, re1, re2, Re1, Re2, a, b, u, l;

  naa=(int)strlen (BLAST_AA_ALPHABET);
  for ( a=0; a< naa; a++)
    for ( b=0; b< naa; b++)
      {
	re1=BLAST_AA_ALPHABET[a];
	re2=BLAST_AA_ALPHABET[b];
	if (re1=='*' || re2=='*');
	else
	  {

	    Re1=toupper(re1);Re2=toupper(re2);
	    re1-='A';re2-='A';Re1-='A';Re2-='A';
	    
	    l=mat1[re1][re2];
	    u=mat2[re1][re2];
	    mat1[re1][re2]=mat2[re1][re2]=l;
	    mat2[Re1][Re2]=mat2[Re1][Re2]=u;
	  }
      }
  return mat1;
}

/* Off the shelves evaluations */
/*********************************************************************************************/
/*                                                                                           */
/*         OFF THE SHELVES EVALUATION                                              */
/*                                                                                           */
/*********************************************************************************************/


int lat_sum_pair (Alignment *A, char *mat)
{
  int a,b,c, tot=0, v1, v2, score;
  int **matrix;
  
  matrix=read_matrice (mat);
  
  for (a=0; a<A->nseq; a++)
    for ( b=0; b<A->nseq; b++)
      {
	for (c=1; c<A->len_aln; c++)
	  {
	    char r11, r12;

	    r11=A->seq_al[a][c-1];
	    r12=A->seq_al[a][c];
	    if (is_gap(r11) || is_gap(r12))continue;
	    else v1=matrix[r11-'A'][r12-'A'];

	    r11=A->seq_al[b][c-1];
	    r12=A->seq_al[b][c];
	    if (is_gap(r11) || is_gap(r12))continue;
	    else v2=matrix[r11-'A'][r12-'A'];

	    score+=(v1-v2)*(v1-v2);
	    tot++;
	  }
      }
  score=(100*score)/tot;
  return (float)score;
}

	    
     
/* Off the shelves evaluations */
/*********************************************************************************************/
/*                                                                                           */
/*         OFF THE SHELVES EVALUATION                                              */
/*                                                                                           */
/*********************************************************************************************/

int comp_pair ( int len,char *sa, char *sb, int seqA, int seqB,int *tgp_a, int *tgp_b,int gap_op,int gap_ex, int start, int end,int **matrix,int MODE);
int score_gap ( int len, char *sa, char *sb,int seqA, int seqB,int *tgp_a, int *tgp_b,  int op, int ex, int start, int end, int MODE);
void evaluate_tgp_decoded_chromosome ( Alignment *A,int **TGP,int start, int end,int MODE);
int gap_type ( char a, char b);



float  sum_pair ( Alignment*A,char *mat_name, int gap_op, int gap_ex)
    {
	int a,b;
	float pscore=0;

	int start, end;
	static int **tgp;
	double score=0;
	int MODE=1;
	int **matrix;

	matrix=read_matrice (mat_name);
	matrix=mat2inverted_mat (matrix, "acdefghiklmnpqrstvwy");
	
	start=0;
	end=A->len_aln-1;
	
	if ( tgp==NULL)
	    tgp= declare_int (A->nseq,2); 
	
	evaluate_tgp_decoded_chromosome ( A,tgp,start, end,MODE);
	
	for ( a=0; a< A->nseq-1; a++)
		 for (b=a+1; b<A->nseq; b++)
			{
			  pscore= comp_pair (A->len_aln,A->seq_al[a], A->seq_al[b],a, b,tgp[a], tgp[b],gap_op,gap_ex, start, end,matrix, MODE);	
			  score+=pscore*100;
			  /*score+=(double)pscore*(int)(PARAM->OFP)->weight[A->order[a][0]][A->order[b][0]];*//*NO WEIGHTS*/
			}
	
	score=score/(A->nseq*A->nseq);
	
	return (float)score;
	}
	
int comp_pair ( int len,char *sa, char *sb, int seqA, int seqB,int *tgp_a, int *tgp_b,int gap_op,int gap_ex, int start, int end,int **matrix,int MODE)
	{
	  int score=0, a, ex;
	
	

	if ( end-start>=0)
	    score+= score_gap (len, sa,sb, seqA, seqB,tgp_a, tgp_b, gap_op,gap_ex, start, end,MODE);
	
	ex=gap_ex;


	for (a=start; a<=end; a++)
		{
		  if ( is_gap(sa[a]) || is_gap(sb[a]))
		    {
		      if (is_gap(sa[a]) && is_gap(sb[a]));
		      else
			{
			  
			  score +=ex;
			}
		    }
		  else
			{
			score += matrix [sa[a]-'A'][sb[a]-'A'];	
				
			}
		}
	return score;
	}
int score_gap ( int len, char *sa, char *sb,int seqA, int seqB,int *tgp_a, int *tgp_b,  int op, int ex, int start, int end, int MODE)
	{
	  int a,b;
	int ga=0,gb=0;
	int score=0;


	int right_gap, left_gap;





	int type;
	int flag1=0;
	int flag2=0;
	int continue_loop;
	int sequence_pattern[2][3];	    
	int null_gap;
	int natural_gap=1;
	
	/*op= gor_gap_op ( 0,seqA, seqB, PARAM);
	ex= gor_gap_ext ( 0, seqA, seqB, PARAM);*/

	

	for (a=start; a<=end; ++a)			
		{
		  
		type= gap_type ( sa[a], sb[a]);
	
		if ( type==2 && ga<=gb)
			{++ga;
			 gb=0;
			 score += op;
			}
		else if (type==1 && ga >=gb)
			{
			++gb;
			ga=0;
			score +=op;
			}
		else if (type==0)
			{
			ga++;
			gb++;
			}
			
		else if (type== -1)
			ga=gb=0;
		
			
		if (natural_gap==0)
		    {
		    if ( type== -1)
			flag1=flag2=0;
		    else if ( type==0)
			flag2=1;
		    else if ( (type==flag1) && flag2==1)
			{
			score+=op;
			flag2=0;
			}
		    else if ( (type!=flag1) && flag2==1)
			{
			flag1=type;
			flag2=0;
			}
		    else if ( flag2==0)
			flag1=type;
		    }   
		 }
	  /*gap_type -/-:0, X/X:-1 X/-:1, -/X:2*/
/*evaluate the pattern of gaps*/

	continue_loop=1;
	sequence_pattern[0][0]=sequence_pattern[1][0]=0;
	for ( a=start; a<=end && continue_loop==1; a++)
	    {
	    left_gap= gap_type ( sa[a], sb[a]);
	    if ( left_gap!= 0)
		{
		if ( left_gap==-1)
		    {
		    sequence_pattern[0][0]=sequence_pattern[1][0]=0;		
		    continue_loop=0;
		    }
		else 
		    {
		    null_gap=0;
		    for (b=a; b<=end && continue_loop==1; b++)
			{type=gap_type( sa[b], sb[b]);
			if (type==0)
			    null_gap++;    
			if ( type!=left_gap && type !=0)
			    {
			    continue_loop=0;
			    sequence_pattern[2-left_gap][0]= b-a-null_gap;
			    sequence_pattern [1-(2-left_gap)][0]=0;
			    }
			 }
		     if ( continue_loop==1)
			{
			continue_loop=0;
			sequence_pattern[2-left_gap][0]= b-a-null_gap;
			sequence_pattern [1-(2-left_gap)][0]=0;
			}
		     }    
		  }
	       }	
	
	   sequence_pattern[0][2]=sequence_pattern[1][2]=1;
	   for ( a=start; a<=end; a++)
		{
		  if ( !is_gap(sa[a]))
		    sequence_pattern[0][2]=0;
		  if ( !is_gap(sb[a]))
		    sequence_pattern[1][2]=0;

		}
	   continue_loop=1;
	   sequence_pattern[0][1]=sequence_pattern[1][1]=0;		
	   for ( a=end; a>=start && continue_loop==1; a--)
	    {
	    right_gap= gap_type ( sa[a], sb[a]);
	    if ( right_gap!= 0)
		{
		if ( right_gap==-1)
		    {
		    sequence_pattern[0][1]=sequence_pattern[1][1]=0;		
		    continue_loop=0;
		    }
		else 
		    {
		    null_gap=0;
		    for (b=a; b>=start && continue_loop==1; b--)
			{type=gap_type( sa[b], sb[b]);
			if ( type==0)
			    null_gap++;
			if ( type!=right_gap && type !=0)
			    {
			    continue_loop=0;
			    sequence_pattern[2-right_gap][1]= a-b-null_gap;
			    sequence_pattern [1-(2-right_gap)][1]=0;
			    }
		        }
		     if ( continue_loop==1)
			{
			continue_loop=0;
			sequence_pattern[2-right_gap][1]= a-b-null_gap;
			sequence_pattern [1-(2-right_gap)][1]=0;
			}
		     }
		  }
	       }	      		

/*
printf ( "\n*****************************************************");
printf ( "\n%c\n%c", sa[start],sb[start]);
printf ( "\n%d %d %d",sequence_pattern[0][0] ,sequence_pattern[0][1], sequence_pattern[0][2]);
printf ( "\n%d %d %d",sequence_pattern[1][0] ,sequence_pattern[1][1], sequence_pattern[1][2]);
printf ( "\n*****************************************************");
*/

/*correct the scoring*/


	if ( MODE==0)
	    {  
	    if ( FABS(tgp_a[0])>1 && (FABS(tgp_a[0])>FABS( tgp_b[0])))
		score-= (sequence_pattern[0][0]>0)?op:0;	 
	    if ( FABS(tgp_b[0])>1 && (FABS(tgp_b[0])> FABS(tgp_a[0])))
		score-= (sequence_pattern[1][0]>0)?op:0;
	    }
	else if ( MODE ==1 || MODE ==2)
	    {
	    if ( FABS(tgp_a[0])>1 && (FABS(tgp_a[0])>FABS( tgp_b[0])) && (tgp_a[1]!=1 || sequence_pattern[0][2]==0))
		score-= (sequence_pattern[0][0]>0)?op:0;	 
	    if ( FABS(tgp_b[0])>1 && (FABS(tgp_b[0])> FABS(tgp_a[0])) && (tgp_b[1]!=1 || sequence_pattern[1][2]==0))
		score-= (sequence_pattern[1][0]>0)?op:0;
	

	    if ( tgp_a[0]>=1 && tgp_a[0]==tgp_b[0])
		score -=(sequence_pattern[0][0]>0)?op:0;		
	    if ( tgp_b[0]>=1 && tgp_a[0]==tgp_b[0])
		score-= (sequence_pattern[1][0]>0)?op:0; 
		

	    if ( tgp_a[1]==1 && sequence_pattern[0][2]==0)
		score -= ( sequence_pattern[0][1]>0)?op:0;	
	    else if ( tgp_a[1]==1 && sequence_pattern[0][2]==1 && tgp_a[0]<=0)
		score -= ( sequence_pattern[0][1]>0)?op:0;
		

	    if ( tgp_b[1]==1 && sequence_pattern[1][2]==0)
		score -= ( sequence_pattern[1][1]>0)?op:0;		
	    else if ( tgp_b[1]==1 && sequence_pattern[1][2]==1 && tgp_b[0]<=0)
		score -= ( sequence_pattern[1][1]>0)?op:0;		
	    
	    if ( MODE==2)
		{
		if ( tgp_a[0]>0)
		    score -=sequence_pattern[0][0]*ex;
		if ( tgp_b[0]>0)
		    score -= sequence_pattern[1][0]*ex;
		if ( tgp_a[1]>0)
		    score-=sequence_pattern[0][1]*ex;
		if ( tgp_b[1]>0)
		    score-=sequence_pattern[1][1]*ex;
		}
	    }	 

																		 		        			
	return score;	    



	}	
void evaluate_tgp_decoded_chromosome ( Alignment *A,int **TGP,int start, int end,int MODE)
    {
    int a,b;    
    int continue_loop;
    
    
    
    if (MODE==11 || MODE==13|| MODE==14)
    	{
    	if ( start==0)for ( a=0; a<A->nseq; a++)TGP[a][0]=-1;
    	else for ( a=0; a<A->nseq; a++)TGP[a][0]=(is_gap(A->seq_al[a][start-1])==1)?0:1;
    	
    	if ( end==A->len_aln-1)for ( a=0; a<A->nseq; a++)TGP[a][1]=-1;
    	else for ( a=0; a<A->nseq; a++)TGP[a][1]=(is_gap(A->seq_al[a][start-1])==1)?0:1;
    	}
    else
    	{
    	/* 0: in the middle of the alignement 
       	1: natural end
       	2: q left gap is the continuation of another gap that was open outside the bloc ( don't open it)
    	*/

    	for ( a=0; a< A->nseq; a++)
	  {
	    TGP[a][0]=1;
	    TGP[a][1]=1;
	    for ( b=0; b< start; b++)
	      if ( !is_gap(A->seq_al[a][b]))
		TGP[a][0]=0;
	    if ( start>0 )
	      {
		if (is_gap(A->seq_al[a][start-1]) && TGP[a][0]!=1)
		  {TGP[a][0]=-1;
		    continue_loop=1;
		    for ( b=(start-1); b>=0 && continue_loop==1; b--)
		      {TGP[a][0]-= ( is_gap(A->seq_al[a][b])==1)?1:0;
			continue_loop= (is_gap(A->seq_al[a][b])==1)?continue_loop:0;
		      }
		  }
	      }
	      else if (is_gap(A->seq_al[a][start-1]) && TGP[a][0]==1)
		{
		  TGP[a][0]=1;
		  continue_loop=1;
		  for ( b=(start-1); b>=0 && continue_loop==1; b--)
		    {TGP[a][0]+= ( is_gap(A->seq_al[a][b])==1)?1:0;
		      continue_loop= (is_gap(A->seq_al[a][b])==1)?continue_loop:0;
		    }
		} 
	    for ( b=(A->len_aln-1); b>end; b--)
	      if ( !is_gap(A->seq_al[a][b]))
		TGP[a][1]=0;
	  }
    	}	
    }	
int gap_type ( char a, char b)
    {
    /*gap_type -/-:0, X/X:-1 X/-:1, -/STAR:2*/

      if ( is_gap(a) && is_gap(b))
	return 0;
      else if ( !is_gap(a) && !is_gap(b))
	return -1;
      else if ( !is_gap(a))
	return 1;
      else if ( !is_gap(b))
	return 2;
      else
	return -1;
    }
 
