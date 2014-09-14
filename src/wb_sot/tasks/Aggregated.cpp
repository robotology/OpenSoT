/*
 * Copyright (C) 2014 Walkman
 * Author: Alessio Rocchi
 * email:  alessio.rocchi@iit.it
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

#include <wb_sot/tasks/Aggregated.h>
#include <yarp/math/Math.h>
#include <assert.h>

using namespace wb_sot::tasks;
using namespace yarp::math;

Aggregated::Aggregated(const std::list< TaskType* >& tasks,
                       const unsigned int x_size) :
    Task(std::string("aggregated"),x_size), _tasks(tasks)
{
    /* calling update to generate bounds */
    this->update(yarp::sig::Vector(x_size, 0.0));
}

Aggregated::~Aggregated()
{
}

void Aggregated::update(const yarp::sig::Vector& x) {
    this->getConstraints().clear();
    _A.resize(0,x.size());
    _b.resize(0);
    for(std::list< TaskType *>::iterator i = _tasks.begin();
        i != _tasks.end(); ++i) {
        (*i)->update(x);
        _A = yarp::math::pile(_A,(*i)->getA());
        _b = yarp::math::cat(_b, (*i)->getb());
        for(std::list< BoundType >::iterator j = (*i)->getConstraints().begin();
            j!= (*i)->getConstraints().end(); ++j) {
            this->getConstraints().push_back(*j);
        }
    }
}