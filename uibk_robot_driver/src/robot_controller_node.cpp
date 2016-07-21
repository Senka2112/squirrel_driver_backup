#include <uibk_robot_driver/robot_controller.hpp>
#include <ros/ros.h>
#include <ros/publisher.h>
#include <ros/subscriber.h>
#include <memory>
#include <thread>
#include <std_msgs/Float32MultiArray.h>
#include <std_msgs/Float64MultiArray.h>
#include <std_msgs/Int32.h>
#include <std_msgs/Float64.h>
#include <sensor_msgs/JointState.h>
#include <mutex>

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>



using namespace std;


bool runController = true;
std::shared_ptr<RobotController> robotinoController;
//std::shared_ptr<Arm> robotinoArm;
//std::shared_ptr<std::thread> armThread;

bool newMoveCommandStateSet = false;
bool newPtpCommandStateSet = false;
std::vector<double> moveCommandState;
std::vector<double> ptpCommandState;
std::mutex moveCommandMutex;
std::mutex ptpCommandMutex;

void stopHandler(int s);
void moveCommandStateHandler(std_msgs::Float64MultiArray arr);
void ptpCommandStateHandler(std_msgs::Float64MultiArray arr);
void switchModeHandler(std_msgs::Int32 mode);

vector<int> transformVector(vector<double> v);
vector<double> transformVector(vector<int> v);
vector<double> computeDerivative(vector<int> v1, vector<int> v2, double timeStep);
vector<double> computeDerivative(vector<double> v1, vector<double> v2, double timeStep);

int currentMode = 0;

int main(int argc, char** args) {

    ros::init(argc, args, "robot_controller_node");
    ros::NodeHandle node; sleep(1);

    shared_ptr<RobotController> robotinoController= shared_ptr<RobotController>(new RobotController(node));
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = stopHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);



    robotinoController->initBase();
    robotinoController->initArm({1, 2, 3, 4, 5},{std::make_pair<double, double>(-125000.0 / Motor::TICKS_FOR_180_DEG * M_PI, 130000 / Motor::TICKS_FOR_180_DEG * M_PI),
                                                 std::make_pair<double, double>(-140000 / Motor::TICKS_FOR_180_DEG * M_PI, 185000 / Motor::TICKS_FOR_180_DEG * M_PI),
                                                 std::make_pair<double, double>(-150000 / Motor::TICKS_FOR_180_DEG * M_PI, 150000 / Motor::TICKS_FOR_180_DEG * M_PI),
                                                 std::make_pair<double, double>(-100000 / Motor::TICKS_FOR_180_DEG * M_PI, 100000 / Motor::TICKS_FOR_180_DEG * M_PI),
                                                 std::make_pair<double, double>(-140000 / Motor::TICKS_FOR_180_DEG * M_PI, 140000 / Motor::TICKS_FOR_180_DEG * M_PI)});
    //commandState = robotinoArm->getCurrentState();
    moveCommandState = robotinoController->getCurrentStates();
    //cout << "hereeer5" << endl;
    auto cycleTime = robotinoController->getArmCycleTime();
    auto maxStepPerCycle = robotinoController->getArmMaxStepPerCycle();

    std_msgs::Float64 cycleMsg; cycleMsg.data = cycleTime;
    std_msgs::Float64 maxStepPerCycleMsg; maxStepPerCycleMsg.data = maxStepPerCycle;

    auto statePublisher = node.advertise<sensor_msgs::JointState>("/real/robotino/joint_control/get_state", 1);
    auto modePublisher = node.advertise<std_msgs::Float32MultiArray>("/real/robotino/settings/get_command_state", 1);
    auto cycleTimePublisher = node.advertise<std_msgs::Float64>("/real/robotino_arm/settings/get_clock_cycle", 1);
    auto maxStepPerCyclePublisher = node.advertise<std_msgs::Float64>("/real/robotino_arm/joint_control/get_max_dist_per_cycle", 1);

    auto moveCommandSub= node.subscribe("/real/robotino/joint_control/move", 2, moveCommandStateHandler);
    auto ptpCommandSub= node.subscribe("/real/robotino/joint_control/ptp", 2, ptpCommandStateHandler);
    auto modeSub= node.subscribe("/real/robotino/settings/switch_mode", 1, switchModeHandler);

    std_msgs::Float32MultiArray modeArray;
    modeArray.data.push_back(0.0); modeArray.data.push_back(0.0);

    sensor_msgs::JointState jointStateMsg;
    auto prevPos = robotinoController->getCurrentStates();
    vector<double> prevVel; for(int i = 0; i < robotinoController->getDegOfFreedom(); ++i) prevVel.push_back(0.0);
    ros::Rate r(robotinoController->getArmFrequency());
    double stepTime = 1.0 / robotinoController->getArmFrequency();

    while(runController) {

        jointStateMsg.position = robotinoController->getCurrentStates();
        jointStateMsg.velocity = computeDerivative(jointStateMsg.position, prevPos, stepTime);
        jointStateMsg.effort = computeDerivative(jointStateMsg.velocity, prevVel, stepTime);

        statePublisher.publish(jointStateMsg);
        modeArray.data.at(1) = currentMode;
        modePublisher.publish(modeArray);

        cycleTimePublisher.publish(cycleMsg);
        maxStepPerCyclePublisher.publish(maxStepPerCycleMsg);
        if(currentMode == 10) {
         //cout << "commanding  " << moveCommandState.at(0) << moveCommandState.at(1) << moveCommandState.at(2) << endl;
            moveCommandMutex.lock();
            if(newMoveCommandStateSet) {
              //cout << "here move  " << endl;
                robotinoController->moveAll(moveCommandState);

                newMoveCommandStateSet = false;
            }
            moveCommandMutex.unlock();

            ptpCommandMutex.lock();
            if(newPtpCommandStateSet) {
                //cout << "here ptp " << endl;
                robotinoController->ptpAll(ptpCommandState);

                newPtpCommandStateSet = false;
            }
            ptpCommandMutex.unlock();


        }

        prevPos = jointStateMsg.position;
        prevVel = jointStateMsg.velocity;

        r.sleep();
        ros::spinOnce();

    }

    return 0;

}

