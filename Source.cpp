#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "DenseNN.h"
#include <algorithm>

constexpr int size = 60;
constexpr int size2 = size * size;

class Snake {
public:
	std::vector<olc::vi2d> body;
	int direction = 0;
	bool grow = false;
	//static olc::vi2d dirMv[4];

	void moveForward() {
		olc::vi2d headPos = body[0];

		if (!grow) {
			body.pop_back();
			grow = false;
		}
		
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

class DenseSnake : public Snake {
public:
	NeuralNetwork brain;
	int fitness = 0;

	DenseSnake(NeuralNetwork& brain) : brain(brain) {}
	DenseSnake(std::vector<int>& shape) : brain(shape) {}

	void decide(std::vector<float>& input) {
		// Output:
		// 1. Turn left
		// 2. Turn right
		// 3. Forward
		auto& output = brain.evaluate(input);

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

	std::vector<int> nnShape = { size2+4, 200, 50, 3 };
	std::vector<float> nnInput;

	std::vector<olc::vi2d> apples;
	int nIters = 1;
	int nAgents = 100;
	const int maxRounds = 50;
	int rounds = 0;
	std::vector<DenseSnake> snakes;
	int snakeIndex = 0;

	void prepareInput() {
		auto& snake = snakes[snakeIndex];
		olc::vi2d& head = snake.body[0];
		for (int i = 0; i < size2; i++) {
			nnInput[i] = 0;
		}
		nnInput[head.x + head.y * size] = 1;

		nnInput[size2] = 0;
		nnInput[size2+1] = 0;
		nnInput[size2+2] = 0;
		nnInput[size2+3] = 0;
		nnInput[size2+snake.direction] = 1;

		/*for (olc::vi2d& apple : apples) {
			nnInput[apple.x + apple.y * size] = -1.0f;
		}*/
	}

	void draw(DenseSnake& snake) {
		for (olc::vi2d& pos : snake.body) {
			Draw(pos);
		}
	}

	void draw() {
		Clear(olc::BLACK);
		draw(snakes[snakeIndex]);
		for (olc::vi2d& apple : apples) {
			Draw(apple, olc::DARK_RED);
		}
	}

	bool doesSurvive(DenseSnake& snake) {
		olc::vi2d& head = snake.body[0];
		if (head.x < 0 || head.x >= size || head.y < 0 || head.y >= size) {
			return false;
		}
		for (int i = 1; i < snake.body.size(); i++) {
			olc::vi2d& bodyPart = snake.body[i];
			if (head.x == bodyPart.x && head.y == bodyPart.y) {
				return false;
			}
		}
		return true;
	}

	olc::vi2d randpos() {
		return { randint(0,size - 1), randint(0,size - 1) };
	}

	void makeNextGeneration() {
		// first n survive and make offspring
		const int nSurvive = 10;
		std::sort(snakes.begin(), snakes.end(), [](DenseSnake& lhs, DenseSnake& rhs) {return lhs.fitness > rhs.fitness;});

		std::vector<DenseSnake> newGeneration;
		for (int i = 0; i < nAgents; i++) {
			auto& parent1 = snakes[randint(0, nSurvive)];
			auto& parent2 = snakes[randint(0, nSurvive)];
			auto child = parent1.reproduce(parent2);
			child.body.push_back({ size / 2,size / 2 });
			newGeneration.push_back(child);
		}

		snakes = newGeneration;
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
		for (int i = 0; i < nAgents; i++) {
			auto snake = DenseSnake(nnShape);
			snake.body.push_back({ size / 2, size / 2 });
			snakes.push_back(snake);
		}
		for (int i = 0; i < 100; i++) {
			apples.push_back(randpos());
		}
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		
		if (snakeIndex == snakes.size()) {
			std::cout << "NEW GEN\n";
			snakeIndex = 0;
			makeNextGeneration();
		}

		DenseSnake& snake = snakes[snakeIndex];
		for (int i = 0; i < nIters; i++) {
			prepareInput();
			snake.decide(nnInput);
			snake.moveForward();
			if (!doesSurvive(snake) || rounds >= maxRounds) {
				snakeIndex++;
				rounds = 0;
				std::cout << "NEW SNAKE\n";
				goto END;
			}
			for (olc::vi2d& apple : apples) {
				if (snake.body[0] == apple) {
					apple = randpos();
					snake.fitness++;
					snake.grow = true;
					break;
				}
			}
			rounds++;
		}

		draw();

	END:;
		return true;
	}
};

int main()
{
	srand(std::time(0));

	Window win;
	if (win.Construct(size, size, 10, 10))
		win.Start();
	return 0;
}