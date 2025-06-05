// [No changes at the top]
#include <GLFW/glfw3.h>
#include <windows.h>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <string>
#include "stb_easy_font.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// [Variables]
float playerX = 0.0f;
float paddleWidth = 0.2f;
float paddleHeight = 0.05f;
float blockX = 0.0f;
float blockY = 1.0f;
float blockSpeed = 0.003f;
int score = 0;
int level = 1;
int missedBlocks = 0;
int targetScore = 20;
bool showLevelCompleteScreen = false;
bool blockVisible = true;
bool showGameOverScreen = false;
bool isPaused = false;

bool showStartScreen = true;
bool gameStarted = false;

GLuint bgTextureID = 0;

// [Login]
bool login() {
    std::string username;
    std::string password;

    std::cout << "Username: ";
    std::getline(std::cin, username);
    std::cout << "Password: ";
    std::getline(std::cin, password);

    if (username == "admin" && password == "1234") {
        std::cout << "Login successful! Welcome, admin!\n";
        return true;
    } else {
        std::cout << "Login failed. Exiting...\n";
        return false;
    }
}

float getDeltaTime() {
    static double lastTime = glfwGetTime();
    double currentTime = glfwGetTime();
    float delta = static_cast<float>(currentTime - lastTime);
    lastTime = currentTime;
    return delta;
}

