#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>
#include <memory>
#include <cmath>
#include <cstdio>

// Вперёд объявляем Game, чтобы состояния могли на него ссылаться
class Game;

// Интерфейс состояния клетки (паттерн State)
class ICellState {
public:
    virtual ~ICellState() = default;
    virtual void onLeftClick(Game& game, int x, int y) = 0;
    virtual void onRightClick(Game& game, int x, int y) = 0;
    virtual bool isOpen() const = 0;
    virtual bool isFlagged() const = 0;
};

// Клетка поля
struct Cell {
    bool mined = false;
    int around = 0;
    std::unique_ptr<ICellState> state; // текущее состояние клетки
};

// Логика игры
class Game {
public:
    int W, H, MINES;

    bool gameOver = false;
    bool win = false;
    bool firstClick = true;

    bool explosion = false;
    float explosionTimer = 0.0f;

    // Таймер
    bool timerRunning = false;
    float timeElapsed = 0.0f; // время с первого клика

    std::vector<std::vector<Cell>> field;

    Game(int w=10, int h=10, int mines=10) : W(w), H(h), MINES(mines) {
        resetField();
    }

    void resetField();

    void updateTimer(float dt) {
        if (timerRunning && !gameOver && !win) {
            timeElapsed += dt;
        }
    }

    void startTimerIfNeeded() {
        if (!timerRunning) {
            timerRunning = true;
            timeElapsed = 0.0f;
        }
    }

    void stopTimer() { timerRunning = false; }

    // Генерация мин после первого клика (вокруг safeX/safeY гарантированно нет мин)
    void generateMines(int safeX, int safeY) {
        int placed = 0;
        std::srand((unsigned)std::time(nullptr));

        while (placed < MINES) {
            int x = std::rand() % W;
            int y = std::rand() % H;
            if (field[y][x].mined) continue;
            if (std::abs(x - safeX) <= 1 && std::abs(y - safeY) <= 1) continue;
            field[y][x].mined = true;
            placed++;
        }

        for (int y = 0; y < H; y++)
            for (int x = 0; x < W; x++)
                if (!field[y][x].mined)
                    field[y][x].around = countMines(x, y);
    }

    // Счёт мин вокруг клетки
    int countMines(int x, int y) {
        int cnt = 0;
        for (int dy = -1; dy <= 1; dy++)
            for (int dx = -1; dx <= 1; dx++)
                if (!(dx == 0 && dy == 0)) {
                    int nx = x + dx, ny = y + dy;
                    if (nx >= 0 && nx < W && ny >= 0 && ny < H)
                        if (field[ny][nx].mined) cnt++;
                }
        return cnt;
    }

    // Эти методы вызывают состояния
    void revealFromState(int x, int y);
    void toggleFlagFromState(int x, int y);

    // Раскрытие нулевой области
    void floodFill(int x, int y) {
        for (int dy = -1; dy <= 1; dy++)
            for (int dx = -1; dx <= 1; dx++)
                if (!(dx == 0 && dy == 0)) {
                    int nx = x + dx, ny = y + dy;
                    if (nx >= 0 && nx < W && ny >= 0 && ny < H) {
                        Cell &c = field[ny][nx];
                        if (!c.state->isOpen() && !c.mined) {
                            c.state = makeOpenedState();
                            if (c.around == 0)
                                floodFill(nx, ny);
                        }
                    }
                }
    }

    // Проигрыш: открываем всё поле и фиксируем конец
    void triggerExplosion(int, int) {
        explosion = true;
        explosionTimer = 0.2f;

        for (int yy = 0; yy < H; yy++)
            for (int xx = 0; xx < W; xx++)
                field[yy][xx].state = makeOpenedState();

        gameOver = true;
        stopTimer();
    }

    // Победа, если открыты все неминированные клетки
    void checkWinOpen() {
        for (int y = 0; y < H; y++)
            for (int x = 0; x < W; x++)
                if (!field[y][x].mined && !field[y][x].state->isOpen())
                    return;
        win = true;
        stopTimer();
    }

