#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <stdexcept>
#include <string>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <thread>
#include <conio.h>

using namespace std;

// Размеры игровой комнаты
const int ROOM_WIDTH = 10;
const int ROOM_HEIGHT = 10;

// Символы для отображения элементов
const char PLAYER_CHAR = '@';
const char ENEMY_CHAR = 'E';
const char BOSS_CHAR = 'B';
const char ITEM_CHAR = 'I';
const char WALL_CHAR = '#';
const char EMPTY_CHAR = '.';

// Цветовые коды ANSI для текста
const string PLAYER_COLOR = "\033[1;32m";  // Зеленый
const string ENEMY_COLOR = "\033[1;31m";   // Красный
const string BOSS_COLOR = "\033[1;35m";    // Магента
const string ITEM_COLOR = "\033[1;33m";    // Желтый
const string WALL_COLOR = "\033[1;37m";    // Белый
const string EMPTY_COLOR = "\033[0m";      // Сброс цвета

// Цветовые коды ANSI для фона
const string BG_COLOR = "\033[1;40m";      // Черный фон


// Базовый класс игровых объектов
class GameObject {
protected:
    int posX;
    int posY;
    int health;
public:
    GameObject(int x, int y, int hp) : posX(x), posY(y), health(hp) {}
    virtual ~GameObject() {}

    virtual void update() = 0;  // Виртуальная функция для обновления объекта

    // Геттеры и сеттеры
    int getX() const { return posX; }
    int getY() const { return posY; }
    void setX(int x) { posX = x; }
    void setY(int y) { posY = y; }

    virtual void takeDamage(int damage) {
        health -= damage;
        if (health <= 0) {
            health = 0;
        }
    }

    int getHealth() const { return health; }
};

class Character : public GameObject {
protected:
    int attackDamage;
public:
    Character(int x, int y) : GameObject(x, y, 100), attackDamage(10) {}

    void update() override {
        // Логика обновления персонажа
    }

    void attack(GameObject* target) {
        target->takeDamage(attackDamage);
        if (target->getHealth() <= 0) {
            cout << "Target defeated!" << endl;
            cout << "Press any key to continue..." << endl;
            _getch();
        }
    }

    void receiveDamage(int damage) {
        takeDamage(damage);
        cout << "Player received " << damage << " damage!" << endl;
    }

    bool checkCollision(int x, int y) const {
        return (posX == x && posY == y);
    }
};

class Enemy : public GameObject {
protected:
    int attackDamage;
public:
    Enemy(int x, int y) : GameObject(x, y, 50), attackDamage(5) {}

    void update() override {
        move();

    }

    void move() {
        int direction = rand() % 4;
        int newX = posX, newY = posY;
        switch (direction) {
        case 0: newY--; break; // вверх
        case 1: newY++; break; // вниз
        case 2: newX--; break; // влево
        case 3: newX++; break; // вправо
        }

        // Проверка на границы комнаты и наличие стены
        if (newX > 0 && newX < ROOM_WIDTH - 1 && newY > 0 && newY < ROOM_HEIGHT - 1) {
            posX = newX;
            posY = newY;
        }

    }

    void takeDamage(int damage) override {
        GameObject::takeDamage(damage);
        if (getHealth() <= 0) {
            cout << "Enemy died!" << endl;
        }
    }

    void attack(Character* target) {
        target->receiveDamage(attackDamage);
    }

    bool isDead() const {
        return getHealth() <= 0;
    }

};


class Boss : public Enemy {
private:
    string bossName;
public:
    Boss(int x, int y, const string& name) : Enemy(x, y), bossName(name) {
        health = 200; // У босса больше здоровья
    }

    void update() override {
        // Логика движения и атаки босса
    }

    string getName() const { return bossName; }

    void takeDamage(int damage) override {
        Enemy::takeDamage(damage);
        if (getHealth() <= 0) {
            cout << "Boss died!" << endl;
        }
    }
};

// Класс предмета
class Item : public GameObject {
private:
    string name;
public:
    Item(int x, int y, const string& itemName) : GameObject(x, y, 0), name(itemName) {}

    void update() override {
        // Логика обновления предмета
    }

    string getName() const { return name; }
};

// Класс комнаты
class Room {
private:
    vector<GameObject*> roomObjects;
public:
    void addGameObject(GameObject* obj) {
        roomObjects.push_back(obj);
    }

