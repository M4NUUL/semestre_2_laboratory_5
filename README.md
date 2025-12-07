@startuml
skinparam backgroundColor #ffffff
skinparam classAttributeIconSize 0

class Game {
  -int W
  -int H
  -int MINES
  -bool gameOver
  -bool win
  -bool firstClick
  -bool explosion
  -float explosionTimer
  -bool timerRunning
  -float timeElapsed
  -vector<vector<Cell>> field

  +resetField()
  +revealFromState(x, y)
  +toggleFlagFromState(x, y)
  +generateMines(safeX, safeY)
  +countMines(x, y)
  +floodFill(x, y)
  +triggerExplosion(x, y)
  +checkWin()
}

class Cell {
  -bool mined
  -int around
  -ICellState state
}

Game "1" --> "W*H" Cell : содержит

interface ICellState {
  +onLeftClick(Game, x, y)
  +onRightClick(Game, x, y)
  +isOpen() : bool
  +isFlagged() : bool
}

class ClosedState
class OpenedState
class FlaggedState

ICellState <|-- ClosedState
ICellState <|-- OpenedState
ICellState <|-- FlaggedState

Cell --> ICellState : хранит состояние

class UI {
  -sf::RenderWindow window
  -sf::Font font
  -Game game

  +chooseDifficulty()
  +handleMouse()
  +render()
}

UI --> Game : вызывает методы игры
UI --> "SFML" : использует графику/ввод

@enduml
