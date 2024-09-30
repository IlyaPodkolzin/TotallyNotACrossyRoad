#include <SFML/Graphics.hpp>

using namespace sf;

class Car {
public:
	Sprite carImage;
	float x, y;
	float size_x;
	Car(Texture inputTexture) {
		carImage.setTexture(inputTexture);
		size_x = inputTexture.getSize().x;
	}

	void draw(RenderWindow window) {
		window.draw(carImage);
	}

	void moveBy(float x_diff, float y_diff) {
		x += x_diff;
		y += y_diff;
	}

	void moveTo(float x, float y) {
		this->x = x;
		this->y += y;
	}

	bool checkOutOfBoundary() {
		return x > -size_x;
	}

	Sprite getSprite() {
		return carImage;
	}
};