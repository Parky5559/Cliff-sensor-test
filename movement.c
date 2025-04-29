/*
 * movement.c
 *
 *  Created on: Feb 7, 2025
 *      Author: hbhatt10
 */
#include <string.h>
#include "Timer.h"
#include "lcd.h"
#include "open_interface.h"

double move_forward(oi_t *sensor_data, double distance_mm, int moveSpeed) {
    double sum = 0;
    oi_setWheels(moveSpeed, moveSpeed);
    while (sum < distance_mm) {
        oi_update(sensor_data);
        sum += sensor_data->distance;
    }
    oi_setWheels(0, 0);
    return sum;
}

//Backward function is basically copy past from forward function.
double move_backward(oi_t *sensor_data, double distance_mm, int moveSpeed) {
    double sum = 0;
    oi_setWheels(-moveSpeed, -moveSpeed);
    while (sum < distance_mm) {
        oi_update(sensor_data);
        sum -= sensor_data->distance;
    }
    oi_setWheels(0, 0);
    return sum;
}

void turn_right(oi_t *sensor_data, double degrees) {
    double angle_mod = 15;
    degrees -= angle_mod;
    double currAngle = 0;
    oi_setWheels(-250, 250);
    while (currAngle < degrees) {
        oi_update(sensor_data);
        currAngle -= sensor_data->angle;
    }
    oi_setWheels(0, 0);
}

void turn_left(oi_t *sensor_data, double degrees) {
    double angle_mod = 15;
    degrees -= angle_mod;
    double currAngle = 0;
    oi_setWheels(250, -250);
    while (currAngle < degrees) {
        oi_update(sensor_data);
        currAngle += sensor_data->angle;
    }
    oi_setWheels(0, 0);
}

void move_in_square(oi_t *sensor_data, double side_length, int moveSpeed) {
    int i;
    double distance;
    for (i = 0; i < 4; i++) {
        distance += move_forward(sensor_data, side_length, moveSpeed);
       turn_right(sensor_data, 90);
    }

    lcd_printf("Total Distance:\n %0.2lf", distance);


}


//void forward_bump(oi_t *sensor_data, double distance_mm, int move_speed) {
//    double bump_backtrack_mm = 150;
//    double bump_sidestep_mm = 250;
//    double dist_moved_mm = 0;
//    oi_setWheels(move_speed, move_speed);
//    while (dist_moved_mm < distance_mm) {
//        oi_update(sensor_data);
//        dist_moved_mm += sensor_data->distance;
//        if (sensor_data->bumpLeft) {
//            oi_setWheels(0, 0);
//            move_backward(sensor_data, bump_backtrack_mm, 125);
//            dist_moved_mm -= bump_backtrack_mm;
//            turn_right(sensor_data, 90);
//            move_forward(sensor_data, bump_sidestep_mm, 125);
//            turn_left(sensor_data, 90);
//            oi_setWheels(move_speed, move_speed);
//        }
//        if (sensor_data->bumpRight) {
//            oi_setWheels(0, 0);
//            move_backward(sensor_data, bump_backtrack_mm, 125);
//            dist_moved_mm -= bump_backtrack_mm;
//            turn_left(sensor_data, 90);
//            move_forward(sensor_data, bump_sidestep_mm, 125);
//            turn_right(sensor_data, 90);
//            oi_setWheels(move_speed, move_speed);
//        }
//    }
//    oi_setWheels(0, 0);
//}

void forward_bump(oi_t *sensor_data, double distance_mm, int move_speed)
{
    double distanceMoved = 0;

    while(distanceMoved < distance_mm)
    {
        oi_update(sensor_data);
        distanceMoved += sensor_data->distance;
        lcd_printf("Distance: %.2lf\n", distanceMoved);
        //if left sensor is hit

        int leftBump = sensor_data->bumpLeft;
        int rightBump = sensor_data->bumpRight;
        if(leftBump || rightBump) {
            oi_setWheels(0,0); //first hit stop moving
            move_backward(sensor_data, 150, move_speed); //moves backwards
            distanceMoved -= 150;

            oi_update(sensor_data);
            if(rightBump)
            {
                //his right bumper
                oi_setWheels(0,0);
                turn_left(sensor_data, 90);
                move_forward(sensor_data, 250, move_speed);
                turn_right(sensor_data, 90);
            }
            else
            {
                //hits left bumper
                oi_setWheels(0,0);
                turn_right(sensor_data, 90);
                move_forward(sensor_data, 250, move_speed);
                turn_left(sensor_data, 90);
            }
        }
        else
        {
           oi_setWheels(move_speed, move_speed);//moves forward distance does not matter cause check above
        }
    }
    oi_setWheels(0,0);
}


/**if(sensor_data->bumpLeft) {
            oi_setWheels(0,0); //first hit stop moving
            move_backward(sensor_data, 150, move_speed); //moves backwards
            turn_right(sensor_data, 90);
            move_forward(sensor_data, 250, move_speed);
            turn_left(sensor_data, 90);
        }
        else
        {
            oi_setWheels(0,0); //first hit stop moving
            move_backward(sensor_data, 150, move_speed); //moves backwards
            turn_left(sensor_data, 90);
            move_forward(sensor_data, 250, move_speed);
            turn_right(sensor_data, 90);
        }

}
**/