    // Победа, если все мины отмечены флагами и нет ошибок
    void checkWinFlags() {
        int flagged = 0;
        for (int y = 0; y < H; y++) {
            for (int x = 0; x < W; x++) {
                bool isF = field[y][x].state->isFlagged();
                if (isF) {
                    flagged++;
                    if (!field[y][x].mined) return; // флаг на пустой клетке
                }
            }
        }
        if (flagged == MINES) {
            win = true;
            stopTimer();
        }
    }

    void checkWin() {
        checkWinOpen();
        if (!win) checkWinFlags();
    }

    // Количество выставленных флагов
    int flagsCount() const {
        int f = 0;
        for (int y = 0; y < H; y++)
            for (int x = 0; x < W; x++)
                if (field[y][x].state->isFlagged())
                    f++;
        return f;
    }

    // Фабрики состояний (чтобы переключение было совсем очевидным)
    static std::unique_ptr<ICellState> makeClosedState();
    static std::unique_ptr<ICellState> makeOpenedState();
    static std::unique_ptr<ICellState> makeFlaggedState();
};

// Конкретные состояния клетки

class ClosedState : public ICellState {
public:
    void onLeftClick(Game& game, int x, int y) override {
        game.revealFromState(x, y);
    }
    void onRightClick(Game& game, int x, int y) override {
        game.toggleFlagFromState(x, y);
    }
    bool isOpen() const override { return false; }
    bool isFlagged() const override { return false; }
};

class OpenedState : public ICellState {
public:
    void onLeftClick(Game&, int, int) override {}
    void onRightClick(Game&, int, int) override {}
    bool isOpen() const override { return true; }
    bool isFlagged() const override { return false; }
};

class FlaggedState : public ICellState {
public:
    void onLeftClick(Game&, int, int) override {}
    void onRightClick(Game& game, int x, int y) override {
        game.field[y][x].state = Game::makeClosedState(); // снять флаг
    }
    bool isOpen() const override { return false; }
    bool isFlagged() const override { return true; }
};

// Реализация фабрик состояний
std::unique_ptr<ICellState> Game::makeClosedState()  { return std::make_unique<ClosedState>(); }
std::unique_ptr<ICellState> Game::makeOpenedState()  { return std::make_unique<OpenedState>(); }
std::unique_ptr<ICellState> Game::makeFlaggedState() { return std::make_unique<FlaggedState>(); }

// Очистка поля без копирования unique_ptr
void Game::resetField() {
    field.clear();
    field.resize(H);
    for (int y = 0; y < H; ++y) {
        field[y].resize(W);
        for (int x = 0; x < W; ++x) {
            field[y][x].mined = false;
            field[y][x].around = 0;
            field[y][x].state = makeClosedState();
        }
    }

    gameOver = false;
    win = false;
    firstClick = true;
    explosion = false;
    explosionTimer = 0.0f;

    timerRunning = false;
    timeElapsed = 0.0f;
}

// Открытие клетки из состояния ClosedState
void Game::revealFromState(int x, int y) {
    if (gameOver || win) return;
    Cell &c = field[y][x];

    if (c.state->isOpen() || c.state->isFlagged()) return;

    if (firstClick) {
        generateMines(x, y);
        firstClick = false;
        startTimerIfNeeded();
    }

    c.state = makeOpenedState();

    if (c.mined) {
        triggerExplosion(x, y);
        return;
    }

    if (c.around == 0)
        floodFill(x, y);

    checkWin();
}

// Переключение флага из состояния ClosedState / FlaggedState
void Game::toggleFlagFromState(int x, int y) {
    if (gameOver || win) return;
    Cell &c = field[y][x];

    if (c.state->isOpen()) return;

    if (c.state->isFlagged())
        c.state = makeClosedState();
    else
        c.state = makeFlaggedState();

    checkWin();
}

// Глобальный шрифт
sf::Font font;

