/*
 * File:          glavni_controller.c
 * Date:          23.08.2021.
 * Description:   Zaobilazenje prepreka sa datim koordinatima
 * Author:        Andrej Bantulic, Nemanja Gemovic
 */
#include <webots/robot.h>
#include <webots/motor.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <webots/gps.h>

#define TIME_STEP 32  

int main(int argc, char **argv) {
  
  /* otvaranje fajla */
  
  char *ch;
  FILE *fp;
  
  //Ovde se menja file odakle se citaju tacke po kojima se robot krece
  fp = fopen("C:/Users/Windows User/PycharmProjects/A_zvezda1/Mazes/Maze_5/Path_5.txt", "r");

  if(fp == NULL){
    perror("Error while opening the file.\n");
    exit(EXIT_FAILURE);
  }
  
  double koordinate[60];
  char line[256];
  int i = 0;
  int j = 2;
  
  while(fgets(line, sizeof(line), fp)){
    koordinate[i] = strtod(line, &ch) ;
    i++;
  }
  
  fclose(fp);
  koordinate[i] = 0;
  koordinate[i+1] = 0;
  
  // inicijalizacija webots device-ova i potrebnih promenljivih
  
  wb_robot_init();
  float max_speed=2*M_PI;
  
  WbDeviceTag gps = wb_robot_get_device("gps");
  
  wb_gps_enable(gps, TIME_STEP);
  
  float x, y, z;
  
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
  
  float ukupna_greska = 0;
  float prosecna_greska = 0;
  
  float zapravo_ukupna_distanca = 0;
  float ukupna_distanca = 0;
  
  float rot_start_time = start_time;
  
  while (wb_robot_step(TIME_STEP) != -1) {
    float current_time = wb_robot_get_time();
    
    x = wb_gps_get_values(gps)[0];
    y = wb_gps_get_values(gps)[1];
    z = wb_gps_get_values(gps)[2];
    
    float left_speed = max_speed;
    float right_speed = max_speed;
    
    float length_distance = sqrt((start[0] - end[0]) * (start[0] - end[0]) + (start[1] - end[1]) * (start[1] - end[1]));
  
    float duration_distance = length_distance / linear_velocity;
    
    float temp[2] = {end[0], start[1]};
    
    // Odredjivanje kvadranta u kom se nalazi nova pozicija
    if((end[0] > start[0]) && (end[1] >= start[1])){
      new_angle = atan((end[1]-temp[1])/(temp[0]-start[0]));
    }
    if((end[0] <= start[0]) && (end[1] > start[1])){
      new_angle = M_PI - atan((end[1]-temp[1])/(start[0]-temp[0]));
    }
    if((end[0] < start[0 ]) && (end[1] <= start[1])){
      new_angle = M_PI + atan((temp[1]-end[1])/(start[0]-temp[0]));
    }
    if((end[0] >= start[0]) && (end[1] < start[1])){
      new_angle = 2*(M_PI) - atan((temp[1]-end[1])/(temp[0]-start[0]));
    }
    
    float rotation_side = 1;
    
    // Odredjivanje u kom smeru se okrece ka novoj prepreci i za koliko
    if(new_angle >= current_angle){
      if((new_angle - current_angle) <= (2 * (M_PI) - new_angle + current_angle)){
        rotation_angle = new_angle - current_angle;
        rotation_side = 1;
      }else{
        rotation_angle = 2 * (M_PI) - new_angle + current_angle;
        rotation_side = -1;
      }
    }else{
      if((current_angle - new_angle) <= (2 * (M_PI) + new_angle - current_angle)){
        rotation_angle = current_angle - new_angle;
        rotation_side = -1;
      }else{
        rotation_angle = 2 * (M_PI) + new_angle - current_angle;
        rotation_side = 1;
      }
    }
    
    float duration_of_turn = rotation_angle / rate_of_rotation;
  
    float rot_end_time = rot_start_time + duration_of_turn;
    
    //Rotacija robota
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
    
    /*
     *Jedan trenutak pre rotacije gde se ubacuje nova tacka kao cilj,
     *racunaju greske i u slucaju dolaska do poslednje koordinate izlazi iz
     *petlje. Dodela novih vrednosti starta zavisi od toga da li zelimo da
     *smanjimo gresku simulacije ili ne
     */
    
    if((rot_start_time - current_time > 0.00000001) && (rot_start_time - current_time < 0.032 )){
      ukupna_greska += sqrt((end[0] - z) * (end[0] - z) + (end[1] - x) * (end[1] - x));
      
      zapravo_ukupna_distanca += sqrt((start[0] - z) * (start[0] - z) + (start[1] - x) * (start[1] - x));
      ukupna_distanca += length_distance;
      //start[0] = end[0];
      //start[1] = end[1];
      
      // Ovde se od z i x menja koliko se oduzima translacija robota u Webotsu
      start[0] = z - 0.25;
      start[1] = x - 0.25;
      
      end[0] = koordinate[j] / 4;
      end[1] = koordinate[j+1] / 4;
      j += 2;
      current_angle = new_angle;
      if((end[0] == 0) && (end[1] == 0)){
        x = wb_gps_get_values(gps)[0];
        z = wb_gps_get_values(gps)[2];
        
        end[0] = koordinate[j - 4] / 4;
        end[1] = koordinate[j - 3] / 4;
        
        float konacna_greska = sqrt((end[0] - z) * (end[0] - z) + (end[1] - x) * (end[1] - x));
        
        prosecna_greska = ukupna_greska / ((j - 4) / 2);
        
        printf("%f, %f, %f, %f, %f\n", konacna_greska, ukupna_greska, prosecna_greska, zapravo_ukupna_distanca, ukupna_distanca);
        fflush(stdout);
        
        break;
      }
    }
    
    wb_motor_set_velocity(left_motor, left_speed);
    wb_motor_set_velocity(right_motor, right_speed);
    
    printf("%f, %f, %f\n", current_time, rot_start_time, rot_end_time);
    fflush(stdout);
    
    //printf("(%f, %f) , (%f, %f)\n", start[0], start[1], end[0], end[1]);
    //fflush(stdout);
    
    printf("%f, %f, %f\n", x, y, z);
    fflush(stdout);
    
  };

  // Cleanup kod
  wb_motor_set_velocity(left_motor, 0);
  wb_motor_set_velocity(right_motor, 0);
  wb_robot_cleanup();

  return 0;
}