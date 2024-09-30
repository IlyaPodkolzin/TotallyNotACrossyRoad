#include <SFML/Graphics.hpp>
#include <vector>
#include <algorithm>
#include <ctime>
#include <iostream>
#include <fstream>
#include <string>
using namespace sf;
using namespace std;


float WIDTH = 1000; // ширина экрана
float HEIGHT = 750; // высота экрана
short HERO_JUMP_VELOCITY = 40; // начальная скорость прыжка героя (в любую сторону)
short HERO_HEIGHT = 50; //  высота спрайта героя
short HERO_WIDTH = 100; //  ширина спрайта героя
short ROAD_SEGMENT_HEIGHT = 100; //  высота одной линии дороги/газона
short FRAME_LIMIT = 60; //  ограничение ФПС
short TREE_PROBABILITY = 40; //  вероятность появления дерева на каждой секции линии газона
float CAMERA_MOVE_SPEED = 0.1f; //  скорость движения камеры во время игры
float DEATH_CAMERA_MOVE_SPEED = 1.5f; //  скорость движения камеры после проигрыша
float MIN_CAR_SPEED = 2; //  минимальная скорость машины
float MAX_CAR_SPEED = 4.5; //  максимальная скорость машины
Texture ROAD_TEXTURES[3]; //  массив текстур для линий дороги
vector<Texture> CAR_TEXTURES; //  массив текстур для машин
Texture NATURE_ELEMENTS_TEXTURES[1]; //  массив текстур для растительности
Texture SUPERPOWER_ELEMENTS_TEXTURES[2];
Font FONT;
float LINE_SPACING = 1.5f;
int SUPERPOWERS_PROBABILITY = 2;
int INVULNERABILITY_TIME = 8;
float SLOWING_RATE = 0.5f;
vector<float> coordinatesXAvailableForHero; //  массив координат по X, которые доступны для прохода героя (во избежание ловушек)
long newRecord = 0;
Text rulesTexts[13] = { Text(L"Правила игры:", FONT, 72U), Text(L"Необходимо преодолеть как можно большее расстояние,", FONT, 24U),
									 Text(L"не попав под машину (или на неё).", FONT, 24U), Text(L"Кроме того, нужно оставаться в пределах камеры", FONT, 24U),
									 Text(L"и не позволить ей догнать вас.", FONT, 24U), Text(L"Деревья преграждают дальнейшее движение.", FONT, 24U),
									 Text(L"Суперспособности:", FONT, 36U), Text(L"Гроза - неуязвимость - позволяет проходить через объекты;", FONT, 24U),
									 Text(L"Часы - замедление времени - замедляет скорость движения машин в 2 раза.", FONT, 24U),
									 Text(L"Управление:", FONT, 36U), Text(L"Стрелки - двигаться", FONT, 24U), Text(L"WASD - просто повернуться", FONT, 24U),
									 Text(L"Нажмите NUM3, чтобы выйти из Правил", FONT, 24U) };
Text mainText1("Totally Not", FONT, 96U);
Text mainText2("A Crossy Road", FONT, 96U);
Text additionalStartText1(L"Нажмите Enter, чтобы начать", FONT, 24U);
Text additionalStartText2(L"или Backspace, чтобы выйти", FONT, 24U);
Text additionalStartText3(L"или NUM3, чтобы прочитать правила", FONT, 24U);



class Car { // Класс предназначени для создания машин, едущих по полосам дороги и взаимодействующих с героем
private:
	Sprite carImage; //  спрайт машины
	String carType = "None"; //  тип автомобиля (грузовик, седан...) - на момент создания неизвестен
public:
	Car() {
		Texture texture; 
		texture.loadFromFile("objects\\sprites\\misc\\placeholder.png"); //  загружаем текстуру для машины
		carImage.setTexture(texture); //  добавляем в спрайт текстуру
	}

	void moveBy(float xDiff, float yDiff) { //  функция, передвигающая спрайт на xDiff вправо и yDiff вниз
		carImage.move(xDiff, yDiff);
	}

	Sprite getSprite() { //  функция, возвращающая спрайт машины
		return carImage;
	}

	Vector2f getCoords() { //  функция, возвращающая координаты машины
		return carImage.getPosition();
	}

	vector<float> getSize() {  //  функция, возвращающая размеры спрайта машины
		return { carImage.getLocalBounds().width, carImage.getLocalBounds().height };
	}
	
	String getCarType() { //  функция, возвращающая тип машины
		return carType;
	}

	void setTexture(Texture &texture) { //  функция, возвращающая спрайт машины
		carImage.setTexture(texture);
	}

	void setCoords(float x, float y) { //  функция, задающая спрайту машины координаты
		carImage.setPosition(x, y);
	}

