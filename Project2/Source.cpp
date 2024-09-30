#include <SFML/Graphics.hpp>
#include <vector>
#include <algorithm>
#include <ctime>
#include <iostream>
#include <fstream>
#include <string>
using namespace sf;
using namespace std;


float WIDTH = 1000; // ������ ������
float HEIGHT = 750; // ������ ������
short HERO_JUMP_VELOCITY = 40; // ��������� �������� ������ ����� (� ����� �������)
short HERO_HEIGHT = 50; //  ������ ������� �����
short HERO_WIDTH = 100; //  ������ ������� �����
short ROAD_SEGMENT_HEIGHT = 100; //  ������ ����� ����� ������/������
short FRAME_LIMIT = 60; //  ����������� ���
short TREE_PROBABILITY = 40; //  ����������� ��������� ������ �� ������ ������ ����� ������
float CAMERA_MOVE_SPEED = 0.1f; //  �������� �������� ������ �� ����� ����
float DEATH_CAMERA_MOVE_SPEED = 1.5f; //  �������� �������� ������ ����� ���������
float MIN_CAR_SPEED = 2; //  ����������� �������� ������
float MAX_CAR_SPEED = 4.5; //  ������������ �������� ������
Texture ROAD_TEXTURES[3]; //  ������ ������� ��� ����� ������
vector<Texture> CAR_TEXTURES; //  ������ ������� ��� �����
Texture NATURE_ELEMENTS_TEXTURES[1]; //  ������ ������� ��� ��������������
Texture SUPERPOWER_ELEMENTS_TEXTURES[2];
Font FONT;
float LINE_SPACING = 1.5f;
int SUPERPOWERS_PROBABILITY = 2;
int INVULNERABILITY_TIME = 8;
float SLOWING_RATE = 0.5f;
vector<float> coordinatesXAvailableForHero; //  ������ ��������� �� X, ������� �������� ��� ������� ����� (�� ��������� �������)
long newRecord = 0;
Text rulesTexts[13] = { Text(L"������� ����:", FONT, 72U), Text(L"���������� ���������� ��� ����� ������� ����������,", FONT, 24U),
									 Text(L"�� ����� ��� ������ (��� �� ��).", FONT, 24U), Text(L"����� ����, ����� ���������� � �������� ������", FONT, 24U),
									 Text(L"� �� ��������� �� ������� ���.", FONT, 24U), Text(L"������� ����������� ���������� ��������.", FONT, 24U),
									 Text(L"����������������:", FONT, 36U), Text(L"����� - ������������ - ��������� ��������� ����� �������;", FONT, 24U),
									 Text(L"���� - ���������� ������� - ��������� �������� �������� ����� � 2 ����.", FONT, 24U),
									 Text(L"����������:", FONT, 36U), Text(L"������� - ���������", FONT, 24U), Text(L"WASD - ������ �����������", FONT, 24U),
									 Text(L"������� NUM3, ����� ����� �� ������", FONT, 24U) };
Text mainText1("Totally Not", FONT, 96U);
Text mainText2("A Crossy Road", FONT, 96U);
Text additionalStartText1(L"������� Enter, ����� ������", FONT, 24U);
Text additionalStartText2(L"��� Backspace, ����� �����", FONT, 24U);
Text additionalStartText3(L"��� NUM3, ����� ��������� �������", FONT, 24U);



class Car { // ����� ������������� ��� �������� �����, ������ �� ������� ������ � ����������������� � ������
private:
	Sprite carImage; //  ������ ������
	String carType = "None"; //  ��� ���������� (��������, �����...) - �� ������ �������� ����������
public:
	Car() {
		Texture texture; 
		texture.loadFromFile("objects\\sprites\\misc\\placeholder.png"); //  ��������� �������� ��� ������
		carImage.setTexture(texture); //  ��������� � ������ ��������
	}

	void moveBy(float xDiff, float yDiff) { //  �������, ������������� ������ �� xDiff ������ � yDiff ����
		carImage.move(xDiff, yDiff);
	}

	Sprite getSprite() { //  �������, ������������ ������ ������
		return carImage;
	}

	Vector2f getCoords() { //  �������, ������������ ���������� ������
		return carImage.getPosition();
	}

	vector<float> getSize() {  //  �������, ������������ ������� ������� ������
		return { carImage.getLocalBounds().width, carImage.getLocalBounds().height };
	}
	
	String getCarType() { //  �������, ������������ ��� ������
		return carType;
	}

	void setTexture(Texture &texture) { //  �������, ������������ ������ ������
		carImage.setTexture(texture);
	}

	void setCoords(float x, float y) { //  �������, �������� ������� ������ ����������
		carImage.setPosition(x, y);
	}

	void setSize(int startX, int startY, int sizeX, int sizeY) { //  �������, �������� ������� ������ ������
		carImage.setTextureRect(IntRect(startX, startY, sizeX, sizeY));
	}

	void setScale(float scaleX, float scaleY) { //  �������, �������� ������� ������ ������� (������ 1 - ����������, ������ 1 - ����������) ��� 2 ���������
		carImage.scale(scaleX, scaleY);
	}

	void setCarType(String carType) { //  �������, �������� ��� ������
		this->carType = carType;
	}
};


class Road { //  ����� ������������ ��� �������� � ������������� ����� ������ ��� ������ ������ � ��������, ������ �� ���, ��� ���������������
private:
	Sprite roadImage; //  ������ ������
	vector<Car> cars; //  ������ �����, ������ �� ������ (���� ��� �� �����)
	vector<Sprite> natureElements; //  ������ ��������, ���� ��� �����
	vector<vector<int>> superPowers;
	bool driveDirection = true; //  ����������� ��������: true - �������, false - ������
	bool notPark = true; //  ������ ��������, ����������, �������� �� ������ ������ (�� ��������� ���)
	float driveSpeed = 0; //  �������� �������� �����
public:
	Road() {
		Texture placeholder;
		placeholder = ROAD_TEXTURES[rand() % 3]; //  ���������� �������� ������ �� ������� ������� ��� ����� � ������
		roadImage.setTexture(placeholder); //  �������� ����������� � ������
	}

	void moveBy(float xDiff, float yDiff) { //  �������, ������������� ������ ������ �� xDiff ����� � yDiff ����
		roadImage.move(xDiff, yDiff);
	}

	void setTexture(Texture &newTexture) { // �������, �������� �������� ������� ������
		roadImage.setTexture(newTexture);
	}

