/*
 * Copyright (C) 2014 Walkman
 * Author: Enrico Mingo, Alessio Rocchi
 * email:  enrico.mingo@iit.it, alessio.rocchi@iit.it
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU Lesser General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * A copy of the license can be found at
 * https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
*/

#ifndef _WB_SOT_SOLVERS_QP_OASES_H_
#define _WB_SOT_SOLVERS_QP_OASES_H_

#include <qpOASES.hpp>
#include <wb_sot/Solver.h>
#include <vector>
#include <iostream>
#include <yarp/sig/all.h>
#include <wb_sot/Task.h>
#include <boost/shared_ptr.hpp>
#include <wb_sot/bounds/Aggregated.h>

using namespace yarp::sig;

namespace wb_sot{
    namespace solvers{

    /**
     * @brief The QPOasesProblem class handle variables, options and execution of a
     * single qpOases problem. Is implemented using yarp::sig Matrix and Vector.
     */
    class QPOasesProblem {
    public:
        /**
         * @brief QPOasesProblem Default constructor. If used remember to use
         * setProblem(const qpOASES::SQProblem& problem) to add a QP problem to
         * the class!
         * Ex.
         *
         *  QPOasesProblem p;
         *  qpOASES::SQProblem testProblem(2, 2, HST_IDENTITY);
         *  p.setProblem(testProblem);
         */
        QPOasesProblem();

        /**
         * @brief QPOasesProblem constructor with creation of a QP problem.
         * @param number_of_variables of the QP problem
         * @param number_of_constraints of the QP problem
         * @param hessian_type of the QP problem
         */
        QPOasesProblem(const int number_of_variables,
                       const int number_of_constraints,
                       qpOASES::HessianType hessian_type = qpOASES::HST_UNKNOWN);

        /**
          * @brief ~QPOasesProblem destructor
          */
        ~QPOasesProblem(){}

        /**
         * @brief setProblem copy a QP Problem in the internal object of the class.
         * @param problem to copy
         */
        void setProblem(const qpOASES::SQProblem& problem);

        /**
         * @brief getProblem return the internal QP problem
         * @return reference to internal QP problem
         */
        const qpOASES::SQProblem& getProblem(){return _problem;}

        /**
         * @brief getOptions return the options of the QP problem
         * @return reference to options
         */
        qpOASES::Options getOptions(){return _problem.getOptions();}

        /**
         * @brief setOptions of the QP problem. Default are set to:
         *      qpOASES::Options opt;
         *      opt.printLevel = qpOASES::PL_HIGH;
         *      opt.setToReliable();
         *      opt.enableRegularisation = qpOASES::BT_TRUE;
         *      opt.epsRegularisation *= 2E2;
         * @param options
         */
        void setOptions(const qpOASES::Options& options);

        /**
         * @brief initProblem initialize the QP problem and get the solution, the dual solution,
         * bounds and constraints.
         * The QP problem has the following structure:
         *
         *      min = ||Hx - g||
         *  st.     lA <= Ax <= uA
         *           l <=  x <= u
         * @param H
         * @param g
         * @param A
         * @param lA
         * @param uA
         * @param l
         * @param u
         * @return true if the problem can be solved
         */
        bool initProblem(const Matrix& H, const Vector& g,
                        const Matrix& A,
                        const Vector& lA, const Vector& uA,
                        const Vector& l, const Vector& u);

        /**
         * This set of function update current problem copying input data. Use these
         * methods to update existing matrices of the QP problem.
         */

        /**
         * @brief updateTask update H and g
         * @param H
         * @param g
         * @return true if the Problem has been initialized using initProblem( ... ) and
         * if the size of H and g is the same as the one in the QP problem.
         */
        bool updateTask(const Matrix& H, const Vector& g);

        /**
         * @brief updateConstraints update A, lA and uA
         * @param A
         * @param lA
         * @param uA
         * @return true if the Problem has been initialized using initProblem( ... ) and
         * if the size of A, lA and uA is the same as the one in the QP problem.
         */
        bool updateConstraints(const Matrix& A, const Vector& lA, const Vector& uA);

        /**
         * @brief updateBounds update l and u
         * @param l
         * @param u
         * @return rue if the Problem has been initialized using initProblem( ... ) and
         * if the size of l and u is the same as the one in the QP problem.
         */
        bool updateBounds(const Vector& l, const Vector& u);

