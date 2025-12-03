classDiagram
    class Cell {
        -bool mined
        -bool open
        -bool flagged
        -int around
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
        
        +Game(w, h, mines)
        +resetField()
        +generateMines(safeX, safeY)
        +countMines(x, y) int
        +openCell(x, y)
        +floodFill(x, y)
        +toggleFlag(x, y)
        +triggerExplosion(x, y)
        +checkWin()
    }

    class MinesweeperApp {
        -sf::RenderWindow window
        -sf::Font font
        -Game game
        -sf::Text status
        -sf::RectangleShape restartBtn
        -sf::Text restartText
        
        +main()
        +chooseDifficulty() int
    }

    Game "1" -- "*" Cell : contains
    MinesweeperApp "1" -- "1" Game : controls