	void setCoords(float x, float y) { //  �������, �������� ���������� ������� ������
		roadImage.setPosition(x, y);
	}

	void setCars(vector<Car> cars) { //  �������, �������� ������ �����, ������ �� ������
		for (int i = 0; i < cars.size(); i++) this->cars.push_back(cars[i]);
	}

	void setNatureElements(vector<Sprite> natureElements) { //  �������, �������� ������ ��������������, ����������� �� ������
		for (int i = 0; i < natureElements.size(); i++) this->natureElements.push_back(natureElements[i]);
	}

	void setSuperPowers(vector<vector<int>> superPowers) { //  �������, �������� ������ �����������������
		for (int i = 0; i < superPowers.size(); i++) this->superPowers.push_back(superPowers[i]);
	}

	void setDriveDirection(bool driveDirection) { //  �������, �������� ����������� ��������
		this->driveDirection = driveDirection;
	}

	void setSpeed(float driveSpeed) { //  �������, �������� �������� �������� ����� �� ������
		this->driveSpeed = driveSpeed;
	}

	void setNotPark(bool notPark) { //  �������, �������� ��� ������ (����� - ���� false, ������ �������� - ���� true)
		this->notPark = notPark;
	}

	Sprite getSprite() { //  �������, ������������ ������ ������
		return roadImage;
	}

	Vector2f getCoords() { //  �������, ������������ ���������� ������
		return roadImage.getPosition();
	}

	vector<Car> getCars() { //  �������, ������������ ������ �����, ������ �� ������
		return cars;
	}

	bool getDriveDirection() { //  �������, ������������ ������ ��������, ������������ ����������� �������� �� ������
		return driveDirection;
	}

	float getSpeed() { //  �������, ������������ �������� �������� �� ������
		return driveSpeed;
	}

	bool getNotGrass() { // �������, ������������ ������ ��������, ������������, �������� �� ������ ������� ��� ������� ��������
		return notPark;
	}

	vector<Sprite> getNatureElements() { //  �������, ������������ ������ ��������������, ����������� �� ������
		return natureElements;
	}

	vector<vector<int>> getSuperPowers() { //  �������, ������������ ������ �����������������
		return superPowers;
	}

	void eraseNatureElement(int position) { //  �������, ��������� ������� �������������� � �������� position �� ������� ��������������
		natureElements.erase(natureElements.begin() + position);
	}

	void makeSuperPowerUnaccessable(int position) {
		superPowers[position][0] = -1;
	}

	void eraseUnaccessableSuperPowers() {
		for (int i = 0; i < superPowers.size(); i++)
			if (superPowers[i][0] == -1) superPowers.erase(superPowers.begin() + i);
	}

	bool superPowerCanBeAccessed(int position) {
		return superPowers[position][0] != -1;
	}

	void updateCarsPosition(int carIndex, bool isSlowedDown) { //  �������, ����������� ��������� ����� �� ������ (����������� �������� ������/�����)
		float actualSpeed = driveSpeed;
		if (isSlowedDown) actualSpeed *= SLOWING_RATE;
		if (driveDirection) cars[carIndex].moveBy(actualSpeed, 0);
		else cars[carIndex].moveBy(-actualSpeed, 0);
		respawnIfWentAway(carIndex);
	}

	void respawnIfWentAway(int carIndex) { //  �������, ��������������, ����� ������ �� ������� �� ������� ������
		float currX = cars[carIndex].getCoords().x;
		float carSize = cars[carIndex].getSize()[0];
		if (currX <= -carSize * 3 || currX >= WIDTH + carSize * 3) { //  ���� ������ ����� �� ������� ������ �� ��� ���� �����
			if (driveDirection) cars[carIndex].moveBy(-carSize * 3 - WIDTH, 0); //  �� ������ ����� ���������� � ��������������� ������
			else cars[carIndex].moveBy(carSize * 3 + WIDTH, 0);
		}
	}
};


class Hero { //  �������, ��������������� ��� �������� � ������������� ����� � ��������� ��� ���������/��������� � ����������� �� ��������
private:
	Sprite heroSprite; //  ������ �����
	Texture heroTexture; //  �������� ��� ������� �����
	float velocity = HERO_JUMP_VELOCITY; //  ��������� �������� ������ ����� - ������� ����������

	float currentVelocityX = 0; //  ������� �������� �������� � ����������� Ox (��� �������� ���������� ����� ���������� ������)
	float currentVelocityY = 0; //  ������� �������� �������� � ����������� Oy (����������)

	String currentMove = "Down"; //  ������� �������� ������ (����������� ��������) - �� ��������� ����
	bool currentlyMoving = false; //  ������ ��������, ������������, �������� �� ������ ����� - �� ��������� ���
	bool allowGoingThroughWalls = false; //  ������ ��������, ������������, ����� �� ����� �������� �� ������� ������ (������ ����� ������) - �� ��������� ���

	bool needsToBeDrivenAway = false; //  ������ ��������, ������������, ������������ �� �������� "���������" ����� - �� ��������� ���
	bool isKilled = false; //  ������ ��������, ������������, �������� �� ���� - �� ��������� ���

	bool isInvulnerable = false;
	bool isSlowingTime = false;
public:
	Hero() {
		heroTexture.loadFromFile("objects\\sprites\\misc\\hero.png");
		heroSprite.setTexture(heroTexture); //  ��������� �������� ����� � ��������� � � ������
		heroSprite.setTextureRect(IntRect(101, 0, HERO_WIDTH, HERO_HEIGHT)); //  ����� ����� ��������, ������� ����� ���������� �� ������� (����� ������� ������)
	}

	void setTextureRect(int startX, int startY, int width, int height) { //  �������, �������� ����� ��������, ������� ����� ���������� �� ������� (��� �������� �������� ����� � ������ �������),
		heroSprite.setTextureRect(IntRect(startX, startY, width, height)); //  ������������� � ����� (startX, startY) �������� width, height
	}

	void setMove(String move) { //  �������, �������� ����������� �������� ������ � ������ ������
		currentMove = move;
	}


	void setPosition(float x, float y) { //  �������, �������� ������� ������� �����
		heroSprite.setPosition(x, y);
	}

	void setIsKilled(bool isKilled) {
		this->isKilled = isKilled;
	}

	void setAllowGoingThroughWalls(bool allowGoingThroughWalls) {
		this->allowGoingThroughWalls = allowGoingThroughWalls;
	}

	void setIsInvulnerable(bool isInvulnerable) {
		this->isInvulnerable = isInvulnerable;
	}

