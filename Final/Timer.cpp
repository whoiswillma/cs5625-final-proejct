#include "Timer.h"

#include <GLFW/glfw3.h>

double Timer::realtime() {
    return glfwGetTime();
}
