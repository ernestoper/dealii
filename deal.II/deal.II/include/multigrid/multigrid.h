//----------------------------  multigrid.h  ---------------------------
//    $Id$
//    Version: $Name$
//
//    Copyright (C) 1998, 1999, 2000, 2001 by the deal.II authors
//
//    This file is subject to QPL and may not be  distributed
//    without copyright and license information. Please refer
//    to the file deal.II/doc/license.html for the  text  and
//    further information on this license.
//
//----------------------------  multigrid.h  ---------------------------
#ifndef __deal2__multigrid_h
#define __deal2__multigrid_h


#include <base/subscriptor.h>
#include <base/smartpointer.h>
#include <lac/sparse_matrix.h>
#include <lac/vector.h>
#include <multigrid/mg_base.h>
#include <lac/forward_declarations.h>
#include <grid/forward_declarations.h>

#include <vector>


/**
 * Implementation of multigrid with matrices.
 * While @p{MGBase} was only an abstract framework for the V-cycle,
 * here we have the implementation of the pure virtual functions defined there.
 * Furthermore, level information is obtained from a triangulation object.
 * 
 * @author Wolfgang Bangerth, Guido Kanschat, 1999
 */
template <int dim>
class Multigrid : public MGBase
{
  public:
				     /**
				      * Constructor. Take the object
				      * describing the multilevel
				      * hierarchy of degrees of
				      * freedom, an object describing
				      * constraints to degrees of
				      * freedom on the global grid
				      * (for example for hanging
				      * nodes), a set of matrices
				      * which describe the equation
				      * discretized on each level,
				      * their respective sparsity
				      * patterns, and an object
				      * mediating the transfer of
				      * solutions between different
				      * levels.
				      *
				      * This function already
				      * initializes the vectors which
				      * will be used later on in the
				      * course of the
				      * computations. You should
				      * therefore create objects of
				      * this type as late as possible.
				      */
//TODO:[GK] meaning of minlevel, maxlevel?
    Multigrid(const MGDoFHandler<dim>                       &mg_dof_handler,
	      const ConstraintMatrix                        &constraints,
	      const MGLevelObject<SparsityPattern>       &level_sparsities,
	      const MGLevelObject<SparseMatrix<double> > &level_matrices,
	      const MGTransferBase                          &transfer,
	      const unsigned int                             minlevel = 0,
	      const unsigned int                             maxlevel = 1000000);
    
				     /**
				      * Transfer from a vector on the
				      * global grid to vectors defined
				      * on each of the levels
				      * separately, i.a. an @p{MGVector}.
				      */
    template<typename number>
    void copy_to_mg (const Vector<number> &src);

				     /**
				      * Transfer from multi-level vector to
				      * normal vector.
				      *
				      * Copies data from active
				      * portions of an MGVector into
				      * the respective positions of a
				      * @p{Vector<double>}. In order to
				      * keep the result consistent,
				      * constrained degrees of freedom
				      * are set to zero.
				      */
    template<typename number>
    void copy_from_mg (Vector<number> &dst) const;

				     /**
				      * Negative @p{vmult} on a level.
				      * @see MGBase.
				      */
//TODO:[GK] what does this function do?    
    virtual void level_vmult (const unsigned int    level,
			      Vector<double>       &dest,
			      const Vector<double> &src,
			      const Vector<double> &rhs);

				     /**
				      * Print a level vector using
				      * @ref{DoFHandler}.
				      */
    virtual void print_vector (const unsigned int level,
			       const Vector<double>& v,
			       const char* name) const;
    
				     /**
				      * Read-only access to level matrices.
				      */
    const SparseMatrix<double>& get_matrix (const unsigned int level) const;

  private:
				     /**
				      * Associated @p{MGDoFHandler}.
				      */
    SmartPointer<const MGDoFHandler<dim> > mg_dof_handler;


/**
 * Sparsity patterns for each level.
 */
    SmartPointer<const MGLevelObject<SparsityPattern> > level_sparsities;
    
				     /**
				      * Matrices for each level. The
				      * matrices are prepared by the
				      * constructor of @p{Multigrid} and
				      * can be accessed for
				      * assembling.
				      */
    SmartPointer<const MGLevelObject<SparseMatrix<double> > > level_matrices;

				     /**
				      * Pointer to the object holding
				      * constraints.
				      */
    SmartPointer<const ConstraintMatrix>                 constraints;
};


/**
 * Implementation of the @p{MGTransferBase} interface for which the transfer
 * operations are prebuilt upon construction of the object of this class as
 * matrices. This is the fast way, since it only needs to build the operation
 * once by looping over all cells and storing the result in a matrix for
 * each level, but requires additional memory.
 *
 * @author Wolfgang Bangerth, Guido Kanschat, 1999
 */
class MGTransferPrebuilt : public MGTransferBase 
{
  public:
				     /**
				      * Destructor.
				      */
    virtual ~MGTransferPrebuilt ();
    
				     /**
				      * Actually build the prolongation
				      * matrices for each level.
				      */
    template <int dim>
    void build_matrices (const MGDoFHandler<dim> &mg_dof);

				     /**
				      * Prolongate a vector from level
				      * @p{to_level-1} to level
				      * @p{to_level}. The previous
				      * content of @p{dst} is
				      * overwritten.
				      *
				      * @p{src} is assumed to be a vector with
				      * as many elements as there are degrees
				      * of freedom on the coarser level of
				      * the two involved levels, while @p{src}
				      * shall have as many elements as there
				      * are degrees of freedom on the finer
				      * level.
				      */
    virtual void prolongate (const unsigned int    to_level,
			     Vector<double>       &dst,
			     const Vector<double> &src) const;

				     /**
				      * Restrict a vector from level
				      * @p{from_level} to level
				      * @p{from_level-1} and add this
				      * restriction to
				      * @p{dst}. Obviously, if the
				      * refined region on level
				      * @p{from_level} is smaller than
				      * that on level @p{from_level-1},
				      * some degrees of freedom in
				      * @p{dst} are not covered and will
				      * not be altered. For the other
				      * degress of freedom, the result
				      * of the restriction is added.
				      *
				      * @p{src} is assumed to be a vector with
				      * as many elements as there are degrees
				      * of freedom on the finer level of
				      * the two involved levels, while @p{src}
				      * shall have as many elements as there
				      * are degrees of freedom on the coarser
				      * level.
				      */
    virtual void restrict_and_add (const unsigned int    from_level,
				   Vector<double>       &dst,
				   const Vector<double> &src) const;

  private:

    std::vector<SparsityPattern>   prolongation_sparsities;

				     /**
				      * The actual prolongation matrix.
				      * column indices belong to the
				      * dof indices of the mother cell,
				      * i.e. the coarse level.
				      * while row indices belong to the
				      * child cell, i.e. the fine level.
				      */
    std::vector<SparseMatrix<double> > prolongation_matrices;
};


/* --------------------------- inline functions --------------------- */


template<int dim>
inline
const SparseMatrix<double>&
Multigrid<dim>::get_matrix (const unsigned int level) const
{
  Assert((level>=minlevel) && (level<maxlevel),
	 ExcIndexRange(level, minlevel, maxlevel));
  
  return (*level_matrices)[level];
}


#endif