    void removeGameObject(GameObject* obj) {
        auto it = find(roomObjects.begin(), roomObjects.end(), obj);
        if (it != roomObjects.end()) {
            roomObjects.erase(it);
        }
    }

    void updateRoom() {
        for (auto obj : roomObjects) {
            obj->update();
        }
    }

    vector<GameObject*>& getGameObjects() {
        return roomObjects;
    }
};

// Класс инвентаря
template<typename T>
class Inventory {
private:
    vector<T*> items;
public:
    void addItem(T* item) {
        items.push_back(item);
    }

    void removeItem(T* item) {
        auto it = find(items.begin(), items.end(), item);
        if (it != items.end()) {
            items.erase(it);
        }
    }

    T* getItem(int index) {
        if (index >= 0 && index < items.size()) {
            return items[index];
        }
        return nullptr;
    }

    int getSize() const {
        return items.size();
    }
};

// Класс игровой сессии
class GameSession {
private:
    Room gameRoom;
    Character* player;
    Boss* finalBoss;
    Inventory<Item> playerInventory;
    bool gameRunning;
    int usedMoves = 0;

public:
    GameSession() : player(nullptr), finalBoss(nullptr), gameRunning(true) {}

    void initialize() {
        // Создание персонажа
        player = new Character(5, 5);

        // Создание предметов и добавление их в комнату
        Item* healthPotion = new Item(2, 3, "Health Potion");
        gameRoom.addGameObject(healthPotion);

        // Создание врагов и добавление их в комнату
        for (int i = 0; i < 3; ++i) {
            gameRoom.addGameObject(new Enemy(rand() % ROOM_WIDTH, rand() % ROOM_HEIGHT));
        }

        // Создание босса
        finalBoss = new Boss(8, 8, "Final Boss");

        // Добавление босса в комнату
        gameRoom.addGameObject(finalBoss);

        // Добавление игрока в комнату
        gameRoom.addGameObject(player);

    }

    void update() {
        gameRoom.updateRoom();

        usedMoves++;

        // Удаление убитых врагов
        auto& objects = gameRoom.getGameObjects();
        objects.erase(remove_if(objects.begin(), objects.end(), [](GameObject* obj) {
            if (auto enemy = dynamic_cast<Enemy*>(obj)) {
                return enemy->isDead();
            }
            return false;
            }), objects.end());

        checkGameOver();  // Добавление проверки на завершение игры
    }

    void drawGame() {
        // Очистка экрана
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif

        // Отображение игрового поля
        cout << "\n====  Carnage  ====" << endl;
        for (int y = 0; y < ROOM_HEIGHT; ++y) {
            for (int x = 0; x < ROOM_WIDTH; ++x) {
                bool objectFound = false;
                for (auto obj : gameRoom.getGameObjects()) {
                    if (obj->getX() == x && obj->getY() == y) {
                        if (dynamic_cast<Character*>(obj)) {
                            cout << PLAYER_COLOR << PLAYER_CHAR << EMPTY_COLOR << " ";
                        }
                        else if (dynamic_cast<Enemy*>(obj)) {
                            cout << ENEMY_COLOR << ENEMY_CHAR << EMPTY_COLOR << " ";
                        }
                        else if (dynamic_cast<Boss*>(obj)) {
                            cout << BOSS_COLOR << BOSS_CHAR << EMPTY_COLOR << " ";
                        }
                        else if (dynamic_cast<Item*>(obj)) {
                            cout << ITEM_COLOR << ITEM_CHAR << EMPTY_COLOR << " ";
                        }
                        objectFound = true;
                        break;
                    }
                }
                if (!objectFound) {
                    // Пустое место (пустая комната или стена)
                    cout << WALL_COLOR << (x == 0 || x == ROOM_WIDTH - 1 || y == 0 || y == ROOM_HEIGHT - 1 ? WALL_CHAR : EMPTY_CHAR) << EMPTY_COLOR << " ";
                }
            }
            cout << endl;
        }
        cout << "===================" << endl;
    }

    void showInventory() {
        // Очистка экрана
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif

        // Установка цвета фона
        cout << BG_COLOR;

        // Отображение инвентаря
        cout << "\n=== Inventory ===" << endl;
        for (int i = 0; i < playerInventory.getSize(); ++i) {
            Item* item = playerInventory.getItem(i);
            if (item) {
                cout << i + 1 << ". " << item->getName() << endl;
            }
        }
        cout << "=================" << endl;

        // Ожидание нажатия клавиши для возврата к игре
        cout << "Press any key to continue..." << endl;
        _getch();
    }

