#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <GL/glut.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>

// Define thread functions
void *gameEngine(void *arg);
void *userInterface(void *arg);
void *ghostController(void *arg);
int directionFlag = -1; // -1: No movement, 0: Up, 1: Down, 2: Left, 3: Right
int g1_move = 1;
int g2_move = 1;int g3_move=1; int g4_move=1;
#define NUM_GHOSTS 4 // Define the number of ghosts
int pelletCount=0;
bool gameStarted = false;

// Data structure to store ghost positions
typedef struct {
    int x;
    int y;
    // bool isVulnerable;
    // int vulnerabilityTimer;
} Ghost;
bool ghostsVulnerable = false;
long long int vulnerableGhostsTimer = 100000000;
Ghost ghosts[NUM_GHOSTS]; // Array to store ghost positions

// Thread data structures
pthread_t engineThread, uiThread, ghostThreads[4];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex4 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex5 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex6 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex7 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

// Game state variables
int pacmanX = 150, pacmanY = 145; // Initial position of Pac-Man
int score = 0; // Game score
int lives = 3; // Number of lives
int ghostPositions[4][2]; // Ghost positions (x, y)
// Game board constants
#define BOARD_WIDTH 480
#define BOARD_HEIGHT 480
#define CELL_SIZE 15
#define WALL_THICKNESS 4

int gameBoardX=BOARD_HEIGHT / CELL_SIZE;
int gameBoardY=BOARD_WIDTH / CELL_SIZE;
// Define game board layout
char gameBoard[BOARD_HEIGHT / CELL_SIZE][BOARD_WIDTH / CELL_SIZE] = {
    "#######################",
    "#....*.....#.....*....#",
    "#.###.####.#.####.###.#",
    "#.###.####.#.####.###.#",
    "#.....................#",
    "#.###.#.#######.#.###.#",
    "#.....#.#######.#.....#",
    "#####.#....#....#.#####",
    "#####.#### # ####.#####",
    "#####.#         #.#####",
    "#####.# ### ### #.#####",
    "# *  .# #MM MM# #. *  #",
    "#####.# #MM MM# #.#####",
    "#####.# ####### #.#####",
    "#####.#         #.#####",
    "#####.#.#######.#.#####",
    "#####.#.#######.#.#####",
    "#..........*..........#",
    "#.###.####.#.####.###.#",
    "#...#.####.#.####.#...#",
    "###.#.####.#.####.#.###",
    "#..........#..........#",
    "#.########.#.########.#",
    "#.########.#.########.#",
    "#..........*..........#",
    "#######################",
};

// Initialize game board
void initializeGameBoard() {
    // Initialize entire board as empty (0)

}

void drawGameBoard() {
    for (int i = 0; i < BOARD_HEIGHT / CELL_SIZE; i++) {
        for (int j = 0; j < BOARD_WIDTH / CELL_SIZE; j++) {
            if (gameBoard[i][j] == '#') {
                // Draw walls
                glColor3f(0.0, 0.0, 1.0); // Blue color for walls
                glBegin(GL_QUADS);
                glVertex2f(j * CELL_SIZE, i * CELL_SIZE);
                glVertex2f((j + 1) * CELL_SIZE, i * CELL_SIZE);
                glVertex2f((j + 1) * CELL_SIZE, (i + 1) * CELL_SIZE);
                glVertex2f(j * CELL_SIZE, (i + 1) * CELL_SIZE);
                glEnd();
            }
            else if (gameBoard[i][j] == '*') {
                glColor3f(0.5, 0.0, 0.5); // Purple color for power pellet
                glPointSize(6.0); // Increase point size for visibility
                glBegin(GL_POINTS);
                glVertex2f(j * CELL_SIZE + CELL_SIZE / 2, i * CELL_SIZE + CELL_SIZE / 2);
        glEnd();
            }
            else if (gameBoard[i][j] == '.') {
                // Draw pellets
                glColor3f(1.0, 1.0, 1.0); // White color for pellets
                glPointSize(3.0);
                glBegin(GL_POINTS);
                glVertex2f(j * CELL_SIZE + CELL_SIZE / 2, i * CELL_SIZE + CELL_SIZE / 2);
                glEnd();
            }
            
        }
    }
}

// Function to draw Pac-Man
void drawPacman() {
    glPushMatrix();
    glColor3f(1.0, 1.0, 0.0); // Yellow color for Pac-Man
    glTranslatef(pacmanX, pacmanY, 0.0);

    // Draw Pac-Man body (triangle)
        glBegin(GL_TRIANGLES);
        glVertex2f(0, CELL_SIZE / 2); // Top point
        glVertex2f(-CELL_SIZE / 2, -CELL_SIZE / 2); // Bottom left point
        glVertex2f(CELL_SIZE / 2, -CELL_SIZE / 2); // Bottom right point
    glEnd();

    glPopMatrix();
}

