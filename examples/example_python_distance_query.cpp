/**
 @author Karsten Knese
 @author Alessio Rocchi
 */

#include <OpenSoT/constraints/velocity/SelfCollisionAvoidance.h>
#include <geometry_msgs/Transform.h>
#include <idynutils/collision_utils.h>
#include <ros/ros.h>
#include <sensor_msgs/JointState.h>
#include <tf/transform_broadcaster.h>
#include <visualization_msgs/Marker.h>
#include <visualization_msgs/MarkerArray.h>
#include <fstream>

static const std::string JOINT_STATE_TOPIC = "/joint_states";
static const std::string RESULT_MARKER_TOPIC = "distance_query/result_marker";

int id_counter = 0;
int id_lines = 0;
std::string base_frame = "base_link";

const int kelly_colors_hex_size = 20;
const int kelly_colors_hex[] = {
    0xFFB300, // Vivid Yellow
    0x803E75, // Strong Purple
    0xFF6800, // Vivid Orange
    0xA6BDD7, // Very Light Blue
    0xC10020, // Vivid Red
    0xCEA262, // Grayish Yellow
    0x817066, // Medium Gray

    // The following don't work well for people with defective color vision
    0x007D34, // Vivid Green
    0xF6768E, // Strong Purplish Pink
    0x00538A, // Strong Blue
    0xFF7A5C, // Strong Yellowish Pink
    0x53377A, // Strong Violet
    0xFF8E00, // Vivid Orange Yellow
    0xB32851, // Strong Purplish Red
    0xF4C800, // Vivid Greenish Yellow
    0x7F180D, // Strong Reddish Brown
    0x93AA00, // Vivid Yellowish Green
    0x593315, // Deep Yellowish Brown
    0xF13A13, // Vivid Reddish Orange
    0x232C16  // Dark Olive Green
};

const unsigned int GREY =   0x817066;
const unsigned int RED =    0xC10020;
const unsigned int PURPLE = 0x803E75;
const unsigned int GREEN =  0x007D34;
const unsigned int YELLOW = 0xFFB300;
const unsigned int BLUE =   0x00538A;

bool draw_point(const double x, const double y, const double z,
                const std::string frame, visualization_msgs::Marker& marker, unsigned int color=0xff0000) {
    //DRAW REFERENCE
    marker.header.frame_id = frame;
    marker.header.stamp = ros::Time().now();
    marker.ns = "goal";
    marker.id = ++id_counter;
    marker.type = visualization_msgs::Marker::SPHERE;
    marker.action = visualization_msgs::Marker::ADD;

    marker.pose.position.x = x;
    marker.pose.position.y = y;
    marker.pose.position.z = z;
    marker.pose.orientation.x = 0.0;
    marker.pose.orientation.y = 0.0;
    marker.pose.orientation.z = 0.0;
    marker.pose.orientation.w = 1.0;
    marker.scale.x = 0.02;
    marker.scale.y = 0.02;
    marker.scale.z = 0.02;

    /* // use the map of kelly_colors
    color = kelly_colors_hex[id_lines];*/
    marker.color.r = (float)(((color & 0xff0000) >> 16)/255.0);
    marker.color.g = (float)(((color & 0x00ff00) >> 8 )/255.0);
    marker.color.b = (float)(((color & 0x0000ff)      )/255.0);

    marker.color.a = 1.0;
    return true;
}

bool draw_line(const double x1, const double y1, const double z1,
               const double x2, const double y2, const double z2,
               visualization_msgs::Marker& marker, unsigned int color=0xff0000) {
    //DRAW REFERENCE
    marker.header.frame_id = "Waist";
    marker.header.stamp = ros::Time().now();
    marker.ns = "goal";
    marker.id = ++id_counter;
    marker.type = visualization_msgs::Marker::LINE_STRIP;
    marker.action = visualization_msgs::Marker::ADD;

    geometry_msgs::Point p1; p1.x = x1; p1.y = y1; p1.z = z1;
    geometry_msgs::Point p2; p2.x = x2; p2.y = y2; p2.z = z2;
    marker.points.push_back(p1);
    marker.points.push_back(p2);

    marker.scale.x = 0.005;
    marker.scale.y = 0.005;
    marker.scale.z = 0.005;

    /* // use the map of kelly_colors
    color = kelly_colors_hex[id_lines];*/
    marker.color.r = (float)(((color & 0xff0000) >> 16)/255.0);
    marker.color.g = (float)(((color & 0x00ff00) >> 8 )/255.0);
    marker.color.b = (float)(((color & 0x0000ff)      )/255.0);

    marker.color.a = 1.0;

    id_lines = (id_lines+1)% kelly_colors_hex_size; // can be used in the future to index the kelly_colors array
    return true;
}

