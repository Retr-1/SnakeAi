#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "DenseNN.h"

//enum Direction {
//	NORTH,
//	EAST,
//	SOUTH,
//};

constexpr int size = 60;

class Snake {
public:
	std::vector<olc::vi2d> body;
	int direction;
	//static olc::vi2d dirMv[4];

	void moveForward() {
		olc::vi2d headPos = body[0];
		body.pop_back();
		
		switch (direction) {
		case 0:
			headPos.y -= 1;
			break;
		case 1:
			headPos.x += 1;
			break;
		case 2:
			headPos.y += 1;
			break;
		case 3:
			headPos.x -= 1;
			break;
		}

		body.insert(body.begin(), headPos);

	}

	void turnLeft() {
		direction = (direction - 1 + 4) % 4;
	}

	void turnRight() {
		direction++;
		direction %= 4;
	}
};
//olc::vi2d Snake::dirMv = {{0,-1}, {1,0}, {0,1}, {}}

class DenseSnake : public Snake {
public:
	NeuralNetwork brain;

	DenseSnake(NeuralNetwork& brain) : brain(brain) {}
	DenseSnake(std::vector<int>& shape) : brain(shape) {}
	DenseSnake() {}

	void decide(std::vector<float>& input) {
		// Output:
		// 1. Turn left
		// 2. Turn right
		// 3. Forward

		auto& output = brain.evaluate(input);
		//float bestVal = output[0];
		//int bestIndex = 0;
		//for (int i = 1; i < 3; i++) {
		//	if (output[i] > bestVal) {
		//		bestVal = output[i];
		//		bestIndex = i;
		//	}
		//}

		if (output[0] > output[1] && output[0] > output[2]) {
			turnLeft();
		}
		else if (output[1] > output[2]) {
			turnRight();
		}
	}

	DenseSnake reproduce(DenseSnake& partner) {
		const float mutation = 0.01f;
		NeuralNetwork nn = brain.intercourse(partner.brain);
		DenseSnake child(nn);
		child.brain.mutate(mutation);
		return child;

	}
};


// Override base class with your custom functionality
class Window : public olc::PixelGameEngine
{

	std::vector<int> nnShape = { size * size + 3, 100, 3 };

	olc::vi2d apple;
	int nIters = 1;
	int nAgents = 100;
	std::vector<DenseSnake> snakes;
	int snakeIndex = 0;

	// INPUTS:
	// 1. size*size == grid of obstacles, including snake's body
	// 2. float == pos of head
	// 3. float == pos of apple
	// 4. float == direction 
	std::vector<float> nnInput;

	void prepareInput() {
		auto& snake = snakes[snakeIndex];
		std::fill(nnInput.begin(), nnInput.begin() + size * size, 0);
		for (int i = 0; i < snake.body.size(); i++) {
			nnInput[snake.body[i].x + snake.body[i].y * size] = 1;
		}
		nnInput[size*size] = snake.body[snake.body.size()-1]
	}
	
public:
	Window()
	{
		// Name your application
		sAppName = "Window";
	}

	bool OnUserCreate() override
	{
		// Called once at the start, so create things here
		nnInput.resize(nnShape[0]);
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		// Called once per frame, draws random coloured pixels
		DenseSnake& snake = snakes[snakeIndex];
		for (int i = 0; i < nIters; i++) {
			snake.decide()
		}
		return true;
	}
};

int main()
{
	Window win;
	if (win.Construct(size, size, 20, 20))
		win.Start();
	return 0;
}