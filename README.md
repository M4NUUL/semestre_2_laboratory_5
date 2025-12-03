classDiagram
    class Cell {
        -bool mined
        -bool open
        -bool flagged
        -int around
        +Cell()
    }

    class Game {
        -int W
        -int H
        -int MINES
        -bool gameOver
        -bool win
        -bool firstClick
        -bool explosion
        -float explosionTimer
        -vector~vector~Cell~~ field
        
        +Game(int w, int h, int mines)
        +resetField() void
        +generateMines(int safeX, int safeY) void
        +countMines(int x, int y) int
        +openCell(int x, int y) void
        +floodFill(int x, int y) void
        +toggleFlag(int x, int y) void
        +triggerExplosion(int x, int y) void
        +checkWin() void
    }

    class MinesweeperApp {
        -sf::RenderWindow window
        -sf::Font font
        -Game game
        -sf::Text status
        -sf::RectangleShape restartBtn
        -sf::Text restartText
        -int cellSize
        -int offsetY
        
        +main() int
        -chooseDifficulty(sf::RenderWindow& window) int
        -handleMouseClick(sf::Event::MouseButtonEvent mouse) void
        -renderGame() void
        -updateStatus() void
    }

    class SFML_Window {
        <<External>>
        +sf::RenderWindow
        +sf::Event
        +sf::Mouse
    }

    class SFML_Graphics {
        <<External>>
        +sf::Font
        +sf::Text
        +sf::RectangleShape
        +sf::Color
    }

    Game "1" --> "*" Cell : содержит
    MinesweeperApp "1" --> "1" Game : управляет
    MinesweeperApp --> SFML_Window : использует
    MinesweeperApp --> SFML_Graphics : использует
    Game --> MinesweeperApp : уведомляет о событиях
