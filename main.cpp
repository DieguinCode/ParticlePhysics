#include "Libraries/vectors.h"
#include "Libraries/point.h"
#include "glad/include/glad/glad.h"
#include <GLFW/glfw3.h>
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <vector>
#include <array>
#include <random>

// Janela 800x800
const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 800;

// Limites do Plano Cartesiano 2D -- Desenhado na Janela via OpenGL
float xMin, xMax, yMin, yMax;

//Variáveis Globais
std::vector<ponto2D> segs;
bool isIntersected = false;
ponto2D intersectionPoint;

void randomSegs(){
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> distrib_x(xMin, xMax);
    std::uniform_real_distribution<> distrib_y(yMin, yMax);
    
    for(int i = 0; i < 4; ++i){
        double x = distrib_x(gen);
        double y = distrib_y(gen);
        segs.push_back(ponto2D(x, y));    
    }

}

void FindTheIntersectPoint(const ponto2D&a, const ponto2D&b, const ponto2D&c, const ponto2D&d){
    
    double det = (b.x - a.x) * (d.y - c.y) - (b.y - a.y) * (d.x - c.x);

    double t = ((c.x - a.x) * (d.y - c.y) - (c.y - a.y) * (d.x - c.x)) / det;
    double u = ((c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x)) / det;

    if (t >= 0 && t <= 1 && u >= 0 && u <= 1) {
        intersectionPoint = { a.x + t * (b.x - a.x), a.y + t * (b.y - a.y) };
        std::cout << "Ponto de interseção: (" << intersectionPoint.x << ", " << intersectionPoint.y << ")\n";
    }
}

void checkIntersect(){
    
    if (segs.size() != 4) {
        std::cout << "Problema com a quantidade de pontos para formar dois segmentos" << std::endl;
        return;
    }

    ponto2D a = segs[0];
    ponto2D b = segs[1];
    ponto2D c = segs[2];
    ponto2D d = segs[3];

    // A função crossProduct(p1, p2, p3) calcula o determinante 
    // que indica a posição relativa de p3 em relação ao segmento p1p2.
    auto crossProduct = [](const ponto2D& p1, const ponto2D& p2, const ponto2D& p3) {
        return (p2.x - p1.x) * (p3.y - p1.y) - (p2.y - p1.y) * (p3.x - p1.x);
    };

    double d1 = crossProduct(a, b, c);
    double d2 = crossProduct(a, b, d);
    double d3 = crossProduct(c, d, a);
    double d4 = crossProduct(c, d, b);

    // Se os produtos cruzados tiverem sinais opostos, os segmentos se cruzam
    if ((d1 * d2 < 0) && (d3 * d4 < 0)) {
        isIntersected = true;
        FindTheIntersectPoint(a,b,c,d);
    } 
    else {
        isIntersected = false;
    }

    std::cout << "isIntersected = " << (isIntersected ? "true" : "false") << std::endl;
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && (segs.size() < 4)) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        // Mouse --> Coordenadas de Mundo.
        double x = static_cast<double>((xpos / WIDTH) * (xMax - xMin) + xMin);
        double y = static_cast<double>(((HEIGHT - ypos) / HEIGHT) * (yMax - yMin) + yMin);
        segs.push_back(ponto2D(x, y));
        if (segs.size() == 4) {
            checkIntersect();
        }
    }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        if (segs.size() == 4) {return;}
        randomSegs();
        checkIntersect();
    }
    if (key == GLFW_KEY_E && action == GLFW_PRESS) {
        segs.clear();
        isIntersected = false;
        intersectionPoint.x = 0.0; intersectionPoint.y = 0.0;
    }
}

unsigned int compileShader(unsigned int type, const char* source) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Erro ao compilar shader: " << infoLog << std::endl;
    }

    return shader;
}

unsigned int setupCartesianPlane(float xMin, float xMax, float yMin, float yMax) {
    std::array<float, 12> planeVertices = {
        xMin, 0.0f, 0.0f,   xMax, 0.0f, 0.0f,  // Eixo X
        0.0f, yMin, 0.0f,   0.0f, yMax, 0.0f   // Eixo Y
    };

    unsigned int vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return vao;
}

