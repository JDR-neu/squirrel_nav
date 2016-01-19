// LocalPlanner.h --- 
// 
// Filename: LocalPlanner.h
// Description: Local planner based on robotino_local_planner
// Author: indorewala@servicerobotics.eu
// Maintainer: Federico Boniardi (boniardi@cs.uni-freiburg.de)
// Created: Fri Nov 14 01:10:39 2014 (+0100)
// Version: 0.1.0
// Last-Updated: Wed Nov 26 15:47:36 2014 (+0100)
//           By: Federico Boniardi
//     Update #: 3
// URL: 
// Keywords: 
// Compatibility: 
//   ROS Hydro, ROS Indigo
// 

// Commentary: 
//   
//   The code therein is adapted from the package robotino_local_planner
//   by indorewala@servicerobotics.eu, available at
//
//     http://svn.openrobotino.org/robotino-ros-pkg/branches/robotino_navigation/robotino_navigation/
//
//   and licensed under BSD software license. Further documentation available at
//
//     http://wiki.ros.org/robotino_local_planner
//
//     
//   Tested on: - ROS Hydro on Ubuntu 12.04
//               - ROS Indigo on Ubuntu 14.04
//    RGBD source: ASUS Xtion pro
//
//   *WARNING*: the following controller implements simple pure pursuit trajectory tracker. No
//              velocity profile is used.
//      *TODO*: compute a velocity profile for proper controller      
// 

// Code:

#ifndef SQUIRREL_NAVIGATION_LOCALPLANNER_H_
#define SQUIRREL_NAVIGATION_LOCALPLANNER_H_

#include <ros/ros.h>

#include <base_local_planner/trajectory_planner_ros.h>
#include <nav_core/base_local_planner.h>

#include <costmap_2d/costmap_2d_ros.h>

#include <angles/angles.h>

#include <tf/tf.h>
#include <tf/transform_listener.h>

#include <geometry_msgs/Twist.h>
#include <geometry_msgs/Point32.h>
#include <nav_msgs/Odometry.h>
#include <nav_msgs/Path.h>
#include <std_msgs/Bool.h>
#include <visualization_msgs/Marker.h>
#include <visualization_msgs/MarkerArray.h>

#include <cmath>
#include <string>
#include <vector>

#include "squirrel_navigation/Common.h"

namespace squirrel_navigation {
  
class LocalPlanner : public nav_core::BaseLocalPlanner {
 public:
  LocalPlanner( void );
  ~LocalPlanner( void );

  void initialize( std::string, tf::TransformListener*, costmap_2d::Costmap2DROS* );

  bool computeVelocityCommands( geometry_msgs::Twist& );
  bool isGoalReached( void );
  bool setPlan( const std::vector<geometry_msgs::PoseStamped>& );

 private:  
  typedef enum { ROTATING_TO_START, MOVING, ROTATING_TO_GOAL, FINISHED } state_t;
  typedef enum { DIJKSTRA, LATTICE } planner_t;

  base_local_planner::TrajectoryPlannerROS* trajectory_planner_;
  
  tf::TransformListener* tf_;

  std::vector<geometry_msgs::PoseStamped> global_plan_;

  geometry_msgs::PoseStamped base_odom_;
  
  nav_msgs::Path global_plan_msg_;
  
  ros::Subscriber odom_sub_, update_sub_;
  
  ros::Publisher next_heading_pub_;
  
  state_t state_;

  boost::mutex odom_lock_;

  int curr_heading_index_, next_heading_index_;

  planner_t planner_type_;

  bool verbose_;
  
  // Parameters
  double heading_lookahead_;
  double max_linear_vel_, min_linear_vel_;
  double max_rotation_vel_, min_rotation_vel_, max_in_place_rotation_vel_;
  double yaw_goal_tolerance_, xy_goal_tolerance_;
  int num_window_points_;

  // Name and references
  std::string name_;

  bool move_( geometry_msgs::Twist& );
  bool rotateToGoal_( geometry_msgs::Twist& );
  bool rotateToStart_( geometry_msgs::Twist& );
  bool newGoal_( const std::vector<geometry_msgs::PoseStamped>& );
  double calLinearVel_( void );
  double calRotationVel_( double );
  double mapToMinusPIToPI_( double );
  void computeNextHeadingIndex_( void );
  void odomCallback_( const nav_msgs::OdometryConstPtr& );
  void plannerUpdateCallback_( const std_msgs::Bool::ConstPtr& );
  void publishNextHeading_( bool show = true );

  inline double linearDistance_( const geometry_msgs::Point p1, const geometry_msgs::Point p2 )
  {
    double dx=p1.x-p2.x, dy=p1.y-p2.y;
    return std::sqrt(dx*dx+dy*dy);
  }

  inline double angularDistance_( const geometry_msgs::Quaternion& p1, const geometry_msgs::Quaternion& p2 )
  {
    double da=tf::getYaw(p1)-tf::getYaw(p2);
    return std::abs(angles::normalize_angle(da));
  }

  inline double cutOff_( double a )
  {
    double th = angles::normalize_angle(a);
    return std::pow(0.5+0.5*std::cos(a),6); // just a good shape bell shape
  };
};

} // namespace squirrel_navigation

#endif /* SQUIRREL_NAVIGATION_LOCALPLANNER_H_ */

// 
// LocalPlanner.h ends here
