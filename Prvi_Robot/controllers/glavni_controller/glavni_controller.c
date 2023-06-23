/*
 * File:          my_controller_1_C.c
 * Date:
 * Description:
 * Author:
 * Modifications:
 */

/*
 * You may need to add include files like <webots/distance_sensor.h> or
 * <webots/motor.h>, etc.
 */
#include <webots/robot.h>
#include <webots/motor.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * You may want to add macros here.
 */
#define TIME_STEP 32  

/*
 * This is the main program.
 * The arguments of the main function can be specified by the
 * "controllerArgs" field of the Robot node
 */
int main(int argc, char **argv) {
  
  /* otvaranje fajla */
  
  char *ch;
  FILE *fp;

  fp = fopen("C:/Users/Windows User/PycharmProjects/A_zvezda1/Tests/Test_4/Path_4.txt", "r"); // read mode

  if(fp == NULL){
    perror("Error while opening the file.\n");
    exit(EXIT_FAILURE);
  }
  double koordinate[60];
  int i = 0;
  int j = 2;
  
  char line[256];
  
  while(fgets(line, sizeof(line), fp)){
    //if(&ch == "/"){
      //break;
    //}
    koordinate[i] = strtod(line, &ch) ;
    //printf("%f\n", koordinate[i] );
    //fflush(stdout);
    i++;
  }
  fclose(fp);
  koordinate[i] = 0;
  koordinate[i+1] = 0;
  
  /* necessary to initialize webots stuff */
  
  wb_robot_init();
  float max_speed=2*M_PI;
  
  WbDeviceTag left_motor = wb_robot_get_device("motor_1");
  WbDeviceTag right_motor = wb_robot_get_device("motor_2");
  
  wb_motor_set_position(left_motor, HUGE_VALF);
  wb_motor_set_position(right_motor, HUGE_VALF);
  
  wb_motor_set_velocity(left_motor, 0.0);
  wb_motor_set_velocity(right_motor, 0.0);
  
  float start[2] = {0.0 , 0.0};
  float end[2] = {koordinate[0]/4 , koordinate[1]/4};
  
  float rotation_angle = 0;
  
  float wheel_radius = 0.025;
  float linear_velocity = wheel_radius * max_speed;
  
  float start_time = wb_robot_get_time();
  
  float distance_between_wheels=0.09;
  
  float rate_of_rotation = (2*linear_velocity)/distance_between_wheels;
  
  float current_angle = 0;
  float new_angle = 0;
  
  //float kvadrant = 0;
  float rot_start_time = start_time;
  /*
   * You should declare here WbDeviceTag variables for storing
   * robot devices like this:
   *  WbDeviceTag my_sensor = wb_robot_get_device("my_sensor");
   *  WbDeviceTag my_actuator = wb_robot_get_device("my_actuator");
   */

  /* main loop
   * Perform simulation steps of TIME_STEP milliseconds
   * and leave the loop when the simulation is over
   */
  while (wb_robot_step(TIME_STEP) != -1) {
  
    float current_time = wb_robot_get_time();
    
    float left_speed = max_speed;
    float right_speed = max_speed;
    
    float length_distance = sqrt((start[0]-end[0])*(start[0]-end[0]) + (start[1]-end[1])*(start[1]-end[1]));
  
    float duration_distance = length_distance / linear_velocity;
    
    printf("(%f, %f), (%f,%f)\n", start[0], start[1], end[0], end[1]);
    fflush(stdout);
    
    float temp[2] = {end[0], start[1]};
    
    if((end[0] > start[0])&&(end[1] >= start[1])){
      //kvadrant = 1;
      new_angle = atan((end[1]-temp[1])/(temp[0]-start[0])); 
    }
    if((end[0] <= start[0])&&(end[1] > start[1])){
      //kvadrant = 2;
      new_angle = M_PI - atan((end[1]-temp[1])/(start[0]-temp[0])) ;
    }
    if((end[0] < start[0])&&(end[1] <= start[1])){
      //kvadrant = 3;
      new_angle = M_PI + atan((temp[1]-end[1])/(start[0]-temp[0])) ;
    }
    if((end[0] >= start[0])&&(end[1] < start[1])){
      //kvadrant = 4;
      new_angle = 2*(M_PI) - atan((temp[1]-end[1])/(temp[0]-start[0]));
    }
    
    float rotation_side = 1;
    
    if(new_angle >= current_angle){
      if((new_angle-current_angle)<=(2*(M_PI)-new_angle+current_angle)){
        rotation_angle = new_angle - current_angle;
        rotation_side =1;
      }else{
        rotation_angle = 2*(M_PI)-new_angle+current_angle;
        rotation_side = -1;
      }
    }else{
      if((current_angle-new_angle)<=(2*(M_PI)+new_angle-current_angle)){
        rotation_angle = current_angle - new_angle;
        rotation_side = -  1;
      }else{
        rotation_angle = 2*(M_PI)+new_angle-current_angle;
        rotation_side = 1;
      }
    }
    
    float duration_of_turn = rotation_angle/rate_of_rotation;
  
    float rot_end_time = rot_start_time + duration_of_turn;
    
    if((rot_start_time < current_time) && (current_time < rot_end_time)) {
      if(rotation_side == 1){
        left_speed = -max_speed;
        right_speed = max_speed;
      }else{
        left_speed = max_speed;
        right_speed = -max_speed;
      }
    }else{
      if(current_time > rot_end_time){
        rot_start_time = current_time + duration_distance;
        rot_end_time = rot_start_time + duration_of_turn;
      }
    }
    if((rot_start_time - current_time > 0.00001)&&(rot_start_time - current_time < 0.032 )){
      start[0] = end [0];
      start[1] = end [1];
      end[0] = koordinate[j]/4;
      end[1] = koordinate[j+1]/4;
      j+=2;
      current_angle = new_angle;
      if((end[0] == 0)&&(end[1] == 0)){
        break;
      }
      
      //wb_robot_cleanup();
    }
    
    //printf("%f, %f\n", length_distance, duration_distance);
    //fflush(stdout);
    
    /*left_speed= -1*right_speed*/
    
    wb_motor_set_velocity(left_motor, left_speed);
    wb_motor_set_velocity(right_motor, right_speed);
    /*
     * Read the sensors :
     * Enter here functions to read sensor data, like:
     *  double val = wb_distance_sensor_get_value(my_sensor);
     */
     

    /* Process sensor data here */
    printf("%f, %f, %f\n", current_time, rot_start_time, rot_end_time);
    fflush(stdout);
    
    //printf("%f, %f, %f\n", current_angle, new_angle, rotation_angle);
    //fflush(stdout);
   
    
    /*
     * Enter here functions to send actuator commands, like:
     * wb_motor_set_position(my_actuator, 10.0);
     */
  };

  /* Enter your cleanup code here */

  /* This is necessary to cleanup webots resources */
  wb_motor_set_velocity(left_motor, 0);
  wb_motor_set_velocity(right_motor, 0);
  wb_robot_cleanup();

  return 0;
}
