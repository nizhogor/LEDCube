#include <DropBall.h>
#include <math.h>

// 9	1001
// 10	1010 A
// 11	1011 B
// 12	1100 C
// 13   1101 D
// 14	1110 E
// 15   1111 F

#define g 9.81 // m/s^2
#define BALL_HEIGHT 4

const unsigned char ball_shape[][8] =
{
	{ 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x18, 0x3c, 0x3c, 0x18, 0x00, 0x00 },
	{ 0x00, 0x00, 0x18, 0x3c, 0x3c, 0x18, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00 }
};

DropBallClass::DropBallClass(char(*cube)[CUBE_SIZE], float layer_height_centimeters)
{
	this->cube = (char(*)[CUBE_SIZE]) (cube);
	if (layer_height_centimeters != 0)
		this->layer_height = layer_height_centimeters / 100.0;
	else
		this->layer_height = 0.028; //meters
}

void DropBallClass::drawBall(double altitude, int delay_ms)
{
	int bottom_level = altitude / layer_height;
	int image_level = 0;

	for (int level = bottom_level; level - bottom_level < BALL_HEIGHT; ++level)
	{
		for (int y = 0; y < CUBE_SIZE; ++y)
		{
			if (inRange(level))
			{
				cube[level][y] = ball_shape[image_level][y];
			}
		}
		image_level++;
	}
	delay(delay_ms);
}

void explodePoint(int x, int y, int z, int delay_ms)
{
	if (getvoxel(x, y, z))
	{
		trace_line(x, y, z, rand() % CUBE_SIZE, rand() % CUBE_SIZE, rand() % CUBE_SIZE, delay_ms);
	}
}

void explodeSurface(int delay_ms)
{
	int x, y, z;
	int run = 2;
	while (run < 4)
	{
		x = y = run;
		for (z = 0; z < BALL_HEIGHT; ++z)
		{
			for (; x < CUBE_SIZE - run; ++x)
			{
				explodePoint(x, y, z, delay_ms);
				//delay(delay_ms);
			}
			x--;
			for (; y < CUBE_SIZE - run; y++)
			{
				explodePoint(x, y, z, delay_ms);
				//delay(delay_ms);
			}
			y--;
			for (; x >= run; --x)
			{
				explodePoint(x, y, z, delay_ms);
				//delay(delay_ms);
			}
			x++;
			for (; y >= run; --y)
			{
				explodePoint(x, y, z, delay_ms);
				//delay(delay_ms);
			}
			y++;
		}
		run++;
	}
}

void DropBallClass::effectDropBall(int drop_velocity_m_per_s, int bounce_rate_percent) {

	//ratio speed to delay velosity = 1m/s ~ delay_ms = 100 ms(could be different for your setup)
	int delayToVelocity = 100;
	float bounce_rate = bounce_rate_percent / 100.0;
	double Vc = drop_velocity_m_per_s;
	double Vf = drop_velocity_m_per_s;
	//as delay increases, speed drops, therefore 
	int delay_ms = delayToVelocity / Vf;
	double altitude = 0.5; //by default ball 0.5 meters above ground
	// as long as bounces back to non zero layer
	while (altitude > layer_height) {

		while (altitude >= 0)
		{
			Vf = sqrt(Vc * Vc + 2 * g * layer_height);
			if (Vc == 0)
				Vc = Vf;
			delay_ms = delayToVelocity / Vf;
			Vc = Vf;

			drawBall(altitude, delay_ms);
			if (altitude != 0)
			{
				fill(0x00);
			}

			//Serial.print("down altitude ");
			//Serial.println(altitude);
			//Serial.print("delay ");
			//Serial.println(delay_ms);
			//Serial.print("speed ");
			//Serial.println(Vf);

			altitude -= layer_height;
		}

		altitude = 0;
		if (delay_ms == 0)
			delay_ms = 5;
		delay(10);// pause at the bottom to absorb shock
		Vc *= bounce_rate;

		//fly up until gravity pulls back
		while (Vc > 0.01)
		{
			int Vf2 = Vc * Vc - 2 * g * layer_height;
			if (Vf2 > 0)
			{
				Vf = sqrt(Vf2);
				delay_ms = delayToVelocity / Vf;
			}
			else
				Vf = 0;

			Vc = Vf;
			if (altitude != 0)
			{
				fill(0x00);
			}
			drawBall(altitude, delay_ms);

			//Serial.print("up altitude ");
			//Serial.println(altitude);
			//Serial.print("delay ");
			//Serial.println(delay_ms);
			//Serial.print("speed ");
			//Serial.println(Vf);
			altitude += layer_height;
		}
	}

	delay(500);
	explodeSurface(3);
	return;

}

