/*
 * scan.c
 *
 *  Created on: Mar 31, 2025
 *      Author: fjellman
 */


#include "cyBot_Scan.h"
#include <stdio.h>
//#include "uart.h"
#include "new_uart_interrupt.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "adc.h"


int maxDistance = 70;

void setMaxDistance(int maxDist)
{
    maxDistance = maxDist;
}


int getMaxDistance()
{
    return maxDistance;
}


float IR_to_cm(uint16_t irVal)
{
//    if(irVal <= 40)
//    {
//        return 81;
//    }

//    if(irVal <= 80)
//    {
//        irVal = 81;
//    }
//
//    return (67870.0f) / (irVal - 40);

    if(irVal <= 450)
    {
        return 9999.0f;
    }

    float average = (float)irVal;
    return 120000.0f * powf(average, -1.23f);
}


typedef struct
{
    int minDegree;
    int maxDegree;
    float Irsensor;
    //float Pingsensor;
    int midPoint;
    float objectWidth;
} ObjectData;

ObjectData *detectedObject = NULL;

void shortScan()
{
    cyBOT_init_Scan(0b0101);

    adc_init();

    right_calibration_value = 274750;
    left_calibration_value = 1214500;

    cyBOT_Scan_t scan;
    //float Ping;
    int IRValue;
    int degree;
    int objectCount = 0;

    int inObject = 0;

    int temp;


    int minIndex = 0;

    int IRscan[3];
    int IRtotal;

    float currentMinIR = 9999.0;


    //char data[20];
    for(degree = 75; degree <= 105; degree++)
    {
        //setMaxDistance(0);

        cyBOT_Scan(degree, &scan);
        int j;
        IRscan[0] = adc_read();

        for(j = 1; j < 3; j++)
        {
//            cyBOT_Scan(degree, &scan);
//            IRscan[j] =  scan.IR_raw_val;
            IRscan[j] = adc_read();
        }

        IRtotal = IRscan[0] + IRscan[1] + IRscan[2];

        IRValue = IRtotal / 3;





        float IR_cm = IR_to_cm(IRValue);

        //if(IRValue > 1000 && Ping < 70)
        if(IR_cm > 0 && IR_cm <= 70)
        {
            //Object Detect
            //char Test[20];
            //sprintf(Test, "CM VAL %f", IR_cm);
            uart_sendChar('1');
            //uart_sendStr(Test);

            if (!inObject)
            {
                currentMinIR = 9999;

                detectedObject = realloc(detectedObject, (objectCount + 1) * sizeof(*detectedObject));
                detectedObject[objectCount].minDegree = degree;
                detectedObject[objectCount].maxDegree = degree;





                inObject = 1;
                objectCount++;

            }
            else
            {
                detectedObject[objectCount - 1].maxDegree = degree;

            }

            if (IR_cm < currentMinIR)
            {
                currentMinIR = IR_cm;
            }



        }
        else
        {
            //leaving object
            inObject = 0;
            uart_sendChar('0');


            if(objectCount > 0)
            {
                int i = objectCount - 1;
                temp = detectedObject[i].maxDegree - detectedObject[i].minDegree;
                detectedObject[i].Irsensor = currentMinIR;
                detectedObject[i].objectWidth = calculateLinearWidth(currentMinIR, temp);


                detectedObject[i].midPoint = (detectedObject[i].minDegree + detectedObject[i].maxDegree) / 2;
            }


        }


        //After 180 ping







        int i;
        //float smallestWidth = 9999.0;


        int length = objectCount;

        for(i = 0; i < length; i++)
        {

            if(detectedObject[i].objectWidth < detectedObject[minIndex].objectWidth)
            {
                minIndex = i;

            }

        }




    }


    if (inObject && objectCount > 0)
    {
        int i = objectCount - 1;
        int temp = detectedObject[i].maxDegree - detectedObject[i].minDegree;
        detectedObject[i].Irsensor = currentMinIR;

        if (currentMinIR < 9999 && currentMinIR > 0)
            detectedObject[i].objectWidth = calculateLinearWidth(currentMinIR, temp);
        else
            detectedObject[i].objectWidth = 0;

        detectedObject[i].midPoint = (detectedObject[i].minDegree + detectedObject[i].maxDegree) / 2;
    }



    uart_sendChar('\n');
    uart_sendChar('\r');

    //int b

    char Buffer[200];

    int b;
    int min = 9999;

    for(b = 0; b < objectCount; b++)
    {
        //pringing object data
        sprintf(Buffer, "Min Degree: %d MaxDegree: %d, Irsensor: %f, Midpoint: %d, ObjectWidth: %f",
                detectedObject[b].minDegree, detectedObject[b].maxDegree, detectedObject[b].Irsensor, detectedObject[b].midPoint, detectedObject[b].objectWidth);

        uart_sendStr(Buffer);

        uart_sendChar('\n');
        uart_sendChar('\r');



        //GOOD LUCH
        if(detectedObject[b].Irsensor < min && objectCount != 0)
        {
            setMaxDistance(detectedObject[b].Irsensor);
        }





        //determining final object
        if(detectedObject[b].objectWidth >= 18.5)
        {
            char FoundObject[20];
            sprintf(FoundObject, "Found! Angle: %d", detectedObject[b].midPoint);
            uart_sendStr(FoundObject);
        }


    }

    //making sure that if no object is seen then objectDistance is 70 for distance it can move

    if(objectCount == 0)
    {
        uart_sendStr("Clear to go straight min distance");
        setMaxDistance(70);
        uart_sendChar('\n');
        uart_sendChar('\r');
    }
    else
    {
        uart_sendStr("Cannot go straight object in path");
        uart_sendChar('\n');
        uart_sendChar('\r');
    }
}



