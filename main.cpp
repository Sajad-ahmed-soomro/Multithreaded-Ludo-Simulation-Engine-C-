#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <random>
#include <pthread.h>
#include <cstring>
#include <unistd.h>  // For sleep() and usleep()
// Constants for board visualization
const int BOARD_SIZE = 15;
const float CELL_SIZE = 40.0f;
const float BOARD_DIMENSION = BOARD_SIZE * CELL_SIZE;
pthread_mutex_t diceLock;
pthread_mutex_t gridLock;

int diceValue = 0; // Shared resource for dice value
bool gameRunning = true; // To control the game state

// Colors for the four players
sf::Color playerColors[] = { sf::Color::Red, sf::Color::Green, sf::Color::Blue, sf::Color::Yellow };

// Global shared resources
struct GameState {
    int board[15][15];  // Grid representation
    bool diceInUse;     // Flag for dice availability
    pthread_mutex_t boardMutex;
    pthread_mutex_t diceMutex;
    pthread_cond_t diceCond;
    bool gameRunning;   // Flag to control thread execution
    int currentPlayer;  // Track whose turn it is
} gameState;

struct Parameters {
    int playerId;
    int hitRecord;
};
sf::Vector2f player1Tokens[4] = {
    {CELL_SIZE, CELL_SIZE},
    {2 * CELL_SIZE, CELL_SIZE},
    {CELL_SIZE, 2 * CELL_SIZE},
    {2 * CELL_SIZE, 2 * CELL_SIZE}
};


pthread_mutex_t turnMutex = PTHREAD_MUTEX_INITIALIZER;

// Function declarations
void drawGrid(sf::RenderWindow& window);
void drawHomeArea(sf::RenderWindow& window, float x, float y, sf::Color color);
void drawStartingGrid(sf::RenderWindow& window, float x, float y, sf::Color outlineColor);
void drawColoredPaths(sf::RenderWindow& window);
void drawSafeCells(sf::RenderWindow& window);
void drawCenterStar(sf::RenderWindow& window);

void drawGrid(sf::RenderWindow& window) {
    sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            cell.setPosition(sf::Vector2f(i * CELL_SIZE, j * CELL_SIZE));
            cell.setFillColor(sf::Color::White);
            cell.setOutlineColor(sf::Color::Black);
            cell.setOutlineThickness(1);
            window.draw(cell);
        }
    }
}

void drawHomeArea(sf::RenderWindow& window, float x, float y, sf::Color color) {
    sf::RectangleShape homeArea(sf::Vector2f(CELL_SIZE * 6, CELL_SIZE * 6));
    homeArea.setPosition(sf::Vector2f(x, y));
    homeArea.setFillColor(color);
    window.draw(homeArea);
}

void drawStartingGrid(sf::RenderWindow& window, float x, float y, sf::Color outlineColor) {
    sf::RectangleShape smallCell(sf::Vector2f(CELL_SIZE * 2, CELL_SIZE * 2));
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            smallCell.setFillColor(sf::Color::White);
            smallCell.setOutlineColor(outlineColor);
            smallCell.setOutlineThickness(2);
            smallCell.setPosition(sf::Vector2f(x + i * 2 * CELL_SIZE, y + j * 2 * CELL_SIZE));
            window.draw(smallCell);
        }
    }
}

void drawColoredPaths(sf::RenderWindow& window) {
    sf::RectangleShape pathCell(sf::Vector2f(CELL_SIZE, CELL_SIZE));

    // Yellow path 
    for (int i = 1; i <= 5; ++i) {
        pathCell.setFillColor(playerColors[3]);  // Yellow
        pathCell.setPosition(sf::Vector2f(7 * CELL_SIZE, i * CELL_SIZE));
        window.draw(pathCell);
    }

    // Red path 
    for (int i = 1; i <= 5; ++i) {
        pathCell.setFillColor(playerColors[0]);  // Red
        pathCell.setPosition(sf::Vector2f(i * CELL_SIZE, 7 * CELL_SIZE));
        window.draw(pathCell);
    }

    // Blue path 
    for (int i = 9; i <= 13; ++i) {
        pathCell.setFillColor(playerColors[2]);  // Blue
        pathCell.setPosition(sf::Vector2f(7 * CELL_SIZE, i * CELL_SIZE));
        window.draw(pathCell);
    }

    // Green path 
    for (int i = 9; i <= 13; ++i) {
        pathCell.setFillColor(playerColors[1]);  // Green
        pathCell.setPosition(sf::Vector2f(i * CELL_SIZE, 7 * CELL_SIZE));
        window.draw(pathCell);
    }
}