	void setIsSlowingTime(bool isSlowingTime) {
		this->isSlowingTime = isSlowingTime;
	}

	void move() { //  �������, ������������� �������� �������� ������
		heroSprite.move(currentVelocityX, currentVelocityY); // ������ ���� ����� ������������� � ��� ��� ���� ����������� � �������� ���������� �� oX � oY (���� ��� �� ����� 0)
		
		if (currentVelocityY > 0) currentVelocityY -= 10;  // ������ ���� �������� ������������ ����������� - �������� ������ ����������
		else if (currentVelocityY < 0) currentVelocityY += 10;

		if (currentVelocityX > 0) currentVelocityX -= 10;
		else if (currentVelocityX < 0) currentVelocityX += 10;

		if (currentVelocityX == 0 && currentVelocityY == 0) currentlyMoving = false;  // ���� ����� �� �������� - currentlyMoving �������� �������� false (�������� � ������� ������ ������� ���)
	}

	void moveBy(float x, float y) { //  �������, ������������� ������ �� ������������ ��� �� �� (x, y)
		heroSprite.move(x, y);
	}


	int update(Event event, int stepCount) { //  �������, ����������� ��������� ����� � ������������ � ���������
		if (event.type == Event::KeyPressed && !currentlyMoving && !isKilled) { // ���� ������ ������� � ����� �� ��������
			if (event.key.code == Keyboard::Left) { //  ���� ������� - ������� �����, ������ ��������� �����������,
				heroSprite.setTextureRect(IntRect(0, 0, HERO_WIDTH, HERO_HEIGHT)); //  ������������ �� ����� ��������, �� ������� ����� ������� �����
				currentVelocityX = -velocity; //  � ������� �������� ��������������� ��������� �������� (������ 0, �.� �����, ����� ���� �� ������ 0)
				this->setMove("Left"); //  ������� �������� �������� ����������� �������� ������
				currentlyMoving = true; //  �������� "����� �� ������ ������ ��������" ��������������� � ��������
			}
			else if (event.key.code == Keyboard::Right) { // ���������� � ���������: ������� ������, �����, ����
				heroSprite.setTextureRect(IntRect(101, 0, HERO_WIDTH, HERO_HEIGHT));
				currentVelocityX = velocity;
				this->setMove("Right");
				currentlyMoving = true;
			}
			else if (event.key.code == Keyboard::Up) {
				heroSprite.setTextureRect(IntRect(0, 52, HERO_WIDTH, HERO_HEIGHT));
				currentVelocityY = -velocity;
				this->setMove("Up");
				currentlyMoving = true;
				stepCount++;
			}
			else if (event.key.code == Keyboard::Down) {
				heroSprite.setTextureRect(IntRect(101, 52, HERO_WIDTH, HERO_HEIGHT));
				currentVelocityY = velocity;
				this->setMove("Down");
				currentlyMoving = true;
				stepCount--;
			}

			else if (event.key.code == Keyboard::A) heroSprite.setTextureRect(IntRect(0, 0, HERO_WIDTH, HERO_HEIGHT)); //  ���� ������� ������� WASD - ������ ��������
			else if (event.key.code == Keyboard::D) heroSprite.setTextureRect(IntRect(101, 0, HERO_WIDTH, HERO_HEIGHT)); //  ����������� ������� �����
			else if (event.key.code == Keyboard::W) heroSprite.setTextureRect(IntRect(0, 52, HERO_WIDTH, HERO_HEIGHT));
			else if (event.key.code == Keyboard::S) heroSprite.setTextureRect(IntRect(101, 52, HERO_WIDTH, HERO_HEIGHT));

		}
		return stepCount;
	}

	void abortMove() { //  �������, ���������� �������� ����� (��������, ��� ������� �� ���� ������)
		currentVelocityX = 0;
		currentVelocityY = 0;
		if (currentMove == "Up") heroSprite.move(0, velocity);
		else if (currentMove == "Down") heroSprite.move(0, -velocity);
		else if (currentMove == "Left") heroSprite.move(velocity, 0);
		else if (currentMove == "Right") heroSprite.move(-velocity, 0);
	}

	Vector2f getCoords() { //  �������, ������������ ���������� �����
		return heroSprite.getPosition();
	}

	Sprite getSprite() { //  �������, ������������ ������ �����
		return heroSprite;
	}

	float getVelocity() { //  �������, ������������ ������� �������� �������� ����� (� ����� �����������)
		return velocity;
	}

	bool getCurrentlyMoving() { //  �������, ������������ ������ ��������, ������������, �������� �� ������ �����
		return currentlyMoving;
	}

	bool getAllowGoingThroughWalls() { //  �������, ������������ ������ ��������, ������������, ����� �� ����� ����� �� ������� ������
		return allowGoingThroughWalls;
	}

	bool getNeedsToBeDrivenAway() { //  �������, ������������ ������ ��������, ������������, ������ �� ������ ������������� �������� "���������" �����
		return needsToBeDrivenAway;
	}

	bool getIsKilled() { // �������, ������������ ������ ��������, ������������, �������� �� �����
		return isKilled;
	}

	bool getIsInvulnerable() {
		return isInvulnerable;
	}

	bool getIsSlowingTime() {
		return isSlowingTime;
	}

	void checkDeadlyIntersection(Car car, float carSpeed, bool driveDirection) { //  �������, �������������� ������������ ������ � �������
		FloatRect carBounds = car.getSprite().getGlobalBounds(); //  ������� ���������� ������� ������
		
		if (carBounds.intersects(heroSprite.getGlobalBounds()) && !isInvulnerable) { //  ���� ��������� ����������� ������ ������ � ��������� �����
			currentVelocityX = 0; // ������� �������� �� oX ����������
			if (currentlyMoving && (currentMove == "Up" || currentMove == "Down") || needsToBeDrivenAway) { //  ���� ����� ������ � ������ ��������� ���� ��� �����
				driveAway(driveDirection, carBounds, carSpeed, car.getCarType()); //  ����������� �������� ���������
				needsToBeDrivenAway = true; //  �������� "����� ��� �������" ��������������� � ������
			}
			else if (driveDirection && heroSprite.getPosition().x < carBounds.left || !driveDirection && heroSprite.getPosition().x > carBounds.left) { //  ����������,
				driveAway(driveDirection, carBounds, carSpeed, car.getCarType()); //  ���� ����� ������ � ������ ����� (������ �)
				needsToBeDrivenAway = true;
			}
			else if (!needsToBeDrivenAway && (!driveDirection && heroSprite.getPosition().x < carBounds.left || driveDirection && heroSprite.getPosition().x > carBounds.left))
				squish(driveDirection, carBounds, carSpeed); //  ���� ����� �������� � ������ ������� (������ ������� ������) - ������������� �������� "�����������" �����
		}
	}

