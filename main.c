#include <stdint.h>
#include "uart-interrupt.h"
#include "Timer.h"
#include "open_interface.h"

int main(void) {
    timer_init();
    uart_interrupt_init();
    oi_t *sensor_data = oi_alloc();
    oi_init(sensor_data);

    while (1) {
        oi_update(sensor_data);
        char buffer[100];
        sprintf(buffer, "Cliff Left: %u\tFront Left: %u\tFront Right: %u\tRight: %u\r\n", sensor_data->cliffLeftSignal,
                sensor_data->cliffFrontLeftSignal,
                sensor_data->cliffFrontRightSignal,
                sensor_data->cliffRightSignal);
        uart_sendStr(buffer);
        //less than 2500 run
        if (sensor_data->cliffFrontLeftSignal <= 2500) {
            oi_setWheels(100, 100);
        }
        else { //bigger than 2500 stop
            oi_setWheels(0, 0);
            break;

        }

        timer_waitMillis(200);
    }
}


