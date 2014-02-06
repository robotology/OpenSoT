#ifndef _SOT_VELKINCON_CTRL_H_
#define _SOT_VELKINCON_CTRL_H_

#include <yarp/os/RateThread.h>
#include <urdf/model.h>
#include <iCub/iDynTree/DynTree.h>
#include <kdl_parser/kdl_parser.hpp>
#include "yarp_interface.h"

class sot_VelKinCon_ctrl : public yarp::os::RateThread
 {
 public:
     sot_VelKinCon_ctrl(const double period, int argc, char* argv[]);

     virtual bool threadInit();
     virtual void run();
 private:
     KDL::Tree coman_tree; // A KDL Tree
     urdf::Model coman_model; // A URDF Model
     iCub::iDynTree::DynTree coman_iDyn3; // iDyn3 Model
     std::vector<std::string> left_arm_joint_names;
     std::vector<std::string> right_arm_joint_names;
     std::vector<std::string> left_leg_joint_names;
     std::vector<std::string> right_leg_joint_names;
     std::vector<std::string> torso_joint_names;

     int waist_LinkIndex;
     int right_arm_LinkIndex;
     int left_arm_LinkIndex;
     std::string right_arm_name;
     std::string left_arm_name;
     std::string waist_link_name;
     // Joint ids for right arm and left arm
     std::vector<unsigned int> right_arm_joint_numbers;
     std::vector<unsigned int> left_arm_joint_numbers;
     std::vector<unsigned int> waist_joint_numbers;

     yarp::sig::Vector q_ref; // Vector of desired joint configurations [1x23]
     yarp::sig::Vector dq_ref; // Vector of desired joint velocities [1x23]
     yarp::sig::Vector ddq_ref; // Vector of desired joint accelerations [1x23]
     yarp::sig::Vector q; // Vector of measured joint angles
     yarp::sig::Vector q_left_arm; // Vector of measured joint angles
     yarp::sig::Vector q_left_leg; // Vector of measured joint angles
     yarp::sig::Vector q_right_arm; // Vector of measured joint angles
     yarp::sig::Vector q_right_leg; // Vector of measured joint angles
     yarp::sig::Vector q_torso; // Vector of measured joint angles
     /** Some Theory: **/
     /**
        We are considering the optimization problem:
            (Ax-b)'Q(Ax-b) = ... = x'Hx + x'g
        where:
            H = A'QA
            g = -2A'Qb

        For the inverse kinematic problem:
            x = dq
            Q = general weights
            A = J
            b = dx (desired Cartesian velocity to the goal)
     **/
     yarp::sig::Matrix Q_postural; //Matrix of weights for the postural task

     yarp_interface IYarp;

     void iDyn3Model();
     void updateiDyn3Model(const bool set_world_pose);
     void setControlledKinematicChainsLinkIndex();
     void setControlledKinematicChainsJointNumbers();
     void setQPostural();
     void getFeedBack();
     void setJointNames()
     {
         right_arm_joint_names.push_back("RShSag");
         right_arm_joint_names.push_back("RShLat");
         right_arm_joint_names.push_back("RShYaw");
         right_arm_joint_names.push_back("RElbj");
         right_arm_joint_names.push_back("RForearmPlate");
         right_arm_joint_names.push_back("RWrj1");
         right_arm_joint_names.push_back("RWrj2");

         left_arm_joint_names.push_back("LShSag");
         left_arm_joint_names.push_back("LShLat");
         left_arm_joint_names.push_back("LShYaw");
         left_arm_joint_names.push_back("LElbj");
         left_arm_joint_names.push_back("LForearmPlate");
         left_arm_joint_names.push_back("LWrj1");
         left_arm_joint_names.push_back("LWrj2");

         right_leg_joint_names.push_back("RHipSag");
         right_leg_joint_names.push_back("RHipLat");
         right_leg_joint_names.push_back("RHipYaw");
         right_leg_joint_names.push_back("RKneeSag");
         right_leg_joint_names.push_back("RAnkLat");
         right_leg_joint_names.push_back("RAnkSag");

         left_leg_joint_names.push_back("LHipSag");
         left_leg_joint_names.push_back("LHipLat");
         left_leg_joint_names.push_back("LHipYaw");
         left_leg_joint_names.push_back("LKneeSag");
         left_leg_joint_names.push_back("LAnkLat");
         left_leg_joint_names.push_back("LAnkSag");

         torso_joint_names.push_back("WaistSag");
         torso_joint_names.push_back("WaistLat");
         torso_joint_names.push_back("WaistYaw");
     }
 };

#endif
