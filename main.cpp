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
float xMin = -100.0f;
float xMax = 100.0f;
float yMin = -100.0f;
float yMax = 100.0f;

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

// Lida com o caso especial que o ponto Q é colinear ao segmento PR e verifica se
// Q está dentro dos limites do segmento da reta
bool onSegment(const ponto2D& p, const ponto2D& q, const ponto2D& r) {
    return q.x <= std::max(p.x, r.x) && q.x >= std::min(p.x, r.x) &&
           q.y <= std::max(p.y, r.y) && q.y >= std::min(p.y, r.y);
}

// Usa Cross Product para verificar a orientação entre o ponto e o Segmento
// 0 --> Colinear
// 1 --> Sentido Horário
// 2 --> Sentido Anti-Horário

// Mesma ideia usada em Triangulação de pontos.
int orientation(const ponto2D& p, const ponto2D& q, const ponto2D& r) {
    double val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);
    if (val == 0) return 0;
    return (val > 0) ? 1 : 2;
}

// A ideia vai ser lidar com: a particula no frame atual, a particula do frame seguinte (projetada dado a dir),
// o ponto a e o ponto b (Segmento de reta que pode gerar a colisão).
bool doIntersect(const ponto2D& p1, const ponto2D& q1, const ponto2D& p2, const ponto2D& q2) {
    int o1 = orientation(p1, q1, p2);
    int o2 = orientation(p1, q1, q2);
    int o3 = orientation(p2, q2, p1);
    int o4 = orientation(p2, q2, q1);

    if (o1 != o2 && o3 != o4) {return true;} // Caso Base: Retas que se cruzam com sentidos e angulos distintos.

    //Exemplo:
    /*

                                a
                                |
                                |
                                |    
        ponto_neste_frame ------------- ponto_no_proximo_frame
                                |
                                |
                                |
                                b
    */


    // Caso Especial de Colinearidade
    if (o1 == 0 && onSegment(p1, p2, q1)) {return true;}
    if (o2 == 0 && onSegment(p1, q2, q1)) {return true;}
    if (o3 == 0 && onSegment(p2, p1, q2)) {return true;}
    if (o4 == 0 && onSegment(p2, q1, q2)) {return true;}

    return false;
}

//Calcula a Normal de um Segmento de Reta
vec3 calculateNormal(const ponto2D& a, const ponto2D& b){
    vec3 normal{b.y - a.y, a.x - b.x, 0.0};
    normal.normalize();
    return normal;
}

// Reflete uma direção usando a normal do segmento
vec3 reflect(const vec3& dir, const vec3& normal) {
    double dotProduct = dir.dot(normal);
    vec3 reflection{dir.get_x() - 2 * dotProduct * normal.get_x(), dir.get_y() - 2 * dotProduct * normal.get_y(), 0.0};
    return reflection;
}

bool pointIntersectsSegment(const ponto2D& point, const vec3& dir, const ponto2D& a, const ponto2D& b) {
    ponto2D projectedPoint;
    projectedPoint.x = point.x + dir.get_x() * speed;
    projectedPoint.y = point.y + dir.get_y() * speed;
    return doIntersect(point, projectedPoint, a, b);
}


// É chamada a cada frame para verificar inteseção da particula com algum segmento
void checkIntersect(const ponto2D& a, const ponto2D& b){
    for (auto& particle : particles) {
        ponto2D& point = particle.first;
        vec3& dir = particle.second;
        
        if (pointIntersectsSegment(point, dir, a, b)) {
            std::cout << "Colisão detectada!!!" << std::endl;

            vec3 normal = calculateNormal(a, b);
            vec3 newDirection = reflect(dir, normal);

            dir = newDirection;
        }
    }
}

// Normal dos Limites da janela (Trivial)
vec3 getBorderNormal(const ponto2D& pos) {
    if (pos.x <= xMin) return vec3{1.0f, 0.0f, 0.0f};
    if (pos.x >= xMax) return vec3{-1.0f, 0.0f, 0.0f};
    if (pos.y <= yMin) return vec3{0.0f, 1.0f, 0.0f};
    if (pos.y >= yMax) return vec3{0.0f, -1.0f, 0.0f};
    return vec3{0.0f, 0.0f, 0.0f};
}

// Check a colisão com os limites da janela gráfica
void intersectWithLimits() {
    for (auto& particle : particles) {
        ponto2D& pos = particle.first;
        vec3& dir = particle.second;

        vec3 normal = getBorderNormal(pos);
        if (normal.get_x() != 0.0f || normal.get_y() != 0.0f) {
            vec3 newDirection = reflect(dir, normal);
            dir = newDirection;
        }
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
    if (!glfwInit()) {
        std::cerr << "Erro ao inicializar GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Particle Physics", nullptr, nullptr);
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
        intersectWithLimits();
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
                    checkIntersect(segs[i], segs[i+1]);
                }  
            }else{
                for(int i = 0; i < segs.size() - 1; i = i + 2){
                    drawSegment(segs[i], segs[i+1], shaderProgram, projection, 1.0f, 0.0f, 0.0f);
                    checkIntersect(segs[i], segs[i+1]);
                }
            }

        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}