// Меню выбора сложности
int chooseDifficulty(sf::RenderWindow &window) {
    sf::Text title("Select Difficulty", font, 40);
    title.setFillColor(sf::Color::Black);
    title.setPosition(120, 50);

    sf::RectangleShape easyBtn(sf::Vector2f(150, 50));
    easyBtn.setFillColor(sf::Color(150, 250, 150));
    easyBtn.setPosition(200, 150);
    sf::Text easyText("Easy", font, 30);
    easyText.setPosition(250, 155);
    easyText.setFillColor(sf::Color::Black);

    sf::RectangleShape normalBtn(sf::Vector2f(150, 50));
    normalBtn.setFillColor(sf::Color(250, 250, 150));
    normalBtn.setPosition(200, 250);
    sf::Text normalText("Normal", font, 30);
    normalText.setPosition(235, 255);
    normalText.setFillColor(sf::Color::Black);

    sf::RectangleShape hardBtn(sf::Vector2f(150, 50));
    hardBtn.setFillColor(sf::Color(250, 150, 150));
    hardBtn.setPosition(200, 350);
    sf::Text hardText("Hard", font, 30);
    hardText.setPosition(260, 355);
    hardText.setFillColor(sf::Color::Black);

    while (true) {
        sf::Event e;
        while (window.pollEvent(e)) {
            if (e.type == sf::Event::Closed) return 0;
            if (e.type == sf::Event::MouseButtonPressed) {
                int mx = e.mouseButton.x;
                int my = e.mouseButton.y;
                if (easyBtn.getGlobalBounds().contains(mx, my)) return 1;
                if (normalBtn.getGlobalBounds().contains(mx, my)) return 2;
                if (hardBtn.getGlobalBounds().contains(mx, my)) return 3;
            }
        }
        window.clear(sf::Color::White);
        window.draw(title);
        window.draw(easyBtn);
        window.draw(easyText);
        window.draw(normalBtn);
        window.draw(normalText);
        window.draw(hardBtn);
        window.draw(hardText);
        window.display();
    }
}

// Формат времени mm:ss
static std::string formatTime(float sec) {
    int total = (int)sec;
    int m = total / 60;
    int s = total % 60;
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%02d:%02d", m, s);
    return std::string(buf);
}