	void setSize(int startX, int startY, int sizeX, int sizeY) { //  функция, задающая спрайту машины размер
		carImage.setTextureRect(IntRect(startX, startY, sizeX, sizeY));
	}

	void setScale(float scaleX, float scaleY) { //  функция, задающая спрайту машины масштаб (меньше 1 - уменьшение, больше 1 - увеличение) для 2 измерений
		carImage.scale(scaleX, scaleY);
	}

	void setCarType(String carType) { //  функция, задающая тип машины
		this->carType = carType;
	}
};


class Road { //  Класс предназначен для создания и отрисовывания полос дороги или газона вместе с машинами, едущим по ним, или растительностью
private:
	Sprite roadImage; //  спрайт дороги
	vector<Car> cars; //  массив машин, едущих по полосе (если это не газон)
	vector<Sprite> natureElements; //  массив деревьев, если это газон
	vector<vector<int>> superPowers;
	bool driveDirection = true; //  направление движения: true - направо, false - налево
	bool notPark = true; //  булево значение, отражающее, является ли дорога газоно (по умолчанию нет)
	float driveSpeed = 0; //  скорость движения машин
public:
	Road() {
		Texture placeholder;
		placeholder = ROAD_TEXTURES[rand() % 3]; //  выбирается текстура дороги из массива текстур для дорог и газона
		roadImage.setTexture(placeholder); //  текстура вставляется в спрайт
	}

	void moveBy(float xDiff, float yDiff) { //  функция, передвигающая спрайт полосы на xDiff влево и yDiff вниз
		roadImage.move(xDiff, yDiff);
	}

	void setTexture(Texture &newTexture) { // функция, задающая текстуру спрайта дороги
		roadImage.setTexture(newTexture);
	}

	void setCoords(float x, float y) { //  функция, задающая координаты спрайта дороги
		roadImage.setPosition(x, y);
	}

	void setCars(vector<Car> cars) { //  функция, задающая массив машин, едущих по дороге
		for (int i = 0; i < cars.size(); i++) this->cars.push_back(cars[i]);
	}

	void setNatureElements(vector<Sprite> natureElements) { //  функция, задающая массив растительности, находящейся на газоне
		for (int i = 0; i < natureElements.size(); i++) this->natureElements.push_back(natureElements[i]);
	}

	void setSuperPowers(vector<vector<int>> superPowers) { //  функция, задающая массив суперспособностей
		for (int i = 0; i < superPowers.size(); i++) this->superPowers.push_back(superPowers[i]);
	}

	void setDriveDirection(bool driveDirection) { //  функция, задающая направление движения
		this->driveDirection = driveDirection;
	}

	void setSpeed(float driveSpeed) { //  функция, задающая скорость движения машин по полосе
		this->driveSpeed = driveSpeed;
	}

	void setNotPark(bool notPark) { //  функция, задающая тип дороги (газон - если false, полоса движения - если true)
		this->notPark = notPark;
	}

	Sprite getSprite() { //  функция, возвращающая спрайт дороги
		return roadImage;
	}

	Vector2f getCoords() { //  функция, возвращающая координаты дороги
		return roadImage.getPosition();
	}

	vector<Car> getCars() { //  функция, возвращающая массив машин, едущих по дороге
		return cars;
	}

	bool getDriveDirection() { //  функция, возвращающая булево значение, определяющее направление движения по дороге
		return driveDirection;
	}

	float getSpeed() { //  функция, возвращающая скорость движения по дороге
		return driveSpeed;
	}

	bool getNotGrass() { // функция, возвращающая булево значение, определяющее, является ли дорога газоном или полосой движения
		return notPark;
	}

	vector<Sprite> getNatureElements() { //  функция, возвращающая массив растительности, находящейся на газоне
		return natureElements;
	}

	vector<vector<int>> getSuperPowers() { //  функция, возвращающая массив суперспособностей
		return superPowers;
	}

	void eraseNatureElement(int position) { //  функция, убирающая элемент растительности с индексом position из массива растительности
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

	void updateCarsPosition(int carIndex, bool isSlowedDown) { //  функция, обновляющая положение машин на полосе (постепенное движение вправо/влево)
		float actualSpeed = driveSpeed;
		if (isSlowedDown) actualSpeed *= SLOWING_RATE;
		if (driveDirection) cars[carIndex].moveBy(actualSpeed, 0);
		else cars[carIndex].moveBy(-actualSpeed, 0);
		respawnIfWentAway(carIndex);
	}

	void respawnIfWentAway(int carIndex) { //  функция, контролирующая, чтобы машины не уходили за пределы камеры
		float currX = cars[carIndex].getCoords().x;
		float carSize = cars[carIndex].getSize()[0];
		if (currX <= -carSize * 3 || currX >= WIDTH + carSize * 3) { //  если машина вышла за границы камеры на две свои длины
			if (driveDirection) cars[carIndex].moveBy(-carSize * 3 - WIDTH, 0); //  то машина вновь появляется с противоположной сторны
			else cars[carIndex].moveBy(carSize * 3 + WIDTH, 0);
		}
	}
};


class Hero { //  функция, предназначенная для создания и отрисовывания героя и изменения его положения/состояния в зависимости от ситуации
private:
	Sprite heroSprite; //  спрайт героя
	Texture heroTexture; //  текстура для спрайта героя
	float velocity = HERO_JUMP_VELOCITY; //  начальная скорость прыжка героя - задаётся константой

