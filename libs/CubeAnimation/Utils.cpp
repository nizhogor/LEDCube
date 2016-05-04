#include <Utils.h>

// This function validates that we are drawing inside the cube.
unsigned char inRange(point p)
{
	if (p.x >= 0 && p.x < 8 && p.y >= 0 && p.y < 8 && p.z >= 0 && p.z < 8)
	{
		return 0x01;
	}
	else
	{
		// One of the coordinates was outside the cube.
		return 0x00;
	}
}

unsigned char inRange(int plane)
{
	if (plane >= 0 && plane < 8)
	{
		return 0x01;
	}
	else
	{
		return 0x00;
	}
}

void trace_line(int x1, int y1, int z1, int x2, int y2, int z2, int delay_ms)
{
	float xy;	// how many voxels do we move on the y axis for each step on the x axis
	float xz;	// how many voxels do we move on the y axis for each step on the x axis 
	unsigned char x, y, z;
	unsigned char lasty, lastz;

	// We always want to draw the line from x=0 to x=7.
	// If x1 is bigget than x2, we need to flip all the values.
	if (x1 > x2)
	{
		int tmp;
		tmp = x2; x2 = x1; x1 = tmp;
		tmp = y2; y2 = y1; y1 = tmp;
		tmp = z2; z2 = z1; z1 = tmp;
	}


	if (y1 > y2)
	{
		xy = (float)(y1 - y2) / (float)(x2 - x1);
		lasty = y2;
	}
	else
	{
		xy = (float)(y2 - y1) / (float)(x2 - x1);
		lasty = y1;
	}

	if (z1 > z2)
	{
		xz = (float)(z1 - z2) / (float)(x2 - x1);
		lastz = z2;
	}
	else
	{
		xz = (float)(z2 - z1) / (float)(x2 - x1);
		lastz = z1;
	}

	// For each step of x, y increments by:
	for (x = x1; x <= x2; x++)
	{
		y = (xy*(x - x1)) + y1;
		z = (xz*(x - x1)) + z1;
		setvoxel(x, y, z);
		delay(delay_ms);
		clrvoxel(x, y, z);
		//clrvoxel(prev_z, prev_y, prev_z);
	}
	//clrvoxel(prev_z, prev_y, prev_z);

}