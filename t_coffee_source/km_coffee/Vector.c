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
// #include "Vector.h"
#include<iostream>
#include<cstring>
#include<string>
#include "km_coffee_header.h"




//char *groups[]={"LlVvIiMmCc","AaGgSsTtPp","FfYyWw","EeDdNnQqKkRrHh"};
//size_t n_groups = 4;
// char *groups[]={"FfLlVvIiMmCcAaGgSsTtPpYyWw","DdEeNnQqKkRrHh"};
// size_t n_groups = 2;

// double hydrophobic[]={ 0.159, 0, 0.778, -1.289, -1.076, 1.437, -0.131, -0553, 1.388, 0, -1.504, 1.236, 1.048, -0.866, 0, -0.104, -0.836, -1.432, -0.549, -0.292, 0, 1.064, 1.064, 0, 0.476, 0 };
//A-Ala, B-0, C-Cys, D-Asp, E-Glu, F-Phe, G-Gly, H-His, I-Ile, J-0, K-Lys, L-Leu, M-Met, N-Asn, O-0, P-Pro, Q-Gln, R-Arg, S-Ser, T-Thr, U-0, V-Val, W-Trp, X, Y-Tyr, Z


void calc_H(const char *seq, double *hydrophobic, double *H)
{
	size_t ProtLength = strlen(seq), i;
	double sumH=0;
	int strangeCh=0, a;

	for (i=0; i<ProtLength; ++i)
	{
		sumH += hydrophobic[ toupper(seq[i])-65 ];
		if( hydrophobic[ toupper(seq[i])-65 ] == 0 ){ strangeCh++; }
	}

	(*H)=sumH/(ProtLength-strangeCh);
}



void calc_IEP(char *seq, double *pI )
{
	size_t ProtLength = strlen(seq);

	char Asp = 'D';
	char Glu = 'E';
	char Cys = 'C';
	char Tyr = 'Y';
	char His = 'H';
	char Lys = 'K';
	char Arg = 'R';

	int AspNumber = 0;
	int GluNumber = 0;
	int CysNumber = 0;
	int TyrNumber = 0;
	int HisNumber = 0;
	int LysNumber = 0;
	int ArgNumber = 0;

	int i = 0;

	for ( i=0; i<=ProtLength-1; ++i)              // looking for charged amino acids
	{
		if (toupper(seq[i]) == Asp)
			++AspNumber;

		if (toupper(seq[i]) == Glu)
			++GluNumber;

		if (toupper(seq[i]) == Cys)
			++CysNumber;

		if (toupper(seq[i]) == Tyr)
			++TyrNumber;

		if (toupper(seq[i]) == His)
			++HisNumber;

		if (toupper(seq[i]) == Lys)
			++LysNumber;

		if (toupper(seq[i]) == Arg)
			++ArgNumber;
	}

	double NQ = 0.0; //net charge in given pH

	double QN1=0;  //C-terminal charge
	double QN2=0;  //D charge
	double QN3=0;  //E charge
	double QN4=0;  //C charge
	double QN5=0;  //Y charge
	double QP1=0;  //H charge
	double QP2=0;  //NH2 charge
	double QP3=0;  //K charge
	double QP4=0;  //R charge

	double pH = 6.5;             //starting point pI = 6.5 - theoretically it should be 7, but
	//average protein pI is 6.5 so we increase the probability of finding the solution
	double pHprev = 0.0;
	double pHnext = 14.0;        //0-14 is possible pH range
	double E = 0.01;             //epsilon means precision [pI = pH ± E]
	double temp = 0.0;

	//%%%%%%%%%%%%%%%%%%%%%%%%%   CALCUTE IEP   %%%%%%%%%%%%%%%%%%%%%%%%

	for(;;){                //the infinite loop --- pK values from Wikipedia

		QN1=-1/(1+pow(10,(3.65-pH)));
		QN2=-AspNumber/(1+pow(10,(3.9-pH)));
		QN3=-GluNumber/(1+pow(10,(4.07-pH)));
		QN4=-CysNumber/(1+pow(10,(8.18-pH)));
		QN5=-TyrNumber/(1+pow(10,(10.46-pH)));
		QP1=HisNumber/(1+pow(10,(pH-6.04)));
		QP2=1/(1+pow(10,(pH-8.2)));
		QP3=LysNumber/(1+pow(10,(pH-10.54)));
		QP4=ArgNumber/(1+pow(10,(pH-12.48)));

		NQ=QN1+QN2+QN3+QN4+QN5+QP1+QP2+QP3+QP4;


		if(pH>=14.0){
			printf("ERROR: Something is wrong - pH is higher than 14.\n");
			exit(1);
		}

		//%%%%%%%%%%%   BISECTION   %%%%%%%%%%%%%

		if(NQ<0){              //out of range -- the new pH value must be smaller
			temp = pH;
			pH = pH-((pH-pHprev)/2);
			pHnext = temp;
		}
		else{                 //too small pH value, so we have to increase it --- swap
			temp = pH;
			pH = pH + ((pHnext-pH)/2);
			pHprev = temp;
		}

		if ((pH-pHprev<E)&&(pHnext-pH<E)) //finding isoelectric point
			break;
	}

	(*pI)=pH; 	//printf("ProtLength: %d  pI: %lf\n",ProtLength, pH );
}