void drawSafeCells(sf::RenderWindow& window) {
    sf::RectangleShape safeCell(sf::Vector2f(CELL_SIZE, CELL_SIZE));

    // Yellow: Start (8,1), Safe (12,6)
    safeCell.setFillColor(sf::Color(255, 255, 153)); // Light Yellow
    safeCell.setOutlineColor(sf::Color::Black);
    safeCell.setOutlineThickness(2);
    safeCell.setPosition(sf::Vector2f(8 * CELL_SIZE, 1 * CELL_SIZE));
    window.draw(safeCell);
    safeCell.setPosition(sf::Vector2f(12 * CELL_SIZE, 6 * CELL_SIZE));
    window.draw(safeCell);

    // Red: Start (1,6), Safe (6,2)
    safeCell.setFillColor(sf::Color(255, 153, 153)); // Light Red
    safeCell.setPosition(sf::Vector2f(1 * CELL_SIZE, 6 * CELL_SIZE));
    window.draw(safeCell);
    safeCell.setPosition(sf::Vector2f(6 * CELL_SIZE, 2 * CELL_SIZE));
    window.draw(safeCell);

    // Blue: Start (6,13), Safe (2,8)
    safeCell.setFillColor(sf::Color(153, 204, 255)); // Light Blue
    safeCell.setPosition(sf::Vector2f(6 * CELL_SIZE, 13 * CELL_SIZE));
    window.draw(safeCell);
    safeCell.setPosition(sf::Vector2f(2 * CELL_SIZE, 8 * CELL_SIZE));
    window.draw(safeCell);

    // Green: Start (13,8), Safe (8,12)
    safeCell.setFillColor(sf::Color(153, 255, 153)); // Light Green
    safeCell.setPosition(sf::Vector2f(13 * CELL_SIZE, 8 * CELL_SIZE));
    window.draw(safeCell);
    safeCell.setPosition(sf::Vector2f(8 * CELL_SIZE, 12 * CELL_SIZE));
    window.draw(safeCell);
}

void drawCenterStar(sf::RenderWindow& window) {
    sf::ConvexShape triangle;
    triangle.setPointCount(3);

    // Top triangle (Yellow)
    triangle.setPoint(0, sf::Vector2f(6 * CELL_SIZE, 6 * CELL_SIZE));
    triangle.setPoint(1, sf::Vector2f(9 * CELL_SIZE, 6 * CELL_SIZE));
    triangle.setPoint(2, sf::Vector2f(7.5 * CELL_SIZE, 7.5 * CELL_SIZE));
    triangle.setFillColor(playerColors[3]);
    window.draw(triangle);

    // Bottom triangle (Blue)
    triangle.setPoint(0, sf::Vector2f(6 * CELL_SIZE, 9 * CELL_SIZE));
    triangle.setPoint(1, sf::Vector2f(9 * CELL_SIZE, 9 * CELL_SIZE));
    triangle.setPoint(2, sf::Vector2f(7.5 * CELL_SIZE, 7.5 * CELL_SIZE));
    triangle.setFillColor(playerColors[2]);
    window.draw(triangle);

    // Left triangle (Red)
    triangle.setPoint(0, sf::Vector2f(6 * CELL_SIZE, 6 * CELL_SIZE));
    triangle.setPoint(1, sf::Vector2f(6 * CELL_SIZE, 9 * CELL_SIZE));
    triangle.setPoint(2, sf::Vector2f(7.5 * CELL_SIZE, 7.5 * CELL_SIZE));
    triangle.setFillColor(playerColors[0]);
    window.draw(triangle);

    // Right triangle (Green)
    triangle.setPoint(0, sf::Vector2f(9 * CELL_SIZE, 6 * CELL_SIZE));
    triangle.setPoint(1, sf::Vector2f(9 * CELL_SIZE, 9 * CELL_SIZE));
    triangle.setPoint(2, sf::Vector2f(7.5 * CELL_SIZE, 7.5 * CELL_SIZE));
    triangle.setFillColor(playerColors[1]);
    window.draw(triangle);
}

