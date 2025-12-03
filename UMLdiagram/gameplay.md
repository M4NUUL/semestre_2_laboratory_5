sequenceDiagram
    participant M as MinesweeperApp
    participant G as Game
    participant C as Cell
    participant SFML as SFML System
    
    M->>SFML: Инициализация окна
    M->>M: chooseDifficulty()
    M->>G: Создание игры (W, H, MINES)
    G->>G: resetField()
    
    loop Игровой цикл
        M->>SFML: Обработка событий
        alt Нажата кнопка рестарта
            M->>G: resetField()
        else Нажата ячейка ЛКМ
            M->>G: openCell(x, y)
            G->>G: firstClick?
            alt Первый клик
                G->>G: generateMines(x, y)
                G->>G: countMines() для всех ячеек
            end
            G->>C: Проверка mined
            alt Есть мина
                G->>G: triggerExplosion(x, y)
                G->>G: Открыть все ячейки
                G->>G: gameOver = true
            else Нет мины
                G->>G: floodFill(x, y)
                G->>G: checkWin()
            end
        else Нажата ячейка ПКМ
            M->>G: toggleFlag(x, y)
        end
        
        M->>SFML: Отрисовка
        M->>G: Получение состояния ячеек
        M->>SFML: Отрисовка ячеек
        M->>G: Получение статуса игры
        M->>SFML: Отрисовка статуса
    end