Vector * seq2vec_kmer(const Seq *seq, short k, unsigned int *factor, size_t vec_len, size_t vec_num, short *alphabet, int *used)
{
	Vector *t = (Vector*) my_malloc(sizeof(Vector));
	t->data = (double*) my_calloc(vec_len, sizeof(double));
	t->id = vec_num;
	size_t i;
	unsigned int value = 0;
	for (i = 0; i<k; ++i)
		value += factor[i] * alphabet[(int)seq->seq[i]];
	++t->data[used[value]];
	size_t j=0;
	unsigned char c;
	size_t seq_len = seq->size;
	t->seq_len = seq_len-k+1;
	for (i = k; i<seq_len; ++i)
	{
		c = alphabet[(int)seq->seq[j]];
		value -= factor[0]*c;
		value *= factor[k-2];
		value += alphabet[(int)seq->seq[i]];
		++j;
		++t->data[used[value]];
	}

	return t;
}


double l2norm(Vector *vec, size_t size)
{
	int i;
	double norm=0;
	double *data = vec->data;
	for (i=0; i<size; ++i)
		norm += data[i]*data[i];
	return sqrt(norm);
}

void normalize(VectorSet *vec_set)
{
	size_t vec_len=vec_set->dim;
	size_t n_vecs = vec_set->n_vecs;

	//calculate mean
	Vector *mean = (Vector*)my_malloc(sizeof(Vector));
	mean->data = (double*) my_calloc(vec_len, sizeof(double));
	double *md=mean->data;
	double *vec;
	size_t i,j;
	for (i=0;i<n_vecs;++i)
	{
		vec=vec_set->vecs[i]->data;
		for (j=0;j<vec_len;++j)
			md[j]+=vec[j];
	}
	for (j=0;j<vec_len;++j)
		md[j]/=n_vecs;

	//calculate standard deviation
	Vector *sd = (Vector*)my_malloc(sizeof(Vector));
	sd->data = (double*)my_calloc(vec_len, sizeof(double));
	double *sdd = sd->data;
	for (i=0;i<n_vecs;++i)
	{
		vec=vec_set->vecs[i]->data;
		for (j=0;j<vec_len;++j)
			sdd[j]+=(vec[j]-md[j])*(vec[j]-md[j]);
	}
	for (j=0;j<vec_len;++j)
		sdd[j] = sqrt(sdd[j]/n_vecs);

	for (i=0;i<n_vecs;++i)
	{
		vec=vec_set->vecs[i]->data;
		for (j=0;j<vec_len;++j)
			vec[j]= (vec[j]-md[j])/sdd[j];
	}
}