void createMarkerArray(std::list<LinkPairDistance>& results,
                       const boost::shared_ptr<visualization_msgs::MarkerArray>& markers,
                       iDynUtils& model,
                       unsigned int markerColor=0xff0000,
                       unsigned int lineColor=0xff0000) {
    typedef std::list<LinkPairDistance>::iterator iter_pairs;
    for(iter_pairs it = results.begin(); it != results.end(); ++it)
    {
        visualization_msgs::Marker m1;
        std::pair<std::string, std::string> linkNames = it->getLinkNames();
        std::pair<KDL::Frame, KDL::Frame> transforms = it->getLink_T_closestPoint();
        draw_point( transforms.first.p.x(),
                    transforms.first.p.y(),
                    transforms.first.p.z(),
                    linkNames.first, m1, markerColor);


        visualization_msgs::Marker m2;
        draw_point( transforms.second.p.x(),
                    transforms.second.p.y(),
                    transforms.second.p.z(),
                    linkNames.second, m2, markerColor);

        visualization_msgs::Marker l12;
        KDL::Frame p1 = model.iDyn3_model.getPositionKDL(
                            model.iDyn3_model.getLinkIndex("Waist"),
                            model.iDyn3_model.getLinkIndex(linkNames.first))*transforms.first;
        KDL::Frame p2 = model.iDyn3_model.getPositionKDL(
                            model.iDyn3_model.getLinkIndex("Waist"),
                            model.iDyn3_model.getLinkIndex(linkNames.second))*transforms.second;
        draw_line(  p1.p.x(), p1.p.y(), p1.p.z(),
                    p2.p.x(), p2.p.y(), p2.p.z(),
                    l12, lineColor);

        markers->markers.push_back(m1);
        markers->markers.push_back(m2);
        markers->markers.push_back(l12);
    }
}