// Thread-safe dice rolling function
int rollDice(int playerId) {
    pthread_mutex_lock(&gameState.diceMutex);

    // Only allow rolling if it's the player's turn
    if (gameState.currentPlayer != playerId) {
        pthread_mutex_unlock(&gameState.diceMutex);
        return 0;
    }

    while (gameState.diceInUse) {
        pthread_cond_wait(&gameState.diceCond, &gameState.diceMutex);
    }
    gameState.diceInUse = true;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 6);
    int result = dis(gen);

    std::cout << "Player " << playerId << " rolled: " << result << std::endl;

    gameState.diceInUse = false;
    pthread_cond_signal(&gameState.diceCond);
    pthread_mutex_unlock(&gameState.diceMutex);
    return result;
}

void movePlayer(int playerId, int diceRoll) {
    pthread_mutex_lock(&gameState.boardMutex);

    // Validate input parameters
    if (playerId < 0 || playerId >= 4 || diceRoll < 1 || diceRoll > 6) {
        pthread_mutex_unlock(&gameState.boardMutex);
        return;
    }

    // Find current position
    int playerPosX = -1, playerPosY = -1;
    bool foundToken = false;

    for (int i = 0; i < BOARD_SIZE && !foundToken; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            if (gameState.board[i][j] == playerId + 1) {
                playerPosX = i;
                playerPosY = j;
                foundToken = true;
                break;
            }
        }
    }

    if (!foundToken) {
        pthread_mutex_unlock(&gameState.boardMutex);
        return;
    }

    // Calculate new position (improved path logic)
    int newPosX = playerPosX;
    int newPosY = playerPosY;

    // Simple movement logic (can be expanded for actual Ludo rules)
    if (playerPosX == 7) {  // Moving vertically
        newPosY = (playerPosY + diceRoll) % BOARD_SIZE;
    }
    else {  // Moving horizontally
        newPosX = (playerPosX + diceRoll) % BOARD_SIZE;
    }

    // Check if new position is valid and unoccupied
    if (newPosX >= 0 && newPosX < BOARD_SIZE &&
        newPosY >= 0 && newPosY < BOARD_SIZE &&
        gameState.board[newPosX][newPosY] == 0) {

        gameState.board[playerPosX][playerPosY] = 0;
        gameState.board[newPosX][newPosY] = playerId + 1;
    }

    pthread_mutex_unlock(&gameState.boardMutex);
}

void* masterThread(void* arg) {
    while (gameState.gameRunning) {
        pthread_mutex_lock(&gameState.boardMutex);
        // Monitor game state
        usleep(100000);  // 100ms sleep
        pthread_mutex_unlock(&gameState.boardMutex);
    }
    return nullptr;
}

void moveToken(int tokenIndex, int steps) {
    // Example: Move token in a straight line (modify as per Ludo rules)
    player1Tokens[tokenIndex].x += steps * CELL_SIZE;
    if (player1Tokens[tokenIndex].x >= BOARD_DIMENSION) {
        player1Tokens[tokenIndex].x = CELL_SIZE; // Reset to starting position
        player1Tokens[tokenIndex].y += CELL_SIZE; // Move to next row
    }
}