void keyboard(int key, int xx, int yy) {
    switch (key) {
        case GLUT_KEY_UP:
            directionFlag = 0; // Move up
           // printf("up\n");
            break;
        case GLUT_KEY_DOWN:
            directionFlag = 1; // Move down
           // printf("down\n");
            break;
        case GLUT_KEY_LEFT:
            directionFlag = 2; // Move left
           // printf("left\n");
            break;
        case GLUT_KEY_RIGHT:
            directionFlag = 3; // Move right
           // printf("right\n");
            break;
    }
}

// Function to update Pac-Man's position based on direction flag
void updatePacmanPosition() {
    switch (directionFlag) {
        case 0: // Move up
            if (pacmanY - CELL_SIZE >= 0 && gameBoard[pacmanY / CELL_SIZE - 1][pacmanX / CELL_SIZE] != '#') {
                pacmanY -= 2;
            }
            break;
        case 1: // Move down
            if (pacmanY + CELL_SIZE < BOARD_HEIGHT && gameBoard[pacmanY / CELL_SIZE + 1][pacmanX / CELL_SIZE] != '#') {
                pacmanY += 2;
            }
            break;
        case 2: // Move left
            if (pacmanX - CELL_SIZE >= 0 && gameBoard[pacmanY / CELL_SIZE][pacmanX / CELL_SIZE - 1] != '#') {
                pacmanX -= 2;
            }
            break;
        case 3: // Move right
            if (pacmanX + CELL_SIZE < BOARD_WIDTH && gameBoard[pacmanY / CELL_SIZE][pacmanX / CELL_SIZE + 1] != '#') {
                pacmanX += 2;
            }
            break;
    }
    glutPostRedisplay();
}

// Function to draw Ghosts
void drawGhosts() {
    glPushMatrix();
    glColor3f(1.0, 0.0, 0.0); // Red color for Ghosts

    // Draw each ghost at its respective position
    for (int i = 0; i < NUM_GHOSTS; i++) {
        glPushMatrix();
        glTranslatef(ghosts[i].x, ghosts[i].y, 0.0);
        
        // Set color based on vulnerability state
        if (ghostsVulnerable) {
            // Blue color for vulnerable ghosts
            glColor3f(0.0, 0.0, 1.0);
        } else {
            // Red color for non-vulnerable ghosts
            glColor3f(1.0, 0.0, 0.0);
        }
        // Draw Ghost body (triangle)
        glBegin(GL_TRIANGLES);
        glVertex2f(0, CELL_SIZE / 2); // Top point
        glVertex2f(-CELL_SIZE / 2, -CELL_SIZE / 2); // Bottom left point
        glVertex2f(CELL_SIZE / 2, -CELL_SIZE / 2); // Bottom right point
        glEnd();
        glPopMatrix();
    }

    glPopMatrix();
}

