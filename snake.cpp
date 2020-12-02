#include <iostream>
#include <conio.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <Windows.h>
using namespace std;

const int height = 26, length = 52,
start_posx = 1, start_posy = 1,
snake_size = 8, snake_size_bot = snake_size;
int lvl = 3;

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

typedef vector<Points> ArrPoints;



class Snake {
public:
    int size, score = 0;
    ArrPoints segments;
    Points offset = { 1, 0 };
    Points pos;

    Snake(int);
    void start_position();
    void update_offset(int, int);
    void update_position();
    void check_out_of_borders();
    void move();
    int check_food(Points, Points);
    bool check();
    bool check_conflict(ArrPoints&, int);
};


class SnakeBot : public Snake {
private:
    ArrPoints snakeUser;
    int* snakeUserSize;

public:
    Points distanceToFood;
    Points distanceToFood2;

    SnakeBot(int, ArrPoints&, int*);
    void start_position();
    void set_distance(Points, Points);
    void update_position();
    void def_direction();
    void update_distance();
    bool check();
    float concentration_segments(int, int, int);
    bool check_segment_on_dist(int, int);
};



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
Points food(int, int, ArrPoints&, int, ArrPoints&, int, Points);                            // Координаты еды
bool check_food_in_snake(Points, ArrPoints&, int);                                          // Проверка на влезание еды в сегменты змей
int set_lvl();                                                                              // Установка уровня
bool game();
int sign(int);
Points get_offset(int);
Points min_point(Points, Points);



Snake::Snake(int size) {
    this->size = size;
    //start_position(start_pos_x, start_pos_y);
}