Vector * seq2vec_dists(const Seq *seq, char * groups[], size_t n_groups, size_t vec_num)
{
	size_t s_len = seq->size;
	char *tmp_seq=seq->seq;
	size_t i,j;
	Vector *t = (Vector*)my_malloc(sizeof(Vector));
	t->id = vec_num;
	t->data =(double*) my_calloc(n_groups*2, sizeof(double));
	double *data = t->data;
	int *last_pos =(int*) my_malloc(n_groups*sizeof(int));
	for (i=0; i<n_groups; ++i)
		last_pos[i]=-1;
	for (i=0; i<s_len; ++i)
	{
		for (j=0; j<n_groups; ++j)
		{
			if (strchr(groups[j],tmp_seq[i]))
			{
				++data[j*2];
				if (last_pos[j] >-1)
					data[j*2+1]+=(i-last_pos[j]);
				last_pos[j]=i;
				break;
			}
		}
	}

	for (j=0; j<n_groups; ++j)
		if (data[j*2] > 0)
			data[j*2+1]/=data[j*2];

	free(last_pos);
	return t;
}


VectorSet* seqset2vecs_dist(SeqSet *seq_set, char *groups[], size_t n_groups)
{
	VectorSet *vec_set =(VectorSet*) my_malloc(sizeof(VectorSet));
	vec_set->dim = seq_set->seqs[0]->size;
	size_t n_seqs = seq_set->n_seqs;

	Vector **vecs= (Vector**)malloc(n_seqs*sizeof(Vector*));
	vec_set->n_vecs=n_seqs;
	vec_set->vecs=vecs;

	size_t i;
	for (i = 0; i<n_seqs; ++i)
		vecs[i] = seq2vec_dists(seq_set->seqs[i], groups, n_groups, i);
	vec_set->dim=n_groups*2;
// 	print_vecs(vec_set, "strange");
	return vec_set;
}



VectorSet* seqset2vecs_whatever(SeqSet *seq_set, char *groups[], size_t n_groups)
{

	double hydrophobic[]={ 0.159, 0, 0.778, -1.289, -1.076, 1.437, -0.131, -0553, 1.388, 0, -1.504, 1.236, 1.048, -0.866, 0, -0.104, -0.836, -1.432, -0.549, -0.292, 0, 1.064, 1.064, 0, 0.476, 0 };

	VectorSet *vec_set = (VectorSet*)my_malloc(sizeof(VectorSet));
	//vec_set->dim = seq_set->seqs[0]->size;
	size_t n_seqs = seq_set->n_seqs;

	Vector **vecs= (Vector**)malloc(n_seqs*sizeof(Vector*));
	vec_set->n_vecs=n_seqs;
	vec_set->vecs=vecs;

	size_t i;
	vec_set->dim=n_groups*2+1;

	for (i = 0; i<n_seqs; ++i)
	{
		vecs[i] = seq2vec_dists(seq_set->seqs[i], groups, n_groups, i);
		vecs[i] = (Vector*)realloc(vecs[i], vec_set->dim*sizeof(double));
// 		calc_H(seq_set->seqs[i]->seq, hydrophobic, &(vecs[i]->data[vec_set->dim-2]));
		calc_IEP(seq_set->seqs[i]->seq,  &(vecs[i]->data[vec_set->dim-1]));
	}
// 	print_vecs(vec_set, "strange");

// 	void calc_H(const char *seq, double *hydrophobic, double *H)

	return vec_set;
}

//A-Ala, B-0, C-Cys, D-Asp, E-Glu, F-Phe, G-Gly, H-His, I-Ile, J-0, K-Lys, L-Leu, M-Met, N-Asn, O-0, P-Pro, Q-Gln, R-Arg, S-Ser, T-Thr, U-0, V-Val, W-Trp, X, Y-Tyr, Z







