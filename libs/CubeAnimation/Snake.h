#ifndef Snake_h
#define Snake_h

#include <list>
#include <sstream>
#include <Utils.h>
#include <list>

#define DEBUG true
#define MAX_SNAKE_LENGTH 7
#define COLISSION_BLINKING_TIMEOUT_MS 2900
#define BLINKING_TIME 1000

class SnakeClass
{
public:
	SnakeClass(char cube[][CUBE_SIZE], int delay);

	void growSnake(point p);
	void effectSnake();
	void printShape();

private:
	unsigned long delay_ms;
	int snakeLength;
	char(*cube)[CUBE_SIZE];
	std::list<point> snake_shape;
	static point * foodPtr;

	void drawSnake();
	void drawFood();
	void drawCollision();
	void blinkHead();
	void moveSnake(point p);
	void feedSnake();
	bool isGoodPoint(point p);
	point findNextMoveRandomly(bool & success);
	point findNextMoveDeliberately(bool & success);
	static void putFood();
	bool isFoodPoint(point p);
};

#endif