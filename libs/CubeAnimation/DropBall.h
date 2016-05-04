#ifndef DropBall_h
#define DropBall_h

#include <Utils.h>

class DropBallClass
{
public:
	DropBallClass(char cube[][CUBE_SIZE], float layer_height_centimeters);
	void drawBall(double altitude, int delay_ms);
	void effectDropBall(int drop_velocity_m_per_s, int bounce_rate_percent);

private:
	char (*cube)[CUBE_SIZE];
	double layer_height;
};

#endif