	void squish(bool driveDirection, FloatRect carBounds, float carSpeed) { //  �������, ������������� �������� ����������� ������
		allowGoingThroughWalls = true; //  � ����� ������� ����� ����� �������� �� ������� ������ (������ ������ ��� ����������)
		if (!driveDirection) { //  ����� ������� � ������� ����� ���� � ����������� ������ � ������� ������������
			if (heroSprite.getScale().x < 1.5 / carSpeed) this->moveBy(-carSpeed, 0); //  ������� ������������ ������� �� �������� ������
			heroSprite.scale((carBounds.left - heroSprite.getGlobalBounds().left) / (heroSprite.getScale().x * HERO_WIDTH), 1);
		}
		else {
			if (heroSprite.getScale().x < 1.5 / carSpeed) this->moveBy(carSpeed, 0);
			{
				heroSprite.scale((heroSprite.getGlobalBounds().left + heroSprite.getGlobalBounds().width - (carBounds.left + carBounds.width)) / (heroSprite.getScale().x * HERO_WIDTH), 1);
				heroSprite.setPosition(carBounds.left + carBounds.width + 1, heroSprite.getPosition().y);
			}
		}
		isKilled = true; // �������� "����� ��������" ������������ � ������
	}

	void driveAway(bool driveDirection, FloatRect carBounds, float carSpeed, String carType) { //  �������, ������������� �������� ��������� �����
		currentVelocityY = 0; //  ������� �������� �������� ����� �� oY ���������� � ���� (��� ������ �������� ����� ��������� ���������������)
		if (!isKilled) heroSprite.scale(0.5, 0.5); // ���� �������� ������ �������� - ��������� ������ � 2 ���� �� ������ ������� (����� ����������� �� ������)
		if (!driveDirection) { //  � ����������� �� ���� ������ � ����������� �������� ����� ������������� �� ��� ���, ����� ��������, ��� �� ���� �� ���� ������
			if (carType == "sedan") heroSprite.setPosition(carBounds.left - 10, carBounds.top - 5);
			else if (carType == "truck") heroSprite.setPosition(carBounds.left - 20, carBounds.top + 31);
			else if (carType == "mini") { 
				heroSprite.setPosition(carBounds.left + 92, carBounds.top - 18);
				heroSprite.setRotation(45);
			}
			else if (carType == "drag") heroSprite.setPosition(carBounds.left + 1, carBounds.top - 10);
			else if (carType == "hot_rod") heroSprite.setPosition(carBounds.left + 9, carBounds.top + 11);
			else if (carType == "jeep") heroSprite.setPosition(carBounds.left + 166, carBounds.top + 9);
			else if (carType == "truck_old") heroSprite.setPosition(carBounds.left - 12, carBounds.top + 40);
			else if (carType == "darkworld_car") heroSprite.setPosition(carBounds.left - 6, carBounds.top + 7);
			else if (carType == "convertible") heroSprite.setPosition(carBounds.left + 155, carBounds.top - 2);
		}
		else {
			if (carType == "sedan") heroSprite.setPosition(carBounds.left + 159, carBounds.top - 5);
			else if (carType == "truck") heroSprite.setPosition(carBounds.left + 220, carBounds.top + 31);
			else if (carType == "mini") {
				heroSprite.setPosition(carBounds.left - 33, carBounds.top + 18);
				heroSprite.setRotation(-45);
			}
			else if (carType == "drag") heroSprite.setPosition(carBounds.left + 99, carBounds.top - 10);
			else if (carType == "hot_rod") heroSprite.setPosition(carBounds.left + 130, carBounds.top + 11);
			else if (carType == "jeep") heroSprite.setPosition(carBounds.left - 18, carBounds.top + 9);
			else if (carType == "truck_old") heroSprite.setPosition(carBounds.left + 214, carBounds.top + 40);
			else if (carType == "darkworld_car") heroSprite.setPosition(carBounds.left + 110, carBounds.top + 7);
			else if (carType == "convertible") heroSprite.setPosition(carBounds.left - 5, carBounds.top - 2);
		}
		isKilled = true; //  �������� "����� ��������" ���������� � ������
		allowGoingThroughWalls = true; //  � ����� ������� ����� ����� �������� �� ������� ������ (������ ������ ��� ����������)
	}
};


float minPossibleSpace(int carAmount, int carSize) { //  �������, ������������ ����������� ��������� ���������� ����� �������� �� ������ ��� �������� ���������� ����� � ����������� ������ (��� ����, ����� ����� ���� ��� ������������ �����)
	return (WIDTH + 3 * carSize - carAmount * carSize) / carAmount;
}


