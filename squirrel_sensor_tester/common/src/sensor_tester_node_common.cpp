// ROS message includes
#include "ros/ros.h"
#include <std_msgs/Float64MultiArray.h>
#include <std_srvs/Empty.h>

/* protected region user include files on begin */
/* protected region user include files end */

class sensor_tester_node_config
{
public:
};

class sensor_tester_node_data
{
// autogenerated: don't touch this class
public:
    //input data
    std_msgs::Float64MultiArray in_TestPublish;
    //output data
};

class sensor_tester_node_impl
{
    /* protected region user member variables on begin */
	sensor_tester_node_data myInputs;
    /* protected region user member variables end */

public:
    sensor_tester_node_impl() 
    {
        /* protected region user constructor on begin */
        /* protected region user constructor end */
    }

    void configure(sensor_tester_node_config config) 
    {
        /* protected region user configure on begin */
        /* protected region user configure end */
    }

    void update(sensor_tester_node_data &data, sensor_tester_node_config config)
    {
        /* protected region user update on begin */
    	std::cout << "I have read: " << myInputs.in_TestPublish << std::endl;
        /* protected region user update end */
    }

    bool callback_TestService(std_srvs::Empty::Request  &req, std_srvs::Empty::Response &res , sensor_tester_node_config config)
    {
        /* protected region user implementation of service callback for TestService on begin */
        /* protected region user implementation of service callback for TestService end */
        return true;
    }

    /* protected region user additional functions on begin */
    /* protected region user additional functions end */
};