vector<double> transformVector(vector<int> v) {
    vector<double> retVal;
    for(auto val : v)
        retVal.push_back((double) val);
    return retVal;
}

vector<int> transformVector(vector<double> v) {
    vector<int> retVal;
    for(auto val : v)
        retVal.push_back((int) val);
    return retVal;
}

vector<double> computeDerivative(vector<int> v1, vector<int> v2, double timeStep) {
    vector<double> der;
    for(int i = 0; i < v1.size(); ++i)
        der.push_back((v2.at(i) - v1.at(i)) / timeStep);
    return der;
}

vector<double> computeDerivative(vector<double> v1, vector<double> v2, double timeStep) {
    vector<double> der;
    for(int i = 0; i < v1.size(); ++i)
        der.push_back((v2.at(i) - v1.at(i)) / timeStep);
    return der;
}

void stopHandler(int s) {

    runController = false;
    robotinoController->shutdown();
    exit(0);

}

void moveCommandStateHandler(std_msgs::Float64MultiArray arr) {

    moveCommandMutex.lock();
    if(arr.data.size() == 8) {//8 degrees of freedom
        moveCommandState = arr.data;
        newMoveCommandStateSet = true;
    } else {
        cerr << "your joint data has wrong dimension (of " << arr.data.size() << ")" << endl;
    }
    moveCommandMutex.unlock();

}

void ptpCommandStateHandler(std_msgs::Float64MultiArray arr) {

   ptpCommandMutex.lock();
    if(arr.data.size() == 8) {//8 degrees of freedom
        ptpCommandState = arr.data;
        newPtpCommandStateSet = true;
    } else {
        cerr << "your joint data has wrong dimension (of " << arr.data.size() << ")" << endl;
    }
    ptpCommandMutex.unlock();

}


void switchModeHandler(std_msgs::Int32 mode) {
    currentMode = mode.data;

}