/*
 * Copyright (C) 2014 Walkman
 * Author: Alessio Rocchi, Enrico Mingo
 * email:  alessio.rocchi@iit.it, enrico.mingo@iit.it
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

#include <wb_sot/tasks/velocity/CoM.h>
#include <yarp/math/Math.h>
#include <drc_shared/cartesian_utils.h>
#include <exception>
#include <cmath>

using namespace wb_sot::tasks::velocity;
using namespace yarp::math;

CoM::CoM(   const yarp::sig::Vector& x,
            iDynUtils &robot,
            const bool updateModel) :
    Task("com", x, 3), _robot(robot), _updateModel(updateModel)
{
    _support_foot_link_index = _robot.left_leg.index;
    _swing_foot_link_index = _robot.right_leg.index;

    /* first update. Setting desired pose equal to the actual pose */
    this->update(_x0);

    /* initializing to zero error */
    _desiredPosition = _actualPosition;
    _b = _zeroVector;

    _W.resize(3,3);
    _W.eye();

    //_referenceInputPort.open("/wb_sot/tasks/velocity/CoM/" + _task_id + "/set_ref:i");
}

CoM::~CoM()
{
   //_referenceInputPort.close();
}

void CoM::update(const yarp::sig::Vector &x) {
    /** TODO when using a cartesian task, we could update the model at the aggregate level
             instead of each cartesian task, to save computation time */
    if(_updateModel)
        _robot.updateiDyn3Model(x, _zeroVector, _zeroVector);

    /************************* COMPUTING TASK *****************************/

    _actualPosition = _robot.coman_iDyn3.getCOM("",_support_foot_link_index);
    //This part of code is an HACK due to a bug in iDynTree
    int floating_base_old_index = _robot.coman_iDyn3.getFloatingBaseLink();

    _robot.coman_iDyn3.setFloatingBaseLink(_support_foot_link_index);
    assert(_robot.coman_iDyn3.getCOMJacobian(_A));
    _robot.coman_iDyn3.setFloatingBaseLink(floating_base_old_index);

    _A = _A.removeCols(0,6);    // remove floating base
    _A = _A.removeRows(3,3);    // remove orientation

    _b = _desiredPosition - _actualPosition;
    /**********************************************************************/
}

void CoM::setReference(const yarp::sig::Vector& desiredPosition) {
    _desiredPosition = desiredPosition;
}



