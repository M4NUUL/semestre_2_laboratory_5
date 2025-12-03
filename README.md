**Диаграмма классов (Class Diagram)**

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
        -int CELL_SIZE
        -int OFFSET
        
        +main() int
        -chooseDifficulty(sf::RenderWindow& window) int
        -drawGame(sf::RenderWindow& window) void
        -handleEvents(sf::Event& event) void
    }

    class SFML_System {
        <<External>>
        +RenderWindow
        +Font
        +Text
        +RectangleShape
        +Color
        +Event
        +Mouse
    }

    Game "1" --> "*" Cell : contains
    MinesweeperApp "1" --> "1" Game : controls
    MinesweeperApp --> SFML_System : uses
    Game --> SFML_System : (indirect)


**Диаграмма последовательности - Основной цикл**
sequenceDiagram
    participant User
    participant App as MinesweeperApp
    participant Game
    participant CellGrid
    participant SFML

    User->>App: Запуск программы
    App->>SFML: Инициализация окна
    App->>SFML: Загрузка шрифта
    
    alt Выбор сложности
        App->>SFML: Показ меню выбора
        User->>App: Выбор уровня
        App->>Game: Создание игры(W, H, MINES)
        Game->>CellGrid: Инициализация поля
    end
    
    loop Основной игровой цикл
        User->>SFML: Взаимодействие с мышью
        SFML->>App: Обработка событий
        
        alt Клик по кнопке рестарта
            App->>Game: resetField()
            Game->>CellGrid: Сброс всех ячеек
        else Клик по игровому полю
            App->>Game: openCell(x, y)
            Game->>CellGrid: Получить ячейку
            CellGrid->>Game: Информация о ячейке
            
            alt Первый клик
                Game->>Game: generateMines(x, y)
                Game->>CellGrid: Расставить мины
                Game->>CellGrid: Вычислить around
            end
            
            alt Ячейка с миной
                Game->>Game: triggerExplosion(x, y)
                Game->>CellGrid: Открыть все ячейки
                Game->>App: gameOver = true
            else Ячейка пустая
                alt around == 0
                    Game->>Game: floodFill(x, y)
                    loop Рекурсивное открытие
                        Game->>CellGrid: Открыть соседей
                    end
                end
                Game->>Game: checkWin()
                alt Все безопасные открыты
                    Game->>App: win = true
                end
            end
        end
        
        App->>Game: Получить состояние поля
        Game->>App: Данные ячеек
        App->>SFML: Отрисовка интерфейса
        App->>SFML: Отрисовка игрового поля
        SFML->>User: Отображение обновленного состояния
    end
    
    User->>App: Закрытие окна
    App->>SFML: Закрытие окна
