#include <iostream>
#include <conio.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <Windows.h>
using namespace std;

const int height = 26, length = 52,
start_posx = 1, start_posy = 1,
snake_size = 4, snake_size_bot = snake_size;

// Координаты точки - индексы в массиве символов

struct Points {
    int x, y;

    Points& operator+=(Points& b) {
        x += b.x;
        y += b.y;
        return *this;
    }

    Points& operator-=(Points& b) {
        x -= b.x;
        y -= b.y;
        return *this;
    }
};

bool operator== (Points a, Points b) {
    return (a.x == b.x && a.y == b.y);
}

bool operator!= (Points a, Points b) {
    return (a.x != b.x || a.y != b.y);
}

bool operator>= (Points a, Points b) {
    return (a.x >= b.x && a.y >= b.y);
}

bool operator<= (Points a, Points b) {
    return (a.x <= b.x && a.y <= b.y);
}

Points operator+ (Points a, Points b) {
    return { a.x + b.x, a.y + b.y };
}

Points operator- (Points a, Points b) {
    return { a.x - b.x, a.y - b.y };
}


class Snake {
public:
    long size, score = 0;
    vector<Points> segments;
    Points offset = { 1, 0 };
    Points pos;

    Snake(int);
    void start_position(int, int);
    void update_offset(int, int);
    void update_position();
    void check_out_of_borders();
    void move();
    int check_food(Points, Points);
    bool check();
};


class SnakeBot : public Snake {
public:
    Points distanceToFood;

    SnakeBot(int);
    void start_position();
    void set_distance(Points);
    void update_position();
    void def_direction();
    void update_distance();
    bool check();
    float concentration_segments(int, int, int);
    bool check_segment_on_dist(int, int);
};


