#pragma once
#include <vector>
#include "Random.h"
#include "olcPixelGameEngine.h"

float sigmoid(float x) {
	return 1 / (1 + exp(-x));
}

class NeuralNetwork {
	// Dense Neural Network
	std::vector<std::vector<std::vector<float>>> weights;
	std::vector<int> shape;
	std::vector<std::vector<float>> values;

public:
	NeuralNetwork(std::vector<int>& shape) : shape(shape) {
		for (int i = 0; i < shape.size() - 1; i++) {
			weights.push_back(std::vector<std::vector<float>>());
			for (int j = 0; j < shape[i]; j++) {
				weights[i].push_back(std::vector<float>());
				for (int k = 0; k < shape[i + 1]; k++) {
					weights[i][j].push_back(random2());
				}
			}
		}

		for (int i = 0; i < shape.size(); i++) {
			values.push_back(std::vector<float>());
			for (int j = 0; j < shape[i]; j++) {
				values[i].push_back(0.0f);
			}
		}
	}

	std::vector<float>& evaluate(std::vector<float>& input) {
		for (int i = 0; i < shape[0]; i++) {
			values[0][i] = input[i];
		}

		for (int layer = 1; layer < shape.size(); layer++) {
			for (int b = 0; b < shape[layer]; b++) {
				double sum = 0;
				for (int a = 0; a < shape[layer - 1]; a++) {
					sum += values[layer - 1][a] * weights[layer - 1][a][b];
				}
				values[layer][b] = sigmoid(sum);
			}
		}

		return values[shape.size() - 1];
	}

	void mutate(float chance) {
		const float lr = 0.2f;
		for (int i = 0; i < shape.size() - 1; i++) {
			for (int j = 0; j < shape[i]; j++) {
				for (int k = 0; k < shape[i + 1]; k++) {
					if (chance >= random()) {
						//Approach 1
						/*float target = random2();
						float delta = (target - weights[i][j][k]) * lr;
						weights[i][j][k] += delta;*/

						//Approach 2
						weights[i][j][k] += random2() * lr;
					}
				}
			}
		}
	}

	NeuralNetwork intercourse(const NeuralNetwork& partner) {
		NeuralNetwork child(shape);
		for (int i = 0; i < shape.size() - 1; i++) {
			for (int j = 0; j < shape[i]; j++) {
				for (int k = 0; k < shape[i + 1]; k++) {
					if (random() > 0.5f) {
						child.weights[i][j][k] = weights[i][j][k];
					}
					else {
						child.weights[i][j][k] = partner.weights[i][j][k];
					}
				}
			}
		}
		return child;
	}

	void save(std::string filename) {

	}

	void draw(olc::PixelGameEngine* canvas, int x, int y) {
		const int nodeR = 10;
		const int layerGap = 60;
		const int nodeGap = 40;

		int biggest = 0;
		for (int s : shape) {
			biggest = std::max(biggest, s);
		}

		int maxHeight = biggest * nodeR * 2 + (biggest - 1) * nodeGap;
		std::vector<olc::vi2d> positions;

		int sx = x;
		for (int layer = 0; layer < shape.size(); layer++) {
			int height = shape[layer] * nodeR * 2 + (shape[layer] - 1) * nodeGap;
			int sy = y + (maxHeight - height) / 2;
			for (int n = 0; n < shape[layer]; n++) {
				canvas->FillCircle({ sx + nodeR, sy + nodeR }, nodeR, olc::GREY);
				positions.push_back(olc::vi2d(sx + nodeR, sy + nodeR));
				sy += nodeR * 2 + nodeGap;
			}

			sx += nodeR * 2 + layerGap;
		}

		int c = 0;
		for (int layer = 0; layer < shape.size() - 1; layer++) {
			for (int n = 0; n < shape[layer]; n++) {
				for (int n2 = 0; n2 < shape[layer + 1]; n2++) {
					auto& positionA = positions[c + n];
					auto& positionB = positions[c + shape[layer] + n2];
					float weight = weights[layer][n][n2];
					float shade = (weight + 1) / 2 * 255;
					olc::Pixel color(shade, shade, shade);
					//std::cout << weight << ' ' << (weight + 1) / 2 * 255 <<' '<< (int)color.g << '\n';
					canvas->DrawLine(positionA, positionB, color);
				}
			}
			c += shape[layer];
		}
	}
};