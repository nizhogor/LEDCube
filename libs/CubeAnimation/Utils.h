#ifndef Utils_h
#define Utils_h

#include <Arduino.h>	
#define CUBE_SIZE 8
#define LIMIT 7

struct point {
	int x;
	int y;
	int z;
};

unsigned char inRange(point p);
unsigned char inRange(int level);
void trace_line(int x1, int y1, int z1, int x2, int y2, int z2, int delay_ms);

// in customvoxel.ino
extern void fill(unsigned char);
extern void setvoxel(int x, int y, int z);
extern unsigned char getvoxel(int x, int y, int z);
extern void clrvoxel(int x, int y, int z);

#endif