int* identify_fields(const SeqSet *seq_set, short k, unsigned int *factor, size_t *vec_len, short *alphabet )
{

	size_t vec_length = *vec_len;
	int *used = (int*) my_calloc(vec_length, sizeof(int));
	int *value_arg = (int*)my_calloc(vec_length, sizeof(int));
	int *value_test = (int*)my_malloc(vec_length * sizeof(int));

	unsigned int value;
	Seq *seq;
	size_t seq_len, j, m;
	size_t vec_num = seq_set->n_seqs;

	short l;
/*
	seq = seq_set->seqs[0];
	value = 0;
	for (l = 0; l<k; ++l)
	{
		printf("%c\n", (int)seq->seq[l]);
		value += factor[l] * alphabet[(int)seq->seq[l]];
	}

	++value_arg[value];


	j=0;
	seq_len = seq->size;
	for (m = k; m<seq_len; ++m)
	{
		value -= factor[0]*alphabet[(int)seq->seq[j]];
		value *= factor[k-2];
		value += alphabet[(int)seq->seq[m]];
		++j;
		++value_arg[value];
	}*/


	size_t i;
	size_t x;
	for (i = 0; i<vec_num; ++i)
	{
		for (x = 0; x < vec_length; ++x)
		{
			value_test[x]=0;
		}
		seq = seq_set->seqs[i];
		value = 0;
		for (l = 0; l<k; ++l)
			value += factor[l] * alphabet[(int)seq->seq[l]];
		value_test[value]=1;


		j=0;
		seq_len = seq->size;
		for (m = k; m<seq_len; ++m)
		{
			value -= factor[0]*alphabet[(int)seq->seq[j]];
			value *= factor[k-2];
			value += alphabet[(int)seq->seq[m]];
			++j;
			++value_test[value];
			//if (value >124)
			//printf("%li %c%c%c %i %i %i\n", value,(int)seq->seq[j],(int)seq->seq[j-1],(int)seq->seq[j-2], alphabet[(int)seq->seq[j]], alphabet[(int)seq->seq[j-1]], alphabet[(int)seq->seq[j-2]]);
		}


		for (x = 0; x < vec_length; ++x)
		{
			if (value_test[x] != value_arg[x])
			{
				used[x]=1;
			}
		}

	}
	free(value_test);
	free(value_arg);

	j=0;
	for (i=0; i<vec_length; ++i)
	{
		if (used[i] != 0)
			used[i] = j++;
	}
	*vec_len=j;
	//printf("DIM=%li\n", *vec_len);
	return used;
}

int
my_variance_sort (const void *a, const void *b)
{
	double i = *(double*)a;
	double j = *(double*)b;
	if (i<j)
		return 1;
	if (i>j)
		return -1;
	else
		return 0;
}

typedef struct
{
	size_t id;
	double dist;
} dist_pair;


int dist_pair_compare (const void * a, const void * b)
{
	return ( ((dist_pair*)b)->dist - ((dist_pair*)a)->dist );
}

int extract_compare (const void * a, const void * b)
{
	return ( *(int*)a - *(int*)b );
}


