#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <random>

const int WIDTH = 1920;
const int HEIGHT = 1080;

const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 position;
    layout (location = 1) in vec3 normal; // New attribute for normal vector
    
    out vec3 FragPos; // Pass the fragment position to the fragment shader
    out vec3 Normal; // Pass the normal vector to the fragment shader

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main() {
        gl_Position = projection * view * model * vec4(position, 1.0);
        FragPos = vec3(model * vec4(position, 1.0)); // Transform position to world space
        Normal = mat3(transpose(inverse(model))) * normal; // Transform normal to world space
    }
)";

const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;

    in vec3 FragPos; // Received from vertex shader
    in vec3 Normal; // Received from vertex shader

    uniform vec3 lightPos; // Position of the light source
    uniform vec3 lightColor; // Color of the light source
    uniform vec3 cubeObjectColor; // Color of the cube object
    

    void main() {
        // Calculate light direction and normalize it
        vec3 lightDir = normalize(lightPos - FragPos);
        
        // Calculate diffuse lighting (Lambert's cosine law)
        float diff = max(dot(Normal, lightDir), 0.0);
        
        // Calculate final color by combining object color and light color
        vec3 cubeResult = cubeObjectColor * diff * lightColor;
        
        
        FragColor = vec4(cubeResult, 1.0); // Output final color for the cube and pyr
        
    }
)";

GLuint shaderProgram;

GLfloat cubeVertices[] = {
    // Front face
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
    0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
    0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
    -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
    // Back face
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
    0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
    0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
    -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
};

GLuint cubeIndices[] = {
    0, 1, 2,
    2, 3, 0,
    4, 5, 6,
    6, 7, 4,
    0, 3, 7,
    7, 4, 0,
    1, 2, 6,
    6, 5, 1,
    0, 1, 5,
    5, 4, 0,
    2, 3, 7,
    7, 6, 2,
};

GLfloat pyramidVertices[] = {
    // Base
    -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
    0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
    0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
    // Apex
    0.0f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
};

GLuint pyramidIndices[] = {
    0, 1, 2,
    2, 3, 0,
    0, 1, 4,
    1, 2, 4,
    2, 3, 4,
    3, 0, 4,
};

GLfloat cameraSpeed = 0.1f;
glm::vec3 cameraPos(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);

float yaw = -90.0f;
float pitch = 0.0f;
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
bool firstMouse = true;

bool rotateClockwise = false;
bool rotateCounterclockwise = false;
float rotationAngle = 0.0f;

float cubeR = 1.0f;
float cubeG = 0.5f;
float cubeB = 0.2f;

float pyramidR = 0.2f;
float pyramidG = 0.5f;
float pyramidB = 1.0f;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_W)
            cameraPos += cameraSpeed * cameraFront;
        if (key == GLFW_KEY_S)
            cameraPos -= cameraSpeed * cameraFront;
        if (key == GLFW_KEY_A)
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (key == GLFW_KEY_D)
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GL_TRUE); // Set the window to close
        }
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.05f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        rotateCounterclockwise = true;
        rotateClockwise = false;

        // Generate random color for cube and pyr
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(0.0f, 1.0f);

        cubeR = dis(gen);
        cubeG = dis(gen);
        cubeB = dis(gen);
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        rotateClockwise = true;
        rotateCounterclockwise = false;
        std::random_device rd;
        std::mt19937 gen(rd()); 
        std::uniform_real_distribution<float> dis(0.0f, 1.0f); 

        cubeR = dis(gen); 
        cubeG = dis(gen); 
        cubeB = dis(gen); 
    }
}

bool initShaders() {
    GLuint vertexShader, fragmentShader;
    GLint success;
    GLchar infoLog[512];

    // Vertex Shader
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "Vertex shader compilation failed: " << infoLog << std::endl;
        return false;
    }

    // Fragment Shader
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "Fragment shader compilation failed: " << infoLog << std::endl;
        return false;
    }

    // Shader Program
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
        return false;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return true;
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL Cube and Pyramid", NULL, NULL);

    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    GLuint cubeVAO, cubeVBO, cubeEBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glGenBuffers(1, &cubeEBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    GLuint pyramidVAO, pyramidVBO, pyramidEBO;
    glGenVertexArrays(1, &pyramidVAO);
    glGenBuffers(1, &pyramidVBO);
    glGenBuffers(1, &pyramidEBO);

    glBindVertexArray(pyramidVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pyramidVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pyramidEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pyramidIndices), pyramidIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Set up camera
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    glViewport(0, 0, WIDTH, HEIGHT);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
    glm::mat4 view;
    glm::mat4 model;

    glfwSetKeyCallback(window, keyCallback);

    if (!initShaders()) {
        std::cerr << "Shader initialization failed" << std::endl;
        return -1;
    }

    while (!glfwWindowShouldClose(window)) {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (rotateClockwise) {
            rotationAngle += 0.5f;
            if (rotationAngle >= 360.0f)
                rotationAngle -= 360.0f;
        }
        else if (rotateCounterclockwise) { 
            rotationAngle -= 0.5f;
            if (rotationAngle < 0.0f)
                rotationAngle += 360.0f;
        }

        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        // Set up light position and color
        glm::vec3 lightPos(2.0f, 3.0f, 2.0f); // Example light position

        GLint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
        glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));

        GLint lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
        glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f); // Default color

        // Draw the cube
        glUseProgram(shaderProgram);
        glBindVertexArray(cubeVAO);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
        model = glm::rotate(model, glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        GLint cubeObjectColorLoc = glGetUniformLocation(shaderProgram, "cubeObjectColor");
        glUniform3f(cubeObjectColorLoc, cubeR, cubeG, cubeB); // Set cube object color
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        // Draw the pyramid
        glBindVertexArray(pyramidVAO);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(1.0f, 0.0f, -1.0f));
        model = glm::rotate(model, glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        GLint pyramidObjectColorLoc = glGetUniformLocation(shaderProgram, "pyramidObjectColor");
        glUniform3f(pyramidObjectColorLoc, pyramidR, pyramidG, pyramidB); // Set pyramid object color
        glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0); 

        glfwPollEvents(); 
        glfwSwapBuffers(window); 
    }

    glDeleteProgram(shaderProgram); 
    glDeleteVertexArrays(1, &cubeVAO); 
    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &cubeEBO);
    glDeleteVertexArrays(1, &pyramidVAO);
    glDeleteBuffers(1, &pyramidVBO);
    glDeleteBuffers(1, &pyramidEBO);

    glfwTerminate();

    return 0;
}