// Function to render text on the screen
void renderBitmapString(float x, float y, void *font, const char *string) {
    const char *c;
    glRasterPos2f(x, y);
    for (c = string; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
}

// Function to display the score on the screen
void displayScore() {
    glColor3f(1.0, 1.0, 1.0); // White color
    glLoadIdentity();
    glPushMatrix();
    glTranslatef(10, 10, 0); // Adjust position as needed
    char scoreText[20];
    char livesText[20];
    char pacposx[20];
    char pacposy[20];
    char enemyx[20];
    char enemyy[20];
    char timer[20];
    sprintf(enemyx, "enemy0: %d", ghosts[2].x);
    sprintf(enemyy, ", %d", ghosts[2].y);
    sprintf(timer, "Timer: %lld", vulnerableGhostsTimer);
    sprintf(livesText, "Lives: %d", lives);
    sprintf(pacposx, "pac: %d", pacmanX);
    sprintf(pacposy, ", %d", pacmanY);
    sprintf(scoreText, "Score: %d", score);
    renderBitmapString(0, 0, GLUT_BITMAP_HELVETICA_12, scoreText);
    renderBitmapString(110, 0, GLUT_BITMAP_HELVETICA_12, livesText);
    renderBitmapString(330, 0, GLUT_BITMAP_HELVETICA_12, pacposx);
    renderBitmapString(390, 0, GLUT_BITMAP_HELVETICA_12, pacposy);
    renderBitmapString(330, 14, GLUT_BITMAP_HELVETICA_12, enemyx);
    renderBitmapString(400, 14, GLUT_BITMAP_HELVETICA_12, enemyy);
    renderBitmapString(330, 28, GLUT_BITMAP_HELVETICA_12, timer);
    glPopMatrix();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    drawGameBoard(); // Draw game board
    updatePacmanPosition(); // Update Pac-Man's position
    drawPacman(); // Draw Pac-Man
    drawGhosts(); // Draw Ghosts
    displayScore(); // Display score
    glutSwapBuffers();
}

// void display2() {
//     glClear(GL_COLOR_BUFFER_BIT);
//     glLoadIdentity();
//     drawGameBoard(); // Draw game board

//     //if (gameStarted) {
//         //drawGameBoard();
//         updatePacmanPosition(); // Update Pac-Man's position
//         drawPacman();          // Draw Pac-Man
//         displayScore();        // Display score
//         drawGhosts(); 
//         // Draw other game elements
//     //} else {
//       //  drawStartScreen();
//         //gameStarted = 1;
//     //}
//     //startScreen(); // Display start screen

//     // Only display Pac-Man, ghosts, and score when the game is running
//     // if (directionFlag != -1) {
//     //     updatePacmanPosition(); // Update Pac-Man's position
//     //     drawPacman();          // Draw Pac-Man
//     //     displayScore();        // Display score
//     //     drawGhosts();          // Draw Ghosts
//     // }

//     glutSwapBuffers();
// }


void *gameEngine(void *arg) {
    glutDisplayFunc(display);
    glutSpecialFunc(keyboard);
   
    for (int i = 0; i < NUM_GHOSTS; i++) {
        ghosts[i].x = 110 + i * CELL_SIZE * 3; // Initial x position
        ghosts[i].y = 110; // Initial y position
    }
    ghosts[1].x = 255; // Initial x position
        ghosts[1].y = 255; 
        ghosts[2].x = 150; // Initial x position
        ghosts[2].y = 265;
        ghosts[0].x = 85; // Initial x position
        ghosts[0].y = 110;
    while (1) {
        pthread_mutex_lock(&mutex);
        // Check for collision with ghosts
        for (int i = 0; i < NUM_GHOSTS; i++) {
        	
            if (((pacmanX <= ghosts[i].x + 12) && (pacmanX >= ghosts[i].x - 12)) && ((pacmanY <= ghosts[i].y + 12) && (pacmanY >= ghosts[i].y - 12))) {
                // Collision with ghost, decrease lives
                if (ghostsVulnerable)
                {
                    if (i == 3) 
                    {
                        ghosts[3].x = 110 + i * CELL_SIZE * 3; // Initial x position
                        ghosts[3].y = 110;
                    }

                    else if (i == 1)
                    {
                        ghosts[1].x = 255; // Initial x position
                        ghosts[1].y = 255; 
                    }
                    
                    else if (i == 2)
                    {
                        ghosts[2].x = 150; // Initial x position
                        ghosts[2].y = 265; 
                    }

                    else if (i == 0)
                    {
                        ghosts[0].x = 85; // Initial x position
                        ghosts[0].y = 110; 
                    }

                    score += 50;

                }
                else {
                lives--;
                    if (lives <= 0) {
                        // Game over condition
                        printf("Game over! Final Score: %d\n", score);
                        exit(0);
                    } else {
                        // Reset Pac-Man position
                        pacmanX = 315;
                        pacmanY = 360;
                    }
                }
            }
        }
        pthread_mutex_unlock(&mutex);
        // Update score if Pac-Man consumes a pellet
        pthread_mutex_lock(&mutex2);
        if (gameBoard[pacmanY / CELL_SIZE][pacmanX / CELL_SIZE] == '.') {
            score += 10;
            ++pelletCount;
            gameBoard[pacmanY / CELL_SIZE][pacmanX / CELL_SIZE] = 0; // Remove pellet
        }
        if (pelletCount >= 191){
            printf("You Won!: Final Score: %d\n", score);
            exit(0);
        }
        pthread_mutex_unlock(&mutex2);
        
        pthread_mutex_lock(&mutex6);
        if (gameBoard[pacmanY / CELL_SIZE][pacmanX / CELL_SIZE] == '*') {
        score += 25;
        gameBoard[pacmanY / CELL_SIZE][pacmanX / CELL_SIZE] = 0; // Remove pellet

        // Respawn logic
        int respawnX, respawnY;
        do {
        // Generate random respawn position
            respawnX = rand() % (BOARD_WIDTH / CELL_SIZE);
            respawnY = rand() % (BOARD_HEIGHT / CELL_SIZE);
        } while (gameBoard[respawnY][respawnX] == '#' || (respawnX >= 22) || (respawnY>=26)); // Check if respawn position is a wall

        // Update game board with the respawned power pellet
        gameBoard[respawnY][respawnX] = '*'; 
        ghostsVulnerable = true;
    }pthread_mutex_unlock(&mutex6);
    // Decrement vulnerability timer for ghosts
            if (ghostsVulnerable) {
                vulnerableGhostsTimer--;
                if (vulnerableGhostsTimer <= 0) {
                    // Timer expired, reset vulnerability state
                    ghostsVulnerable = false;
                    vulnerableGhostsTimer = 100000000;
                }
            }
        

    }
    pthread_exit(NULL);
}

void handlePowerPelletEaten() {
    // make thae ghosts turn blue
}


void display2() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    drawGameBoard(); // Draw game board

    //if (gameStarted) {
        //drawGameBoard();
        updatePacmanPosition(); // Update Pac-Man's position
        drawPacman();          // Draw Pac-Man
        displayScore();        // Display score
        drawGhosts(); 
        // Draw other game elements
    //} else {
      //  drawStartScreen();
        //gameStarted = 1;
    //}
    //startScreen(); // Display start screen

    // Only display Pac-Man, ghosts, and score when the game is running
    // if (directionFlag != -1) {
    //     updatePacmanPosition(); // Update Pac-Man's position
    //     drawPacman();          // Draw Pac-Man
    //     displayScore();        // Display score
    //     drawGhosts();          // Draw Ghosts
    // }

    glutSwapBuffers();
}


