
#include <iostream>
#include <math.h>
#include <stdlib.h>
#include "bmp.hpp"
#include "http.hpp"

//Screen dimension constants
const int SCREEN_WIDTH = 160;
const int SCREEN_HEIGHT = 128;
BMP frame_buffer(SCREEN_WIDTH, SCREEN_HEIGHT);

struct BodyPart {
    BodyPart *next;
    unsigned short x;
    unsigned short y;
};

struct Apple{
    unsigned short x;
    unsigned short y;
};

BodyPart *head = NULL;
Apple *apple = NULL;

int  frame    = 0;
int  partSize = 10;
int  dead     = 0;
int  speed    = 15;
char direction;

BodyPart* addBodyPart(unsigned short x, unsigned short y) {
    BodyPart* part = (BodyPart*) malloc(sizeof(BodyPart));
    part->next = NULL;
    part->x = x;
    part->y = y;
    return part;
}

bool hasBodyAt(int x, int y, BodyPart* start = NULL) {
    BodyPart *current = start == NULL ? head : start;
    do {
        if (current->x == x && current->y == y) return true;
        else current = current->next;
    } while (current != NULL);
    return false;
}

void addRandomApple(){

    if(apple != NULL){
        free(apple);
        apple = NULL;
    }
    
    srand(frame); int x = rand()%10;
    srand(frame); int y = rand()%10;

    if (!hasBodyAt(x, y)) {
        apple = (Apple*) malloc(sizeof(Apple));
        apple->x = x;
        apple->y = y;
        speed --;
    }
}

bool removeTail(){

    BodyPart *current = head;
    BodyPart *before  = NULL;

    while (current->next != NULL) {
        before = current;
        current = current->next;
    }

    if(current != NULL && before != NULL){
        before->next = NULL;
        free(current);
        return true;
    } else {
        return false;
    }
}

void killSnake(){
    do {} while (removeTail());
    free(head);
    head = NULL;
}

void onEnter(){
    direction = 'a';
    speed = 14;
    head = addBodyPart(2, 8);
    head->next = addBodyPart(2, 7);
    head->next->next = addBodyPart(2, 6);
    addRandomApple();
}

void nextFrame(HTTP *server) {

    int fd = server->clientfd;
    frame++;

    if(dead > 0) {

        double blend = MAX(0, MIN(1 , dead > 60 ? 0 : ((double)(30 - (dead-30))/30.0)));
        frame_buffer.fill_region(0, 0, 11*10, 11*10,
            0xC0 - ((int) (blend * (0xC0 - 0x22))),
            0x33 - ((int) (blend * (0x33 - 0x22))),
            0x33 - ((int) (blend * (0x33 - 0x22))));

        if (dead > 60){
            BodyPart *current = head;

            do {
                bool shaking = dead > 120 && dead < 160;
                double blend = MAX(0, MIN(1 , dead > 90 ? 0 : ((double)(30 - (dead-60)) / 30.0)));
                frame_buffer.fill_region( (shaking ? rand()%3 - 1 : 0) + current->x * (partSize + 1),
                                          (shaking ? rand()%3 - 1 : 0) + current->y * (partSize + 1),
                                          partSize, partSize,
                                          0xFF - ((int) (blend * (0xFF - 0xC0))),
                                          0xFF - ((int) (blend * (0xFF - 0x33))),
                                          0xFF - ((int) (blend * (0xFF - 0x33))));
                current = current->next;
            } while (current != NULL);
        }

        dead--;

        if(!dead) {
            killSnake();
            onEnter();
        }

    } else {

        frame_buffer.fill_region(0, 0, 160, 128, 0x00, 0x00, 0x00);
        frame_buffer.fill_region(0, 0, 11*10, 11*10, 0x22, 0x22, 0x22);
        BodyPart *current = head;

        do {
            frame_buffer.fill_region( current->x * (partSize + 1), current->y * (partSize + 1),
                                      partSize, partSize, 0x00 + current->y * 20, 0xC0, 0xDE - current->y * 20);
            current = current->next;
        } while (current != NULL);

        if (apple != NULL) {
            int glow = (int) (sin(((double)frame)/5.0) * 50);
            frame_buffer.fill_region(11*apple->x + 1, 11*apple->y + 1, 8, 8, 0xDE, 60 + glow, 60 + glow);
        }

        if (server->pad.up    == PAD_DOWN && direction != 's') direction = 'w';
        if (server->pad.left  == PAD_DOWN && direction != 'd') direction = 'a';
        if (server->pad.down  == PAD_DOWN && direction != 'w') direction = 's';
        if (server->pad.right == PAD_DOWN && direction != 'a') direction = 'd';

        if (apple == NULL) addRandomApple();

        if (frame % MAX(6, speed) == 0){

            BodyPart *oldHead = head;
            switch (direction) {
                case 'w': head = addBodyPart(head->x, (head->y + 1)%10); break;
                case 'd': head = addBodyPart((head->x + 1)%10, head->y); break;
                case 'a': head = addBodyPart(head->x > 0 ? head->x - 1 : 9, head->y); break;
                case 's': head = addBodyPart(head->x, head->y > 0 ? head->y - 1 : 9); break;
                default: oldHead = NULL;
            }

            if (oldHead != NULL) head->next = oldHead; 

            if (hasBodyAt(head->x, head->y, head->next)) dead = 180;
            else if (apple == NULL) addRandomApple();
            else if (head->x == apple->x && head->y == apple->y) addRandomApple();
            else removeTail();
        }
    }

    frame_buffer.writeToFD(fd);
}

int main(){

    onEnter();
    fprintf(stdout, "- %s%s%s -\n","\033[92m", "snake over http","\033[0m");
    HTTP http("1996", nextFrame);
    
    return 0;
}