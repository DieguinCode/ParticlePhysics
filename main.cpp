#include "Libraries/vectors.h"
#include "Libraries/point.h"
#include "glad/include/glad/glad.h"
#include <GLFW/glfw3.h>
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <vector>
#include <array>
#include <random>
#include <utility>
#include <cstdlib>
#include <ctime>

// Janela 800x800
const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 800;

// Limites do Plano Cartesiano 2D -- Desenhado na Janela via OpenGL
float xMin, xMax, yMin, yMax;

//Variáveis Globais
std::vector<ponto2D> segs;
ponto2D mainPoint; // Sempre nasce na origem e vai ter sentido 45 Graus no 1º Quadrante.
vec3 mainDirection{(std::cos(M_PI/4)), (std::sin(M_PI/4)), 0.0};
std::vector<std::pair<ponto2D, vec3>> particles = {std::make_pair(mainPoint, mainDirection)};
float speed = 0.1f;


// Gera 4 segmentos de retas aleatórios
void randomSegs(){
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> distrib_x(xMin, xMax);
    std::uniform_real_distribution<> distrib_y(yMin, yMax);
    
    for(int i = 0; i < 8; ++i){
        double x = distrib_x(gen);
        double y = distrib_y(gen);
        segs.emplace_back(ponto2D(x, y));    
    }

}

// Gera um sentido aleatório que a particula seguirá ao nascer
vec3 randomDirection(){

    // Angulo entre 0 e 2PI
    double angle = static_cast<double>(rand()) / RAND_MAX  * 2.0f * M_PI;

    return vec3{std::cos(angle), std::sin(angle), 0.0};
}

// É chamada a cada frame para verificar inteseção da particula com algum segmento
void checkIntersect(){
    // TODO
}

//Calcula a Normal de um Segmento de Reta --> TODO : Check se está correto
vec3 calculateNormal(const ponto2D& p1, const ponto2D& p2){
    
    // Vetor Direção do Segmento
    double dx = p2.x - p1.x;
    double dy = p2.y - p1.y;

    // Em 2D: Normal 90Graus para esquerda ou 90Graus para a direita
    vec3 n{-dy, dx, 0.0};

    // Vetor Origem do plano cartesiano --> Ponto Medio do Segmento
    double midpointX = (p1.x + p2.x) / 2.0;
    double midpointY = (p1.y + p2.y) / 2.0;

    // Vetor Ponto Medio --> Origem do plano cartesiano
    double originX = -midpointX;
    double originY = -midpointY;

    if(n.dot(vec3{originX, originY, 0.0}) < 0){
        vec3 newN{-n.get_x(), -n.get_y(), 0.0};
        newN.normalize();
        return newN; 
    }else{
        n.normalize();
        return n;
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        // Mouse --> Coordenadas de Mundo.
        double x = static_cast<double>((xpos / WIDTH) * (xMax - xMin) + xMin);
        double y = static_cast<double>(((HEIGHT - ypos) / HEIGHT) * (yMax - yMin) + yMin);
        segs.emplace_back(ponto2D{x, y});
    }
    if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS){
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        // Mouse --> Coordenadas de Mundo.
        double x = static_cast<double>((xpos / WIDTH) * (xMax - xMin) + xMin);
        double y = static_cast<double>(((HEIGHT - ypos) / HEIGHT) * (yMax - yMin) + yMin);
        particles.emplace_back(ponto2D{x, y}, randomDirection());
    }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        if (segs.size() == 8) {return;}
        randomSegs();
    }
    if (key == GLFW_KEY_E && action == GLFW_PRESS) {
        segs.clear();
        particles.clear();
        mainPoint.x = 0.0; mainPoint.y = 0.0;
        particles.emplace_back(mainPoint, vec3{(std::cos(M_PI/4)), (std::sin(M_PI/4)), 0.0});
    }
    if(key == GLFW_KEY_N && action == GLFW_PRESS){
        particles.emplace_back(ponto2D{0.0, 0.0}, randomDirection());
    }
}

// Faz todas as particulas do vector de particulas andarem seguindo a direção daquela particula.
void moveParticles(){
    for(int i = 0; i < particles.size(); ++i){
        ponto2D pos = particles[i].first;
        vec3 dir = particles[i].second;

        dir.normalize();

        vec3 v = dir * speed;
        
        particles[i].first = ponto2D{(pos.x + v.get_x()), (pos.y + v.get_y())};
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
    
    glPointSize(7.0f);
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

        moveParticles();
        for(const auto& p : particles){
            drawPoint(p.first, shaderProgram, projection, 0.5f, 0.5f, 0.5f);
        }
        if(!segs.empty()){
            for(int i = 0; i < segs.size(); ++i){
                drawPoint(segs[i], shaderProgram, projection, 1.0f, 0.0f, 0.0f);
            }
            if(segs.size() % 2 == 0){
                for(int i = 0; i < segs.size(); i = i + 2){
                    drawSegment(segs[i], segs[i+1], shaderProgram, projection, 1.0f, 0.0f, 0.0f);
                }  
            }else{
                for(int i = 0; i < segs.size() - 1; i = i + 2){
                    drawSegment(segs[i], segs[i+1], shaderProgram, projection, 1.0f, 0.0f, 0.0f);
                }
            }

        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}