SeqSet *
find_distant(SeqSet *seq_set, VectorSet *set)
{
	size_t n_vecs = set->n_vecs;
	size_t dim = set->dim;
	size_t i,j;

	dist_pair *avg_dist = (dist_pair*)malloc(n_vecs*sizeof(dist_pair));
	dist_pair *nearest_dist = (dist_pair*)malloc(n_vecs*sizeof(dist_pair));
	for (i =0; i<n_vecs; ++i)
	{
		avg_dist[i].id=set->vecs[i]->id;
		avg_dist[i].dist=0;
		nearest_dist[i].id=set->vecs[i]->id;
		nearest_dist[i].dist = INT_MAX;
	}

	double tmp_dist;
	for (i=0; i<n_vecs; ++i)
	{
		for (j=i+1; j<n_vecs; ++j)
		{
			tmp_dist= km_sq_dist(set->vecs[i], set->vecs[j], dim);
			avg_dist[i].dist += tmp_dist;
			avg_dist[j].dist += tmp_dist;
			if (tmp_dist < nearest_dist[i].dist)
				nearest_dist[i].dist = tmp_dist;
			if (tmp_dist < nearest_dist[j].dist)
				nearest_dist[j].dist = tmp_dist;
		}
	}



	qsort (nearest_dist, n_vecs, sizeof(dist_pair), dist_pair_compare);
	qsort (avg_dist, n_vecs, sizeof(dist_pair), dist_pair_compare);
	//printf("Most dist to nearest:\n");

	int to_extract = 10;
	int *extract_ids = (int*)malloc(to_extract*sizeof(int));
	for (i=0; i <to_extract; ++i)
	{
		extract_ids[i] = nearest_dist[i].id;
		//printf("%s %f\n", seq_set->seqs[nearest_dist[i].id]->name, nearest_dist[i].dist);
	}
	qsort(extract_ids, to_extract, sizeof(int), extract_compare);

	//SeqSet *extractedSet= malloc(sizeof(SeqSet));
	//extractedSet->reserved=to_extract;
	//extractedSet->n_seqs = to_extract;
	//extractedSet->seqs = malloc(to_extract*sizeof(Sequence*));
	int pos=0;
	j=0;
	for (i=0; i<n_vecs; ++i)
	{
		if (extract_ids[pos] == i)
			++pos;
		else
			set->vecs[j++] = set->vecs[i];
	}
	set->n_vecs -=10;


// 	printf("\nMost avg dist:\n");
// 	for (i=0; i <10; ++i)
// 	{
// 		printf("%s %f\n", seq_set->seqs[avg_dist[i].id]->name, avg_dist[i].dist);
// 	}

	free(avg_dist);
	free(nearest_dist);
	return NULL;//extractedSet;
}


int*
identify_fields_variance(const SeqSet *seq_set, short k, unsigned int *factor, size_t *vec_len, short *alphabet)
{
	size_t vec_length = *vec_len;
	int *used = (int*)my_calloc(vec_length, sizeof(int));
	double *mean =(double*) my_calloc(vec_length, sizeof(double));
	double *variance = (double*)my_calloc(vec_length, sizeof(double));
	double *value_test = (double*)my_malloc(vec_length * sizeof(double));

	size_t i;

	unsigned int value;
	Seq *seq;
	size_t seq_len, j, m;
	size_t n_vecs = seq_set->n_seqs;

// 	calc mean
	short l;
	for (i = 1; i<n_vecs; ++i)
	{
		seq = seq_set->seqs[i];
		value = 0;
		for (l = 0; l<k; ++l)
			value += factor[l] * alphabet[(int)seq->seq[l]];
		++mean[value];

		j=0;
		seq_len = seq->size;
		for (m = k; m<seq_len; ++m)
		{
			value -= factor[0]*alphabet[(int)seq->seq[j]];
			value *= factor[k-2];
			value += alphabet[(int)seq->seq[m]];
			++j;
			++mean[value];
		}
	}

	for (i = 0; i<vec_length; ++i)
		mean[i] /= n_vecs;


	//calc variance
	size_t x;
	for (i = 1; i<n_vecs; ++i)
	{
		for (x = 0; x < vec_length; ++x)
			value_test[x]=0;

		seq = seq_set->seqs[i];
		value = 0;
		for (l = 0; l<k; ++l)
			value += factor[l] * alphabet[(int)seq->seq[l]];


		j=0;
		seq_len = seq->size;
		for (m = k; m<seq_len; ++m)
		{
			value -= factor[0]*alphabet[(int)seq->seq[j]];
			value *= factor[k-2];
			value += alphabet[(int)seq->seq[m]];
			++j;
			++value_test[value];
		}

		for (j = 0; j<vec_length; ++j)
			variance[j] += (value_test[j]-mean[j])*(value_test[j]-mean[j]);
	}
	free(value_test);
	free(mean);

	for (i = 0; i<vec_length; ++i)
		variance[i] /= n_vecs;

	qsort (variance, vec_length, sizeof(double), my_variance_sort );
// 	for (i = 0; i<vec_length; ++i)
// 		printf("%f ", variance[i]);
// 	printf("\n");
	double threshold=variance[199];

	j=0;
	for (i=0; i<vec_length; ++i)
	{
		if (variance[i] >= threshold)
			used[i] = j++;
	}
	free(variance);
	*vec_len=j;
// 	printf("DIM=%li\n", *vec_len);
	return used;
}


