#include <iostream>

#include <sys/types.h>
#include <sys/inotify.h>
#include <unistd.h>

#include "bmp.hpp"

#define EVENT_SIZE    (sizeof (struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))

int main(){


    // startup inotify
    int length, i = 0;
    int fd;
    int wd;
    char buffer[EVENT_BUF_LEN];

    fd = inotify_init1(IN_NONBLOCK);
    if ( fd < 0 ) {
        printf( "can't init inotify");
        return -1;
    }

    wd = inotify_add_watch( fd, "/var/sim/lv-kubone/runtime/inputmap.txt", IN_MODIFY);

    BMP frame_buffer(160, 128);
    bool quit = false;
    int frame = 5;
    while(!quit) {

        length = read( fd, buffer, EVENT_BUF_LEN );
        if (length <= 0) {
            usleep(200);
            continue;
        }

        frame++;

        frame_buffer.set_pixel(frame%100, frame%100 + 2, 0xC0, 0xFE, 0xF0);
        frame_buffer.set_pixel(frame%100, frame%100 + 3, 0xC0, 0xFE, 0xF0);
        frame_buffer.set_pixel(frame%100, frame%100 + 4, 0xC0, 0xFE, 0xF0);

        frame_buffer.write("frame.bmp");
        printf("frame \n");
    }
    return 0;
}