/*void* playerThread(void* arg) {
    Parameters* params = (Parameters*)arg;
    if (!params) return nullptr;

    int playerId = params->playerId;
    if (playerId < 0 || playerId >= 4) return nullptr;

    // Initialize player tokens
    pthread_mutex_lock(&gameState.boardMutex);
    int startRow = (playerId < 2) ? 1 : 12;
    int startCol = (playerId % 2 == 0) ? 1 : 12;

    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            gameState.board[startRow + i][startCol + j] = playerId + 1;
        }
    }
    pthread_mutex_unlock(&gameState.boardMutex);

    while (gameState.gameRunning) {
        // Wait for player's turn
        if (gameState.currentPlayer == playerId) {
            int diceRoll = rollDice(playerId);
            if (diceRoll > 0) {
                movePlayer(playerId, diceRoll);

                // Update turn to next player
                pthread_mutex_lock(&gameState.boardMutex);
                gameState.currentPlayer = (gameState.currentPlayer + 1) % 4;
                pthread_mutex_unlock(&gameState.boardMutex);
            }
        }
        usleep(500000);  // 500ms sleep
    }
    return nullptr;
}
*/
void* playerThread(void* arg) {
    int playerId = *(int*)arg;
    int tokenIndex = 0; // Example: Move the first token
    std::cout << "Player " << playerId << " started.\n";

    while (gameRunning) {
        pthread_mutex_lock(&diceLock);
        diceValue = rand() % 6 + 1;
        std::cout << "Player " << playerId << " rolled a " << diceValue << ".\n";
        pthread_mutex_unlock(&diceLock);

        pthread_mutex_lock(&gridLock);
        moveToken(tokenIndex, diceValue);
        pthread_mutex_unlock(&gridLock);

        sleep(1);
    }

    std::cout << "Player " << playerId << " thread exiting.\n";
    pthread_exit(NULL);
}
void drawPlayerTokens(sf::RenderWindow& window, sf::Vector2f tokens[], sf::Color color) {
    sf::CircleShape token(CELL_SIZE / 2);
    token.setFillColor(color);
    for (int i = 0; i < 4; ++i) {
        token.setPosition(tokens[i]);
        window.draw(token);
    }
}

void drawTokensInHome(sf::RenderWindow& window)
{
    sf::CircleShape token(CELL_SIZE / 2);

    // Red tokens
    token.setFillColor(playerColors[0]);
    token.setPosition(sf::Vector2f(CELL_SIZE, CELL_SIZE));
    window.draw(token);
    token.setPosition(sf::Vector2f(2 * CELL_SIZE, CELL_SIZE));
    window.draw(token);
    token.setPosition(sf::Vector2f(CELL_SIZE, 2 * CELL_SIZE));
    window.draw(token);
    token.setPosition(sf::Vector2f(2 * CELL_SIZE, 2 * CELL_SIZE));
    window.draw(token);

    // Green tokens (Corrected positions)
    token.setFillColor(playerColors[1]);
    token.setPosition(sf::Vector2f(12 * CELL_SIZE, 12 * CELL_SIZE));
    window.draw(token);
    token.setPosition(sf::Vector2f(13 * CELL_SIZE, 12 * CELL_SIZE));
    window.draw(token);
    token.setPosition(sf::Vector2f(12 * CELL_SIZE, 13 * CELL_SIZE));
    window.draw(token);
    token.setPosition(sf::Vector2f(13 * CELL_SIZE, 13 * CELL_SIZE));
    window.draw(token);

    // Blue tokens
    token.setFillColor(playerColors[2]);
    token.setPosition(sf::Vector2f(CELL_SIZE, 12 * CELL_SIZE));
    window.draw(token);
    token.setPosition(sf::Vector2f(2 * CELL_SIZE, 12 * CELL_SIZE));
    window.draw(token);
    token.setPosition(sf::Vector2f(CELL_SIZE, 13 * CELL_SIZE));
    window.draw(token);
    token.setPosition(sf::Vector2f(2 * CELL_SIZE, 13 * CELL_SIZE));
    window.draw(token);

    // Yellow tokens (Corrected positions)
    token.setFillColor(playerColors[3]);
    token.setPosition(sf::Vector2f(12 * CELL_SIZE, CELL_SIZE));
    window.draw(token);
    token.setPosition(sf::Vector2f(13 * CELL_SIZE, CELL_SIZE));
    window.draw(token);
    token.setPosition(sf::Vector2f(12 * CELL_SIZE, 2 * CELL_SIZE));
    window.draw(token);
    token.setPosition(sf::Vector2f(13 * CELL_SIZE, 2 * CELL_SIZE));
    window.draw(token);
}
void drawDice(sf::RenderWindow& window, int diceValue) {
    sf::Font font;
    if (!font.loadFromFile("fonts/arial.ttf")) { // Ensure the path matches your project structure
        std::cerr << "Failed to load font.\n";
        return;
    }

    sf::Text diceText;
    diceText.setFont(font);
    diceText.setCharacterSize(50);
    diceText.setFillColor(sf::Color::Black);
    diceText.setString(std::to_string(diceValue));
    diceText.setPosition(sf::Vector2f(BOARD_DIMENSION / 2 - CELL_SIZE, BOARD_DIMENSION / 2 - 2 * CELL_SIZE));

    sf::RectangleShape diceBox(sf::Vector2f(CELL_SIZE * 2, CELL_SIZE * 2));
    diceBox.setFillColor(sf::Color::White);
    diceBox.setOutlineColor(sf::Color::Black);
    diceBox.setOutlineThickness(2);
    diceBox.setPosition(sf::Vector2f(BOARD_DIMENSION / 2 - CELL_SIZE, BOARD_DIMENSION / 2 - 2 * CELL_SIZE));

    window.draw(diceBox);
    window.draw(diceText);
}