	float currentVelocityX = 0; //  текущая скорость движения в координатах Ox (для имитации замедления после очередного прыжка)
	float currentVelocityY = 0; //  текущая скорость движения в координатах Oy (аналогично)

	String currentMove = "Down"; //  текущее движение игрока (направление движения) - по умолчанию вниз
	bool currentlyMoving = false; //  булево значение, определяющая, движется ли сейчас игрок - по умолчанию нет
	bool allowGoingThroughWalls = false; //  булево значение, определяющее, можно ли герою выходить за пределы камеры (только после смерти) - по умолчанию нет

	bool needsToBeDrivenAway = false; //  булево значение, определяющее, производится ли анимация "похищения" героя - по умолчанию нет
	bool isKilled = false; //  булево значение, определяющее, окончена ли игра - по умолчанию нет

	bool isInvulnerable = false;
	bool isSlowingTime = false;
public:
	Hero() {
		heroTexture.loadFromFile("objects\\sprites\\misc\\hero.png");
		heroSprite.setTexture(heroTexture); //  загружаем текстуру героя и всатвляем её в спрайт
		heroSprite.setTextureRect(IntRect(101, 0, HERO_WIDTH, HERO_HEIGHT)); //  задаём вырез текстуры, который нужно отобразить на спрайте (герой смотрит вправо)
	}

	void setTextureRect(int startX, int startY, int width, int height) { //  функция, задающая вырез текстуры, который нужно отображать на спрайте (для имитации поворота героя в нужную сторону),
		heroSprite.setTextureRect(IntRect(startX, startY, width, height)); //  начинаяющийся в точке (startX, startY) размером width, height
	}

	void setMove(String move) { //  функция, задающая направление движения игрока в данный момент
		currentMove = move;
	}


