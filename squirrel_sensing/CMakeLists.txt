cmake_minimum_required(VERSION 2.8.3)
project(squirrel_sensing)

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS
	roscpp
    actionlib
    dynamic_reconfigure 
	std_msgs
)

# protected region additional user defined REQUIREMENTS on begin #
# protected region additional user defined REQUIREMENTS end #

generate_dynamic_reconfigure_options(cfg/sensor_node.cfg )

###################################
## catkin specific configuration ##
###################################
## The catkin_package macro generates cmake config files for your package
## Declare things to be passed to dependent projects
## LIBRARIES: libraries you create in this project that dependent projects also need
## CATKIN_DEPENDS: catkin_packages dependent projects also need
## DEPENDS: system dependencies of this project that dependent projects also need
catkin_package(
  INCLUDE_DIRS
  CATKIN_DEPENDS 
  	roscpp
    dynamic_reconfigure
	std_msgs
   
)

###########
## Build ##
###########

## Specify additional locations of header files
## Your package locations should be listed before other locations
include_directories(common/src)
include_directories(${catkin_INCLUDE_DIRS})


## Declare a cpp executable
add_executable(sensor_node ros/src/sensor_node_ros.cpp)

## Add cmake target dependencies of the executable/library
## as an example, message headers may need to be generated before nodes
add_dependencies(sensor_node sensor_node_gencfg)
add_dependencies(sensor_node ${squirrel_sensing_EXPORTED_TARGETS})

## Specify libraries to link a library or executable target against
target_link_libraries(sensor_node
  ${catkin_LIBRARIES}
)

# protected region additional user defined BUILD STATEMENTS on begin #
# protected region additional user defined BUILD STATEMENTS end #


#############
## Install ##
#############

## Mark executables and/or libraries for installation
install(TARGETS sensor_node
  ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

# protected region user Cmake macros on begin #
# protected region user Cmake macros end #
