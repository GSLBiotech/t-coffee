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
#ifndef KMEANS_H_
#define KMEANS_H_


#include "stdio.h"
#include "float.h"
#include "math.h"

#include "Vector.h"
#include "Stack.h"



typedef struct KM_node{
	size_t start;
	size_t end;
	struct KM_node **children;
	size_t id;
	size_t n_children;
} KM_node;


void
delKM_node(KM_node *node);


/**
 * \brief Initializes the centers by using the kmeans++ initialization.
 * \tparam double The type of data stored in the Vector.
 * \param vecs The set of vectors.
 * \param k The number of centers.
 * \param centers The vector where the centers will be saved in.
 * \param start The start of the position to consider.
 * \param end The end position to consider.
 *
 */
// void
// plusplus_init(const vector<boost::shared_ptr<Vector<double> > > &vecs, unsigned int k, vector<boost::shared_ptr<Vector<double> > > &centers, size_t start, size_t end);


/**
 * \brief Initializes the centers by randomly taking k vectors.
 * \param vecs The set of vectors.
 * \param k The number of centers.
 * \param centers The vector where the centers will be saved in.
 * \param start The start of the position to consider.
 * \param end The end position to consider.
 *
 */
// void
// random_init(const vector<boost::shared_ptr<Vector<double> > > &vecs, unsigned int k, vector<boost::shared_ptr<Vector<double> > > &centers, size_t start, size_t end);


/**
* \brief Initializes the centers using the KKZ method.
* Initialization as proposed in: I. Katsavounidis, C.-C. J. Kuo, and Z. Zhang. A new initialization tech-nique for generalized Lloyd iteration. IEEE Signal Processing Letters, 1(10):144?146, 1994
* Basic idea: It tries to find the most distant points in the dataset from each other using a greedy approach.
* \param vecs The set of vectors.
* \param k The number of centers.
* \param centers The vector where the centers will be saved in.
* \param start The start of the position to consider.
*
*/
VectorSet *
kkz_init(const VectorSet *vec_set, unsigned int k, size_t start, size_t end);



/**
 * \brief Initializes the centers by taking the first k vectors.
 * \param vecs The set of vectors.
 * \param k The number of centers.
 * \param centers The vector where the centers will be saved in.
 * \param start The start of the position to consider.
 *
 */
VectorSet *
first_init(const VectorSet *vec_set, unsigned int k,  size_t start);

int
my_assignment_sort (const void *i, const void *j);

//bool my_assignment_sort (boost::shared_ptr<Vector<unsigned int> > i, boost::shared_ptr<Vector<unsigned int> > j);

/**
 * \brief Repeated clustering until size threshold is met.
 *
 * \param vecs The vectors to cluster.
 * \param k the number of clusters to use.
 * \param init The methods to use for initializing the first centers: "random", "++" or "first"
 * \param error_threshold The change of error to stop kmeans
 */
KM_node*
hierarchical_kmeans(VectorSet *vec_set, unsigned int k, unsigned int k_leaf, const char *init, double error_threshold);
KM_node*
hierarchical_kmeans2(VectorSet *vec_set, unsigned int k, unsigned int k_leaf, const char *init, double error_threshold);

/**
 * \brief The kmeans clustering algorithm.
 *
 * \param vecs The vectors to cluster.
 * \param k the number of clusters to use.
 * \param init The methods to use for initializing the first centers: "random", "++" or "first"
 * \param error_threshold The change of error to stop kmeans
 */
void
kmeans(VectorSet *vecs, unsigned int k, const char *init, double error_threshold);

/**
 * \brief The kmeans clustering algorithm.
 *
 * \param vecs The vectors to cluster.
 * \param k the number of clusters to use.
 * \param init The methods to use for initializing the first centers
 * \param error_threshold The change of error to stop kmeans.
 * \param start The startint point for the clustering.
 * \param end The end point for the clustering.
 */
void
kmeans_sub(const VectorSet *vecs, unsigned int k, const char *init, double error_threshold, size_t start, size_t end);

/**
* \brief The kmeans clustering algorithm.
*
* \compared to kmeans_sub, this version produces more equal sized clusters
*
* \param vecs The vectors to cluster.
* \param k the number of clusters to use.
* \param init The methods to use for initializing the first centers
* \param error_threshold The change of error to stop kmeans.
* \param start The startint point for the clustering.
* \param end The end point for the clustering.
* \param return The centers found.
*/
VectorSet *
kmeans_sub_eq_size(const VectorSet *vecs, unsigned int k, const char *init, double error_threshold, size_t start, size_t end);


double
km_common(const double *vec1, const double *vec2, size_t dim);


#endif