void renderText(float x, float y, const char* text, float scale = 1.0f) {
    static char buffer[99999];
    int num_quads = stb_easy_font_print(0, 0, (char*)text, NULL, buffer, sizeof(buffer));

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 800, 0, 600, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glTranslatef(x, y, 0);
    glScalef(scale, -scale, 1);

    glColor3f(1, 1, 0);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 16, buffer);
    glDrawArrays(GL_QUADS, 0, num_quads * 4);
    glDisableClientState(GL_VERTEX_ARRAY);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void playSound(const char* fileName) {
    PlaySound(fileName, NULL, SND_FILENAME | SND_ASYNC);
}
void playBackgroundMusic(const char* fileName) {
    PlaySound(fileName, NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
}
void stopBackgroundMusic() {
    PlaySound(NULL, 0, 0);
}

bool isMouseInside(float mouseX, float mouseY, float btnX, float btnY, float btnWidth, float btnHeight) {
    return (mouseX >= btnX && mouseX <= btnX + btnWidth && mouseY >= btnY && mouseY <= btnY + btnHeight);
}

void renderStartScreen(int windowWidth, int windowHeight) {
    glClearColor(0, 0, 0.1f, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    float startX = windowWidth * 0.4f;
    float startY = windowHeight * 0.65f;
    float quitX = windowWidth * 0.4f;
    float quitY = windowHeight * 0.45f;

    renderText(startX, startY, "Start", 3.0f);
    renderText(quitX, quitY, "Quit", 3.0f);
}

void resetBlock() {
    blockY = 1.0f;
    blockX = ((rand() % 200) / 100.0f) - 1.0f;
    blockVisible = true;
}

bool loadBackgroundTexture() {
    int width, height, channels;
    unsigned char* data = stbi_load("texture.png", &width, &height, &channels, 0);
    if (!data) {
        std::cerr << "Failed to load texture.png\n";
        return false;
    }

    glGenTextures(1, &bgTextureID);
    glBindTexture(GL_TEXTURE_2D, bgTextureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);
    return true;
}

void drawBackground() {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, bgTextureID);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor3f(1, 1, 1);

    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(-1, -1);
    glTexCoord2f(1, 0); glVertex2f(1, -1);
    glTexCoord2f(1, 1); glVertex2f(1, 1);
    glTexCoord2f(0, 1); glVertex2f(-1, 1);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

void renderHUD() {
    char scoreText[64];
    sprintf(scoreText, "Score: %d", score);
    renderText(10, 570, scoreText, 1.5f);

    char missedText[64];
    sprintf(missedText, "Missed: %d / 5", missedBlocks);
    renderText(10, 540, missedText, 1.5f);

    char targetText[64];
    sprintf(targetText, "Target: %d", targetScore);
    renderText(10, 510, targetText, 1.5f);
}

int main() {
    if (!login()) return -1;
    if (!glfwInit()) return -1;

    GLFWwindow* window = glfwCreateWindow(800, 600, "Paddle Game with Shadow", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);

    glEnable(GL_TEXTURE_2D);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);

    if (!loadBackgroundTexture()) {
        std::cerr << "Error: Could not load background texture!\n";
        glfwTerminate();
        return -1;
    }

    while (!glfwWindowShouldClose(window)) {
        float dt = getDeltaTime();
        int windowWidth, windowHeight;
        glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
        double mouseX_d, mouseY_d;
        glfwGetCursorPos(window, &mouseX_d, &mouseY_d);
        float mouseX = static_cast<float>(mouseX_d);
        float mouseY = static_cast<float>(windowHeight - mouseY_d);

        if (showStartScreen) {
            renderStartScreen(windowWidth, windowHeight);
            float startX = windowWidth * 0.4f, startY = windowHeight * 0.6f;
            float startWidth = windowWidth * 0.2f, startHeight = windowHeight * 0.1f;
            float quitX = windowWidth * 0.4f, quitY = windowHeight * 0.4f;

            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
                if (isMouseInside(mouseX, mouseY, startX, startY, startWidth, startHeight)) {
                    showStartScreen = false;
                    gameStarted = true;
                    playBackgroundMusic("sounds/background.wav");
                    while (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
                        glfwPollEvents();
                } else if (isMouseInside(mouseX, mouseY, quitX, quitY, startWidth, startHeight)) {
                    break;
                }
            }

            glfwSwapBuffers(window);
            glfwPollEvents();
            continue;
        }

        if (!gameStarted) {
            glfwSwapBuffers(window);
            glfwPollEvents();
            continue;
        }

        // Pause/resume input
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !isPaused) {
            isPaused = true;
            playSound("sounds/pause.wav");
            while (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
                glfwPollEvents();
        }
        if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS && isPaused) {
            isPaused = false;
            playSound("sounds/resume.wav");
            while (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
                glfwPollEvents();
        }

        if (!showGameOverScreen && !showLevelCompleteScreen && !isPaused) {
            playerX = (mouseX / windowWidth) * 2 - 1;
            playerX = std::fmax(playerX, -1 + paddleWidth / 2);
            playerX = std::fmin(playerX, 1 - paddleWidth / 2);

            blockY -= blockSpeed;

            if (blockVisible && blockY < -1 + paddleHeight + 0.05f) {
                if (blockX > playerX - paddleWidth / 2 && blockX < playerX + paddleWidth / 2) {
                    score++;
                    playSound("sounds/catch.wav");
                    blockVisible = false;
                    if (score >= targetScore) {
                        showLevelCompleteScreen = true;
                        playSound("sounds/level_complete.wav");
                    }
                } else if (blockY < -1) {
                    missedBlocks++;
                    playSound("sounds/miss.wav");
                    blockVisible = false;
                    if (missedBlocks >= 5) {
                        showGameOverScreen = true;
                        playSound("sounds/game_over.wav");
                        stopBackgroundMusic();
                    }
                }
            }

            if (!blockVisible) resetBlock();
        }

        glClear(GL_COLOR_BUFFER_BIT);
        drawBackground();

        if (showGameOverScreen) {
            renderText(windowWidth * 0.3f, windowHeight * 0.5f, "GAME OVER", 5.0f);
            renderText(windowWidth * 0.25f, windowHeight * 0.4f, "Press ESC to Exit", 2.0f);
        } else if (showLevelCompleteScreen) {
            renderText(windowWidth * 0.25f, windowHeight * 0.5f, "LEVEL COMPLETE!", 5.0f);
            renderText(windowWidth * 0.25f, windowHeight * 0.4f, "Press N for Next Level", 2.0f);
        } else {
            glColor3f(0.0f, 0.8f, 0.1f);
            glBegin(GL_QUADS);
            glVertex2f(playerX - paddleWidth / 2, -1 + paddleHeight);
            glVertex2f(playerX + paddleWidth / 2, -1 + paddleHeight);
            glVertex2f(playerX + paddleWidth / 2, -1);
            glVertex2f(playerX - paddleWidth / 2, -1);
            glEnd();

            if (blockVisible) {
                glColor3f(0.9f, 0.1f, 0.1f);
                glBegin(GL_QUADS);
                glVertex2f(blockX - 0.05f, blockY);
                glVertex2f(blockX + 0.05f, blockY);
                glVertex2f(blockX + 0.05f, blockY - 0.05f);
                glVertex2f(blockX - 0.05f, blockY - 0.05f);
                glEnd();
            }

            renderHUD();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();

        if (showLevelCompleteScreen && glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
            level++;
            targetScore += 10;
            score = 0;
            missedBlocks = 0;
            showLevelCompleteScreen = false;
            resetBlock();
            playBackgroundMusic("sounds/background.wav");
            while (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
                glfwPollEvents();
        }

        if (showGameOverScreen && glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            break;
        }
    }

    stopBackgroundMusic();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
