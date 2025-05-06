#include "open_interface.h"
#include "Timer.h"
#include "new_uart_interrupt.h"
#include "adc.h"
#include "cyBot_Scan.h"

volatile char received_char;





int boundary_detect(oi_t *sensor_data) {
    oi_update(sensor_data);

    if (sensor_data->cliffLeftSignal >= 2600 || sensor_data->cliffRightSignal >= 2600 || sensor_data->cliffFrontLeftSignal >= 2600 || sensor_data->cliffFrontRightSignal >= 2600) {
        return 1;
    }

    return 0;
}

int hole_detect(oi_t *sensor_data) {
    oi_update(sensor_data);

    if (sensor_data->cliffLeftSignal <= 1000 || sensor_data->cliffRightSignal <= 1000 || sensor_data->cliffFrontLeftSignal <= 1000 || sensor_data->cliffFrontRightSignal <= 1000) {
        return 1;
    }

    return 0;
}

int tall_tree_detect(oi_t *sensor_data) {
    oi_update(sensor_data);

    if (sensor_data->bumpLeft || sensor_data->bumpRight) {
        return 1;
    }

    return 0;
}

int main(void) {
    timer_init();
    uart_interrupt_init();
    oi_t *sensor_data = oi_alloc();
    oi_init(sensor_data);


    lcd_init();
    //cyBOT_init_Scan(0b0111);





    int boundary;
    int hole;
    int tall_tree;

    int sum = 0;

    double currentDegree = 90;

    char sendAngle[100];

    //makes sure we scan after hitting max distance
    int pleaseScan = 1;

    //uart_sendStr("onnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn");

    while (1) {

        boundary = boundary_detect(sensor_data);
        hole = hole_detect(sensor_data);
        tall_tree = tall_tree_detect(sensor_data);

        switch(received_char) {

            case 'w':
                if (boundary == 1 || hole == 1 || tall_tree == 1)
                {

                    oi_setWheels(0, 0);
                    if (boundary == 1) {
                        uart_sendStr("\r\nBoundary detected. Cannot move forward\r\n");
                    }
                    else if (hole == 1) {
                        uart_sendStr("\r\nHole detected. Cannot move forward\r\n");
                    }
                    else if (tall_tree == 1) {
                        uart_sendStr("\r\nRock Detected. Cannot move forward\r\n");
                    }
                }
                else
                {
                    //Move forward Logic

                    if(sum < getMaxDistance() && pleaseScan)
                    {
                        oi_setWheels(25, 25);
                        sum += sensor_data->distance;
                        //uart_sendChar((char)sum);
                        lcd_printf("%d", sum);
                        char newThing[3];
                        sprintf(newThing, "%d", sum);
                        uart_sendStr(newThing);
                    }
                    else
                    {
                        oi_setWheels(0, 0);
                        pleaseScan = 0;
                        uart_sendStr("Please rescan");
                        sum = 0;
                    }
                }
                break;

            case 's':
                    oi_setWheels(-25,-25);


                break;
                
                case 'a':
                if (currentDegree < 180.0) {
                    double angle_turned = 0.0;
                    sensor_data->angle = 0;
                    oi_setWheels(25, -25);
                    while (angle_turned < 1.0) {
                        timer_waitMillis(10);
                        oi_update(sensor_data);
                        angle_turned += fabs(sensor_data->angle);
                    }
                    oi_setWheels(0, 0);
                    currentDegree += 1.0;
                    if (currentDegree > 180.0) currentDegree = 180.0;
            
                    double displayedDegree = ((currentDegree - 60.0) * (180.0 / 61.0));
                    if (displayedDegree < 0) displayedDegree = 0;
                    if (displayedDegree > 180) displayedDegree = 180;
            
                    lcd_printf("Turning Left: %.1lf", displayedDegree);
                    sprintf(sendAngle, "Angle: %0.2lf, Distance Moved: %d, Max Distance: %d", displayedDegree, sum, getMaxDistance());
                    uart_sendStr(sendAngle);
                }
                break;
            
            
            
            

                case 'd':
                if (currentDegree > 0.0) {
                    double angle_turned = 0.0;
                    sensor_data->angle = 0;
                    oi_setWheels(-25, 25);
                    while (angle_turned < 1.0) {
                        timer_waitMillis(10);
                        oi_update(sensor_data);
                        angle_turned += fabs(sensor_data->angle);
                    }
                    oi_setWheels(0, 0);
                    currentDegree -= 1.0;
                    if (currentDegree < 0.0) currentDegree = 0.0;
            
                    double displayedDegree = ((currentDegree - 60.0) * (180.0 / 61.0));
                    if (displayedDegree < 0) displayedDegree = 0;
                    if (displayedDegree > 180) displayedDegree = 180;
            
                    lcd_printf("Turning Right: %.1lf", displayedDegree);
                    sprintf(sendAngle, "Angle: %0.2lf, Distance Moved: %d, Max Distance: %d", displayedDegree, sum, getMaxDistance());
                    uart_sendStr(sendAngle);
                }
                break;
            
            
            
            
            case '1':
                //short scan
                //fastScan();
                shortScan();
                break;
            case '2':
                //long scan
                longScan();
                pleaseScan = 1;
                currentDegree = 90;
                break;
            case '9':
                if (currentDegree <= 90)
                {
                    double angle_turned = 0.0;
                    sensor_data->angle = 0;
                    oi_setWheels(25, -25);
                    while (angle_turned < 90.0) {
                        timer_waitMillis(10);
                        oi_update(sensor_data);
                        angle_turned += fabs(sensor_data->angle);
                    }
                    oi_setWheels(0, 0);
                    currentDegree += 90.0;
                    if (currentDegree > 180.0) currentDegree = 180.0;
                    lcd_printf("Turning Left 90: %.1lf", currentDegree);

                    sprintf(sendAngle, "Angle: %0.2lf, Distance Moved: %d, Max Distance: %d", currentDegree, sum, getMaxDistance());
                    uart_sendStr(sendAngle);
                }
                break;
            case '0':
                if (currentDegree >= -180.0)
                {
                    double angle_turned = 0.0;
                    sensor_data->angle = 0;
                    oi_setWheels(-25, 25);
                    while (angle_turned < 90.0) {
                        timer_waitMillis(10);
                        oi_update(sensor_data);
                        angle_turned += fabs(sensor_data->angle);
                    }
                    oi_setWheels(0, 0);
                    currentDegree -= 90.0;
                    if (currentDegree < -180.0) currentDegree = 90.0;
                    lcd_printf("Turning Right 90: %.1lf", currentDegree);

                    sprintf(sendAngle, "Angle: %0.2lf, Distance Moved: %d, Max Distance: %d", currentDegree, sum, getMaxDistance());
                    uart_sendStr(sendAngle);
                }
                break;
            default:
                oi_setWheels(0, 0);
                break;
        }
        received_char = 0;

//        timer_waitMillis(10); //just in case, to avoid duplicated input
    }


}