void drawPoint(const ponto2D& p, unsigned int shaderProgram, glm::mat4 projection, float red, float green, float blue){
    float pointVertices[] = {
        p.x, p.y, 0.0f
    };
    
    unsigned int vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pointVertices), pointVertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3f(glGetUniformLocation(shaderProgram, "color"), red, green, blue);
    
    glPointSize(5.0f);
    glDrawArrays(GL_POINTS, 0, 1);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

void drawSegment(const ponto2D& p1, const ponto2D& p2, unsigned int shaderProgram, glm::mat4 projection, float red, float green, float blue) {
    float lineVertices[] = {
        p1.x, p1.y, 0.0f,
        p2.x, p2.y, 0.0f
    };
    
    unsigned int vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), lineVertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3f(glGetUniformLocation(shaderProgram, "color"), red, green, blue);
    
    glDrawArrays(GL_LINES, 0, 2);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

int main(){
    std::cout << "Informe os limites para x (xMin xMax): ";
    std::cin >> xMin >> xMax;
    std::cout << "Informe os limites para y (yMin yMax): ";
    std::cin >> yMin >> yMax;

    if (!glfwInit()) {
        std::cerr << "Erro ao inicializar GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Segment Intersection", nullptr, nullptr);
    if (!window) {
        std::cerr << "Erro ao criar janela GLFW." << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetKeyCallback(window, keyCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Erro ao carregar GLAD." << std::endl;
        return -1;
    }

    glViewport(0, 0, WIDTH, HEIGHT);

    const char* vertexShaderSource = R"(
        #version 430 core
        layout (location = 0) in vec3 aPos;
        uniform mat4 projection;
        void main() {
            gl_Position = projection * vec4(aPos, 1.0);
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 430 core
        out vec4 FragColor;
        uniform vec3 color;
        void main() {
            FragColor = vec4(color, 1.0);
        }
    )";
        
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Erro ao vincular shaders: " << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    unsigned int cartesianVAO = setupCartesianPlane(xMin, xMax, yMin, yMax);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::ortho(xMin, xMax, yMin, yMax);

        glBindVertexArray(cartesianVAO);
        glUseProgram(shaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3f(glGetUniformLocation(shaderProgram, "color"), 0.0f, 1.0f, 0.0f);
        glDrawArrays(GL_LINES, 0, 4);

        if(!segs.empty()){
            if(segs.size() == 1){
                drawPoint(segs[0], shaderProgram, projection, 1.0f, 0.0f, 0.0f);
            }else if(segs.size() == 2){
                drawPoint(segs[0], shaderProgram, projection, 1.0f, 0.0f, 0.0f);
                drawPoint(segs[1], shaderProgram, projection, 1.0f, 0.0f, 0.0f);

                drawSegment(segs[0], segs[1], shaderProgram, projection, 1.0f, 0.0f, 0.0f);
            }else if(segs.size() == 3){
                drawPoint(segs[0], shaderProgram, projection, 1.0f, 0.0f, 0.0f);
                drawPoint(segs[1], shaderProgram, projection, 1.0f, 0.0f, 0.0f);
                
                drawSegment(segs[0], segs[1], shaderProgram, projection, 1.0f, 0.0f, 0.0f);

                drawPoint(segs[2], shaderProgram, projection, 0.0f, 0.0f, 1.0f);
            }else if(segs.size() == 4){
                drawPoint(segs[0], shaderProgram, projection, 1.0f, 0.0f, 0.0f);
                drawPoint(segs[1], shaderProgram, projection, 1.0f, 0.0f, 0.0f);
                
                drawSegment(segs[0], segs[1], shaderProgram, projection, 1.0f, 0.0f, 0.0f);

                drawPoint(segs[2], shaderProgram, projection, 0.0f, 0.0f, 1.0f);
                drawPoint(segs[3], shaderProgram, projection, 0.0f, 0.0f, 1.0f);

                drawSegment(segs[2], segs[3], shaderProgram, projection, 0.0f, 0.0f, 1.0f);

                if(isIntersected){
                    drawPoint(intersectionPoint, shaderProgram, projection, 0.3f, 0.6f, 0.4f);
                }
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}