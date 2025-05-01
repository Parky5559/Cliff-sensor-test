#include "open_interface.h"
#include "Timer.h"
#include "new_uart_interrupt.h"

volatile char received_char;

int boundary_detect(oi_t *sensor_data) {
    oi_update(sensor_data);

    if (sensor_data->cliffLeftSignal >= 21000 || sensor_data->cliffRightSignal >= 21000 || sensor_data->cliffFrontLeftSignal >= 21000 || sensor_data->cliffFrontRightSignal >= 21000) {
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

    int boundary;
    int hole;
    int tall_tree;

    uart_sendStr("onnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn");

    while (1) {

        boundary = boundary_detect(sensor_data);
        hole = hole_detect(sensor_data);
        tall_tree = tall_tree_detect(sensor_data);

        switch(received_char) {

            case 'w':
                if (boundary == 0 && hole == 0 && tall_tree == 0) {
                    oi_setWheels(500, 500);
                }
                else {
                    oi_setWheels(0, 0);
                    if (boundary == 1) {
                        uart_sendStr("\r\nBoundary detected. Cannot move forward\r\n");
                    }
                    else if (hole == 1) {
                        uart_sendStr("\r\nHole detected. Cannot move forward\r\n");
                    }
                    else if (tall_tree == 1) {
                        uart_sendStr("\r\nTall Tree detected. Cannot move forward\r\n");
                    }
                }
                break;

            case 's':
                oi_setWheels(-100, -100);

                break;

            case 'a':
                oi_setWheels(100, -100);
                break;

            case 'd':
                oi_setWheels(-100, 100);
                break;

            default:
                oi_setWheels(0, 0);
                break;
        }
        received_char = 0;

//        timer_waitMillis(10); //just in case, to avoid duplicated input
    }


}