VectorSet*
seqset2vecs_kmer(SeqSet *seq_set, short k, short alphabet_size, short *alphabet)
{
	VectorSet *vec_set = (VectorSet*)my_malloc(sizeof(VectorSet));
	vec_set->dim = seq_set->seqs[0]->size;
	size_t n_seqs = seq_set->n_seqs;
	unsigned int *factor = (unsigned int*)my_malloc(k*sizeof(unsigned int));
	factor[k-1] = 1;

	short j;
	for (j=k-2; j>=0; --j)
		factor[j] = factor[j+1] *alphabet_size;
	size_t vec_len = factor[0] *alphabet_size;

	Vector **vecs= (Vector**)malloc(n_seqs*sizeof(Vector*));
	vec_set->n_vecs=n_seqs;
	vec_set->vecs=vecs;
	int *used = identify_fields(seq_set, k, factor, &vec_len, alphabet);
	size_t i;
	for (i = 0; i<n_seqs; ++i)
		vecs[i] = seq2vec_kmer(seq_set->seqs[i], k, factor, vec_len, i, alphabet, used);
	vec_set->dim=vec_len;
// 	size_t l;
// 	double max_val=0;
// 	double tmp_val;
// 	double all=0;
// 	for(i=0; i<n_seqs; ++i)
// 	{
// 		max_val=0;
// 		for (l=0; l<n_seqs; ++l)
// 		{
// 			if (i!=l)
// 			{
// 			tmp_val=km_sq_dist(vecs[i], vecs[l], vec_len);
// 			if (tmp_val>max_val)
// 				max_val=tmp_val;
// 			}
// 		}
// 		all+=max_val;
// 		printf("max_val:%f\n", max_val);
// 	}

// 	printf("ALL: %f\n", all/n_seqs);
// 		km_
// 	exit(1);

/*
	//TEST see if taking distances to x vectors make a better vector and calculate distances to them is better
	double *distances = my_calloc(n_seqs, sizeof(double));
	double tmp_score;


// 	calc nearest point to everybody.
	for (i = 0; i<n_seqs; ++i)
	{
		for (j = i+1; j<n_seqs; ++j)
		{
			tmp_score=km_muscle_dist(vecs[i], vecs[j], vec_len, k);
			distances[i] += tmp_score;
			distances[j] += tmp_score;
		}
	}

	unsigned int id =-1;
	tmp_score=DBL_MAX;
	for (i=0; i<n_seqs; ++i)
	{
		if (distances[i] < tmp_score)
		{
			tmp_score = distances[i];
			id=i;
		}
	}


// 	Take the next farthest points to any one existing in the set
	int n_dist_id=10;
	Vector **dist_points = my_malloc(n_dist_id*sizeof(Vector));
	dist_points[0]=new_vec(vecs[id], vec_len);
	for (i = 0; i<n_seqs; ++i)
		distances[i] = DBL_MAX;

	int l;
	double max_score=0;
	int max_id=-1;
	for (k=1; k<n_dist_id; ++k)
	{
		max_score=0;
		for (i=0; i< n_seqs; ++i)
		{
			tmp_score = km_muscle_dist(vecs[i], dist_points[k-1], vec_len, k);

			if (tmp_score <distances[i])
				distances[i] = tmp_score;
			if (distances[i] > max_score)
			{
				max_score = distances[i];
				max_id = i;
			}
		}
// 		printf("tmp_score: %i\n", max_id);
		dist_points[k]=new_vec(vecs[max_id], vec_len);
	}

	Vector *old_data;
	for (i=0; i< n_seqs; ++i)
	{
		old_data=vecs[i];
		vecs[i]=new_vec_nodata(old_data, n_dist_id);
		for(j=0; j<n_dist_id; ++j)
			vecs[i]->data[j] = km_muscle_dist(dist_points[j], old_data, vec_len, k);
		free(old_data->data);
		free(old_data);
	}


	vec_len=n_dist_id;

	//TEST test_end;

//*/

// 	find_distant(seq_set, vec_set);
//	exit(1);

	free(used);
	free(factor);
	return vec_set;
}


