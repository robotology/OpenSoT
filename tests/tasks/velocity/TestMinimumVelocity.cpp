#include <idynutils/tests_utils.h>
#include <gtest/gtest.h>
#include <OpenSoT/tasks/velocity/MinimumVelocity.h>
#include <OpenSoT/constraints/velocity/JointLimits.h>
#include <yarp/math/Math.h>
#include <advr_humanoids_common_utils/conversion_utils_YARP.h>

using namespace yarp::math;

namespace {

class testMinimumVelocityTask: public ::testing::Test
{
protected:

    testMinimumVelocityTask()
    {

    }

    virtual ~testMinimumVelocityTask() {

    }

    virtual void SetUp() {

    }

    virtual void TearDown() {

    }

};

TEST_F(testMinimumVelocityTask, testMinimumVelocityTask_)
{
    yarp::sig::Vector q(6, 0.0);
    for(unsigned int i = 0; i < q.size(); ++i)
        q[i] = tests_utils::getRandomAngle();

    yarp::sig::Vector dq_zeros(q.size(), 0.0);

    OpenSoT::tasks::velocity::MinimumVelocity minimumVelocity(q.size());

    EXPECT_TRUE(conversion_utils_YARP::toYARP(minimumVelocity.getA()) == yarp::sig::Matrix(q.size(), q.size()).eye());
    EXPECT_TRUE(conversion_utils_YARP::toYARP(minimumVelocity.getWeight()) == yarp::sig::Matrix(q.size(), q.size()).eye());

    EXPECT_TRUE(minimumVelocity.getConstraints().size() == 0);

    double K = 0.1;
    minimumVelocity.setLambda(K);
    EXPECT_DOUBLE_EQ(minimumVelocity.getLambda(), K);

    minimumVelocity.update(conversion_utils_YARP::toEigen(q));
    EXPECT_TRUE(conversion_utils_YARP::toYARP(minimumVelocity.getb()) == dq_zeros);

    for(unsigned int i = 0; i < 100; ++i)
    {
        minimumVelocity.update(conversion_utils_YARP::toEigen(q));
        q += minimumVelocity.getLambda()*conversion_utils_YARP::toYARP(minimumVelocity.getb());
    }

    for(unsigned int i = 0; i < q.size(); ++i)
        EXPECT_NEAR(q[i], q[i], 1E-3);
}

}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