        /**
         * @brief updateProblem update the whole problem
         * @param H
         * @param g
         * @param A
         * @param lA
         * @param uA
         * @param l
         * @param u
         * @return true if the previous update methods return true
         */
        bool updateProblem(const Matrix& H, const Vector& g,
                           const Matrix& A,
                           const Vector& lA, const Vector& uA,
                           const Vector& l, const Vector& u);

        /*
         * This set of function add input data to the problem
         */
        bool addTask(const Matrix& H, const Vector& g, const bool init_problem = true);
        bool addConstraints(const Matrix& A, const Vector& lA, const Vector& uA,
                            const bool init_problem = true);
        bool addBounds(const Vector& l, const Vector& u, const bool init_problem = true);
        bool addProblem(const Matrix& H, const Vector& g,
                           const Matrix& A,
                           const Vector& lA, const Vector& uA,
                           const Vector& l, const Vector& u);

        /**
         * @brief solve the QP problem
         * @return true if the QP problem is initialized and solved
         */
        bool solve();

        /**
         * @brief getSolution return the actual solution of the QP problem
         * @return solution
         */
        const Vector& getSolution(){return _solution;}

        /**
         * @brief getHessianType return the hessian type f the problem
         * @return hessian type
         */
        qpOASES::HessianType getHessianType(){return _problem.getHessianType();}

        /**
         * @brief setHessianType of the problem
         * @param ht hessian type
         */
        void setHessianType(const qpOASES::HessianType ht){_problem.setHessianType(ht);}

        /**
         * @brief getnWSR return maximum number of working set recalculations
         * @return maximum number of working set recalculations
         */
        int getnWSR(){return _nWSR;}

        /**
         * @brief setnWSR set maximum number of working set recalculations
         * @param nWSR Maximum number of working set recalculations
         */
        void setnWSR(const int nWSR){_nWSR = nWSR;}

        /**
         * @brief isQProblemInitialized
         * @return true if the internal problem is initialized
         */
        bool isQProblemInitialized(){return _is_initialized;}

        /**
         * @brief resetProblem call the reset method of the SQProblem
         * @return true if reset
         */
        bool resetProblem(){return _problem.reset();}

        /**
         * @brief getActiveBounds return the active bounds of the solved QP problem
         * @return active bounds
         */
        const qpOASES::Bounds& getActiveBounds(){return _bounds;}

        /**
         * @brief getActiveConstraints return the active constraints of the solved QP problem
         * @return active constraints
         */
        const qpOASES::Constraints& getActiveConstraints(){return _constraints;}

    protected:
        /**
         * @brief _problem is the internal SQProblem
         */
        qpOASES::SQProblem _problem;

        /**
         * @brief _bounds are the active bounds of the SQProblem
         */
        qpOASES::Bounds _bounds;

        /**
         * @brief _constraints are the active constraints of the SQProblem
         */
        qpOASES::Constraints _constraints;

        /**
         * @brief _nWSR is the maximum number of working set recalculations
         */
        int _nWSR;

        /**
         * @brief _is_initialized is set to true when the problem is initialized
         */
        bool _is_initialized;

        /**
         * Define a cost function: ||Hx - g||
         */
        Matrix _H;
        Vector _g;

        /**
         * Define a set of constraints weighted with A: lA <= Ax <= uA
         */
        Matrix _A;
        Vector _lA;
        Vector _uA;
        double * _A_ptr;
        double *_lA_ptr;
        double *_uA_ptr;


        /**
         * Define a set of bounds on solution: l <= x <= u
         */
        Vector _l;
        Vector _u;
        double *_l_ptr;
        double *_u_ptr;

        /**
         * Solution and dual solution of the QP problem
         */
        Vector _solution;
        Vector _dual_solution;
    };


    class QPOasesTask: public QPOasesProblem
    {
    public:
        QPOasesTask(const boost::shared_ptr< Task<Matrix, Vector> >& task);

        ~QPOasesTask();

        bool solve();

    protected:
        boost::shared_ptr< Task<Matrix, Vector> > _task;

        bool prepareData();
    };





    class QPOases_sot: public Solver<Matrix, Vector>
    {
    public:
        QPOases_sot();

        ~QPOases_sot(){}

        bool solve(Vector& solution);

    protected:

    };

    }
}

#endif