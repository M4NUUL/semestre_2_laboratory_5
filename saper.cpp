#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>

struct Cell {
    bool mined = false;
    bool open = false;
    bool flagged = false;
    int around = 0;
};

class Game {
public:
    int W, H, MINES;

    bool gameOver = false;
    bool win = false;
    bool firstClick = true;
    bool explosion = false;
    float explosionTimer = 0.0;

    std::vector<std::vector<Cell>> field;

    Game(int w=10, int h=10, int mines=10) : W(w), H(h), MINES(mines) {
        resetField();
    }

    void resetField() {
        field = std::vector<std::vector<Cell>>(H, std::vector<Cell>(W));
        gameOver = false;
        win = false;
        firstClick = true;
        explosion = false;
        explosionTimer = 0.0;
    }

    void generateMines(int safeX, int safeY) {
        int placed = 0;
        srand(time(NULL));
        while (placed < MINES) {
            int x = rand() % W;
            int y = rand() % H;
            if (field[y][x].mined) continue;
            if (abs(x - safeX) <= 1 && abs(y - safeY) <= 1) continue;
            field[y][x].mined = true;
            placed++;
        }

        for (int y = 0; y < H; y++)
            for (int x = 0; x < W; x++)
                if (!field[y][x].mined)
                    field[y][x].around = countMines(x, y);
    }

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

    void openCell(int x, int y) {
        if (gameOver || win) return;
        Cell &c = field[y][x];
        if (c.open || c.flagged) return;

        if (firstClick) {
            generateMines(x, y);
            firstClick = false;
        }

        c.open = true;

        if (c.mined) {
            triggerExplosion(x, y);
            return;
        }

        if (c.around == 0)
            floodFill(x, y);

        checkWin();
    }

    void floodFill(int x, int y) {
        for (int dy = -1; dy <= 1; dy++)
            for (int dx = -1; dx <= 1; dx++)
                if (!(dx == 0 && dy == 0)) {
                    int nx = x + dx, ny = y + dy;
                    if (nx >= 0 && nx < W && ny >= 0 && ny < H) {
                        Cell &c = field[ny][nx];
                        if (!c.open && !c.mined) {
                            c.open = true;
                            if (c.around == 0)
                                floodFill(nx, ny);
                        }
                    }
                }
    }

    void toggleFlag(int x, int y) {
        if (gameOver || win) return;
        Cell &c = field[y][x];
        if (!c.open) c.flagged = !c.flagged;
    }

    void triggerExplosion(int x, int y) {
        explosion = true;
        explosionTimer = 0.2f;
        field[y][x].open = true;

        for (int yy = 0; yy < H; yy++)
            for (int xx = 0; xx < W; xx++)
                field[yy][xx].open = true;

        gameOver = true;
    }

    void checkWin() {
        for (int y = 0; y < H; y++)
            for (int x = 0; x < W; x++)
                if (!field[y][x].mined && !field[y][x].open)
                    return;
        win = true;
    }
};

// глобальный шрифт
sf::Font font;

// функция выбора уровня
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

int main() {
    if (!font.loadFromFile("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf")) {
        printf("Не удалось загрузить шрифт!\n");
        return -1;
    }

    sf::RenderWindow window(sf::VideoMode(800, 800), "Minesweeper");

    int choice = chooseDifficulty(window);
    int W, H, MINES;
    switch(choice) {
        case 1: W=10; H=10; MINES=10; break;
        case 2: W=14; H=14; MINES=20; break;
        case 3: W=20; H=20; MINES=40; break;
        default: W=10; H=10; MINES=10; break;
    }

    Game game(W,H,MINES);

    const int CELL = 40;
    const int OFFSET = 50;

    sf::Text status("", font, 40);
    status.setFillColor(sf::Color::Red);
    status.setPosition(180, 5);

    sf::RectangleShape restartBtn(sf::Vector2f(150, 40));
    restartBtn.setFillColor(sf::Color(200, 200, 200));
    restartBtn.setPosition(600, 10);

    sf::Text restartText("Restart", font, 20);
    restartText.setFillColor(sf::Color::Black);
    restartText.setPosition(630, 15);

    sf::Clock clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        if (game.explosion) {
            game.explosionTimer -= dt;
            if (game.explosionTimer < 0) game.explosion = false;
        }

        sf::Event e;
        while (window.pollEvent(e)) {
            if (e.type == sf::Event::Closed) window.close();

            if (e.type == sf::Event::MouseButtonPressed) {
                int mx = e.mouseButton.x;
                int my = e.mouseButton.y;

                if (restartBtn.getGlobalBounds().contains(mx, my)) {
                    game.resetField();
                    continue;
                }

                if (my > OFFSET) {
                    int x = mx / CELL;
                    int y = (my - OFFSET) / CELL;
                    if (x >= 0 && x < game.W && y >= 0 && y < game.H) {
                        if (e.mouseButton.button == sf::Mouse::Left)
                            game.openCell(x, y);
                        else if (e.mouseButton.button == sf::Mouse::Right)
                            game.toggleFlag(x, y);
                    }
                }
            }
        }

        if (game.win) status.setString("YOU WIN!");
        else if (game.gameOver) status.setString("YOU LOSE!");
        else status.setString("");

        window.clear(sf::Color::White);

        for (int y = 0; y < game.H; y++) {
            for (int x = 0; x < game.W; x++) {
                sf::RectangleShape r(sf::Vector2f(CELL - 2, CELL - 2));
                r.setPosition(x * CELL + 1, y * CELL + OFFSET + 1);

                const Cell &c = game.field[y][x];

                if (c.open) {
                    r.setFillColor(c.mined ? (game.explosion ? sf::Color::Yellow : sf::Color::Red)
                                            : sf::Color(220, 220, 220));
                    window.draw(r);

                    if (!c.mined && c.around > 0) {
                        sf::Text t;
                        t.setFont(font);
                        t.setString(std::to_string(c.around));
                        t.setCharacterSize(24);
                        t.setFillColor(sf::Color::Blue);
                        t.setPosition(x * CELL + 10, y * CELL + OFFSET + 5);
                        window.draw(t);
                    }
                } else {
                    r.setFillColor(c.flagged ? sf::Color(255, 255, 0) : sf::Color(100, 100, 100));
                    window.draw(r);

                    if (c.flagged) {
                        sf::Text f;
                        f.setFont(font);
                        f.setString("F");
                        f.setCharacterSize(24);
                        f.setFillColor(sf::Color::Red);
                        f.setPosition(x * CELL + 10, y * CELL + OFFSET + 3);
                        window.draw(f);
                    }
                }
            }
        }

        window.draw(restartBtn);
        window.draw(restartText);
        window.draw(status);
        window.display();
    }

    return 0;
}