Road rebuild_road(float x, float y, int roadChoice) { //  �������, ���������� ������ ������/�����
	Road newRoad;
	newRoad.setTexture(ROAD_TEXTURES[roadChoice]);
	newRoad.setCoords(x, y); //  �������� ����� ��������� ������ Road, ��� ������� ������ � ��������� ������ � ������������ ������ (x, y)
	if (roadChoice != 2) { //  ���� �������� - �� �������� ������
		coordinatesXAvailableForHero.clear(); //  ������� ������ ���������, ��������� ��� ������� �����, ��������� �� ������ ��� ����������� ��������, �������� ��������, ������������� ������ � ��������� ������ ���������� �����
		newRoad.setDriveDirection(rand() % 2); //  �������� ������� ����������� ��������
		int carAmount = 2 + rand() % 4; //  �������� ������� ���������� ����� �� 2 �� 6 (��� ���� �� ����� - �������� � ���� �����, �������� �� �������)
		Car randomCar; //  �������� ������ ������ Car, ����������� ������ �� ��������� ��������� �� ������� ������� ��� ����� �������� � ��� ��������
		int carTextureIndex = rand() % CAR_TEXTURES.size();
		Texture randomTexture = CAR_TEXTURES[carTextureIndex];
		int rX = randomTexture.getSize().x;
		int rY = randomTexture.getSize().y;
		randomCar.setSize(0, 0, rX, rY);
		randomCar.setTexture(CAR_TEXTURES[carTextureIndex]);
		if (carTextureIndex < 104) randomCar.setCarType("sedan"); //  � ����������� �� ������� ��������, ������ ������������� ����������� ��� (������ ������ ���� �����������)
		else if (carTextureIndex >= 104 && carTextureIndex < 167) randomCar.setCarType("truck");
		else if (carTextureIndex >= 167 && carTextureIndex < 230) randomCar.setCarType("mini");
		else if (carTextureIndex >= 230 && carTextureIndex < 287) randomCar.setCarType("hot_rod");
		else if (carTextureIndex >= 287 && carTextureIndex < 347) randomCar.setCarType("jeep");
		else if (carTextureIndex >= 347 && carTextureIndex < 350) randomCar.setCarType("truck_old");
		else if (carTextureIndex >= 350 && carTextureIndex < 356) randomCar.setCarType("darkworld_car");
		else if (carTextureIndex >= 356 && carTextureIndex < 433) randomCar.setCarType("convertible");
		else if (carTextureIndex >= 433 && carTextureIndex < 490) randomCar.setCarType("drag");
		if (newRoad.getDriveDirection()) randomCar.setScale(-1, 1); //  ���� �������� �� ������ �������������� ������ - ������ ������ ���������������
		float carSpace = minPossibleSpace(carAmount, rX); //  ����������� ���������� ��������� ���������� ����� �������� � ������ ������� �����
		while (carSpace < 2.5 * HERO_WIDTH) { //  ���� ���������� ������ ���� ���� ����� - ����������� ���������� ����� � �������� ���������� ���������
			carAmount--;
			carSpace = minPossibleSpace(carAmount, rX);
		}
		vector<Car> inputCars;
		float randS = rand() % 100 + 0.1f; //  ����������� ��������� ���������� �� ������ ������ �� ����� ������ (����� �������� ����������� �������� �����)
		for (int i = 0; i < carAmount; i++) { //  � ��������� ������ inputCars ����� ����������� ������ ������ Car (randomCar) ��������� (carAmount) ���
			inputCars.push_back(randomCar); // ��� ������ "�����" ������ �������� ���������� � ������������ � ������������ ��������, �� ������ ������ ������ ������������
			if (newRoad.getDriveDirection()) inputCars[i].setCoords((carSpace + rX) * i + randS, y + (ROAD_SEGMENT_HEIGHT - randomTexture.getSize().y) / 2);
			else inputCars[i].setCoords(WIDTH - (carSpace + rX) * i - randS, y + (ROAD_SEGMENT_HEIGHT - randomTexture.getSize().y) / 2);
		}
		newRoad.setCars(inputCars); //  ���������� �������-������ ��������� ������ �����
		newRoad.setSpeed(MIN_CAR_SPEED + static_cast <float> (rand()) / static_cast <float> (RAND_MAX / (MAX_CAR_SPEED - MIN_CAR_SPEED))); //  ����� ��� ������������� �������� �������� ��������
		newRoad.setNotPark(true); //  � �������� "������ ������ �� �������� �������" ������������ � ��������
	}

	else { //  ���� �������� - �����
		vector<Sprite> newNature; //  �������� ������ ��������������
		if (coordinatesXAvailableForHero.size() == 0) { //  ���� �������� ��� ������ ������ (������ coordinatesXAvailableForHero ������), �� �� ������ ��� ������ ������� ������ ����� ������ �
			for (int z = 0; z < 10; z++) {	//  ����������, ����� �� ��� �������� ����� ����������� TREE_PROBABILITY
				Sprite randomNature;
				randomNature.setTexture(NATURE_ELEMENTS_TEXTURES[rand() % 1]);
				randomNature.setPosition(z * 100.f, y - 50.f);
				if (rand() % 100 <= TREE_PROBABILITY) newNature.push_back(randomNature);
				else coordinatesXAvailableForHero.push_back(z * 100.f);
			}
			if (newNature.size() == 10) {
				int neededToBeDeleted = rand() % 10;
				newNature.erase(newNature.begin() + neededToBeDeleted);
				coordinatesXAvailableForHero.push_back(neededToBeDeleted * 100.f);
			}
		}
		else {
			for (int z = 0; z < 10; z++) { //  ���� �� ������ �� ������, �� �� ������������� �������� ��������� ���������� �� ������� (���� �� ����� ������ 1, ����� �� ��������� ����� ��� �������)
				Sprite randomNature;
				randomNature.setTexture(NATURE_ELEMENTS_TEXTURES[rand() % 1]);
				randomNature.setPosition(z * 100.f, y - 50.f);
				int approvedInd = rand() % (coordinatesXAvailableForHero.size());
				float approvedCoord = coordinatesXAvailableForHero[approvedInd];
				if (coordinatesXAvailableForHero.size() == 1) approvedCoord = -1;
				vector<float> ::iterator it = find(coordinatesXAvailableForHero.begin(), coordinatesXAvailableForHero.end(), 100 * z);
				if (rand() % 100 <= TREE_PROBABILITY && (it == coordinatesXAvailableForHero.end() || z * 100 == approvedCoord))  // � ������������� ��������� �������� �� ��, ��� �� ���������� �� ��������� � ������� (�� ����������� ���������)
					newNature.push_back(randomNature);
				if (approvedCoord != -1) coordinatesXAvailableForHero.erase(coordinatesXAvailableForHero.begin() + approvedInd); // � ��������� ���������� ������� �� �������
			}
		}
		if (newNature.size() == 0) coordinatesXAvailableForHero.clear();
		newRoad.setNatureElements(newNature); //  ���������� ����� � ������� ������� Road ��������� ������ ��������������
		newRoad.setNotPark(false); //  �������� "������ ������ �� �������� �������" ��������������� � ����
	}

	vector<vector<int>> newSuperPowers;
	for (int i = 0; i < 10; i++) {
		bool canPlaceSuperPower = true;
		for (int j = 0; j < newRoad.getNatureElements().size(); j++)
			if (i * 100 == newRoad.getNatureElements()[j].getPosition().x) canPlaceSuperPower = false;
		if (!canPlaceSuperPower) continue;
		if (rand() % 1000 <= SUPERPOWERS_PROBABILITY) {
			vector<int> randomSuperPower = { rand() % 2, i * 100 };
			newSuperPowers.push_back(randomSuperPower);
		}
	}

	newRoad.setSuperPowers(newSuperPowers);

	return newRoad; //  ������� ���������� ����� ������ � ������ �����������
}