void Snake::start_position() {
    for (int x = start_posx; x < start_posx + snake_size; x++) {
        segments.push_back({ x, start_posy });
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

bool Snake::check() {
    for (int i = 0; i < size - 1; i++) {
        if (segments[size - 1] == segments[i]) {
            return false;
        }
    }
    return true;
}

bool Snake::check_conflict(ArrPoints& snake, int size) {
    for (int i = 0; i < size; ++i) {
        if (segments[this->size - 1] == snake[i]) {
            return false;
        }
    }
    return true;
}



SnakeBot::SnakeBot(int size, ArrPoints& snakeUser, int* snakeUserSize):Snake(size) {
    offset = { -1, 0 };
    this->snakeUser = snakeUser;
    this->snakeUserSize = snakeUserSize;
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

void SnakeBot::set_distance(Points food1, Points food2) {
    distanceToFood = food1 - pos;
    if (lvl > 2) {
        distanceToFood2 = food2 - pos;
        distanceToFood = min_point(distanceToFood, distanceToFood2);
    }
    //distanceToFood2 = food2 - pos;
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
    if (check() && check_segment_on_dist(distWithKoef * koefDistance, dir) &&
        (lvl > 1 || lvl == 1 && check_conflict(snakeUser, *snakeUserSize))) {
        pos += offset;
        return;
    }
    Points offsetBroken = offset;
 
    // Выбор оптимального пути путем подсчета количества сегментов в 4 областях относительно головы
    // движение туда, где наименьшая концентрация сегментов
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
    if (check() && (lvl > 1 || lvl == 1 && check_conflict(snakeUser, *snakeUserSize))) {
        pos += offset;
        return;
    }

    // Простая выборка направления
    for (int tempCounter = 0; tempCounter < 4; ++tempCounter) {
        offset = get_offset(tempCounter);
        if (tempCounter == indMinimum || offset == offsetBroken)
            continue;
        segments[size - 1] = pos + offset;

        if (check() && (lvl > 1 || lvl == 1 && check_conflict(snakeUser, *snakeUserSize))) {
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
    if (lvl == 1) {
        for (int i = 0; i < *snakeUserSize; ++i) {
            if (snakeUser[i] >= compareStart && snakeUser[i] <= compareEnd)
                ++amountSegments;
        }
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

    if (lim < 0) {
        for (int i = 0; i < size - 1; ++i) {
            if (segments[i] >= point && segments[i] <= segments[size - 1])
                return false;
        }
        if (lvl == 1)
            for (int i = 0; i < *snakeUserSize; ++i) {
                if (snakeUser[i] >= point && snakeUser[i] <= segments[size - 1])
                    return false;
            }
    }
    else {
        for (int i = 0; i < size - 1; ++i) {
            if (segments[i] <= point && segments[i] >= segments[size - 1])
                return false;
        }
        if (lvl == 1)
            for (int i = 0; i < *snakeUserSize; ++i) {
                if (snakeUser[i] <= point && snakeUser[i] >= segments[size - 1])
                return false;
            }
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
        gameCondition = game();
    }
    return 0;
}

bool game() {
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
    
    lvl = set_lvl();

    // Начальное определение змеи
    Snake snakeUser(snake_size);
    snakeUser.start_position();
    // Бот
    SnakeBot snakeBot(snake_size_bot, snakeUser.segments, &snakeUser.size);
    snakeBot.start_position();

    for (int x = start_posx; x < start_posx + snake_size; x++) {
        symbols[start_posy][x] = '#';
    }

    for (int x = length - 2; x > length - 2 - snake_size_bot; x--) {
        symbols[height - 2][x] = '#';
    }

    char key;
    bool is_conflict = false;

    Points posFood = food(length, height, snakeUser.segments, snakeUser.size, snakeBot.segments, snakeBot.size, { 0, 0 });
    Points posFoodBot = food(length, height, snakeUser.segments, snakeUser.size, snakeBot.segments, snakeBot.size, posFood);
    symbols[posFood.y][posFood.x] = '*';
    symbols[posFoodBot.y][posFoodBot.x] = '*';

    snakeBot.set_distance(posFoodBot, posFood);

    system("cls");
    show_cursor(false);

    while (true) {
        setpos(0, 0);
        Sleep(lvl);

        if (_kbhit()) {
            key = key_pressed();
            Points newOffset = { key_handler(key), key_handler(key, 1) };
            if (!(newOffset.x == -snakeUser.offset.x && newOffset.x != 0 || 
                newOffset.y == -snakeUser.offset.y && newOffset.y != 0))
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

        if (!snakeBot.check() || (lvl == 1 && !snakeBot.check_conflict(snakeUser.segments, snakeUser.size))) {
            snakeBot.def_direction();
            snakeBot.set_distance(posFoodBot, posFood);
        }
        else
            snakeBot.update_distance();

        symbols[snakeUser.pos.y][snakeUser.pos.x] = '#';
        symbols[snakeBot.pos.y][snakeBot.pos.x] = '#';

        if (!snakeUser.check() || !snakeUser.check_conflict(snakeBot.segments, snakeBot.size) || 
            (lvl > 1 && !snakeBot.check_conflict(snakeUser.segments, snakeUser.size))) {
            is_conflict = true;
            break;
        }

        // Если еда съедена, то боту рассчитываются шаги до новой точки
        int checkFoodUser = snakeUser.check_food(posFood, posFoodBot);
        int checkFoodBot = snakeBot.check_food(posFood, posFoodBot);
        if (checkFoodUser || checkFoodBot) {
            if (checkFoodBot == 1 || checkFoodUser == 1) {
                posFood = food(length, height, snakeUser.segments, snakeUser.size, snakeBot.segments, snakeBot.size, posFoodBot);
                symbols[posFood.y][posFood.x] = '*';
            }
            else if (checkFoodBot == 2 || checkFoodUser == 2){
                posFoodBot = food(length, height, snakeUser.segments, snakeUser.size, snakeBot.segments, snakeBot.size, posFood);
                symbols[posFoodBot.y][posFoodBot.x] = '*';
                // Расчет новой дистанции
                if (lvl <= 2)
                    snakeBot.set_distance(posFoodBot, posFood);
            }
            if (lvl > 2)
                snakeBot.set_distance(posFoodBot, posFood);
        }
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

    show_cursor(true);

    cout << "Ваш счет >> " << snakeUser.score << " | Счет бота >> " << snakeBot.score << "\n";
    if (is_conflict)
        cout << "Неудачно получилось";
    else {
        if (snakeUser.score > snakeBot.score)
            cout << "ПОБЕДА";
        else if (snakeUser.score == snakeBot.score)
            cout << "Почти победа";
        else
            cout << "Поражение";
    }
    cout << "\n\n" << "Для выхода нажмите ESC, в случае продолжения нажмите любую другую клавишу" << "\n\n";
    char k = key_pressed();
    return (k != 27);
}



bool check_food_in_snake(Points food, ArrPoints& snake, int size) {
    for (int i = 0; i < size; i++)
        if (snake[i] == food)
            return false;
    return true;
}

Points food(int l, int h, ArrPoints& snake1, int size1, ArrPoints& snake2, int size2, Points food2) {
    Points food;
    do {
        food = { random(l - 2, 1), random(h - 2, 1) };
    } while (!check_food_in_snake(food, snake1, size1) || !check_food_in_snake(food, snake2, size2) ||
        food == food2);
    return food;
}

int set_lvl() {
    double x;
    cout << "Установка уровня 1 (легкий) - 3 (сложный) -> "; cin >> x;
    cout << "\r";
    if (cin.fail()) {
        cin.clear();
        cin.ignore(32767, '\n');
    }
    else {
        if (int(x) - x == 0 && x >= 1 && x <= 3) {
            return x;
        }
    }
    return set_lvl();
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

Points min_point(Points point1, Points point2) {
    if ((abs(point1.x) + abs(point1.y)) < (abs(point2.x) + abs(point2.y)))
        return point1;
    return point2;
}