Vector *
new_vec(Vector *vec, int vec_len)
{
	Vector *new_vec=(Vector*)my_malloc(sizeof(Vector));
	new_vec->data=(double*)my_malloc(vec_len*sizeof(double));
	memcpy(new_vec->data, vec->data, vec_len*sizeof(double));
	new_vec->id = vec->id;
	new_vec->seq_len = vec->seq_len;
	new_vec->assignment = vec->assignment;
	return new_vec;
}

Vector *
new_vec_nodata(Vector *vec, int vec_len)
{
	Vector *new_vec=(Vector*)my_malloc(sizeof(Vector));
	new_vec->data=(double*)my_malloc(vec_len*sizeof(double));
	new_vec->id = vec->id;
	new_vec->seq_len = vec->seq_len;
	new_vec->assignment = vec->assignment;
	return new_vec;
}

void
delVecSet(VectorSet* set)
{
	size_t n_vecs = set->n_vecs;
	int i;
	Vector** vecs = set->vecs;
	for (i=0; i < n_vecs; ++i)
	{
		free(vecs[i]->data);
		free(vecs[i]);
	}
	free(vecs);
	free(set);
}


double
km_sq_dist(const Vector *vec1, const Vector *vec2, size_t dim)
{
	double dist = 0;
	double tmp;
	size_t i;
	double *d1=vec1->data;
	double *d2=vec2->data;
	for (i=0; i<dim; ++i)
	{
		tmp = d1[i]-d2[i];
		dist += (tmp * tmp);
	}
	return dist;
}


double
km_angle_dist(const Vector *vec1, const Vector *vec2, size_t dim)
{
	double dist = 0, len1=0, len2=0;
	size_t i;
	double *d1=vec1->data;
	double *d2=vec2->data;
	for (i=0; i<dim; ++i)
	{
		dist += d1[i]*d2[i];
		len1 += d1[i]*d1[i];
		len2 += d2[i]*d2[i];
	}
	double val =dist/(sqrt(len1)*sqrt(len2));
	if (val > 1.0)
		val = 1.0;
	return acos(val);
}


double
km_muscle_dist(const Vector *vec1, const Vector *vec2, size_t dim, int k)
{
	double dist = 0;
	size_t i;
	double *d1=vec1->data;
	double *d2=vec2->data;
	for (i=0; i<dim; ++i)
		dist += (d1[i] < d2[i])?d1[i]:d2[i];
	int min_len= (vec1->seq_len<vec2->seq_len)?vec1->seq_len:vec2->seq_len;
	min_len = min_len-k+1;
	return 1-dist/min_len;
}


void
print_vecs(VectorSet *set, char *out_f)
{
	size_t n_vecs = set->n_vecs;
	size_t dim = set->dim;
	size_t i,j;
	FILE *out_F = fopen(out_f, "w");
	double *data;
	for (i=0; i<n_vecs; ++i)
	{
		data=set->vecs[i]->data;
		fprintf(out_F, "%i ", i);
		for (j=0; j<dim; ++j)
			fprintf(out_F, "%f ", data[j]);
		fprintf(out_F, "\n");
	}
	fclose(out_F);
}

void
read_vecs(VectorSet *set, char *in_f)
{
	FILE *in_F = fopen(in_f, "r");
	const int LINE_LENGTH=1001;
	char line[LINE_LENGTH];
	size_t i=0, j=0;
	double *data;
	char *value;
	while (fgets(line, LINE_LENGTH, in_F)!=NULL)
	{
		data = set->vecs[i++]->data;
		data[0] = atof(strtok(line, " \t\n"));
		while ((value=strtok(NULL, " \t\n"))!=NULL)
			data[++j] = atof(value);
	}
	set->dim=j+1;
}

