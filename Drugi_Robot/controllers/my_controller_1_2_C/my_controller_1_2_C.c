#include <webots/robot.h>
#include <webots/motor.h>
#include <webots/position_sensor.h>
#include <math.h>
#include <stdio.h>

#define TIME_STEP 64

/* "controllerArgs" field of the Robot node
 */
int main(int argc, char **argv) {
  /* necessary to initialize webots stuff */
  wb_robot_init();
  float max_speed=2*M_PI;
  
  WbDeviceTag left_motor = wb_robot_get_device("motor_1");
  WbDeviceTag right_motor = wb_robot_get_device("motor_2");
  
  wb_motor_set_position(left_motor, HUGE_VALF);
  wb_motor_set_position(right_motor, HUGE_VALF);
  
  wb_motor_set_velocity(left_motor, 0.0);
  wb_motor_set_velocity(right_motor, 0.0);
  
  WbDeviceTag left_ps = wb_robot_get_device("ps_1");
  wb_position_sensor_enable(left_ps,TIME_STEP);
  
  WbDeviceTag right_ps = wb_robot_get_device("ps_2");
  wb_position_sensor_enable(right_ps,TIME_STEP);
  
  float wheel_radius = 0.025;
  float linear_velocity = wheel_radius * max_speed;
  
  float duration_side;
  
  float start_time = wb_robot_get_time();
  
  float distance_between_wheels=0.09;
  
  float rotation_angle;
  float rate_of_rotation = (2*linear_velocity)/distance_between_wheels;
  float duration_of_turn;
  
  float rot_start_time = start_time + duration_side;
  float rot_end_time = rot_start_time + duration_of_turn;
  
  //addition to code 1
  
  float ps_values[2] = {0.0, 0.0};
  float dist_values[2] = {0.0, 0.0};
  
  float wheel_obim = 2 * M_PI * wheel_radius;
  float encoder_unit = wheel_obim/(2*M_PI);
  
  //robot pose
  
  double robot_pose[3] = {0.0, 0.0, 0.0}; //x, t, theta
  float last_ps_values[2] = {0.0, 0.0};
  
  //nodes
  
  float nodes[3][2]={{-1.6, 0.6}{1.4, -0.8}{-1.2, 1.0}};
  
  int j = 0;
  
  while (wb_robot_step(TIME_STEP) != -1) {
  
    if(j == 3){
      j = 0;
    }
    
    ps_values[0] = wb_position_sensor_get_value(left_ps);
    ps_values[1] = wb_position_sensor_get_value(right_ps);
    
    
    printf("-------------------\n");
    fflush(stdout);
    printf("position sensor values: {%f} {%f}\n", ps_values[0], ps_values[1]);
    fflush(stdout);
    
    float diff;
    for(int i = 0; i < 2; i++){
      diff = ps_values[i] - last_ps_values[i];
      if(diff < 0.001){
        diff = 0;
        ps_values[i] = last_ps_values[i];
      }
      dist_values[i] = diff * encoder_unit;
    }
    
    //Compute linear and angular velocity for robot
    
    double v = (dist_values[0] + dist_values[1]) / 2.0;
    double w = (dist_values[0] - dist_values[1]) / distance_between_wheels;
    
    float dt = 1;
    robot_pose[2] += (w * dt);
    
    double vx = v * cos(robot_pose[2]);
    double vy = v * sin(robot_pose[2]);
    
    robot_pose[0] += (vx * dt);
    robot_pose[1] += (vy * dt);
    
    printf("robot_pose: {%f} {%f} {%f}\n", robot_pose[0], robot_pose[1], robot_pose[2]);
    fflush(stdout);
    
    for(int i = 0; i < 2; i++){
      last_ps_values[i] = ps_values[i];
    }
  
    float current_time = wb_robot_get_time();
    
    float left_speed = max_speed;
    float right_speed = max_speed;
    
    rotation_angle = atan((robot_pose[1]-nodes[j][1])/(robot_pose[0]-nodes[j][0]))*(180/M_PI);
    
    duration side = (sqrt((robot_pose[1]-nodes[j][1])*(robot_pose[1]-nodes[j][1])+(robot_pose[0]-nodes[j][0])*(robot_pose[0]-nodes[j][0])))/linear_velocity;
    
    duration_of_turn = rotation_angle/rate_of_rotation;
    
    if((rot_start_time < current_time) && (current_time < rot_end_time)) {
      left_speed = -max_speed;
      right_speed = max_speed;
    }else{
      if(current_time > rot_end_time){
        rot_start_time = current_time + duration_side;
        rot_end_time = rot_start_time + duration_of_turn;
      }
    }
    
    wb_motor_set_velocity(left_motor, left_speed);
    wb_motor_set_velocity(right_motor, right_speed);
   
  };

  wb_robot_cleanup();

  return 0;
}