//
void setpos(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void show_cursor(bool visibility) {
    HANDLE ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(ConsoleHandle, &cursorInfo);
    cursorInfo.dwSize = 100;
    cursorInfo.bVisible = visibility;
    SetConsoleCursorInfo(ConsoleHandle, &cursorInfo);
}

char key_pressed();                                                                         // Символ с клавиатуры
int random(int, int = 0);                                                                   // Рандомное число со смещением
int key_handler(char, int = 0);                                                             // Обработчик нажатого символа (возвращает смещения x y)                                                   // Проверка на корректность движения
Points food(int, int, vector<Points>&, int, vector<Points>&, int, Points);                  // Координаты еды
bool check_food_in_snake(Points, vector<Points>&, int);                                     // Проверка на влезание еды в сегменты змей
bool check_food(vector<Points>& snake, int, Points);                                        // Проверка на взаимодействие с едой
int set_speed();                                                                            // Установка скорости = задержки
bool game(int, int, int, int, int);
int sign(int);
Points get_offset(int);


Snake::Snake(int size) {
    this->size = size;
    //start_position(start_pos_x, start_pos_y);
}

void Snake::start_position(int start_pos_x, int start_pos_y) {
    for (int x = start_pos_x; x < start_pos_x + snake_size; x++) {
        segments.push_back({ x, start_pos_y });
    }
    pos = segments[size - 1];
}

void Snake::update_offset(int x, int y) {
    offset = { x, y };
}

void Snake::update_position() {
    pos = segments[size - 1] + offset;
}

void Snake::check_out_of_borders() {
    if (pos.x >= length - 1 || pos.x <= 0 ||
        pos.y >= height - 1 || pos.y <= 0) {

        if (pos.x >= length - 1)
            pos.x = 1;
        else if (pos.x <= 0)
            pos.x = length - 2;
        else if (pos.y >= height - 1)
            pos.y = 1;
        else if (pos.y <= 0)
            pos.y = height - 2;
    }
}

void Snake::move() {
    for (int i = 0; i < size - 1; i++)
        segments[i] = segments[i + 1];
    segments[size - 1] = pos;
}

int Snake::check_food(Points foodUser, Points foodBot) {
    bool checkUserFood = (segments[size - 1] == foodUser);
    bool checkBotFood = (segments[size - 1] == foodBot);
    if (checkUserFood || checkBotFood) {
        auto iter = segments.cbegin();
        segments.insert(iter, segments[0]);
        ++size;
        ++score;
        if (checkUserFood) {
            return 1;
        }
        return 2;
    }
    return 0;
}
// Добавить чек на врезание в другую змею
bool Snake::check() {
    for (int i = 0; i < size - 1; i++) {
        if (segments[size - 1] == segments[i]) {
            return false;
        }
    }
    return true;
}



SnakeBot::SnakeBot(int size):Snake(size) {
    offset = { -1, 0 };
}

void SnakeBot::start_position() {
    for (int x = length - 2; x > length - 2 - size; x--) {
        segments.push_back({ x, height - 2 });
    }
    pos = segments[size - 1];
}

bool SnakeBot::check() {
    if (segments[size - 1].x >= length - 1 || segments[size - 1].x <= 0 ||
        segments[size - 1].y >= height - 1 || segments[size - 1].y <= 0) {
        return false;
    }
    Snake::check();
}

void SnakeBot::set_distance(Points food) {
    distanceToFood = food - pos;
    offset = { sign(distanceToFood.x), sign(distanceToFood.y) };
}

void SnakeBot::update_distance() {
    if (distanceToFood.x)
        distanceToFood.x -= offset.x;
    else
        distanceToFood.y -= offset.y;
}

void SnakeBot::update_position() {
    if (distanceToFood.x)
        pos.x += offset.x;
    else
        pos.y += offset.y;
}
// ERROR
void SnakeBot::def_direction() {
    // Доля от общей длины до еды по кратчайшему пути, которая должна быть свободна
    // дабы не запутаться по этому пути
    float koefDistance = 0.3;
    int distWithKoef, dir;
    // Откат pos и определение направления путем смены движения по x/y на y/x
    if (distanceToFood.x) {
        pos.x -= offset.x;
        offset = { 0, offset.y };
        distWithKoef = offset.y * (height - 3);
        dir = 1;
    }
    else {
        pos.y -= offset.y;
        offset = { offset.x, 0 };
        distWithKoef = offset.x * (length - 3);
        dir = 0;
    }
    segments[size - 1] = pos + offset;
    if (check() && check_segment_on_dist(distWithKoef * koefDistance, dir)) { 
        pos += offset;
        return;
    }

    // Выбор оптимального пути путем подсчета количества сегментов в 4 областях относительно головы
    // движение туда, где наименьшая концентрация
    float concentration[4] = { 0, 0, 0, 0 };
    concentration[0] = concentration_segments(segments[size - 1].x + 1, length - 2, 0);
    concentration[1] = concentration_segments(1, segments[size - 1].x - 1, 0);
    concentration[2] = concentration_segments(segments[size - 1].y + 1, height - 2, 1);
    concentration[3] = concentration_segments(1, segments[size - 1].y - 1, 1);
    float minimum = concentration[0];
    int indMinimum = 0;
    for (int i = 1; i < 4; ++i) {
        if (minimum > concentration[i]) {
            minimum = concentration[i];
            indMinimum = i;
        }
    }
    offset = get_offset(indMinimum);
    segments[size - 1] = pos + offset;
    if (check()) {
        pos += offset;
        return;
    }

    // Простая выборка направления
    for (int tempCounter = 0; tempCounter < 4 && tempCounter != indMinimum; ++tempCounter) {
        offset = get_offset(tempCounter);
        segments[size - 1] = pos + offset;
        if (check()) {
            pos += offset;
            return;
        }
    }
}
// 0 - x, 1 - y
float SnakeBot::concentration_segments(int start, int end, int dir) {
    if (start >= end)
        return 1;
    Points compareStart, compareEnd;
    if (dir) {
        compareStart = { 0, start };
        compareEnd = { length - 2, end };
    }
    else {
        compareStart = { start, 0 };
        compareEnd = { end, height - 2 };
    }
    int amountSegments = 0;
    for (int i = 0; i < size - 1; ++i) {
        if (segments[i] >= compareStart && segments[i] <= compareEnd)
            ++amountSegments;
    }
    return amountSegments / ((compareEnd.x - compareStart.x) * (compareEnd.y - compareStart.y));
}
// 0 - x, 1 - y
bool SnakeBot::check_segment_on_dist(int lim, int dir) {
    if (!lim)
        return false;
    Points point;
    if (dir)
        point = { segments[size - 1].x + lim, segments[size - 1].y };
    else
        point = { segments[size - 1].x, segments[size - 1].y + lim };

    if (lim < 0)
        for (int i = 0; i < size - 1; ++i) {
            if (segments[i] >= point && segments[i] <= segments[size - 1])
                return false;
        }
    else
        for (int i = 0; i < size - 1; ++i) {
            if (segments[i] <= point && segments[i] >= segments[size - 1])
                return false;
        }
    return true;
}



int main()
{
    setlocale(LC_ALL, "rus");
    srand(time(NULL));
    //system("cls")

    bool gameCondition = true;
    while (gameCondition) {
        gameCondition = game(height, length, start_posx, start_posy, snake_size);
    }
    return 0;
}

bool game(int height, int length, int start_posx, int start_posy, int snake_size) {
    char** symbols = new char*[height];
    for (int i = 0; i < height; i++) {
        symbols[i] = new char[length];
    }

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < length; j++) {
            if (j == 0 || j == length - 1 || i == 0 || i == height - 1) {
                symbols[i][j] = '+';
            }
            else {
                symbols[i][j] = ' ';
            }
        }
    }
    // Начальное определение змеи
    Snake snakeUser(snake_size);
    snakeUser.start_position(start_posx, start_posy);
    // Бот
    SnakeBot snakeBot(snake_size_bot);
    snakeBot.start_position();

    for (int x = start_posx; x < start_posx + snake_size; x++) {
        symbols[start_posy][x] = '#';
    }
    // Bot
    for (int x = length - 2; x > length - 2 - snake_size_bot; x--) {
        symbols[height - 2][x] = '#';
    }

    //int newKoefXBot, newKoefYBot;
    char key;

    Points posFood = food(length, height, snakeUser.segments, snakeUser.size, snakeBot.segments, snakeBot.size, { 0, 0 });
    Points posFoodBot = food(length, height, snakeUser.segments, snakeUser.size, snakeBot.segments, snakeBot.size, posFood);
    symbols[posFood.y][posFood.x] = '*';
    symbols[posFoodBot.y][posFoodBot.x] = '*';

    snakeBot.set_distance(posFoodBot);

    int speed = set_speed();
    system("cls");
    show_cursor(false);

    while (true) {
        setpos(0, 0);
        Sleep(speed);

        if (_kbhit()) {
            key = key_pressed();
            snakeUser.update_offset(key_handler(key), key_handler(key, 1));

        }
        snakeUser.update_position();
        // Выход за границы 
        snakeUser.check_out_of_borders();

        symbols[snakeUser.segments[0].y][snakeUser.segments[0].x] = ' ';
        symbols[snakeBot.segments[0].y][snakeBot.segments[0].x] = ' ';
        snakeUser.move();

        snakeBot.update_position();
        snakeBot.move();

        if (!snakeBot.check()) {
            snakeBot.def_direction();
            snakeBot.set_distance(posFoodBot);
        }
        else
            snakeBot.update_distance();

        symbols[snakeUser.pos.y][snakeUser.pos.x] = '#';
        symbols[snakeBot.pos.y][snakeBot.pos.x] = '#';

        if (!snakeUser.check()) {
            break;
        }
        // Если еда съедена, то боту рассчитываются шаги до новой точки
        int checkFoodBot = snakeBot.check_food(posFood, posFoodBot);
        if (checkFoodBot) {
            if (checkFoodBot == 1) {
                posFood = food(length, height, snakeUser.segments, snakeUser.size, snakeBot.segments, snakeBot.size, posFoodBot);
                symbols[posFood.y][posFood.x] = '*';
            }
            else {
                posFoodBot = food(length, height, snakeUser.segments, snakeUser.size, snakeBot.segments, snakeBot.size, posFood);
                symbols[posFoodBot.y][posFoodBot.x] = '*';
                // Расчет новой дистанции
                snakeBot.set_distance(posFoodBot);
            }
        }
        // Если еда съедена, то вставка нового сегмента в конец
        int checkFoodUser = snakeUser.check_food(posFood, posFoodBot);
        if (checkFoodUser) {
            if (checkFoodUser == 1) {
                posFood = food(length, height, snakeUser.segments, snakeUser.size, snakeBot.segments, snakeBot.size, posFoodBot);
                symbols[posFood.y][posFood.x] = '*';
            }
            else {
                posFoodBot = food(length, height, snakeUser.segments, snakeUser.size, snakeBot.segments, snakeBot.size, posFood);
                symbols[posFoodBot.y][posFoodBot.x] = '*';
                // Расчет новой дистанции
                snakeBot.set_distance(posFoodBot);
            }
        }
        //cout << snakeBot.size;
        // print
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < length; j++) {
                cout << symbols[i][j];
            }
            cout << "\n";
        }
        if (snakeUser.score == 50 || snakeBot.score == 50)
            break;
    }
    for (int i = 0; i < height; i++) {
        delete[] symbols[i];
    }
    delete[] symbols;

    char k = 'q';
    system("cls");
    show_cursor(true);
    cout << "Ваш счет >> " << snakeUser.score << " | Счет бота " << snakeBot.score << "\n";
    if (snakeUser.score > snakeBot.score)
        cout << "НАЙС НАЙС НАЙС УЛЬТРА ВЕРИ НАЙС";
    else if (snakeUser.score == snakeBot.score)
        cout << "Какая жалость";
    else
        cout << "Топ 8";
    cout << "\n\n" << "Для выхода нажмите q, в случае продолжения нажмите любую другую клавишу" << "\n\n";
    k = key_pressed();
    return (k != 'q');
}