void drawStartScreen() {
    glColor3f(1.0, 1.0, 1.0); // White color
    glLoadIdentity();
    glPushMatrix();
    glTranslatef(100, 200, 0); // Adjust position as needed
    char startText[] = "Press 'S' to start the game";
    for (int i = 0; startText[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, startText[i]);
    }
    glPopMatrix();
}

void keyboard2(unsigned char key, int x, int y) {
    if (gameStarted) {
        glutPostRedisplay(); // Redraw the screen
    }
}

// void *userInterface(void *arg) {
//     pthread_mutex_lock(&mutex7);
//     // Set up GLUT
//     // glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
//     // glutInitWindowSize(800, 600);
//     // glutCreateWindow("Pac-Man");

//     // Register display function and keyboard function
//     //glutDisplayFunc(display);
//     //glutKeyboardFunc(keyboard2);

//     // Enter the GLUT event loop
//     glutMainLoop();
//     pthread_mutex_unlock(&mutex7);

// }
//Function to simulate user interface
void *userInterface(void *arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        pthread_cond_signal(&cond);
        if (gameStarted)
        {
             glutDisplayFunc(display);
            glutKeyboardFunc(keyboard2);
        }
        pthread_cond_wait(&cond, &mutex);
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}
bool isValidMove(int x, int y) {
    return x >= 0 && x < gameBoardX && y >= 0 && y < gameBoardY && gameBoard[y][x] != '#';
}

