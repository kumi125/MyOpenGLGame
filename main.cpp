#include <GLFW/glfw3.h>
#include <windows.h>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include "stb_easy_font.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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
    if (!glfwInit()) return -1;

    GLFWwindow* window = glfwCreateWindow(800, 600, "Paddle Game with Shadow", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
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

            float startX = windowWidth * 0.4f;
            float startY = windowHeight * 0.6f;
            float startWidth = windowWidth * 0.2f;
            float startHeight = windowHeight * 0.1f;

            float quitX = windowWidth * 0.4f;
            float quitY = windowHeight * 0.4f;
            float quitWidth = windowWidth * 0.2f;
            float quitHeight = windowHeight * 0.1f;

            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
                if (isMouseInside(mouseX, mouseY, startX, startY, startWidth, startHeight)) {
                    showStartScreen = false;
                    gameStarted = true;
                    playBackgroundMusic("sounds/background.wav");
                    while (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
                        glfwPollEvents();
                }
                else if (isMouseInside(mouseX, mouseY, quitX, quitY, quitWidth, quitHeight)) {
                    glfwSetWindowShouldClose(window, GLFW_TRUE);
                }
            }

            glfwSwapBuffers(window);
            glfwPollEvents();
            continue;
        }

        if (showGameOverScreen) {
            glClearColor(0, 0, 0.2f, 1);
            glClear(GL_COLOR_BUFFER_BIT);
            glLoadIdentity();
            renderText(200, 300, "GAME OVER - Press R to Restart", 2.0f);
            if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
                score = 0;
                missedBlocks = 0;
                level = 1;
                blockSpeed = 0.003f;
                targetScore = 20;
                showGameOverScreen = false;
                resetBlock();
            }
            glfwSwapBuffers(window);
            glfwPollEvents();
            continue;
        }

        if (showLevelCompleteScreen) {
            glClearColor(0, 0, 0.2f, 1);
            glClear(GL_COLOR_BUFFER_BIT);
            glLoadIdentity();

            char msg[64];
            sprintf(msg, "Level %d Complete! Press SPACE to continue.", level);
            renderText(100, 300, msg, 2.0f);

            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
                level++;
                score = 0;
                missedBlocks = 0;
                targetScore = 20 + 10 * (level - 1);
                blockSpeed += 0.001f;
                resetBlock();
                showLevelCompleteScreen = false;
            }

            glfwSwapBuffers(window);
            glfwPollEvents();
            continue;
        }

        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) isPaused = true;
        if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) isPaused = false;

        if (isPaused) {
            glClearColor(0, 0, 0.1f, 1);
            glClear(GL_COLOR_BUFFER_BIT);
            glLoadIdentity();
            renderText(320, 300, "GAME PAUSED - Press 'O' to Resume", 2.0f);
            glfwSwapBuffers(window);
            glfwPollEvents();
            continue;
        }

        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) playerX -= 1.5f * dt;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) playerX += 1.5f * dt;

        if (playerX - paddleWidth < -1.0f) playerX = -1.0f + paddleWidth;
        if (playerX + paddleWidth > 1.0f) playerX = 1.0f - paddleWidth;

        if (blockVisible) blockY -= blockSpeed;

        if (blockY < -0.9f + paddleHeight && blockVisible) {
            if (std::abs(blockX - playerX) < paddleWidth) {
                score++;
                playSound("sounds/catch.wav");
            } else {
                missedBlocks++;
                playSound("sounds/miss.wav");
            }
            blockVisible = false;
            if (score >= targetScore) showLevelCompleteScreen = true;
            if (missedBlocks >= 5) showGameOverScreen = true;
        }

        if (!blockVisible) resetBlock();

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        drawBackground();

        glLoadIdentity();
        glColor3f(0, 1, 1);
        glBegin(GL_QUADS);
        glVertex2f(playerX - paddleWidth, -0.95f);
        glVertex2f(playerX + paddleWidth, -0.95f);
        glVertex2f(playerX + paddleWidth, -0.95f + paddleHeight);
        glVertex2f(playerX - paddleWidth, -0.95f + paddleHeight);
        glEnd();

        if (blockVisible) {
            glColor3f(1, 0, 0);
            glBegin(GL_QUADS);
            glVertex2f(blockX - 0.05f, blockY - 0.05f);
            glVertex2f(blockX + 0.05f, blockY - 0.05f);
            glVertex2f(blockX + 0.05f, blockY + 0.05f);
            glVertex2f(blockX - 0.05f, blockY + 0.05f);
            glEnd();
        }

        renderHUD();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    stopBackgroundMusic();
    glfwTerminate();
    return 0;
}
