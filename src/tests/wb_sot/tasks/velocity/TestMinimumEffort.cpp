#include <drc_shared/tests_utils.h>
#include <drc_shared/cartesian_utils.h>
#include <drc_shared/idynutils.h>
#include <gtest/gtest.h>
#include <wb_sot/tasks/velocity/MinimumEffort.h>
#include <yarp/math/Math.h>
#include <yarp/math/SVD.h>

using namespace yarp::math;

namespace {

class testMinimumEffortTask: public ::testing::Test
{
protected:
    iDynUtils _robot;
    int nJ;

    testMinimumEffortTask()
    {
        nJ = _robot.coman_iDyn3.getNrOfDOFs();
    }

    virtual ~testMinimumEffortTask() {

    }

    virtual void SetUp() {

    }

    virtual void TearDown() {

    }

};

TEST_F(testMinimumEffortTask, testMinimumEffortTask_)
{
    // setting initial position with bent legs
    yarp::sig::Vector q_leg(6, 0.0),
                      q_whole(nJ, 1E-2);
    q_leg[0] = -25.0*M_PI/180.0;
    q_leg[3] =  50.0*M_PI/180.0;
    q_leg[5] = -25.0*M_PI/180.0;

    _robot.fromRobotToIDyn(q_leg, q_whole, _robot.left_leg);
    _robot.fromRobotToIDyn(q_leg, q_whole, _robot.right_leg);

    _robot.updateiDyn3Model(q_whole);

    OpenSoT::tasks::velocity::MinimumEffort minimumEffort(q_whole);

    EXPECT_EQ(minimumEffort.getA().rows(), nJ);
    EXPECT_EQ(minimumEffort.getb().size(), nJ);

    EXPECT_TRUE(minimumEffort.getWeight().rows() == nJ);
    EXPECT_TRUE(minimumEffort.getWeight().cols() == nJ);

    EXPECT_TRUE(minimumEffort.getConstraints().size() == 0);

    double K = 0.8;
    minimumEffort.setAlpha(K);
    EXPECT_DOUBLE_EQ(minimumEffort.getAlpha(), K);
    _robot.updateiDyn3Model(q_whole);
    double initial_effort = yarp::math::dot(_robot.coman_iDyn3.getTorques(),
                                    minimumEffort.getWeight()*_robot.coman_iDyn3.getTorques());
    for(unsigned int i = 0; i < 25; ++i)
    {
        minimumEffort.update(q_whole);
        double old_effort = minimumEffort.computeEffort();

        q_whole += pinv(minimumEffort.getA(),1E-6)*minimumEffort.getAlpha()*minimumEffort.getb();

        minimumEffort.update(q_whole);
        EXPECT_LE(minimumEffort.computeEffort(), old_effort);

    }
    _robot.updateiDyn3Model(q_whole);
    double final_effort = yarp::math::dot(_robot.coman_iDyn3.getTorques(),
                                    minimumEffort.getWeight()*_robot.coman_iDyn3.getTorques());
    EXPECT_LT(final_effort, initial_effort);
}

}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