int main(int argc, char** argv) {
    iDynUtils bigman("bigman",
                     std::string(OPENSOT_TESTS_ROBOTS_DIR)+"bigman/bigman.urdf",
                     std::string(OPENSOT_TESTS_ROBOTS_DIR)+"bigman/bigman.srdf");

    ros::init(argc, argv, "distance_computation");
    ros::NodeHandle nh;
    double rate = 100;
    ros::Rate loopRate(rate);

    const sensor_msgs::JointStateConstPtr initJoints =
            ros::topic::waitForMessage<sensor_msgs::JointState>(
                    JOINT_STATE_TOPIC, nh);

    OpenSoT::constraints::velocity::SelfCollisionAvoidance::Ptr sca(
        new OpenSoT::constraints::velocity::SelfCollisionAvoidance(
            bigman.iDyn3_model.getAng(),
            bigman, 0.005, 0.3));

    boost::shared_ptr<ComputeLinksDistance> distance_comp_L_R_Arms(
            new ComputeLinksDistance(bigman));
    distance_comp_L_R_Arms->setCollisionWhiteList(sca->whitelist_L_R_Arms);

    boost::shared_ptr<ComputeLinksDistance> distance_comp_L_Arm_Torso(
            new ComputeLinksDistance(bigman));
    distance_comp_L_Arm_Torso->setCollisionWhiteList(sca->whitelist_L_Arm_Torso);

    boost::shared_ptr<ComputeLinksDistance> distance_comp_R_Arm_Torso(
            new ComputeLinksDistance(bigman));
    distance_comp_R_Arm_Torso->setCollisionWhiteList(sca->whitelist_R_Arm_Torso);

    boost::shared_ptr<ComputeLinksDistance> distance_comp_L_Arm_L_Leg(
            new ComputeLinksDistance(bigman));
    distance_comp_L_Arm_L_Leg->setCollisionWhiteList(sca->whitelist_L_Arm_L_Leg);

    boost::shared_ptr<ComputeLinksDistance> distance_comp_R_Arm_R_Leg(
            new ComputeLinksDistance(bigman));
    distance_comp_R_Arm_R_Leg->setCollisionWhiteList(sca->whitelist_R_Arm_R_Leg);

    boost::shared_ptr<ComputeLinksDistance> distance_comp_L_Arm_R_Leg(
            new ComputeLinksDistance(bigman));
    distance_comp_L_Arm_R_Leg->setCollisionWhiteList(sca->whitelist_L_Arm_R_Leg);

    boost::shared_ptr<ComputeLinksDistance> distance_comp_R_Arm_L_Leg(
            new ComputeLinksDistance(bigman));
    distance_comp_R_Arm_L_Leg->setCollisionWhiteList(sca->whitelist_R_Arm_L_Leg);

    boost::shared_ptr<ComputeLinksDistance> distance_comp(
            new ComputeLinksDistance(bigman));

    std::list<std::pair<std::string,std::string>> whiteList;
    whiteList.insert(whiteList.end(),
                     sca->whitelist_L_R_Arms.begin(),
                     sca->whitelist_L_R_Arms.end());
    whiteList.insert(whiteList.end(),
                     sca->whitelist_L_Arm_Torso.begin(),
                     sca->whitelist_L_Arm_Torso.end());
    whiteList.insert(whiteList.end(),
                     sca->whitelist_R_Arm_Torso.begin(),
                     sca->whitelist_R_Arm_Torso.end());
    whiteList.insert(whiteList.end(),
                     sca->whitelist_L_Arm_L_Leg.begin(),
                     sca->whitelist_L_Arm_L_Leg.end());
    whiteList.insert(whiteList.end(),
                     sca->whitelist_R_Arm_R_Leg.begin(),
                     sca->whitelist_R_Arm_R_Leg.end());
    whiteList.insert(whiteList.end(),
                     sca->whitelist_L_Arm_R_Leg.begin(),
                     sca->whitelist_L_Arm_R_Leg.end());
    whiteList.insert(whiteList.end(),
                     sca->whitelist_R_Arm_L_Leg.begin(),
                     sca->whitelist_R_Arm_L_Leg.end());

    /*
    // lower body - arms collision whitelist for WalkMan (for upper-body manipulation tasks - i.e. not crouching)
    whiteList.push_back(std::pair<std::string,std::string>("LLowLeg","LSoftHandLink"));
    whiteList.push_back(std::pair<std::string,std::string>("LHipMot","LSoftHandLink"));
    whiteList.push_back(std::pair<std::string,std::string>("RLowLeg","RSoftHandLink"));
    whiteList.push_back(std::pair<std::string,std::string>("RHipMot","RSoftHandLink"));

    // torso - arms collision whitelist for WalkMan
    whiteList.push_back(std::pair<std::string,std::string>("DWS","LSoftHandLink"));
    whiteList.push_back(std::pair<std::string,std::string>("DWS","LWrMot2"));
    whiteList.push_back(std::pair<std::string,std::string>("DWS","RSoftHandLink"));
    whiteList.push_back(std::pair<std::string,std::string>("DWS","RWrMot2"));
    whiteList.push_back(std::pair<std::string,std::string>("TorsoProtections","LElb"));
    whiteList.push_back(std::pair<std::string,std::string>("TorsoProtections","LSoftHandLink"));
    whiteList.push_back(std::pair<std::string,std::string>("TorsoProtections","RElb"));
    whiteList.push_back(std::pair<std::string,std::string>("TorsoProtections","RSoftHandLink"));
    whiteList.push_back(std::pair<std::string,std::string>("Waist","LSoftHandLink"));
    whiteList.push_back(std::pair<std::string,std::string>("Waist","LWrMot2"));
    whiteList.push_back(std::pair<std::string,std::string>("Waist","RSoftHandLink"));
    whiteList.push_back(std::pair<std::string,std::string>("Waist","RWrMot2"));

    // arm - am collision whitelist for WalkMan
    whiteList.push_back(std::pair<std::string,std::string>("LShr","RShr"));
    whiteList.push_back(std::pair<std::string,std::string>("LShr","RSoftHandLink"));
    whiteList.push_back(std::pair<std::string,std::string>("LShr","RWrMot2"));
    whiteList.push_back(std::pair<std::string,std::string>("LSoftHandLink","RShr"));
    whiteList.push_back(std::pair<std::string,std::string>("LSoftHandLink","RSoftHandLink"));
    whiteList.push_back(std::pair<std::string,std::string>("LSoftHandLink","RWrMot2"));
    whiteList.push_back(std::pair<std::string,std::string>("LWrMot2","RShr"));
    whiteList.push_back(std::pair<std::string,std::string>("LWrMot2","RSoftHandLink"));
    whiteList.push_back(std::pair<std::string,std::string>("LWrMot2","RWrMot2"));
    */

    distance_comp->setCollisionWhiteList(whiteList);


    ros::Subscriber joint_states_subscriber = nh.subscribe<
            sensor_msgs::JointState>(JOINT_STATE_TOPIC, 1, // Buffer size
            &iDynUtils::updateiDyn3ModelFromJoinStateMsg, &bigman);

    ros::Publisher resultMarkerPub = nh.advertise<
            visualization_msgs::MarkerArray>(RESULT_MARKER_TOPIC, 10);
    boost::shared_ptr<visualization_msgs::MarkerArray> markers(
            new visualization_msgs::MarkerArray);

    std::ofstream _logger_accuracy("SCAFoI_log_q.csv");
    std::ofstream _logger("SCAFoI_log_t.csv");
    _logger << "# log, for each t, the following data:"
            << "# uid, t, t_SCAFoI, t_whole, "
            << "... L_R_Arms_Distance, L_R_Arms_SCAFoI_Activation, ..."
            << "... L_Arm_Torso_Distance, L_Arm_Torso_Activation, ..."
            << "... R_Arm_Torso_Distance, R_Arm_Torso_Activation, ..."
            << "... L_Arm_L_Leg_Distance, L_Arm_L_Leg_Activation, ..."
            << "... R_Arm_R_Leg_Distance, R_Arm_R_Leg_Activation, ..."
            << "... L_Arm_R_Leg_Distance, L_Arm_R_Leg_Activation, ..."
            << "... R_Arm_L_Leg_Distance, R_Arm_L_Leg_Activation" << std::endl;

    _logger_accuracy << "# log, for each q, the following data:"
            << "# uid, L_R_Arms_correct_prediction"
            << "... L_Arm_Torso_correct_prediction, L_Arm_Torso_critical, ..."
            << "... R_Arm_Torso_correct_prediction, R_Arm_Torso_critical, ..."
            << "... L_Arm_L_Leg_correct_prediction, L_Arm_L_Leg_critical, ..."
            << "... R_Arm_R_Leg_correct_prediction, R_Arm_R_Leg_critical, ..."
            << "... L_Arm_R_Leg_correct_prediction, L_Arm_R_Leg_Critical, ..."
            << "... R_Arm_L_Leg_correct_prediction, R_Arm_L_Leg_Critical, q" << std::endl;


    double time_start = yarp::os::SystemClock::nowSystem();
    unsigned int uid = 0;
    double t = time_start;
    yarp::sig::Vector q_prev, q_curr;
    q_prev = bigman.iDyn3_model.getAng(); q_curr = q_prev;
    while (ros::ok()) {
        double distance_comp_tic = yarp::os::SystemClock::nowSystem();
        std::list<LinkPairDistance> results = distance_comp->getLinkDistances();
        double distance_comp = yarp::os::SystemClock::nowSystem() - distance_comp_tic;

        std::list<LinkPairDistance> results_L_R_Arms =
            distance_comp_L_R_Arms->getLinkDistances();
        std::list<LinkPairDistance> results_L_Arm_Torso =
            distance_comp_L_Arm_Torso->getLinkDistances();
        std::list<LinkPairDistance> results_R_Arm_Torso =
            distance_comp_R_Arm_Torso->getLinkDistances();
        std::list<LinkPairDistance> results_L_Arm_L_Leg =
            distance_comp_L_Arm_L_Leg->getLinkDistances();
        std::list<LinkPairDistance> results_R_Arm_R_Leg =
            distance_comp_R_Arm_R_Leg->getLinkDistances();
        std::list<LinkPairDistance> results_L_Arm_R_Leg =
            distance_comp_L_Arm_R_Leg->getLinkDistances();
        std::list<LinkPairDistance> results_R_Arm_L_Leg =
            distance_comp_R_Arm_L_Leg->getLinkDistances();

        q_curr = bigman.iDyn3_model.getAng();

        double SCAFoI_comp_tic = yarp::os::SystemClock::nowSystem();
        sca->predict_SCAFoIs(q_curr,
                             sca->Linkpair_updated_list_all,
                             sca->Linkpair_constrained_list_all);
        double SCAFoI_comp = yarp::os::SystemClock::nowSystem() - SCAFoI_comp_tic;

        uid++;
        t = yarp::os::SystemClock::nowSystem() - time_start;

        _logger << uid << ", " << t << ", "
                << SCAFoI_comp << ", " << distance_comp << ", "
                << results_L_R_Arms.front().getDistance()    << ", " << (int)sca->is_active_SCAFoI_L_R_Arms << ", "
                << results_L_Arm_Torso.front().getDistance() << ", " << (int)sca->is_active_SCAFoI_L_Arm_Torso << ", "
                << results_R_Arm_Torso.front().getDistance() << ", " << (int)sca->is_active_SCAFoI_R_Arm_Torso << ", "
                << results_L_Arm_L_Leg.front().getDistance() << ", " << (int)sca->is_active_SCAFoI_L_Arm_L_Leg << ", "
                << results_R_Arm_R_Leg.front().getDistance() << ", " << (int)sca->is_active_SCAFoI_R_Arm_R_Leg << ", "
                << results_L_Arm_R_Leg.front().getDistance() << ", " << (int)sca->is_active_SCAFoI_L_Arm_R_Leg << ", "
                << results_R_Arm_L_Leg.front().getDistance() << ", " << (int)sca->is_active_SCAFoI_R_Arm_L_Leg << std::endl;

        using namespace yarp::math;
        if(norm(q_curr - q_prev) > 1e-9)
        {
            q_prev = q_curr;
            std::string q_curr_str = q_curr.toString();
            std::replace( q_curr_str.begin(), q_curr_str.end(), '\t', ',');

            _logger_accuracy << uid << ", "
                    << (int)(sca->is_active_SCAFoI_L_R_Arms     ? results_L_R_Arms.front().getDistance() < sca->d_threshold_upper : results_L_R_Arms.front().getDistance() > sca->d_threshold_lower) << ", "
                    << (int)(!sca->is_active_SCAFoI_L_R_Arms    ? results_L_R_Arms.front().getDistance() > sca->_linkPair_threshold/sca->_boundScaling : false) << ", "
                    << (int)(sca->is_active_SCAFoI_L_Arm_Torso  ? results_L_Arm_Torso.front().getDistance() < sca->d_threshold_upper : results_L_Arm_Torso.front().getDistance() > sca->d_threshold_lower) << ", "
                    << (int)(!sca->is_active_SCAFoI_L_Arm_Torso ? results_L_Arm_Torso.front().getDistance() > sca->_linkPair_threshold/sca->_boundScaling : false) << ", "
                    << (int)(sca->is_active_SCAFoI_R_Arm_Torso  ? results_R_Arm_Torso.front().getDistance() < sca->d_threshold_upper : results_R_Arm_Torso.front().getDistance() > sca->d_threshold_lower) << ", "
                    << (int)(!sca->is_active_SCAFoI_R_Arm_Torso ? results_R_Arm_Torso.front().getDistance() > sca->_linkPair_threshold/sca->_boundScaling : false) << ", "
                    << (int)(sca->is_active_SCAFoI_L_Arm_L_Leg   ? results_L_Arm_L_Leg.front().getDistance() < sca->d_threshold_upper : results_L_Arm_L_Leg.front().getDistance() > sca->d_threshold_lower) << ", "
                    << (int)(!sca->is_active_SCAFoI_L_Arm_L_Leg  ? results_L_Arm_L_Leg.front().getDistance() > sca->_linkPair_threshold/sca->_boundScaling : false) << ", "
                    << (int)(sca->is_active_SCAFoI_R_Arm_R_Leg   ? results_R_Arm_R_Leg.front().getDistance() < sca->d_threshold_upper : results_R_Arm_R_Leg.front().getDistance() > sca->d_threshold_lower) << ", "
                    << (int)(!sca->is_active_SCAFoI_R_Arm_R_Leg  ? results_R_Arm_R_Leg.front().getDistance() > sca->_linkPair_threshold/sca->_boundScaling : false) << ", "
                    << (int)(sca->is_active_SCAFoI_L_Arm_R_Leg   ? results_L_Arm_R_Leg.front().getDistance() < sca->d_threshold_upper : results_L_Arm_R_Leg.front().getDistance() > sca->d_threshold_lower) << ", "
                    << (int)(!sca->is_active_SCAFoI_L_Arm_R_Leg  ? results_L_Arm_R_Leg.front().getDistance() > sca->_linkPair_threshold/sca->_boundScaling : false) << ", "
                    << (int)(sca->is_active_SCAFoI_R_Arm_L_Leg   ? results_R_Arm_L_Leg.front().getDistance() < sca->d_threshold_upper : results_R_Arm_L_Leg.front().getDistance() > sca->d_threshold_lower) << ", "
                    << (int)(!sca->is_active_SCAFoI_R_Arm_L_Leg  ? results_R_Arm_L_Leg.front().getDistance() > sca->_linkPair_threshold/sca->_boundScaling : false) << ", "
                    << q_curr_str << std::endl;
        }

        std::list<LinkPairDistance>
            results_in_scafois, // all link pairs which are in an activated SCAFoI
            results_constrained,// all link pairs which are in an activated SCAFoI and are active constraints
            results_green_zone, // all link pairs not in an active SCAFoI, with distance > d_threshold_upper
            results_yellow_zone,// all link pairs not in an active SCAFoI, with distance > d_threshold_lower && distance < d_threshold_upper
            results_red_zone;   // all link pairs not in an active SCAFoI, with distance < d_threshold_lower

        //while(results.size() > 15) results.pop_back();

        if (results.size() > 0) {
            markers->markers.clear();
            id_counter = 0;
            id_lines = 0;

            for(std::list<LinkPairDistance>::iterator it =
                    results.begin(); it != results.end(); ++it)
            {
                if(std::find(sca->Linkpair_updated_list_all.begin(),
                                  sca->Linkpair_updated_list_all.end(),
                                  it->getLinkNames()) != sca->Linkpair_updated_list_all.end()) // in a SCAFoI
                {
                    if(it->getDistance() < sca->d_threshold_lower)
                        results_constrained.push_back(*it);
                    else
                        results_in_scafois.push_back(*it);
                }
                else    // NOT in any SCAFoI :(
                {
                    if(it->getDistance() > sca->d_threshold_upper)
                        results_green_zone.push_back(*it);
                    else if(it->getDistance() > sca->d_threshold_lower)
                        results_yellow_zone.push_back(*it);
                    else
                        results_red_zone.push_back(*it);
                }
            }

            //createMarkerArray(results, markers, bigman);


            if(results_in_scafois.size() > 0)
                createMarkerArray(results_in_scafois, markers, bigman,  RED,     YELLOW);

            if(results_constrained.size() > 0)
                createMarkerArray(results_constrained, markers, bigman,  RED,    RED);

            if(results_green_zone.size() > 0)
                createMarkerArray(results_green_zone, markers, bigman,   GREY,   GREEN);

            if(results_yellow_zone.size() > 0)
                createMarkerArray(results_yellow_zone, markers, bigman,  GREY,   BLUE);

            if(results_red_zone.size() > 0)
                createMarkerArray(results_red_zone, markers, bigman,     PURPLE, RED);

            resultMarkerPub.publish(markers);
        }

        ros::spinOnce();
        loopRate.sleep();
    }

    return EXIT_SUCCESS;
}