void loadCars(string carName, int amount) {
	string carNameAddress;
	Texture car;
	for (int i = 0; i < amount; i++) {
		carNameAddress = "objects\\sprites\\" + carName + "\\" + carName[0] + to_string(i + 1) + ".png";
		car.loadFromFile(carNameAddress);
		CAR_TEXTURES.push_back(car);
	}
}


bool gameSession(RenderWindow& window, long currentRecord) {

	bool canStart = false;
	bool isPaused = false;
	bool readingRules = false;

	int deathTextColorAplha = 0;

	long scoreCountInReality = 0;

	time_t invulnerabilityStartMoment = time(0);
	time_t slowingTimeStartMoment = time(0);
	time_t timeAfterSuperPowerStart = time(0);

	View camera; //  ���������������� ������, ��������� �� �������, � ��������� ���������
	camera.reset(FloatRect(0, 0, WIDTH, HEIGHT));
	float catchingUpSpeed = 0; //  ���������������� ����������, ���������� � ����������� �� ��������� ������ (�������������, ���� ����� ������ ������ ����� �� ������)
	float currentCameraSpeed = CAMERA_MOVE_SPEED; //  ���������������� ������� �������� ������, �� ��������� �� �������� ������ (������ �� ����� �������� �����, �� ����� ������ ������ �� �����)

	Hero turtle = Hero(); //  �������� ������ �����, ������� ����� ������������ � ����� ������ ����� ������ (������)
	turtle.setPosition(WIDTH / 2, HEIGHT - (ROAD_SEGMENT_HEIGHT + HERO_HEIGHT) / 2);

	Road current_roads[9]; //  �������� ������, � ������� �������� 9 �������� ������ Road, ���������� ������ ���� �� ������ �� ������
	for (int k = 0; k < 9; k++) {
		int roadChoice = rand() % 3; //  ��� ������� ������� ���������� �������� ������,
		if (k == 0) {
			roadChoice = 2; // ������ ������ ������ (������ �������� �����) ������ ������ ���� �������
			coordinatesXAvailableForHero.push_back(500);
		}
		current_roads[k] = rebuild_road(0, HEIGHT - (ROAD_SEGMENT_HEIGHT * (k + 1)), roadChoice); // ����� ������� "���������������" � ������������ � ������������ ������, ��������� ��������� � �������� ���������, �� ������� ����� ����������� ������� (���� ������ � ���������� ������ ���� ��������)
	}
	for (int cur = 0; cur < current_roads[0].getSuperPowers().size(); cur++) current_roads[0].makeSuperPowerUnaccessable(cur);
	current_roads[0].eraseUnaccessableSuperPowers();

	for (int s = 0; s < current_roads[0].getNatureElements().size(); s++) { // ��� ������ ������ (�.�. ������), ���� �� ����������� ��������� ����� ����� ������ - ��� ������ ���������
		if (current_roads[0].getNatureElements()[s].getPosition().x == WIDTH / 2) current_roads[0].eraseNatureElement(s);
	}

	float newRoadY = HEIGHT - 10 * ROAD_SEGMENT_HEIGHT; // ����������� ����������, ������������ ���������� ��� oY ��� ��������� ���������� ������������ ������

	while (window.isOpen()) //  ���� �������� ���������
	{

		Event event;
		while (window.pollEvent(event))
		{

			if (event.type == Event::Closed)
				window.close();
			if (event.type == Event::KeyPressed && event.key.code == Keyboard::Enter && turtle.getIsKilled()) return true;
			if (event.type == Event::KeyPressed && event.key.code == Keyboard::Enter && !turtle.getIsKilled() && !readingRules) canStart = true;
			if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape && !turtle.getIsKilled() && canStart) {
				isPaused = 1 - isPaused;
				if (isPaused) {
					Text pauseText(L"�����", FONT, 72U);
					Text additionalPauseText1(L"������� ESC, ����� ����������", FONT, 36U);
					Text additionalPauseText2(L"��� Backspace, ����� �����", FONT, 36U);

					pauseText.setPosition(0, camera.getCenter().y - camera.getSize().y / 2);
					additionalPauseText1.setPosition(0, camera.getCenter().y - camera.getSize().y / 2 + LINE_SPACING * pauseText.getLocalBounds().height);
					additionalPauseText2.setPosition(0, additionalPauseText1.getPosition().y + LINE_SPACING * additionalPauseText1.getLocalBounds().height);

					window.draw(pauseText);
					window.draw(additionalPauseText1);
					window.draw(additionalPauseText2);
					window.display();
				}
			}
			if (event.type == Event::KeyPressed && event.key.code == Keyboard::Backspace && (isPaused || !canStart || turtle.getIsKilled()) && !readingRules) return false;
			if (event.type == Event::KeyPressed && event.key.code == Keyboard::Num3 && !canStart) readingRules = 1 - readingRules;
			if (!isPaused && canStart) scoreCountInReality = turtle.update(event, scoreCountInReality); //  ����������� ��������� ������ � ������������ � ���������
		}

		if (!isPaused) {
			turtle.move(); //  ���������� �������� �����, ���� �� ����� �����-���� ������� ��������

			if (((turtle.getCoords().x > WIDTH - HERO_WIDTH || turtle.getCoords().x < 0) && !turtle.getAllowGoingThroughWalls()) && !turtle.getAllowGoingThroughWalls()) turtle.abortMove(); //  ���� ����� �������� ����� �� ������� ������ ����� ��� ������ (����� ������ ���������, ����� ��� ������� ������) - ������������ ��� ��������
			if (turtle.getCoords().y <= camera.getCenter().y) catchingUpSpeed = (camera.getCenter().y - turtle.getCoords().y) / 8; //  ���� ����� �������� ������������ � �������� ���� ������ - ������������� �� ����� �������� �������� ������
			else catchingUpSpeed = 0; // ����� ��� ���������� � ����

			if (turtle.getCoords().y > camera.getCenter().y + camera.getSize().y / 2) turtle.setIsKilled(true);

			if (turtle.getIsKilled() && currentCameraSpeed < DEATH_CAMERA_MOVE_SPEED) currentCameraSpeed = currentCameraSpeed * 1.05f; //  ���� ����� ����������� - ������� �������� ������ ���������� �������� �� �������� ����� ���������
			if (canStart) camera.move(0, -max(catchingUpSpeed, currentCameraSpeed)); //  ������ �������� ����� � ���������� ��������� ���������
			window.setView(camera); //  ��� ������ �����������
			window.clear(); //  ���� ��������� ��������� �� ���� ��������, ����� ���������� �����

			for (int k = 8; k > -1; k--) {
				window.draw(current_roads[k].getSprite()); //  ������ ���������� �������������� ��� �������: ������, ��������������, �����, ������
				for (int j = 0; j < current_roads[k].getSuperPowers().size(); j++) {
					if (current_roads[k].superPowerCanBeAccessed(j)) {
						Texture superPowerTexture(SUPERPOWER_ELEMENTS_TEXTURES[current_roads[k].getSuperPowers()[j][0]]);
						Sprite superPower(superPowerTexture);
						superPower.setPosition(current_roads[k].getSuperPowers()[j][1] + superPowerTexture.getSize().x / 2, current_roads[k].getCoords().y + superPowerTexture.getSize().y / 2);
						window.draw(superPower);

						if (superPower.getGlobalBounds().intersects(turtle.getSprite().getGlobalBounds()) && !turtle.getIsKilled()) {
							if (current_roads[k].getSuperPowers()[j][0] == 0) {
								turtle.setIsInvulnerable(true);
								invulnerabilityStartMoment = time(0);
							}
							else {
								turtle.setIsSlowingTime(true);
								slowingTimeStartMoment = time(0);
							}
							current_roads[k].makeSuperPowerUnaccessable(j);
						}
					}
				}
				if (!current_roads[k].getNotGrass()) {
					for (Sprite natureType : current_roads[k].getNatureElements()) { // ��� ������� ������ ��������� ���������������� ������� (�.�. ������ ������ ������ ������ ������ ������, ��� ������� �� ��� �������, ������ ��� �� ������ ������ �� �������� ������ �� ������ ����, ��������� ������ �� ������������ �� ���. �������������, ����������� ������ �� �����, ������� � 2� ��������� � �������� ������ ������)
						FloatRect modBounds = { natureType.getGlobalBounds().left, natureType.getGlobalBounds().top + 25, natureType.getGlobalBounds().width, natureType.getGlobalBounds().height - 25 };
						if (modBounds.intersects(turtle.getSprite().getGlobalBounds()) && !turtle.getIsInvulnerable()) turtle.abortMove(); //  ���� ����� ����� ������ ������ ������ - ��� �������� ������������
					}
				}
			}

			if (turtle.getCoords().x > -100) window.draw(turtle.getSprite()); //  ���� ����� ��������� � �������� ������ (��� �� ���������� ������) - �� ��������������


			for (int k = 8; k > -1; k--) {
				current_roads[k].eraseUnaccessableSuperPowers();
				if (current_roads[k].getNotGrass()) {
					for (int j = 0; j < current_roads[k].getCars().size(); j++) {
						window.draw(current_roads[k].getCars()[j].getSprite()); //  ��������� ������ ������ ������, ���� ������ - �� �����

						current_roads[k].updateCarsPosition(j, turtle.getIsSlowingTime()); //  ���������� ��������� ������ �� ���� ������

						turtle.checkDeadlyIntersection(current_roads[k].getCars()[j], current_roads[k].getSpeed(), current_roads[k].getDriveDirection());  // �������� �� ��, ��� ����� �� ��������� �� ������ ������

					}
				}

				else for (Sprite natureType : current_roads[k].getNatureElements()) window.draw(natureType); // ��������� ���� ��������������, ���� ������ - �����

				if (current_roads[k].getCoords().y > camera.getCenter().y + HEIGHT / 2 + 50) { // ���� ������ ����� �� ������� ������ (�����)
					for (int m = 0; m < 8; m++) current_roads[m] = current_roads[m + 1]; // �� ��� ������������� �� ������� (FIFO)
					int roadChoice = rand() % 3; //  �� ��������� ����� ����� ����� ������,
					if (current_roads[7].getNotGrass() == false && roadChoice == 2)
						current_roads[8] = rebuild_road(0, newRoadY, 2);
					else current_roads[8] = rebuild_road(0, newRoadY, roadChoice);
					newRoadY -= ROAD_SEGMENT_HEIGHT; //  � ���������� �� Y ��� ����� ������ �����������
				}

			}

			if (scoreCountInReality > newRecord) newRecord = scoreCountInReality;
			if (turtle.getIsInvulnerable() || turtle.getIsSlowingTime()) {
				timeAfterSuperPowerStart = time(0);
				if (turtle.getIsInvulnerable() && turtle.getIsSlowingTime()) {
					Text timeLeftText1(L"������������: " + to_string(8 - (timeAfterSuperPowerStart - invulnerabilityStartMoment)), FONT, 24U);
					Text timeLeftText2(L"���������� �������: " + to_string(8 - (timeAfterSuperPowerStart - slowingTimeStartMoment)), FONT, 24U);
					timeLeftText1.setPosition(camera.getCenter().x + camera.getSize().x / 2 - timeLeftText1.getLocalBounds().width - 5, camera.getCenter().y - camera.getSize().y / 2 + 30);
					timeLeftText2.setPosition(camera.getCenter().x + camera.getSize().x / 2 - timeLeftText2.getLocalBounds().width - 5, timeLeftText1.getPosition().y + LINE_SPACING * timeLeftText1.getLocalBounds().height);
					window.draw(timeLeftText1);
					window.draw(timeLeftText2);
				}
				else {
					Text timeLeftText(L"������������: " + to_string(8 - (timeAfterSuperPowerStart - invulnerabilityStartMoment)), FONT, 24U);
					if (turtle.getIsSlowingTime()) timeLeftText.setString(L"���������� �������: " + to_string(8 - (timeAfterSuperPowerStart - slowingTimeStartMoment)));
					timeLeftText.setPosition(camera.getCenter().x + camera.getSize().x / 2 - timeLeftText.getLocalBounds().width - 5, camera.getCenter().y - camera.getSize().y / 2 + 30);
					window.draw(timeLeftText);
				}
			}

			if (8 - (timeAfterSuperPowerStart - invulnerabilityStartMoment) < 0) turtle.setIsInvulnerable(false);
			if (8 - (timeAfterSuperPowerStart - slowingTimeStartMoment) < 0) turtle.setIsSlowingTime(false);

			if (canStart) {
				Text scoreText(L"����: " + to_string(newRecord), FONT, 24U);
				scoreText.setPosition(camera.getCenter().x + camera.getSize().x / 2 - scoreText.getLocalBounds().width - 5, camera.getCenter().y - camera.getSize().y / 2);
				window.draw(scoreText);
			}

			if (readingRules) {
				float minusY = 0;
				for (int i = 0; i < 13; i++) minusY += rulesTexts[i].getCharacterSize() * LINE_SPACING;

				rulesTexts[0].setPosition(camera.getCenter().x - rulesTexts[0].getLocalBounds().width / 2, camera.getCenter().y - minusY / 2);
				for (int i = 0; i < 13; i++) {
					if (i != 0) rulesTexts[i].setPosition(camera.getCenter().x - rulesTexts[i].getLocalBounds().width / 2,
						rulesTexts[i - 1].getPosition().y + LINE_SPACING * rulesTexts[i - 1].getLocalBounds().height);
					window.draw(rulesTexts[i]);
				}
			}

			if (!canStart && !readingRules) {
				mainText1.setPosition(WIDTH / 2 - mainText1.getLocalBounds().width / 2,
					HEIGHT / 2 - LINE_SPACING * (mainText1.getLocalBounds().height + mainText2.getLocalBounds().height + additionalStartText1.getLocalBounds().height + additionalStartText2.getLocalBounds().height) / 2);
				mainText2.setPosition(WIDTH / 2 - mainText2.getLocalBounds().width / 2, mainText1.getPosition().y + mainText1.getCharacterSize());
				additionalStartText1.setPosition(WIDTH / 2 - additionalStartText1.getLocalBounds().width / 2, mainText2.getPosition().y + LINE_SPACING * mainText2.getLocalBounds().height);
				additionalStartText2.setPosition(WIDTH / 2 - additionalStartText2.getLocalBounds().width / 2, additionalStartText1.getPosition().y + LINE_SPACING * additionalStartText1.getLocalBounds().height);
				additionalStartText3.setPosition(WIDTH / 2 - additionalStartText3.getLocalBounds().width / 2, additionalStartText2.getPosition().y + LINE_SPACING * additionalStartText2.getLocalBounds().height);
				window.draw(mainText1); window.draw(mainText2); window.draw(additionalStartText1); window.draw(additionalStartText2); window.draw(additionalStartText3);
			}

			if (turtle.getIsKilled()) {
				turtle.setIsSlowingTime(false);
				turtle.setIsInvulnerable(false);
				Text gameOverTexts[5] = { Text(L"���� ��������", FONT, 72U),  Text(L"��� ����: " + to_string(newRecord), FONT, 48U),
										  Text(L"����������� ������: " + to_string(currentRecord), FONT, 48U), Text(L"������� Enter, ����� ������ �������", FONT, 24U),
										  Text(L"��� Backspace, ����� �����", FONT, 24U) };
				if (newRecord > currentRecord) gameOverTexts[2] = Text(L"� ��� ����� ������!", FONT, 48U);

				Color textColor = gameOverTexts[0].getFillColor();
				if (deathTextColorAplha != 255 && currentCameraSpeed >= DEATH_CAMERA_MOVE_SPEED) deathTextColorAplha += 1;
				textColor.a = deathTextColorAplha;

				gameOverTexts[0].setPosition(camera.getCenter().x - camera.getSize().x / 2, camera.getCenter().y - camera.getSize().y / 2);
				for (int i = 0; i < 5; i++) {
					gameOverTexts[i].setFillColor(textColor);
					if (i != 0) gameOverTexts[i].setPosition(gameOverTexts[i - 1].getPosition().x, gameOverTexts[i - 1].getPosition().y + LINE_SPACING * gameOverTexts[i - 1].getLocalBounds().height);
					window.draw(gameOverTexts[i]);
				}
			}

			window.display();
		}
	}
	return true;
}


