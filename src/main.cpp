#include <iostream>
#include "bmp.hpp"
#include "notify.hpp"

#define IN_PATH  "/var/sim/lv-kubone/runtime/inputmap.txt"
#define OUT_PATH "/var/sim/lv-kubone/runtime/frame.bmp"

int main(){

    BMP frame_buffer(160, 128);
    Notifier notifier(IN_PATH);
    int frame = 0;

    while(true) {

        notifier.waitSignal();

        frame++;
        frame_buffer.set_pixel(frame%100, frame%100 + 2, 0xC0, 0xFE, 0xF0);
        frame_buffer.set_pixel(frame%100, frame%100 + 3, 0xC0, 0xFE, 0xF0);
        frame_buffer.set_pixel(frame%100, frame%100 + 4, 0xC0, 0xFE, 0xF0);

        frame_buffer.write(OUT_PATH);
        printf("frame %d\n", frame);
    }

    return 0;
}