int main() {
    // Если не Linux — поменяй путь или положи ttf рядом с exe
    if (!font.loadFromFile("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf")) {
        printf("Не удалось загрузить шрифт!\n");
        return -1;
    }

    const int WINDOW_W = 800;
    const int WINDOW_H = 800;

    sf::RenderWindow window(sf::VideoMode(WINDOW_W, WINDOW_H), "Minesweeper");

    auto startGameByChoice = [&](int choice) -> Game {
        int W, H, MINES;
        switch(choice) {
            case 1: W=10; H=10; MINES=10; break;
            case 2: W=14; H=14; MINES=20; break;
            case 3: W=20; H=20; MINES=40; break;
            default: W=10; H=10; MINES=10; break;
        }
        return Game(W, H, MINES);
    };

    int choice = chooseDifficulty(window);
    Game game = startGameByChoice(choice);

    const int CELL = 40;
    const int OFFSET = 110; // высота шапки

    // Параметры расположения поля (обновляются при смене сложности)
    int boardWidthPx  = 0;
    int boardHeightPx = 0;
    int XOFFSET       = 0;

    auto recomputeBoardLayout = [&]() {
        boardWidthPx  = game.W * CELL;
        boardHeightPx = game.H * CELL;
        XOFFSET = (WINDOW_W - boardWidthPx) / 2;
        if (XOFFSET < 0) XOFFSET = 0;
    };

    recomputeBoardLayout();

    sf::Text status("", font, 40);
    status.setFillColor(sf::Color::Red);
    status.setPosition(180, 5);

    // Кнопка Restart
    sf::RectangleShape restartBtn(sf::Vector2f(150, 40));
    restartBtn.setFillColor(sf::Color(200, 200, 200));
    restartBtn.setPosition(600, 10);

    sf::Text restartText("Restart", font, 20);
    restartText.setFillColor(sf::Color::Black);
    restartText.setPosition(630, 15);

    // Кнопка Menu
    sf::RectangleShape menuBtn(sf::Vector2f(150, 40));
    menuBtn.setFillColor(sf::Color(200, 200, 200));
    menuBtn.setPosition(440, 10);

    sf::Text menuText("Menu", font, 20);
    menuText.setFillColor(sf::Color::Black);
    menuText.setPosition(485, 15);

    // Счётчик мин (под кнопками)
    sf::Text minesIndicator("", font, 22);
    minesIndicator.setFillColor(sf::Color::Black);
    minesIndicator.setPosition(440, 60);

    // Таймер (под кнопками)
    sf::Text timerText("", font, 22);
    timerText.setFillColor(sf::Color::Black);
    timerText.setPosition(440, 82);

    sf::Clock frameClock;

    while (window.isOpen()) {
        float dt = frameClock.restart().asSeconds();

        if (game.explosion) {
            game.explosionTimer -= dt;
            if (game.explosionTimer < 0) game.explosion = false;
        }

        game.updateTimer(dt);

        sf::Event e;
        while (window.pollEvent(e)) {
            if (e.type == sf::Event::Closed) window.close();

            if (e.type == sf::Event::MouseButtonPressed) {
                int mx = e.mouseButton.x;
                int my = e.mouseButton.y;

                if (restartBtn.getGlobalBounds().contains(mx, my)) {
                    game.resetField();
                    recomputeBoardLayout();
                    continue;
                }

                if (menuBtn.getGlobalBounds().contains(mx, my)) {
                    int newChoice = chooseDifficulty(window);
                    game = startGameByChoice(newChoice);
                    recomputeBoardLayout();
                    continue;
                }

                if (my > OFFSET) {
                    // клик должен быть в зоне поля
                    if (mx < XOFFSET || mx >= XOFFSET + boardWidthPx) continue;

                    int x = (mx - XOFFSET) / CELL;
                    int y = (my - OFFSET) / CELL;

                    if (x >= 0 && x < game.W && y >= 0 && y < game.H) {
                        if (e.mouseButton.button == sf::Mouse::Left)
                            game.field[y][x].state->onLeftClick(game, x, y);
                        else if (e.mouseButton.button == sf::Mouse::Right)
                            game.field[y][x].state->onRightClick(game, x, y);
                    }
                }
            }
        }

        if (game.win) status.setString("YOU WIN!");
        else if (game.gameOver) status.setString("YOU LOSE!");
        else status.setString("");

        int remaining = game.MINES - game.flagsCount();
        minesIndicator.setString("Mines: " + std::to_string(remaining));
        timerText.setString("Time: " + formatTime(game.timeElapsed));

        window.clear(sf::Color::White);

        for (int y = 0; y < game.H; y++) {
            for (int x = 0; x < game.W; x++) {
                sf::RectangleShape r(sf::Vector2f(CELL - 2, CELL - 2));
                r.setPosition(XOFFSET + x * CELL + 1, OFFSET + y * CELL + 1);

                const Cell &c = game.field[y][x];

                if (c.state->isOpen()) {
                    r.setFillColor(c.mined
                        ? (game.explosion ? sf::Color::Yellow : sf::Color::Red)
                        : sf::Color(220, 220, 220));
                    window.draw(r);

                    if (!c.mined && c.around > 0) {
                        sf::Text t;
                        t.setFont(font);
                        t.setString(std::to_string(c.around));
                        t.setCharacterSize(24);
                        t.setFillColor(sf::Color::Blue);
                        t.setPosition(XOFFSET + x * CELL + 10, OFFSET + y * CELL + 5);
                        window.draw(t);
                    }
                } else {
                    r.setFillColor(c.state->isFlagged()
                        ? sf::Color(255, 255, 0)
                        : sf::Color(100, 100, 100));
                    window.draw(r);

                    if (c.state->isFlagged()) {
                        sf::Text f;
                        f.setFont(font);
                        f.setString("F");
                        f.setCharacterSize(24);
                        f.setFillColor(sf::Color::Red);
                        f.setPosition(XOFFSET + x * CELL + 10, OFFSET + y * CELL + 3);
                        window.draw(f);
                    }
                }
            }
        }

        window.draw(restartBtn);
        window.draw(restartText);
        window.draw(menuBtn);
        window.draw(menuText);
        window.draw(status);
        window.draw(minesIndicator);
        window.draw(timerText);

        window.display();
    }

    return 0;
}