    void handleInput() {
        if (_kbhit()) {
            char action = _getch(); // Использование _getch() вместо getchar() для немедленного считывания клавиши
            switch (action) {
            case 'w': {
                player->setY(max(1, player->getY() - 1));
                break;
            }
            case 's': {
                player->setY(min(ROOM_HEIGHT - 2, player->getY() + 1));
                break;
            }
            case 'a': {
                player->setX(max(1, player->getX() - 1));
                break;
            }
            case 'd': {
                player->setX(min(ROOM_WIDTH - 2, player->getX() + 1));
                break;
            }
            case 'i': {
                showInventory();
                break;
            }
            case ' ': {
                // Проверка наличия врага в ближайших клетках
                bool enemyFound = false;
                for (auto obj : gameRoom.getGameObjects()) {
                    if (obj != player && abs(obj->getX() - player->getX()) <= 1 && abs(obj->getY() - player->getY()) <= 1) {
                        player->attack(obj);
                        enemyFound = true;
                        break;
                    }
                }

                // Проверка на остаток врагов
                bool anyEnemiesLeft = false;
                for (auto obj : gameRoom.getGameObjects()) {
                    if (dynamic_cast<Enemy*>(obj)) {
                        anyEnemiesLeft = true;
                        break;
                    }
                }

                // Если врагов не осталось, выводим сообщение и ожидаем нажатия клавиши, кроме движений
                if (!anyEnemiesLeft && enemyFound) {
                    cout << "All enemies defeated! Press any key (except movement keys) to continue..." << endl;
                    char key;
                    do {
                        key = _getch();
                    } while (key == 'w' || key == 'a' || key == 's' || key == 'd');
                }
                break;
            }
            }
        }

        // Проверка наличия предмета в текущей позиции игрока
        for (auto obj : gameRoom.getGameObjects()) {
            if (auto item = dynamic_cast<Item*>(obj)) {
                if (player->checkCollision(item->getX(), item->getY())) {
                    // Подбор предмета
                    playerInventory.addItem(item);
                    gameRoom.removeGameObject(item);
                    cout << "Picked up " << item->getName() << "!" << endl;
                    // Ожидание нажатия клавиши для возврата к игре
                    cout << "Press any key to continue..." << endl;
                    _getch();
                    break;

                }
            }
        }
    }

    bool isGameRunning() const {
        return gameRunning;
    }

    void endGame() {
        gameRunning = false;
    }

    void checkGameOver() {
        bool anyEnemiesLeft = false;
        for (auto obj : gameRoom.getGameObjects()) {
            if (dynamic_cast<Enemy*>(obj)) {
                anyEnemiesLeft = true;
                break;
            }
        }
        if (!anyEnemiesLeft) {
            cout << "All enemies have been defeated. Game Over!" << endl;
            cout << "It took you " << usedMoves << " frame refreshes." << endl;
            _getch();
            endGame();
        }
    }

    ~GameSession() {
        delete player;
        delete finalBoss;
        for (auto obj : gameRoom.getGameObjects()) {
            delete obj;
        }

    }
};

// Функция для отображения главного меню
void displayMainMenu() {
    cout << "=== Main Menu ===" << endl;
    cout << "1. Start Game" << endl;
    cout << "2. Exit Game" << endl;
    cout << "=================" << endl;
    cout << "Enter your choice: ";
}

int main() {
    srand(static_cast<unsigned>(time(0))); // Инициализация генератора случайных чисел

    bool exitGame = false;
    while (!exitGame) {
        // Отображение главного меню
        displayMainMenu();

        // Считывание выбора пользователя
        char choice;
        cin >> choice;

        switch (choice) {
        case '1': {
            // Игровая сессия
            GameSession gameSession;
            gameSession.initialize();

            while (gameSession.isGameRunning()) {
                gameSession.update();
                gameSession.drawGame();
                gameSession.handleInput();
                this_thread::sleep_for(chrono::milliseconds(100));
            }
            break;
        }
        case '2': {
            // Выход из игры
            cout << "Exiting Game..." << endl;
            exitGame = true;
            break;
        }
        default:
            cout << "Invalid choice. Please enter 1 or 2." << endl;
            break;
        }
    }
}