void longScan()
{
    cyBOT_init_Scan(0b0101);
    adc_init();

    //cyBOT_SERVO_cal();

    right_calibration_value = 274750;
    left_calibration_value = 1214500;

    cyBOT_Scan_t scan;
    //float Ping;
    int IRValue;
    int degree;
    int objectCount = 0;

    int inObject = 0;

    int temp;

//    int smallestObject = 0;
//    int smallestDegree = 0;
//
//    int previousCount = 0;

    int minIndex = 0;

    int IRscan[3];
    int IRtotal;

    float currentMinIR = 9999.0;


    //char data[20];
    for(degree = 1; degree <= 180; degree+=3)
    {
        //setMaxDistance(0);

        cyBOT_Scan(degree, &scan);
        int j;
        IRscan[0] = adc_read();

        for(j = 1; j < 3; j++)
        {
            //cyBOT_Scan(degree, &scan);
            IRscan[j] =  adc_read();
        }

//        for(j = 0; j < 3; j++)
//        {
//            IRtotal += IRscan[j];
//        }

        IRtotal = IRscan[0] + IRscan[1] + IRscan[2];

        IRValue = IRtotal / 3;


        //setting final value
        //IRValue = scan.IR_raw_val;

        //Ping = scan.sound_dist;
        //IRValue = scan.IR_raw_val;

        //sprintf(data, "%.2f %d %d", Ping, IRValue, degree);

//        if(Ping >= 70)
//        {
//            Ping = 70;
//        }

        float IR_cm = IR_to_cm(IRValue);

        //if(IRValue > 1000 && Ping < 70)
        if(IR_cm > 0 && IR_cm <= 70)
        {
            //Object Detect
            uart_sendChar('1');

            if (!inObject)
            {
                currentMinIR = 9999;
                //object detected // we are first detecting object
                detectedObject = realloc(detectedObject, (objectCount + 1) * sizeof(*detectedObject));
                detectedObject[objectCount].minDegree = degree;
                detectedObject[objectCount].maxDegree = degree;

                //IR_cm = IR_to_cm(IRValue);
                //detectedObject[objectCount].Irsensor = IR_cm;
                //detectedObject[objectCount].midPoint = (detectedObject[objectCount].maxDegree - detectedObject[objectCount].minDegree) / 2;

                //setting IR value
                //float IR_cm = IR_to_cm(IRValue);


                //detectedObject[objectCount].Pingsensor = Ping;


                //added for when less sleepy

                //previousCount = objectCount;



                inObject = 1;
                objectCount++;

//                if(previousCount != 0)
//                {
//                    previousCount = objectCount - 1;
//                }
            }
            else
            {
                detectedObject[objectCount - 1].maxDegree = degree;

            }

            if (IR_cm < currentMinIR)
            {
                currentMinIR = IR_cm;
            }



        }
        else
        {
            //leaving object
            inObject = 0;
            uart_sendChar('0');

            //temp = detectedObject[previousCount].maxDegree - detectedObject[previousCount].minDegree;
            if(objectCount > 0)
            {
                int i = objectCount - 1;
                temp = detectedObject[i].maxDegree - detectedObject[i].minDegree;
                detectedObject[i].Irsensor = currentMinIR;
                detectedObject[i].objectWidth = calculateLinearWidth(currentMinIR, temp);


                detectedObject[i].midPoint = (detectedObject[i].minDegree + detectedObject[i].maxDegree) / 2;
            }

            //now we do linearWidth for the object
//            detectedObject[previousCount].objectWidth = calculateLinearWidth(detectedObject[previousCount].Pingsensor, temp);
//
//            detectedObject[previousCount].midPoint = (detectedObject[previousCount].minDegree + detectedObject[previousCount].maxDegree) / 2;
        }


        //After 180 ping







        int i;
        //float smallestWidth = 9999.0;


        int length = objectCount;

        for(i = 0; i < length; i++)
        {
//            if(detectedObject[i].objectWidth < smallestWidth)
//            {
//                smallestObject = i;
//                smallestDegree = detectedObject[i].maxDegree;
//                smallestWidth = detectedObject[i].objectWidth;
//           }
            if(detectedObject[i].objectWidth < detectedObject[minIndex].objectWidth)
            {
                minIndex = i;

            }

        }

        //char Buffer[20];

        //sprintf(Buffer, "%d %d %d %d %.2f",inObject,  objectCount, minIndex, degree, scan.sound_dist);

        //uart_sendStr(Buffer);
        //uart_sendStr(data);
        //uart_sendChar('\n');
        //uart_sendChar('\r');


    }


    if (inObject && objectCount > 0)
    {
        int i = objectCount - 1;
        int temp = detectedObject[i].maxDegree - detectedObject[i].minDegree;
        detectedObject[i].Irsensor = currentMinIR;

        if (currentMinIR < 9999 && currentMinIR > 0)
            detectedObject[i].objectWidth = calculateLinearWidth(currentMinIR, temp);
        else
            detectedObject[i].objectWidth = 0;

        detectedObject[i].midPoint = (detectedObject[i].minDegree + detectedObject[i].maxDegree) / 2;
    }

    //cyBOT_Scan((int)smallestDegree, &scan);
    //cyBOT_Scan((int)detectedObject[minIndex].midPoint, &scan);
    //setMidpoint((int)detectedObject[minIndex].midPoint);
    //setDistance(detectedObject[minIndex].Pingsensor);

    uart_sendChar('\n');
    uart_sendChar('\r');

    //int b

    char Buffer[200];

    int b;
    int min = 9999;

    for(b = 0; b < objectCount; b++)
    {
        //pringing object data
        sprintf(Buffer, "Min Degree: %d MaxDegree: %d, Irsensor: %f, Midpoint: %d, ObjectWidth: %f",
                detectedObject[b].minDegree, detectedObject[b].maxDegree, detectedObject[b].Irsensor, detectedObject[b].midPoint, detectedObject[b].objectWidth);

        uart_sendStr(Buffer);

        uart_sendChar('\n');
        uart_sendChar('\r');



        //GOOD LUCH
        if(detectedObject[b].Irsensor < min && objectCount != 0)
        {
            setMaxDistance(detectedObject[b].Irsensor);
        }



        //determining final object
        if(detectedObject[b].objectWidth >= 9.5)
        {
            char FoundObject[20];
            sprintf(FoundObject, "Found! Angle: %d", detectedObject[b].midPoint);
            uart_sendStr(FoundObject);
            uart_sendChar('\n');
            uart_sendChar('\r');
        }

        //making sure that if no object is seen then objectDistance is 70 for distance it can move
        if(objectCount == 0)
        {
            setMaxDistance(70);
        }
    }


//    typedef struct
//    {
//        int minDegree;
//        int maxDegree;
//        int Irsensor;
//        float Pingsensor;
//        int midPoint;
//        float objectWidth;
//    } ObjectData;
//
//    ObjectData *detectedObject = NULL;

}

