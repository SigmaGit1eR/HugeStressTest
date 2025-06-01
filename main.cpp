// huge_stress_test.cpp

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>

// --- OpenGL шейдери ---

const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

float hash(float n) {
    return fract(sin(n) * 43758.5453);
}

void main() {
    float x = 0.0;
    for(int i = 0; i < 200000; ++i) {
        float fi = float(i);
        x += sin(fi * 0.001 + gl_FragCoord.x) * cos(fi * 0.001 + gl_FragCoord.y);
        x += exp(-abs(sin(fi * 0.01 + x)));
        x += hash(fi * x);
        x = fract(x);
    }
    FragColor = vec4(vec3(x), 1.0);
}
)";

// --- Функція для компіляції шейдерів та перевірки помилок ---

void checkCompileErrors(unsigned int shader, std::string type) {
    int success;
    char infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
                << infoLog << "\n----------------------------------------" << std::endl;
        }
    }
    else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
                << infoLog << "\n----------------------------------------" << std::endl;
        }
    }
}

// --- CPU навантаження ---

std::atomic<bool> cpuLoadRunning(true);

void cpuLoadThread() {
    volatile double x = 0.0;
    while (cpuLoadRunning) {
        // Важкі обчислення
        for (int i = 0; i < 1000000; ++i) {
            x += sin(i) * cos(i * 1.001);
            x = fmod(x, 1000.0);
        }
    }
}

// --- RAM навантаження ---

std::atomic<bool> ramLoadRunning(true);

void ramLoadThread() {
    std::vector<char*> allocations;
    try {
        while (ramLoadRunning) {
            // Виділяємо по 10 МБ кожні 100 мс
            char* block = new char[10 * 1024 * 1024];
            allocations.push_back(block);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    catch (std::bad_alloc&) {
        std::cerr << "Out of memory (RAM stress test)" << std::endl;
        // Залишаємо всі виділення для утримання пам'яті
        while (true) std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

// --- OpenGL (GPU) налаштування та рендер ---

int main() {
    // Запускаємо CPU навантаження в окремому потоці
    std::thread cpuThread(cpuLoadThread);

    // Запускаємо RAM навантаження в окремому потоці
    std::thread ramThread(ramLoadThread);

    // Ініціалізація GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        cpuLoadRunning = false;
        ramLoadRunning = false;
        cpuThread.join();
        ramThread.join();
        return -1;
    }

    // Повноекранне вікно на головному моніторі
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Huge Stress Test", monitor, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        cpuLoadRunning = false;
        ramLoadRunning = false;
        cpuThread.join();
        ramThread.join();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        cpuLoadRunning = false;
        ramLoadRunning = false;
        cpuThread.join();
        ramThread.join();
        return -1;
    }

    // Створення шейдерів
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    checkCompileErrors(vertexShader, "VERTEX");

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    checkCompileErrors(fragmentShader, "FRAGMENT");

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    checkCompileErrors(shaderProgram, "PROGRAM");

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Квадрат для повного екрану
    float vertices[] = {
        -1.0f,  1.0f,
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Рендер цикл
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Зупинка навантажень
    cpuLoadRunning = false;
    ramLoadRunning = false;

    cpuThread.join();
    ramThread.join();

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
