#include "Snake.h"

point * SnakeClass::foodPtr = NULL;

SnakeClass::SnakeClass(char cube[][CUBE_SIZE], int delay_ms)
{
	if (delay_ms == -1)
	{
		this->delay_ms = 100;
	}
	else
	{
		this->delay_ms = delay_ms;
	}
	this->cube = (char(*)[CUBE_SIZE]) (cube);
	this->snakeLength = 0;

	// all snakes share food
	if (foodPtr == NULL)
	{
		foodPtr = new point;
		foodPtr->x = -1;
	}
}

void SnakeClass::drawFood()
{	
	if (foodPtr->x != -1){
		cube[foodPtr->z][foodPtr->y] |= (1 << foodPtr->x);
	}
}

void SnakeClass::drawSnake()
{
	fill(0x00);
	for (std::list<point>::iterator point = snake_shape.begin(); point != snake_shape.end(); point++)
	{
		cube[point->z][point->y] |= (1 << point->x);
	}
	drawFood();
	delay(delay_ms);
}

void SnakeClass::blinkHead()
{
	point head = snake_shape.front();
	unsigned long start = millis();
	while ((millis() - start) < COLISSION_BLINKING_TIMEOUT_MS)
	{
		cube[head.z][head.y] ^= (1 << head.x);
		drawFood();
		delay(BLINKING_TIME);
	}
}

void SnakeClass::drawCollision()
{
	fill(0x00);
	for (std::list<point>::iterator point = snake_shape.begin(); point != snake_shape.end(); point++)
	{
		cube[point->z][point->y] |= (1 << point->x);
	}
	drawFood();
	blinkHead();
}

void SnakeClass::growSnake(point p)
{
	snake_shape.push_front(p);
	snakeLength++;
}

void SnakeClass::moveSnake(point p)
{
	snake_shape.pop_back();
	snake_shape.push_front(p);
}

void SnakeClass::feedSnake()
{
	growSnake(*foodPtr);
	// mark point as gone
	foodPtr->x = -1;
}

void SnakeClass::printShape()
{
	for (std::list<point>::iterator point = snake_shape.begin(); point != snake_shape.end(); point++)
	{
		Serial.print("snake point: ");
		Serial.print(point->x); Serial.print(point->y); Serial.println(point->z);
	}
}

bool SnakeClass::isGoodPoint(point p)
{
	if (inRange(p))
	{
		if (isFoodPoint(p))
		{
			return true;
		}

		// point is busy
		if (cube[p.z][p.y] & (1 << p.x))
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	else
	{
		return false;
	}
}

point SnakeClass::findNextMoveDeliberately(bool & success) {
	success = true;
	point p = snake_shape.front();
	p.x++;
	if (isGoodPoint(p)){
		return p;
	}
	p.x--;
	p.y++;
	if (isGoodPoint(p)){
		return p;
	}
	p.y--;
	p.z++;
	if (isGoodPoint(p)){
		return p;
	}
	p.z--;
	p.x--;
	if (isGoodPoint(p)){
		return p;
	}
	p.x++;
	p.y--;
	if (isGoodPoint(p)){
		return p;
	}
	p.y++;
	p.z--;
	if (isGoodPoint(p)){
		return p;
	}
	p.z++;
	success = false;
}


point SnakeClass::findNextMoveRandomly(bool & success)
{
	success = true;
	uint8_t direction;
	uint8_t tries = 0;
	point p = snake_shape.front();

	while (tries < 9)
	{
		direction = random(1, 7);
		tries++;

		if (direction == 1)
		{
			p.x++;
			if (isGoodPoint(p))
			{
				return p;
			}
			p.x--;
			continue;
		}
		if (direction == 2)
		{
			p.y++;
			if (isGoodPoint(p))
			{
				return p;
			}
			p.y--;
			continue;
		}
		if (direction == 3)
		{
			p.z++;
			if (isGoodPoint(p))
			{
				return p;
			}
			p.z--;
			continue;
		}
		if (direction == 4)
		{
			p.x--;
			if (isGoodPoint(p))
			{
				return p;
			}
			p.x++;
			continue;
		}
		if (direction == 5)
		{
			p.y--;
			if (isGoodPoint(p))
			{
				return p;
			}
			p.y++;
			continue;
		}
		if (direction == 6)
		{
			p.z--;
			if (isGoodPoint(p))
			{
				return p;
			}
			p.z++;
			continue;
		}
	} // end while (tries < ...
	success = false;
}

void SnakeClass::putFood()
{
	// check if food is placed already
	if (foodPtr->x == -1)
	{
		foodPtr->x = random(0, CUBE_SIZE);
		foodPtr->y = random(0, CUBE_SIZE);
		foodPtr->z = random(0, CUBE_SIZE);
	}
}

bool SnakeClass::isFoodPoint(point p)
{
	if (p.x == foodPtr->x && p.y == foodPtr->y && p.z == foodPtr->z)
		return true;
	return false;
}

void SnakeClass::effectSnake()
{
	while (snakeLength < MAX_SNAKE_LENGTH){
		putFood();
		bool found_move = false;

		point nextPoint = findNextMoveRandomly(found_move);
		if (!found_move)
		{
			nextPoint = findNextMoveDeliberately(found_move);
		}

		if (found_move) {
			if (isFoodPoint(nextPoint))
			{
				feedSnake();
			}
			else
			{
				moveSnake(nextPoint);
			}
			drawSnake();		
		}
		else
		{
			drawCollision();
			return;
		}

	} // end while (snakeLength < 250)
}