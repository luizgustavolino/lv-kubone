#include <iostream>
#include "bmp.hpp"
#include "notify.hpp"
#include "http.hpp"

#define IN_PATH  "/var/sim/lv-kubone/runtime/inputmap.txt"
#define OUT_PATH "/var/sim/lv-kubone/runtime/frame.bmp"

//Screen dimension constants
const int SCREEN_WIDTH = 160;
const int SCREEN_HEIGHT = 128;

int frame = 0;
int speedx = 1;
int speedy = 1;
int partSize = 10;
int x = 5;
int y = 20;
BMP frame_buffer(SCREEN_WIDTH, SCREEN_HEIGHT);

struct BodyPart {
    BodyPart *next;
    int x;
    int y;
};

BodyPart *head = NULL;

void onEnter(){
    head = (BodyPart*) malloc(sizeof(BodyPart));
    head->next = NULL;
    head->x = 2;
    head->y = 4;
}

void nextFrame(int fd) {

    frame++;
    frame_buffer.fill_region(0, 0, 160, 128, 0, 0, 0);

    BodyPart *current = head;

    do {
        frame_buffer.fill_region(current->x * partSize + 1,current->y * partSize + 1,
                                    partSize, partSize, 0x00, 0xC0, 0xDE);
        current = current->next;
    } while (current != NULL);

    x += speedx;
    y += speedy;
    if (x <= 0 || x >= 150) speedx *= -1;
    if (y <= 0 || y >= 118) speedy *= -1;

    frame_buffer.writeToFD(fd);
}

int main(){
    onEnter();
    HTTP http("1996", nextFrame);
    return 0;
}