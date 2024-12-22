#include <GLFW/glfw3.h>
#include "linmath.h"
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <time.h>

using namespace std;

const float DEG2RAD = 3.14159 / 180;

void processInput(GLFWwindow* window);

enum BRICKTYPE { REFLECTIVE, DESTRUCTABLE };

enum ONOFF { ON, OFF };

class Brick {
public:
    float red, green, blue;
    float x, y, width;
    BRICKTYPE brick_type;
    ONOFF onoff;
    int hits_required; // For destructible bricks

    Brick(BRICKTYPE bt, float xx, float yy, float ww, float rr, float gg, float bb, int hits = 1)
        : brick_type(bt), x(xx), y(yy), width(ww), red(rr), green(gg), blue(bb), hits_required(hits), onoff(ON) {}

    void drawBrick() {
        if (onoff == ON) {
            double halfside = width / 2;

            glColor3d(red, green, blue);
            glBegin(GL_POLYGON);

            glVertex2d(x + halfside, y + halfside);
            glVertex2d(x + halfside, y - halfside);
            glVertex2d(x - halfside, y - halfside);
            glVertex2d(x - halfside, y + halfside);

            glEnd();
        }
    }

    void hit() {
        if (brick_type == DESTRUCTABLE && hits_required > 0) {
            hits_required--;
            green -= 0.2f; // Change color to indicate damage
            if (hits_required <= 0) {
                onoff = OFF;
            }
        }
    }
};

class Circle {
public:
    float red, green, blue;
    float radius;
    float x;
    float y;
    float speed = 0.03;
    float dx, dy; // Directional components

    Circle(float xx, float yy, float rad, float r, float g, float b)
        : x(xx), y(yy), radius(rad), red(r), green(g), blue(b), dx(0.02f), dy(0.02f) {}

    void CheckCollision(Brick* brk) {
        if (brk->onoff == ON &&
            (x > brk->x - brk->width && x <= brk->x + brk->width) &&
            (y > brk->y - brk->width && y <= brk->y + brk->width)) {
            dy = -dy; // Reflect vertically
            if (brk->brick_type == REFLECTIVE) {
                dx += 0.01f; // Adjust trajectory
            }
            else if (brk->brick_type == DESTRUCTABLE) {
                brk->hit();
            }
        }
    }

    void MoveOneStep() {
        x += dx;
        y += dy;

        if (x < -1 + radius || x > 1 - radius) {
            dx = -dx; // Bounce horizontally
        }

        if (y < -1 + radius || y > 1 - radius) {
            dy = -dy; // Bounce vertically
        }
    }

    void DrawCircle() {
        glColor3f(red, green, blue);
        glBegin(GL_POLYGON);
        for (int i = 0; i < 360; i++) {
            float degInRad = i * DEG2RAD;
            glVertex2f((cos(degInRad) * radius) + x, (sin(degInRad) * radius) + y);
        }
        glEnd();
    }
};

class Paddle {
public:
    float x, y, width, height;

    Paddle(float xx, float yy, float ww, float hh) : x(xx), y(yy), width(ww), height(hh) {}

    void drawPaddle() {
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_POLYGON);

        glVertex2f(x - width / 2, y - height / 2);
        glVertex2f(x + width / 2, y - height / 2);
        glVertex2f(x + width / 2, y + height / 2);
        glVertex2f(x - width / 2, y + height / 2);

        glEnd();
    }

    void move(float dx) {
        x += dx;
        if (x < -1 + width / 2) x = -1 + width / 2;
        if (x > 1 - width / 2) x = 1 - width / 2;
    }
};

vector<Circle> world;
Paddle paddle(0, -0.9f, 0.4f, 0.05f);

int main(void) {
    srand(time(NULL));

    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWwindow* window = glfwCreateWindow(480, 480, "8-2 Assignment", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    Brick brick(REFLECTIVE, 0.5, -0.33, 0.2, 1, 1, 0);
    Brick brick2(DESTRUCTABLE, -0.5, 0.33, 0.2, 0, 1, 0, 3);
    Brick brick3(DESTRUCTABLE, -0.5, -0.33, 0.2, 0, 1, 1, 2);
    Brick brick4(REFLECTIVE, 0, 0, 0.2, 1, 0.5, 0.5);

    while (!glfwWindowShouldClose(window)) {
        float ratio;
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float)height;
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        processInput(window);

        for (int i = 0; i < world.size(); i++) {
            world[i].CheckCollision(&brick);
            world[i].CheckCollision(&brick2);
            world[i].CheckCollision(&brick3);
            world[i].CheckCollision(&brick4);
            world[i].MoveOneStep();
            world[i].DrawCircle();
        }

        paddle.drawPaddle();
        brick.drawBrick();
        brick2.drawBrick();
        brick3.drawBrick();
        brick4.drawBrick();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate;
    exit(EXIT_SUCCESS);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        paddle.move(-0.05f);
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        paddle.move(0.05f);
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        float r = static_cast<float>(rand()) / RAND_MAX;
        float g = static_cast<float>(rand()) / RAND_MAX;
        float b = static_cast<float>(rand()) / RAND_MAX;
        Circle B(0, 0, 0.05, r, g, b);
        world.push_back(B);
    }
}
