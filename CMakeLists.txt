cmake_minimum_required(VERSION 3.0.2)
project(gamepad_ctrl)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  std_msgs
  geometry_msgs
  sensor_msgs
)

catkin_package(
  LIBRARIES
  INCLUDE_DIRS
  CATKIN_DEPENDS roscpp std_msgs geometry_msgs sensor_msgs
)

include_directories(
# include
  ${catkin_INCLUDE_DIRS}
)
add_executable(joy_stats_pub src/joy_stats_pub.cpp)
target_link_libraries(joy_stats_pub ${catkin_LIBRARIES})
add_dependencies(joy_stats_pub ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