/*void initializeGame() {
    // Initialize mutexes and condition variables
    pthread_mutex_init(&gameState.boardMutex, nullptr);
    pthread_mutex_init(&gameState.diceMutex, nullptr);
    pthread_cond_init(&gameState.diceCond, nullptr);

    // Initialize game state
    memset(gameState.board, 0, sizeof(gameState.board));
    gameState.diceInUse = false;
    gameState.gameRunning = true;
    gameState.currentPlayer = 0;  // Start with player 0

    // Create threads
    pthread_t players[4];
    Parameters params[4];

    // Create master thread first
    pthread_t master;
    pthread_create(&master, nullptr, masterThread, nullptr);

    // Create player threads
    for (int i = 0; i < 4; ++i) {
        params[i].playerId = i;
        params[i].hitRecord = 0;
        pthread_create(&players[i], nullptr, playerThread, (void*)&params[i]);
    }

    // Wait for threads to finish
    for (int i = 0; i < 4; ++i) {
        pthread_join(players[i], nullptr);
    }
    pthread_join(master, nullptr);
}
*/
int main() {
    // Initialize the game (threads will start here)
  //  initializeGame();
    sf::RenderWindow window(sf::VideoMode(BOARD_DIMENSION, BOARD_DIMENSION), "Ludo Game - Phase 1");
    // Initialize random seed for dice rolls
    srand(static_cast<unsigned>(time(0)));
    // Initialize mutexes
    pthread_mutex_init(&diceLock, NULL);
    pthread_mutex_init(&gridLock, NULL);
    // Create a player thread
    pthread_t player1Thread;
    int player1Id = 1;
    if (pthread_create(&player1Thread, NULL, playerThread, &player1Id) != 0) {
        std::cerr << "Failed to create player thread.\n";
        return 1;
    }

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                gameRunning = false;
                window.close();
            }
        }

        window.clear(sf::Color::White);

        // Draw board components
        drawGrid(window);
        drawHomeArea(window, 0, 0, playerColors[0]);
        drawHomeArea(window, 9 * CELL_SIZE, 0, playerColors[3]);
        drawHomeArea(window, 0 * CELL_SIZE, 9 * CELL_SIZE, playerColors[2]);
        drawHomeArea(window, 9 * CELL_SIZE, 9 * CELL_SIZE, playerColors[1]);

        drawStartingGrid(window, CELL_SIZE, CELL_SIZE, playerColors[0]);
        drawStartingGrid(window, 10 * CELL_SIZE, CELL_SIZE, playerColors[3]);
        drawStartingGrid(window, 0 * CELL_SIZE, 10 * CELL_SIZE, playerColors[2]);
        drawStartingGrid(window, 10 * CELL_SIZE, 10 * CELL_SIZE, playerColors[1]);

        drawColoredPaths(window);
        drawSafeCells(window);
        drawCenterStar(window);
        drawTokensInHome(window);

        // Draw dice and tokens
        drawDice(window, diceValue);
        drawPlayerTokens(window, player1Tokens, playerColors[0]);

        window.display();
    }
    // Wait for the thread to finish
    pthread_join(player1Thread, NULL);

    // Destroy mutexes
    pthread_mutex_destroy(&diceLock);
    pthread_mutex_destroy(&gridLock);

    return 0;
}