bool check_food_in_snake(Points food, vector<Points>& snake, int size) {
    for (int i = 0; i < size; i++)
        if (snake[i] == food)
            return false;
    return true;
}

Points food(int l, int h, vector<Points>& snake1, int size1, vector<Points>& snake2, int size2, Points food2) {
    Points food;
    do {
        food = { random(l - 2, 1), random(h - 2, 1) };
    } while (!check_food_in_snake(food, snake1, size1) || !check_food_in_snake(food, snake2, size2) ||
        food == food2);
    return food;
}

int set_speed() {
    double x;
    cout << "Установка скорости 1 - 1000 -> "; cin >> x;
    cout << "\r";
    if (cin.fail()) {
        cin.clear();
        cin.ignore(32767, '\n');
    }
    else {
        if (int(x) - x == 0 && x >= 1 && x <= 1000) {
            return x;
        }
    }
    return set_speed();
}

int random(int n, int mod) {
    return mod + rand() % n;
}

// 1 - y, 0 - x
int key_handler(char key, int mod) {
    int x = 0, y = 0;
    switch (key) {
        case 'w':
            y = -1;
            break;
        case 'a':
            x = -1;
            break;
        case 's':
            y = 1;
            break;
        case 'd':
            x = 1;
            break;
        default:
            return 0;
    }
    if (mod) 
        return y;
    return x;
}

char key_pressed() {
    char key = _getch();
    return key;
}

int sign(int x) {
    if (x > 0)
        return 1;
    if (x < 0)
        return -1;
    return 0;
}

Points get_offset(int mod) {
    switch (mod) {
    case 0:
        return { 1, 0 };
    case 1:
        return { -1, 0 };
    case 2:
        return { 0, 1 };
    case 3:
        return { 0, -1 };
    }
}
