/*
 * movement.h
 *
 *  Created on: Feb 10, 2025
 *      Author: Hrushi Bhatt
 */

#ifndef MOVEMENT_H_
#define MOVEMENT_H_

void forward_bump(oi_t *sensor_data, double distance_mm, int move_speed);

double move_forward(oi_t *sensor_data, double distance_mm, int moveSpeed);

double move_backward(oi_t *sensor_data, double distance_mm, int moveSpeed);


void turn_right(oi_t *sensor_data, double degrees);

void turn_left(oi_t *sensor_data, double degrees);

void move_in_square(oi_t *sensor_data, double side_length, int moveSpeed);




#endif /* MOVEMENT_H_ */