void *ghostController(void *arg) {
    int ghostID = *((int *)arg);
    while (1) {
        //pthread_mutex_lock(&mutex);
        // if (ghostID == 3) {
        //     int dx = (pacmanX - ghosts[ghostID].x) / CELL_SIZE;
        //     int dy = (pacmanY - ghosts[ghostID].y) / CELL_SIZE;
        //     ghosts[ghostID].x += dx * CELL_SIZE;
        //     ghosts[ghostID].y += dy * CELL_SIZE;
        // }
        if (ghostID == 1) {
            // Ghost 1 moves vertically only
            int move = rand() % 2 ? 1 : -1; // Randomly chooses to move up or down
            pthread_mutex_lock(&mutex);
            if (ghosts[ghostID].y - CELL_SIZE >= 0 && gameBoard[ghosts[ghostID].y / CELL_SIZE - 1][ghosts[ghostID].x / CELL_SIZE] != '#' && g1_move == -1) 
                ghosts[ghostID].y += g1_move * CELL_SIZE;
            else if (ghosts[ghostID].y + CELL_SIZE < BOARD_HEIGHT && gameBoard[ghosts[ghostID].y / CELL_SIZE + 1][ghosts[ghostID].x / CELL_SIZE] != '#' && g1_move == 1) 
                ghosts[ghostID].y += g1_move * CELL_SIZE;

            // glutPostRedisplay();
            // sleep(1);
            else 
                g1_move *= -1;
         

            pthread_mutex_unlock(&mutex);

        } 
        else if (ghostID == 0) {
            // Ghost 1 moves vertically only
            int move = rand() % 2 ? 1 : -1; // Randomly chooses to move up or down
            pthread_mutex_lock(&mutex3);
            if (ghosts[ghostID].y - CELL_SIZE >= 0 && gameBoard[ghosts[ghostID].y / CELL_SIZE - 1][ghosts[ghostID].x / CELL_SIZE] != '#' && g4_move == -1) 
                ghosts[ghostID].y += g4_move * CELL_SIZE;
            else if (ghosts[ghostID].y + CELL_SIZE < BOARD_HEIGHT && gameBoard[ghosts[ghostID].y / CELL_SIZE + 1][ghosts[ghostID].x / CELL_SIZE] != '#' && g4_move == 1) 
                ghosts[ghostID].y += g4_move * CELL_SIZE;

            // glutPostRedisplay();
            // sleep(1);
            else 
                g4_move *= -1;
         

            pthread_mutex_unlock(&mutex3);

        } 
        else if (ghostID == 2 ) {
            // Ghost 2 moves horizontally only
            int move = rand() % 2 ? 1 : -1; // Randomly chooses to move left or right
            pthread_mutex_lock(&mutex4);
            if (ghosts[ghostID].x - CELL_SIZE >= 0 && gameBoard[ghosts[ghostID].y / CELL_SIZE][ghosts[ghostID].x / CELL_SIZE - 1] != '#' && g2_move == -1)
                ghosts[ghostID].x += g2_move  * CELL_SIZE;
            else if (ghosts[ghostID].x + CELL_SIZE < BOARD_WIDTH && gameBoard[ghosts[ghostID].y / CELL_SIZE][ghosts[ghostID].x / CELL_SIZE + 1] != '#' && g2_move == 1)
                ghosts[ghostID].x += g2_move * CELL_SIZE;
           // pthread_mutex_unlock(&mutex);
            else    
                g2_move *= -1;
            pthread_mutex_unlock(&mutex4);
        } 
        else if (ghostID == 3) {
            // Ghost 2 moves horizontally only
            int move = rand() % 2 ? 1 : -1; // Randomly chooses to move left or righ
            pthread_mutex_lock(&mutex5);
int dx = pacmanX - ghosts[ghostID].x;
            int dy = pacmanY - ghosts[ghostID].y;

            // Determine the direction to move based on the relative positions
            int moveX = 0, moveY = 0;
            if (abs(dx) > abs(dy)) {
                // Move along the x-axis
                moveX = (dx > 0) ? 1 : -1;
            } else {
                // Move along the y-axis
                moveY = (dy > 0) ? 1 : -1;
            }

            // Check if the move is valid and update ghost position
            if (isValidMove(ghosts[ghostID].x / CELL_SIZE + moveX, ghosts[ghostID].y / CELL_SIZE)) {
                ghosts[ghostID].x += moveX * CELL_SIZE;
            }
            if (isValidMove(ghosts[ghostID].x / CELL_SIZE, ghosts[ghostID].y / CELL_SIZE + moveY)) {
                ghosts[ghostID].y += moveY * CELL_SIZE;
            }
            pthread_mutex_unlock(&mutex5);
        } 


        // Boundary check
        if (ghosts[ghostID].x < 0) ghosts[ghostID].x = 0;
        if (ghosts[ghostID].x >= BOARD_WIDTH) ghosts[ghostID].x = BOARD_WIDTH - CELL_SIZE;
        if (ghosts[ghostID].y < 0) ghosts[ghostID].y = 0;
        if (ghosts[ghostID].y >= BOARD_HEIGHT) ghosts[ghostID].y = BOARD_HEIGHT - CELL_SIZE;

        //glutPostRedisplay();
        usleep(100000);
        //pthread_mutex_unlock(&mutex);

        
    }
    pthread_exit(NULL);
}


// Function to initialize OpenGL
void initOpenGL() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, BOARD_WIDTH, BOARD_HEIGHT, 0);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(BOARD_WIDTH, BOARD_HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Pac-Man");

    // Initialize OpenGL
    initOpenGL();
    pthread_create(&engineThread, NULL, gameEngine, NULL);
    pthread_create(&uiThread, NULL, userInterface, NULL);
    int ghostIDs[NUM_GHOSTS]; // Array to store IDs for each ghost thread
    for (int i = 0; i < NUM_GHOSTS; i++) {
        ghostIDs[i] = i;
        pthread_create(&ghostThreads[i], NULL, ghostController, &ghostIDs[i]);
    }

    glutMainLoop();

    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutex2);
    pthread_mutex_destroy(&mutex3);
    pthread_mutex_destroy(&mutex4);
    pthread_mutex_destroy(&mutex5);
    pthread_mutex_destroy(&mutex6);
    pthread_cond_destroy(&cond);

    return 0;
}