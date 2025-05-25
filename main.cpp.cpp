#include <GLFW/glfw3.h>
#include <cstdio>
#include <ctime>
#include <cmath>
#define STB_EASY_FONT_IMPLEMENTATION
#include "stb_easy_font.h"

float getDeltaTime() {
    static double lastTime = glfwGetTime();
    double currentTime = glfwGetTime();
    float delta = static_cast<float>(currentTime - lastTime);
    lastTime = currentTime;
    return delta;
}

void drawText(float x, float y, const char* text) {
    char buffer[99999]; // ~500 chars
    int num_quads = stb_easy_font_print(x, y, (char*)text, NULL, buffer, sizeof(buffer));

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 16, buffer);
    glDrawArrays(GL_QUADS, 0, num_quads * 4);
    glDisableClientState(GL_VERTEX_ARRAY);
}

int main() {
    if (!glfwInit()) return -1;

    GLFWwindow* window = glfwCreateWindow(800, 600, "Falling Block Game", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);

    float playerX = 0.0f;
    float blockY = 1.0f;
    float blockSpeed = 0.6f; // units per second
    float paddleSpeed = 1.2f; // units per second
    int score = 0;

    while (!glfwWindowShouldClose(window)) {
        float deltaTime = getDeltaTime();

        glClear(GL_COLOR_BUFFER_BIT);

        // Input
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            playerX -= paddleSpeed * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            playerX += paddleSpeed * deltaTime;

        // Clamp player
        if (playerX < -0.8f) playerX = -0.8f;
        if (playerX > 0.8f) playerX = 0.8f;

        // Draw paddle
        glBegin(GL_QUADS);
        glColor3f(1, 1, 1);
        glVertex2f(playerX - 0.2f, -0.9f);
        glVertex2f(playerX + 0.2f, -0.9f);
        glVertex2f(playerX + 0.2f, -0.85f);
        glVertex2f(playerX - 0.2f, -0.85f);
        glEnd();

        // Draw block
        glBegin(GL_QUADS);
        glColor3f(1, 0, 0);
        glVertex2f(-0.05f, blockY);
        glVertex2f(0.05f, blockY);
        glVertex2f(0.05f, blockY - 0.1f);
        glVertex2f(-0.05f, blockY - 0.1f);
        glEnd();

        // Collision check
        if (blockY <= -0.85f && blockY >= -0.9f) {
            if (playerX - 0.2f <= 0.05f && playerX + 0.2f >= -0.05f) {
                score++;
                blockY = 1.0f; // reset
            }
        }

        // Update block
        blockY -= blockSpeed * deltaTime;
        if (blockY < -1.0f) {
            blockY = 1.0f; // missed block
        }

        // Draw score on screen
        char scoreStr[32];
        sprintf(scoreStr, "Score: %d", score);

        glColor3f(1, 1, 1);
        drawText(-0.95f, 0.85f, scoreStr);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