	void setPosition(float x, float y) { //  функция, задающая позицию спрайта героя
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

	void move() { //  функция, проигрывающая анимацию движения игрока
		heroSprite.move(currentVelocityX, currentVelocityY); // каждый кадр игрок передвигается в том или ином направлении с текущими скоростями по oX и oY (если они не равны 0)
		
		if (currentVelocityY > 0) currentVelocityY -= 10;  // каждый кадр скорость передвижения уменьшается - создаётся эффект замедления
		else if (currentVelocityY < 0) currentVelocityY += 10;

		if (currentVelocityX > 0) currentVelocityX -= 10;
		else if (currentVelocityX < 0) currentVelocityX += 10;

		if (currentVelocityX == 0 && currentVelocityY == 0) currentlyMoving = false;  // если игрок не движется - currentlyMoving получает значение false (движения в текущий момент времени нет)
	}

	void moveBy(float x, float y) { //  функция, передвигающая игрока по координатной оси по на (x, y)
		heroSprite.move(x, y);
	}


	int update(Event event, int stepCount) { //  функция, обновляющее состояние героя в соответствии с событиями
		if (event.type == Event::KeyPressed && !currentlyMoving && !isKilled) { // если нажата клавиша и игрок не проиграл
			if (event.key.code == Keyboard::Left) { //  если клавиша - стрелка влево, спрайт персонажа обновляется,
				heroSprite.setTextureRect(IntRect(0, 0, HERO_WIDTH, HERO_HEIGHT)); //  устанавливая ту часть текстуры, на которой герой смотрит влево
				currentVelocityX = -velocity; //  в текущую скорость устанавливается начальная скорость (меньше 0, т.к влево, иначе было бы больше 0)
				this->setMove("Left"); //  задаётся описание текущего направления движения игрока
				currentlyMoving = true; //  значение "герой на данный момент движется" устанавливается в истинное
			}
			else if (event.key.code == Keyboard::Right) { // аналогично с клавишами: стрелка вправо, вверз, вниз
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

			else if (event.key.code == Keyboard::A) heroSprite.setTextureRect(IntRect(0, 0, HERO_WIDTH, HERO_HEIGHT)); //  если выбраны клавиши WASD - просто меняется
			else if (event.key.code == Keyboard::D) heroSprite.setTextureRect(IntRect(101, 0, HERO_WIDTH, HERO_HEIGHT)); //  направление взгляда героя
			else if (event.key.code == Keyboard::W) heroSprite.setTextureRect(IntRect(0, 52, HERO_WIDTH, HERO_HEIGHT));
			else if (event.key.code == Keyboard::S) heroSprite.setTextureRect(IntRect(101, 52, HERO_WIDTH, HERO_HEIGHT));

		}
		return stepCount;
	}

	void abortMove() { //  функция, отменяющая движение героя (например, при встрече на пути дерева)
		currentVelocityX = 0;
		currentVelocityY = 0;
		if (currentMove == "Up") heroSprite.move(0, velocity);
		else if (currentMove == "Down") heroSprite.move(0, -velocity);
		else if (currentMove == "Left") heroSprite.move(velocity, 0);
		else if (currentMove == "Right") heroSprite.move(-velocity, 0);
	}

	Vector2f getCoords() { //  функция, возвращающая координаты героя
		return heroSprite.getPosition();
	}

	Sprite getSprite() { //  функция, возвращающая спрайт героя
		return heroSprite;
	}

	float getVelocity() { //  функция, возвращающая текущую скорость движения героя (в любом направлении)
		return velocity;
	}

	bool getCurrentlyMoving() { //  функция, возвращающая булево значение, определяющее, движется ли сейчас герой
		return currentlyMoving;
	}

	bool getAllowGoingThroughWalls() { //  функция, возвращающая булево значение, определяющее, может ли герой выйти за пределы камеры
		return allowGoingThroughWalls;
	}

	bool getNeedsToBeDrivenAway() { //  функция, возвращающая булево значение, определяющее, должна ли сейчас проигрываться анимация "похищения" героя
		return needsToBeDrivenAway;
	}

	bool getIsKilled() { // функция, возвращающая булево значение, определяющее, проиграл ли игрок
		return isKilled;
	}

	bool getIsInvulnerable() {
		return isInvulnerable;
	}

	bool getIsSlowingTime() {
		return isSlowingTime;
	}

	void checkDeadlyIntersection(Car car, float carSpeed, bool driveDirection) { //  функция, обрабатывающая столкновение игрока с машиной
		FloatRect carBounds = car.getSprite().getGlobalBounds(); //  берутся глобальные границы машины
		
		if (carBounds.intersects(heroSprite.getGlobalBounds()) && !isInvulnerable) { //  если случилось пересечение границ машины с границами героя
			currentVelocityX = 0; // текущая скорость по oX обнуляется
			if (currentlyMoving && (currentMove == "Up" || currentMove == "Down") || needsToBeDrivenAway) { //  если игрок влетел в машину движением вниз или вверх
				driveAway(driveDirection, carBounds, carSpeed, car.getCarType()); //  проигрывать анимацию похищения
				needsToBeDrivenAway = true; //  значение "герой был похищен" устанавливается в истину
			}
			else if (driveDirection && heroSprite.getPosition().x < carBounds.left || !driveDirection && heroSprite.getPosition().x > carBounds.left) { //  аналогично,
				driveAway(driveDirection, carBounds, carSpeed, car.getCarType()); //  если герой влетел в машину сзади (догнал её)
				needsToBeDrivenAway = true;
			}
			else if (!needsToBeDrivenAway && (!driveDirection && heroSprite.getPosition().x < carBounds.left || driveDirection && heroSprite.getPosition().x > carBounds.left))
				squish(driveDirection, carBounds, carSpeed); //  если герой врезался в машину спереди (машина догнала игрока) - проигрывается анимация "сплющивания" героя
		}
	}

	void squish(bool driveDirection, FloatRect carBounds, float carSpeed) { //  функция, проигрывающая анимацию сплющивания игрока
		allowGoingThroughWalls = true; //  с этого момента игрок может выходить за границы камеры (машина сможет его вытолкнуть)
		if (!driveDirection) { //  после касания с машиной герой едет в направлении машины и немного сплющивается
			if (heroSprite.getScale().x < 1.5 / carSpeed) this->moveBy(-carSpeed, 0); //  степень сплющенности зависит от скорости машины
			heroSprite.scale((carBounds.left - heroSprite.getGlobalBounds().left) / (heroSprite.getScale().x * HERO_WIDTH), 1);
		}
		else {
			if (heroSprite.getScale().x < 1.5 / carSpeed) this->moveBy(carSpeed, 0);
			{
				heroSprite.scale((heroSprite.getGlobalBounds().left + heroSprite.getGlobalBounds().width - (carBounds.left + carBounds.width)) / (heroSprite.getScale().x * HERO_WIDTH), 1);
				heroSprite.setPosition(carBounds.left + carBounds.width + 1, heroSprite.getPosition().y);
			}
		}
		isKilled = true; // значение "игрок проиграл" превращается в истину
	}

	void driveAway(bool driveDirection, FloatRect carBounds, float carSpeed, String carType) { //  функция, проигрывающая анимацию похищения героя
		currentVelocityY = 0; //  текущая скорость движения героя по oY обращается в ноль (для данной анимации игрок полностью останавливается)
		if (!isKilled) heroSprite.scale(0.5, 0.5); // если анимация только началась - уменьшить игрока в 2 раза по каждой стороне (чтобы расположить на машине)
		if (!driveDirection) { //  в зависимости от типа машины и направления движения герой располагается на ней так, чтобы казалось, что он едет на этой машине
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
		isKilled = true; //  значение "игрок проиграл" обращается в истину
		allowGoingThroughWalls = true; //  с этого момента игрок может выходить за границы камеры (машина сможет его вытолкнуть)
	}
};


float minPossibleSpace(int carAmount, int carSize) { //  функция, возвращающая минимальное возможное расстояние между машинами на дороге при заданном количестве машин с определённой длиной (для того, чтобы между ними мог проскакивать герой)
	return (WIDTH + 3 * carSize - carAmount * carSize) / carAmount;
}


Road rebuild_road(float x, float y, int roadChoice) { //  функция, изменяющая полосу дороги/газон
	Road newRoad;
	newRoad.setTexture(ROAD_TEXTURES[roadChoice]);
	newRoad.setCoords(x, y); //  создаётся новый экземпляр класса Road, ему задаётся спрайт с текстурой дороги и координатами начала (x, y)
	if (roadChoice != 2) { //  если текстура - не текстура газона
		coordinatesXAvailableForHero.clear(); //  очищаем массив координат, доступных для прохода героя, поскольку на дороге нет статических объектов, мешающих движению, следовательно доступ к следующей дороге однозначно будет
		newRoad.setDriveDirection(rand() % 2); //  случайно задаётся направление движения
		int carAmount = 2 + rand() % 4; //  случайно задаётся количество машин от 2 до 6 (они едут по кругу - заезжают в один конец, выезжают из другого)
		Car randomCar; //  создаётся объект класса Car, назначается спрайт со случайной текстурой из массива текстур для машин размером с эту текстуру
		int carTextureIndex = rand() % CAR_TEXTURES.size();
		Texture randomTexture = CAR_TEXTURES[carTextureIndex];
		int rX = randomTexture.getSize().x;
		int rY = randomTexture.getSize().y;
		randomCar.setSize(0, 0, rX, rY);
		randomCar.setTexture(CAR_TEXTURES[carTextureIndex]);
		if (carTextureIndex < 104) randomCar.setCarType("sedan"); //  в зависимости от индекса текстуры, машине присваивается определённый тип (машины одного типа соседствуют)
		else if (carTextureIndex >= 104 && carTextureIndex < 167) randomCar.setCarType("truck");
		else if (carTextureIndex >= 167 && carTextureIndex < 230) randomCar.setCarType("mini");
		else if (carTextureIndex >= 230 && carTextureIndex < 287) randomCar.setCarType("hot_rod");
		else if (carTextureIndex >= 287 && carTextureIndex < 347) randomCar.setCarType("jeep");
		else if (carTextureIndex >= 347 && carTextureIndex < 350) randomCar.setCarType("truck_old");
		else if (carTextureIndex >= 350 && carTextureIndex < 356) randomCar.setCarType("darkworld_car");
		else if (carTextureIndex >= 356 && carTextureIndex < 433) randomCar.setCarType("convertible");
		else if (carTextureIndex >= 433 && carTextureIndex < 490) randomCar.setCarType("drag");
		if (newRoad.getDriveDirection()) randomCar.setScale(-1, 1); //  если движение по дороге осуществляется вправо - спрайт машины разворачивается
		float carSpace = minPossibleSpace(carAmount, rX); //  вычисляется минимально возможное расстояние между машинами с учётом размера героя
		while (carSpace < 2.5 * HERO_WIDTH) { //  пока расстояние меньше двух длин героя - уменьшается количество машин и делается перерасчёт дистанции
			carAmount--;
			carSpace = minPossibleSpace(carAmount, rX);
		}
		vector<Car> inputCars;
		float randS = rand() % 100 + 0.1f; //  вычисляется случайное расстояние от первой машины до конца дороги (чтобы избежать синхронного движения машин)
		for (int i = 0; i < carAmount; i++) { //  в созданный массив inputCars ранее определённый объект класса Car (randomCar) поступает (carAmount) раз
			inputCars.push_back(randomCar); // для каждой "новой" машины задаются координаты в соответствии с направлением движения, по высоте дороги машина центрируется
			if (newRoad.getDriveDirection()) inputCars[i].setCoords((carSpace + rX) * i + randS, y + (ROAD_SEGMENT_HEIGHT - randomTexture.getSize().y) / 2);
			else inputCars[i].setCoords(WIDTH - (carSpace + rX) * i - randS, y + (ROAD_SEGMENT_HEIGHT - randomTexture.getSize().y) / 2);
		}
		newRoad.setCars(inputCars); //  созданному объекту-дороге передаётся массив машин
		newRoad.setSpeed(MIN_CAR_SPEED + static_cast <float> (rand()) / static_cast <float> (RAND_MAX / (MAX_CAR_SPEED - MIN_CAR_SPEED))); //  также ему присваевается значение скорости движения
		newRoad.setNotPark(true); //  и значение "данная полоса не является газоном" превращается в истинное
	}

	else { //  если текстура - газон
		vector<Sprite> newNature; //  создаётся массив растительности
		if (coordinatesXAvailableForHero.size() == 0) { //  если доступны все ячейки газона (массив coordinatesXAvailableForHero пустой), то мы просто для каждой ячейкий создаём новое дерево и
			for (int z = 0; z < 10; z++) {	//  определяем, будет ли оно посажено через вероятность TREE_PROBABILITY
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
			for (int z = 0; z < 10; z++) { //  если же массив не пустой, то мы дополнительно выбираем случайную координату из массива (если ег длина больше 1, иначе не останется места для прохода)
				Sprite randomNature;
				randomNature.setTexture(NATURE_ELEMENTS_TEXTURES[rand() % 1]);
				randomNature.setPosition(z * 100.f, y - 50.f);
				int approvedInd = rand() % (coordinatesXAvailableForHero.size());
				float approvedCoord = coordinatesXAvailableForHero[approvedInd];
				if (coordinatesXAvailableForHero.size() == 1) approvedCoord = -1;
				vector<float> ::iterator it = find(coordinatesXAvailableForHero.begin(), coordinatesXAvailableForHero.end(), 100 * z);
				if (rand() % 100 <= TREE_PROBABILITY && (it == coordinatesXAvailableForHero.end() || z * 100 == approvedCoord))  // и дополнительно проверяем дерьевья на то, что их координаты не находятся в массиве (за исключением выбранной)
					newNature.push_back(randomNature);
				if (approvedCoord != -1) coordinatesXAvailableForHero.erase(coordinatesXAvailableForHero.begin() + approvedInd); // и выбранную координату удаляем из массива
			}
		}
		if (newNature.size() == 0) coordinatesXAvailableForHero.clear();
		newRoad.setNatureElements(newNature); //  созданному ранее в функции объекту Road передаётся массив растительности
		newRoad.setNotPark(false); //  значение "данная полоса не является газоном" устанавливается в ложь
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

	return newRoad; //  функция возвращает новую дорогу с новыми параметрами
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

	View camera; //  инициализируется камера, следующая за игроком, в начальном положении
	camera.reset(FloatRect(0, 0, WIDTH, HEIGHT));
	float catchingUpSpeed = 0; //  инициализируется переменная, меняющаяся в зависимости от положения игрока (увеличивается, если игрок уходит далеко вперёд от камеры)
	float currentCameraSpeed = CAMERA_MOVE_SPEED; //  инициализируется обычная скорость камеры, не зависящая от движения игрока (камера всё время движется вверх, не давая игроку стоять на месте)

	Hero turtle = Hero(); //  создаётся объект героя, который затем перемещается в центр нижней части экрана (камеры)
	turtle.setPosition(WIDTH / 2, HEIGHT - (ROAD_SEGMENT_HEIGHT + HERO_HEIGHT) / 2);

	Road current_roads[9]; //  создаётся массив, в котором хранятся 9 объектов класса Road, циклически идущих друг за другом на экране
	for (int k = 0; k < 9; k++) {
		int roadChoice = rand() % 3; //  для каждого объекта выбирается текстура дороги,
		if (k == 0) {
			roadChoice = 2; // однако первый объект (откуда начинает игрок) всегда должен быть газоном
			coordinatesXAvailableForHero.push_back(500);
		}
		current_roads[k] = rebuild_road(0, HEIGHT - (ROAD_SEGMENT_HEIGHT * (k + 1)), roadChoice); // далее объекты "перестраиваются" в соответствии с координатами начала, выбранной текстурой и массивом координат, на которых можно расположить деревья (если данная и предыдущая дороги были газонами)
	}
	for (int cur = 0; cur < current_roads[0].getSuperPowers().size(); cur++) current_roads[0].makeSuperPowerUnaccessable(cur);
	current_roads[0].eraseUnaccessableSuperPowers();

	for (int s = 0; s < current_roads[0].getNatureElements().size(); s++) { // для первой дороги (т.е. газона), если на координатах появления героя стоит дерево - оно оттуда удаляется
		if (current_roads[0].getNatureElements()[s].getPosition().x == WIDTH / 2) current_roads[0].eraseNatureElement(s);
	}

	float newRoadY = HEIGHT - 10 * ROAD_SEGMENT_HEIGHT; // объявляется переменная, определяющая координату для oY для очередной циклически появляющейся дороги

	while (window.isOpen()) //  пока работает программа
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
					Text pauseText(L"ПАУЗА", FONT, 72U);
					Text additionalPauseText1(L"Нажмите ESC, чтобы продолжить", FONT, 36U);
					Text additionalPauseText2(L"или Backspace, чтобы выйти", FONT, 36U);

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
			if (!isPaused && canStart) scoreCountInReality = turtle.update(event, scoreCountInReality); //  обновляется положение игрока в соответствии с событиями
		}

		if (!isPaused) {
			turtle.move(); //  происходит движение героя, если он имеет какую-либо текущую скорость

			if (((turtle.getCoords().x > WIDTH - HERO_WIDTH || turtle.getCoords().x < 0) && !turtle.getAllowGoingThroughWalls()) && !turtle.getAllowGoingThroughWalls()) turtle.abortMove(); //  если герой пытается выйти за пределы камеры влево или вправо (кроме случая проигрыша, когда его толкает машина) - игнорировать его действия
			if (turtle.getCoords().y <= camera.getCenter().y) catchingUpSpeed = (camera.getCenter().y - turtle.getCoords().y) / 8; //  если герой начинает приближаться к верхнему краю камеры - увеличивается та самая скорость движения камеры
			else catchingUpSpeed = 0; // иначе она обращается в ноль

			if (turtle.getCoords().y > camera.getCenter().y + camera.getSize().y / 2) turtle.setIsKilled(true);

			if (turtle.getIsKilled() && currentCameraSpeed < DEATH_CAMERA_MOVE_SPEED) currentCameraSpeed = currentCameraSpeed * 1.05f; //  если игрок проигрывает - базовая скорость камеры постепенно меняется на скорость после проигрыша
			if (canStart) camera.move(0, -max(catchingUpSpeed, currentCameraSpeed)); //  камера движется вверх с наибольшей возможной скоростью
			window.setView(camera); //  вид камеры обновляется
			window.clear(); //  окно программы очищается от всех спрайтов, чтобы нарисовать новые

			for (int k = 8; k > -1; k--) {
				window.draw(current_roads[k].getSprite()); //  каждое обновление отрисовываются все спрайты: дороги, растительность, герой, машины
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
					for (Sprite natureType : current_roads[k].getNatureElements()) { // для каждого дерева создаются модифицированные границы (т.к. высота дерева больше высоты полосы дороги, оно выходит за его пределы, однако это не должно влиять на движение игрока по полосе выше, поскольку дерево не присутствует на ней. Следовательно, учитывается только та часть, которая в 2Д находится в пределах полосы газона)
						FloatRect modBounds = { natureType.getGlobalBounds().left, natureType.getGlobalBounds().top + 25, natureType.getGlobalBounds().width, natureType.getGlobalBounds().height - 25 };
						if (modBounds.intersects(turtle.getSprite().getGlobalBounds()) && !turtle.getIsInvulnerable()) turtle.abortMove(); //  если игрок хочет пройти сквозь дерево - его действия игнорируются
					}
				}
			}

			if (turtle.getCoords().x > -100) window.draw(turtle.getSprite()); //  если герой находится в пределах камеры (его не вытолкнула машина) - он отрисовывается


			for (int k = 8; k > -1; k--) {
				current_roads[k].eraseUnaccessableSuperPowers();
				if (current_roads[k].getNotGrass()) {
					for (int j = 0; j < current_roads[k].getCars().size(); j++) {
						window.draw(current_roads[k].getCars()[j].getSprite()); //  отрисовка каждой машины дороги, если дорога - не газон

						current_roads[k].updateCarsPosition(j, turtle.getIsSlowingTime()); //  обновление положения машины на этой дороге

						turtle.checkDeadlyIntersection(current_roads[k].getCars()[j], current_roads[k].getSpeed(), current_roads[k].getDriveDirection());  // проверка на то, что игрок не наткнулся на данную машину

					}
				}

				else for (Sprite natureType : current_roads[k].getNatureElements()) window.draw(natureType); // отрисовка всей растительности, если дорога - газон

				if (current_roads[k].getCoords().y > camera.getCenter().y + HEIGHT / 2 + 50) { // если дорога вышла за пределы камеры (внизу)
					for (int m = 0; m < 8; m++) current_roads[m] = current_roads[m + 1]; // то она выбрасывается из массива (FIFO)
					int roadChoice = rand() % 3; //  на последнее место встаёт новая дорога,
					if (current_roads[7].getNotGrass() == false && roadChoice == 2)
						current_roads[8] = rebuild_road(0, newRoadY, 2);
					else current_roads[8] = rebuild_road(0, newRoadY, roadChoice);
					newRoadY -= ROAD_SEGMENT_HEIGHT; //  а координата по Y для новой дороги обновляется
				}

			}

			if (scoreCountInReality > newRecord) newRecord = scoreCountInReality;
			if (turtle.getIsInvulnerable() || turtle.getIsSlowingTime()) {
				timeAfterSuperPowerStart = time(0);
				if (turtle.getIsInvulnerable() && turtle.getIsSlowingTime()) {
					Text timeLeftText1(L"Неуязвимость: " + to_string(8 - (timeAfterSuperPowerStart - invulnerabilityStartMoment)), FONT, 24U);
					Text timeLeftText2(L"Замедление времени: " + to_string(8 - (timeAfterSuperPowerStart - slowingTimeStartMoment)), FONT, 24U);
					timeLeftText1.setPosition(camera.getCenter().x + camera.getSize().x / 2 - timeLeftText1.getLocalBounds().width - 5, camera.getCenter().y - camera.getSize().y / 2 + 30);
					timeLeftText2.setPosition(camera.getCenter().x + camera.getSize().x / 2 - timeLeftText2.getLocalBounds().width - 5, timeLeftText1.getPosition().y + LINE_SPACING * timeLeftText1.getLocalBounds().height);
					window.draw(timeLeftText1);
					window.draw(timeLeftText2);
				}
				else {
					Text timeLeftText(L"Неуязвимость: " + to_string(8 - (timeAfterSuperPowerStart - invulnerabilityStartMoment)), FONT, 24U);
					if (turtle.getIsSlowingTime()) timeLeftText.setString(L"Замедление времени: " + to_string(8 - (timeAfterSuperPowerStart - slowingTimeStartMoment)));
					timeLeftText.setPosition(camera.getCenter().x + camera.getSize().x / 2 - timeLeftText.getLocalBounds().width - 5, camera.getCenter().y - camera.getSize().y / 2 + 30);
					window.draw(timeLeftText);
				}
			}

			if (8 - (timeAfterSuperPowerStart - invulnerabilityStartMoment) < 0) turtle.setIsInvulnerable(false);
			if (8 - (timeAfterSuperPowerStart - slowingTimeStartMoment) < 0) turtle.setIsSlowingTime(false);

			if (canStart) {
				Text scoreText(L"Очки: " + to_string(newRecord), FONT, 24U);
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
				Text gameOverTexts[5] = { Text(L"Игра окончена", FONT, 72U),  Text(L"Ваш счёт: " + to_string(newRecord), FONT, 48U),
										  Text(L"Действующий рекорд: " + to_string(currentRecord), FONT, 48U), Text(L"Нажмите Enter, чтобы начать сначала", FONT, 24U),
										  Text(L"или Backspace, чтобы выйти", FONT, 24U) };
				if (newRecord > currentRecord) gameOverTexts[2] = Text(L"И это новый рекорд!", FONT, 48U);

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
    srand(unsigned int(time(NULL))); //  прописанная строка позволяет получать немнимые случайные число (т.е. не те, которые были однажды выбраны программой и используются в ней как константы, не изменяясь)
	
    ContextSettings settings;
    settings.antialiasingLevel = 8; //  настройка сглаживаемости отрисовываемых спрайтов

	RenderWindow window(VideoMode(unsigned int(WIDTH), unsigned int(HEIGHT)), "Totally Not A Crossy Road", Style::Close); //  создаётся окно с названием без возможности изменить размер экрана
	window.setKeyRepeatEnabled(false); //  отключается считывание удерживаемых клавиш (чтобы герой не "летал" по карте)
	window.setFramerateLimit(FRAME_LIMIT); //  устанавливается ограничение по кадрам в секунду

    Texture Lane; Lane.loadFromFile("objects\\sprites\\misc\\road.png"); //  задаются текстуры дорог, которые затем помещаются в массив текстур дорог
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

	Texture Nature1; Nature1.loadFromFile("objects\\sprites\\misc\\tree.png"); //  задаются текстуры растительности, которые потом помещаются в массив текстур растительности

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