int main()
{
    srand(unsigned int(time(NULL))); //  ����������� ������ ��������� �������� �������� ��������� ����� (�.�. �� ��, ������� ���� ������� ������� ���������� � ������������ � ��� ��� ���������, �� ���������)
	
    ContextSettings settings;
    settings.antialiasingLevel = 8; //  ��������� �������������� �������������� ��������

	RenderWindow window(VideoMode(unsigned int(WIDTH), unsigned int(HEIGHT)), "Totally Not A Crossy Road", Style::Close); //  �������� ���� � ��������� ��� ����������� �������� ������ ������
	window.setKeyRepeatEnabled(false); //  ����������� ���������� ������������ ������ (����� ����� �� "�����" �� �����)
	window.setFramerateLimit(FRAME_LIMIT); //  ��������������� ����������� �� ������ � �������

    Texture Lane; Lane.loadFromFile("objects\\sprites\\misc\\road.png"); //  �������� �������� �����, ������� ����� ���������� � ������ ������� �����
    Texture Park; Park.loadFromFile("objects\\sprites\\misc\\park.png");

    ROAD_TEXTURES[0] = Lane; ROAD_TEXTURES[1] = Lane; ROAD_TEXTURES[2] = Park;

	loadCars("sedan", 104);
	loadCars("truck", 63);
	loadCars("mini", 63);
	loadCars("hot_rod", 57);
	loadCars("jeep", 60);
	loadCars("truck_old", 3);
	loadCars("darkworld_car", 6);
	loadCars("convertible", 77);
	loadCars("drag", 57);

	Texture Nature1; Nature1.loadFromFile("objects\\sprites\\misc\\tree.png"); //  �������� �������� ��������������, ������� ����� ���������� � ������ ������� ��������������

	NATURE_ELEMENTS_TEXTURES[0] = Nature1;

	Texture superPower1; superPower1.loadFromFile("objects\\sprites\\misc\\invulnerability.png");
	Texture superPower2; superPower2.loadFromFile("objects\\sprites\\misc\\slowingtime.png");

	SUPERPOWER_ELEMENTS_TEXTURES[0] = superPower1; SUPERPOWER_ELEMENTS_TEXTURES[1] = superPower2;

	FONT.loadFromFile("objects\\fonts\\calibri.ttf");

	
	ifstream recordRead("objects\\data\\record.txt");
	int previousRecord = 0;
	string recordString;
	int currentRecord = 0;

	if (recordRead.peek() != EOF) {
		getline(recordRead, recordString);
		currentRecord = stoi(recordString);
	}
	else {
		ofstream recordInput("objects\\data\\record.txt");
		recordInput.close();
	}
	recordRead.close();

	bool stillRunning = true;

	while (stillRunning) {
		stillRunning = gameSession(window, currentRecord);

		if (newRecord > currentRecord) {
			ofstream recordInput("objects\\data\\record.txt");
			recordInput << newRecord;
			recordInput.close();
			currentRecord = newRecord;
		}

		newRecord = 0;
	}

    return 0;
}