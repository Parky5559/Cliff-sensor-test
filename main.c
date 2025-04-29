#include <stdint.h>
#include "uart-interrupt.h"
#include "Timer.h"
#include "open_interface.h"
#include "lcd.h"
#include "movement.h"
#include "cyBot_Scan.h"

void moveFunction();
void moveStraightToCorner();
float calculateLinearWidth(float distance, int angleWidth);
float IR_to_cm(uint16_t irVal);


typedef struct
{
    int minDegree;
    int maxDegree;
    int Irsensor;
    float Pingsensor;
    int midPoint;
    float objectWidth;
} ObjectData;

//ObjectData *detectedObject = NULL;
ObjectData Object;

int main(void) {
    timer_init();
    uart_interrupt_init();
    lcd_init();
    cyBOT_init_Scan(0b0111);

    //cyBOT_SERVO_cal();

    right_calibration_value = 327250;
    left_calibration_value = 1309000;

    //Testing move to a corder
    moveFunction();
    moveStraightToCorner();

}

void moveFunction()
{
    oi_t *sensor_data = oi_alloc();
    oi_init(sensor_data);



        while(1)
        {
               oi_update(sensor_data);

               //oi_setWheels(100, 100);
               lcd_printf("No line");

               if((sensor_data->cliffFrontLeftSignal) > 2600 && (sensor_data->cliffLeftSignal > 2600)) // Both left sensors
               {
                   lcd_printf("Detected Left + Front Left\nFront Left: %u\nJust Left: %u", sensor_data->cliffFrontLeftSignal,sensor_data->cliffLeftSignal);
                   //lcd_printf("Front Left: %u\nJust Left: %u", sensor_data->cliffFrontLeftSignal,sensor_data->cliffLeftSignal);
                   //lcd_printf("Just Left: %u\n", sensor_data->cliffLeftSignal);
                   oi_setWheels(0, 0);
                   turn_right(sensor_data, 60);
                   break;
               }
               else if((sensor_data->cliffFrontRightSignal > 2600) && (sensor_data->cliffRightSignal > 2600)) //Both Right sensors
               {
                   lcd_printf("Detected Right + Front Right\nFront Right: %u\nJust Right: %u\n",sensor_data->cliffFrontRightSignal,sensor_data->cliffRightSignal);

                   oi_setWheels(0, 0);
                   turn_left(sensor_data, 60);
                   break;
               }
               else if((sensor_data->cliffFrontLeftSignal > 2600) && (sensor_data->cliffFrontRightSignal > 2600))//Both front sensors
               {
                   lcd_printf("Both Front\nFront Left: %u\nFront Right: %u\n",sensor_data->cliffFrontLeftSignal, sensor_data->cliffFrontRightSignal);
                   //lcd_printf("Front Left: %u\n", sensor_data->cliffFrontLeftSignal);
                   //lcd_printf("Front Right: %u\n", sensor_data->cliffFrontRightSignal);
                   oi_setWheels(0, 0);
                   turn_right(sensor_data, 90);
                   break;
               }
               else if(sensor_data->cliffLeftSignal > 2600) //far left
               {
                   lcd_printf("Detected Line Left\n%u", sensor_data->cliffLeftSignal);
                   //lcd_printf("%u", sensor_data->cliffLeftSignal);
                   oi_setWheels(0, 0);
                   turn_right(sensor_data, 80);
                   break;
               }
               else if (sensor_data->cliffRightSignal > 2600) //Just far right sensor
               {
                   lcd_printf("Detected Line Right\n%u", sensor_data->cliffRightSignal);
                   //lcd_printf("%u", sensor_data->cliffRightSignal);
                   oi_setWheels(0, 0);
                   turn_left(sensor_data, 80);
                   break;
               }
               else if(sensor_data->cliffFrontLeftSignal > 2600)// Just front left
               {
                   lcd_printf("Just Front Left\n%u", sensor_data->cliffFrontLeftSignal);
                   oi_setWheels(0, 0);
                   turn_right(sensor_data, 80);
                   break;
               }
               else if(sensor_data->cliffFrontRightSignal > 2600) //Just front right
               {
                   lcd_printf("Just Front Right\n%u", sensor_data->cliffFrontRightSignal);
                   oi_setWheels(0,0);
                   turn_left(sensor_data, 80);
                   break;
               }
               else
               {
                   oi_setWheels(50,50);
                   lcd_printf("No Line\nFront left: %u\nFront Right: %u\n", sensor_data->cliffFrontLeftSignal, sensor_data->cliffFrontRightSignal);

               }
               timer_waitMillis(20);

        }

}

void moveStraightToCorner()
{
    adc_init();
    oi_t *sensor_data = oi_alloc();
    oi_init(sensor_data);


    cyBOT_Scan_t scan;

    int degreeMin = 60;
    int degreeMax = 120;

    int inObject = 0;
    int i;

    int Irvalues[3];
    int Irtotal;
    int IRFinal;

    int ping;

    int TherewasObject = 0;


    int numclicks = 0;


    for(i = degreeMin; i <= degreeMax; i+=3)
    {
        //lcd_printf("Degree: %d\nNum clicks: %d", i, numclicks);
        numclicks+=1;


        cyBOT_Scan(i, &scan);
        ping = scan.sound_dist;
        Irvalues[0] = scan.IR_raw_val;

        int j;

        for(j = 1; j < 3; j++)
        {
            cyBOT_Scan(i, &scan);
            Irvalues[j] = scan.IR_raw_val;
        }
        Irtotal = Irvalues[0] + Irvalues[1] + Irvalues[2];

        IRFinal = Irtotal / 3;

        //lcd_printf("%d", IRFinal);


        //ping = scan.sound_dist;

        if(ping > 70)
        {
            ping = 70;
        }

        if(ping > 70) { ping = 70; }

        float IR_cm = IR_to_cm(IRFinal);

        if(IRFinal > 1000 && ping < 70)
        {
            TherewasObject = 1;



            if (!inObject)
            {
                Object.minDegree = i;
                Object.Irsensor = IR_cm;
                Object.Pingsensor = ping;


                Object.maxDegree = i;



                inObject = 1;
            }
            else
            {
                Object.maxDegree = i;
            }

        }
        else
        {
            //leaving Object
            if(inObject)
            {
                int angleWidth = Object.maxDegree - Object.minDegree;

                Object.midPoint = Object.minDegree + (angleWidth / 2);
                Object.objectWidth = calculateLinearWidth(Object.Pingsensor, angleWidth);
            }
            inObject = 0;
        }



    }

    if(TherewasObject == 0)
    {
        move_forward(sensor_data, (70 * 10), 50);
        //lcd_printf("No Object");
    }
    else
    {
        move_forward(sensor_data, (Object.Irsensor * 10), 50); //minus the roomba radius
        //lcd_printf("Object found: %d", (int)Object.Irsensor);
    }

    //Resetting the servo
    cyBOT_Scan(90, &scan);
}



float calculateLinearWidth(float distance, int angleWidth)
{

    float angleRad = (angleWidth * M_PI) / 180;
    float objectWidth =  2 * distance * tan(angleRad / 2);

    return objectWidth;

}

float IR_to_cm(uint16_t irVal)
{
    if(irVal <= 40)
    {
        return 70.0;
    }

    return (67870.0f) / (irVal - 40);
}