float returnPing;

void setDistance(float Ping)
{
    returnPing = Ping;
}

float getPing()
{
    return returnPing;
}






int ReturnMidpoint;

void setMidpoint(int midPoint)
{
    ReturnMidpoint = midPoint;
}

int getMidPoint()
{

    return ReturnMidpoint;
}







int IrToMeters(int adc_value)
{
   float voltage = (adc_value * 3.3) / 4090.2;

   if(voltage <= 0.42) {
       return -1;
   }

   float distance = 27.86 / (voltage - 0.42);
   return distance;
}

float calculateLinearWidth(float distance, int angleWidth)
{

    float angleRad = (angleWidth * M_PI) / 180;
    float objectWidth =  2 * distance * tan(angleRad / 2);

    return objectWidth;


    //float angleRad = angleWidth * (M_PI / 180);
    //return 2 * distance * tan(angleRad / 2.0);

    //return 2 * M_PI * distance * ((double)angleWidth / 360.0);

}

/**
 *         IRscan[0] = scan.IR_raw_val;

        int b;
        for(b = 1; b < 3; b++)
        {
            //getting all the IRScanvalues in an array
            cyBOT_Scan(degree, &scan);
            IRscan[b] = scan.IR_raw_val;
        }

        for(b = 0; b < 2; b++)
        {
            IRtotal += IRscan[b];
        }
 */
