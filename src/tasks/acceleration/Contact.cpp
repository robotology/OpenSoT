#include <OpenSoT/tasks/acceleration/Contact.h>
#include <xbot2_interface/logger.h>

void OpenSoT::tasks::acceleration::Contact::_update()
{
    _robot.getJacobian(_contact_link, _J);
    
    Eigen::Matrix3d w_R_cl = _robot.getPose(_contact_link).linear();
    
    _robot.getJdotTimesV(_contact_link, _jdotqdot);
    
    auto w_adj_cl = XBot::Utils::adjointFromRotation(w_R_cl);
    
    _contact_task = _K*w_adj_cl*(_J*_qddot + _jdotqdot);
    
    _A = _contact_task.getM();
    _b = -_contact_task.getq();
}


void OpenSoT::tasks::acceleration::Contact::_log(XBot::MatLogger2::Ptr logger)
{
}

OpenSoT::tasks::acceleration::Contact::Contact(const std::string& task_id,
                                               const XBot::ModelInterface& robot,
                                               const std::string& contact_link,
                                               const Eigen::MatrixXd& contact_matrix):
    Task< Eigen::MatrixXd, Eigen::VectorXd >(task_id, robot.getNv()),
    _robot(robot),
    _contact_link(contact_link),
    _K(contact_matrix)
{
    _qddot = AffineHelper::Identity(_x_size);

    if(_K.size() == 0){
        _K.setIdentity(6,6);
    }

    if( _K.rows() <= 6 && _K.cols() == 6 )
    {
    }
    else{
        throw std::invalid_argument("Invalid contact matrix");
    }

    update();

    _hessianType = HST_SEMIDEF;

    setWeight(Eigen::MatrixXd::Identity(_K.rows(), _K.rows()));



}

OpenSoT::tasks::acceleration::Contact::Contact(const std::string& task_id, 
                                               const XBot::ModelInterface& robot, 
                                               const std::string& contact_link, 
                                               const OpenSoT::AffineHelper& qddot, 
                                               const Eigen::MatrixXd& contact_matrix): 
    Task< Eigen::MatrixXd, Eigen::VectorXd >(task_id, qddot.getInputSize()),
    _robot(robot),
    _contact_link(contact_link),
    _qddot(qddot),
    _K(contact_matrix)
{
    if(_K.size() == 0){
        _K.setIdentity(6,6);
    }
    
    if( _K.rows() <= 6 && _K.cols() == 6 )
    {
    }
    else{
        throw std::invalid_argument("Invalid contact matrix");
    }
    
    update();

    _hessianType = HST_SEMIDEF;
    
    setWeight(Eigen::MatrixXd::Identity(_K.rows(), _K.rows()));
    
}
