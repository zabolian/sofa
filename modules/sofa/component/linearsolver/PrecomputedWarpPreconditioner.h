/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, version 1.0 beta 4      *
*                (c) 2006-2009 MGH, INRIA, USTL, UJF, CNRS                    *
*                                                                             *
* This library is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as published by    *
* the Free Software Foundation; either version 2.1 of the License, or (at     *
* your option) any later version.                                             *
*                                                                             *
* This library is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License *
* for more details.                                                           *
*                                                                             *
* You should have received a copy of the GNU Lesser General Public License    *
* along with this library; if not, write to the Free Software Foundation,     *
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.          *
*******************************************************************************
*                               SOFA :: Modules                               *
*                                                                             *
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
#ifndef SOFA_COMPONENT_LINEARSOLVER_PrecomputedWarpPreconditioner_H
#define SOFA_COMPONENT_LINEARSOLVER_PrecomputedWarpPreconditioner_H

#include <sofa/core/ObjectFactory.h>
#include <sofa/core/componentmodel/behavior/LinearSolver.h>
#include <sofa/component/linearsolver/MatrixLinearSolver.h>
#include <sofa/simulation/common/MechanicalVisitor.h>
#include <sofa/component/linearsolver/SparseMatrix.h>
#include <sofa/component/linearsolver/FullMatrix.h>
#include <sofa/helper/map.h>
#include <math.h>

//#define VALIDATE_ALGORITM_PrecomputedWarpPreconditioner

namespace sofa
{

namespace component
{

namespace linearsolver
{

using namespace sofa::core;
using namespace sofa::core::componentmodel;
using namespace sofa::defaulttype;

using namespace sofa::core::componentmodel::behavior;
using namespace sofa::simulation;
using namespace sofa::core::objectmodel;

using sofa::helper::system::thread::CTime;
using sofa::helper::system::thread::ctime_t;

template<class TDataTypes>
class PrecomputedWarpPreconditionerInternalData
{
public :
    typedef typename TDataTypes::Coord Coord;
    typedef typename Coord::value_type Real;

    SparseMatrix<Real> JR;
    FullMatrix<Real> JRMinv;
};

/// Linear system solver using the conjugate gradient iterative algorithm
template<class TDataTypes, class TMatrix, class TVector>
class PrecomputedWarpPreconditioner : public sofa::component::linearsolver::MatrixLinearSolver<TMatrix,TVector>
{
public:
    SOFA_CLASS(SOFA_TEMPLATE3(PrecomputedWarpPreconditioner,TDataTypes,TMatrix,TVector),SOFA_TEMPLATE2(sofa::component::linearsolver::MatrixLinearSolver,TMatrix,TVector));
    typedef TDataTypes DataTypes;
    typedef typename DataTypes::VecConst VecConst;
    typedef typename TDataTypes::VecDeriv VecDeriv;
    typedef typename DataTypes::VecCoord VecCoord;
    typedef typename TDataTypes::Coord Coord;
    typedef typename DataTypes::Deriv Deriv;
    typedef typename std::map<unsigned int, Deriv>::const_iterator ConstraintIterator;

    typedef sofa::component::linearsolver::MatrixLinearSolver<TMatrix,TVector> Inherit;
    typedef sofa::core::componentmodel::behavior::BaseMechanicalState::VecId VecId;

    typedef typename Coord::value_type Real;
    typedef MatNoInit<3, 3, Real> Transformation;

    Data<bool> f_verbose;
    Data<bool> use_file;
    Data <std::string> solverName;
    Data<int> init_MaxIter;
    Data<double> init_Tolerance;
    Data<double> init_Threshold;

#ifdef VALIDATE_ALGORITM_PrecomputedWarpPreconditioner
    TMatrix * realSystem;
    TMatrix * invertSystem;
#endif

    PrecomputedWarpPreconditioner();

    void solve (TMatrix& M, TVector& x, TVector& b);
    void invert(TMatrix& M);
    void setSystemMBKMatrix(double mFact=0.0, double bFact=0.0, double kFact=0.0);
    bool addJMInvJt(defaulttype::BaseMatrix* result, defaulttype::BaseMatrix* J, double fact);

    TMatrix * getSystemMatrixInv()
    {
        if (!this->currentGroup->systemMatrix) this->currentGroup->systemMatrix = new TMatrix();
        return this->currentGroup->systemMatrix;
    }

    /// Pre-construction check method called by ObjectFactory.
    /// Check that DataTypes matches the MechanicalState.
    template<class T>
    static bool canCreate(T*& obj, core::objectmodel::BaseContext* context, core::objectmodel::BaseObjectDescription* arg)
    {
        if (dynamic_cast<behavior::MechanicalState<DataTypes>*>(context->getMechanicalState()) == NULL) return false;
        return BaseObject::canCreate(obj, context, arg);
    }

    virtual std::string getTemplateName() const
    {
        return templateName(this);
    }

    static std::string templateName(const PrecomputedWarpPreconditioner<DataTypes,TMatrix,TVector>* = NULL)
    {
        return DataTypes::Name();
    }

protected :
    TVector R;
    TVector T;
    PrecomputedWarpPreconditionerInternalData<TDataTypes> internalData;

    void rotateConstraints();
    void loadMatrix();
    void loadMatrixWithCSparse();
    void loadMatrixWithSolver();

    template<class JMatrix>
    void ComputeResult(defaulttype::BaseMatrix * result,JMatrix& J, float fact);

    bool first;
    bool _rotate;
    bool usePrecond;
    double init_mFact;
    double init_bFact;
    double init_kFact;
    double dt;
    double factInt;
};


} // namespace linearsolver

} // namespace component

} // namespace sofa

#endif
