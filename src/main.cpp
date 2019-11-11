#include <iostream>
#include "bmp.hpp"
#include "notify.hpp"

#define IN_PATH  "/var/sim/lv-kubone/runtime/inputmap.txt"
#define OUT_PATH "/var/sim/lv-kubone/runtime/frame.bmp"

//Screen dimension constants
const int SCREEN_WIDTH = 160;
const int SCREEN_HEIGHT = 128;

int main(){

    BMP frame_buffer(SCREEN_WIDTH, SCREEN_HEIGHT);
    Notifier notifier(IN_PATH);

    int frame = 0;
    int speedx = 1;
    int speedy = 1;
    int x = 5;
    int y = 20;

    while(true) {

        notifier.waitSignal();

        frame++;
        frame_buffer.fill_region(0, 0, 160, 128, 0, 0, 0);
        frame_buffer.fill_region(x, y, 10, 10, 0xC0, 0xDE, 0x00);

        x += speedx;
        y += speedy;
        if (x <= 0 || x >= 150) speedx *= -1;
        if (y <= 0 || y >= 118) speedy *= -1;

        frame_buffer.write(OUT_PATH);
    }

    return 0;
}