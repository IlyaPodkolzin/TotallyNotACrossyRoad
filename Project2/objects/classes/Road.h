#include "D:/objects/Car.h"
#include "vector"
#include <SFML/Graphics.hpp>

using namespace sf;
using namespace std;

class Road {
private:
	Sprite roadImage;
	float x, y;
	vector<Car> carTypes;
	bool driveDirection;
	float driveSpeed;
public:
	Road() {
		Texture placeholder;
		placeholder.loadFromFile("D:/placeholder.png");
		roadImage.setTexture(placeholder);
		x = 0;
		y = 0;
		driveDirection = "right";
		driveSpeed = 10;
	}

	Road(Texture inputTexture, float x, float y, vector<Car> inputCars, bool inputDirection, float inputSpeed) {
		roadImage.setTexture(inputTexture);
		this->x = x;
		this->y = y;
		for (int i = 0; i < inputCars.size(); i++) carTypes[i] = inputCars[i];
		driveDirection = inputDirection;
		driveSpeed = inputSpeed;
	}

	void draw(RenderWindow window) {
		window.draw(roadImage);
		for (int i = 0; i < carTypes.size(); i++) {
			window.draw(carTypes[i].getSprite());
		}
		//отрисовка каждого автомобиля
	}

	void moveBy(float x_diff, float y_diff) {
		x += x_diff;
		y += y_diff;
		roadImage.setPosition(x, y);
	}

	bool checkOutOfBound(float max_height) {
		return x > max_height;
	}

	void setTexture(Texture newTexture) {
		roadImage.setTexture(newTexture);
	}

	void setCoords(float x, float y) {
		this->x = x;
		this->y = y;
		roadImage.setPosition(x, y);
	}

	void setCars(vector<Car> carTypes) {
		for (int i = 0; i < carTypes.size(); i++) this->carTypes[i] = carTypes[i];
	}

	void setDriveDirection(bool driveDirection) {
		this->driveDirection = driveDirection;
	}

	void setSpeed(float driveSpeed) {
		this->driveSpeed = driveSpeed;
	}

	Sprite getSprite() {
		return roadImage;
	}

	float getCoords() {
		return (x, y);
	}

	float getX() {
		return x;
	}

	float getY() {
		return y;
	}

	vector<Car> getCars() {
		return carTypes;
	}

	bool getDriveDirection() {
		return driveDirection;
	}

	float getSpeed() {
		return driveSpeed;
	}
};