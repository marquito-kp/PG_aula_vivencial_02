/* Hello Triangle - código adaptado de https://learnopengl.com/#!Getting-started/Hello-Triangle
 *
 * Adaptado por Rossana Baptista Queiroz
 * para a disciplina de Processamento Gráfico - Unisinos
 * Versão inicial: 7/4/2017
 * Última atualização em 13/08/2024
 *
 * Modificado por Marcos Krol Pacheco em 24/05/2025
*/

#include <iostream>
#include <string>
#include <assert.h>
#include <vector>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

#include <cmath>

// Variáveis para controle de ambiente
std::vector<glm::vec2> clickedPoints;
std::vector<GLuint> VAOs;

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

// Protótipos das funções
GLuint createTriangle(float x0, float y0, float x1, float y1, float x2, float y2);
int setupShader();
// int setupGeometry();

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 800, HEIGHT = 600;

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded
const GLchar *vertexShaderSource = R"(
#version 400
layout (location = 0) in vec3 position;
uniform mat4 projection;
uniform mat4 model;
void main()
{
    gl_Position = projection * model * vec4(position.x, position.y, position.z, 1.0);
}
)";

// Código fonte do Fragment Shader (em GLSL): ainda hardcoded
const GLchar *fragmentShaderSource = R"(
#version 400
uniform vec4 inputColor;
out vec4 color;
void main()
{
    color = inputColor;
}
)";


// Criado uma struct para armazenar a cor do triangulo
struct Triangle
{
    vec3 color;
};

vector<Triangle> triangles;

vector <vec3> colors;
int iColor = 0;

// Função MAIN
int main()
{
    // Inicialização da GLFW
    glfwInit();

    // Muita atenção aqui: alguns ambientes não aceitam essas configurações
    // Você deve adaptar para a versão do OpenGL suportada por sua placa
    // Sugestão: comente essas linhas de código para desobrir a versão e
    // depois atualize (por exemplo: 4.5 com 4 e 5)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Essencial para computadores da Apple
    // #ifdef __APPLE__
    //  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    // #endif

    // Criação da janela GLFW
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Ola Triangulo! -- Rossana", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // Fazendo o registro da função de callback para a janela GLFW
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // GLAD: carrega todos os ponteiros d funções da OpenGL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
    }

    // Obtendo as informações de versão
    const GLubyte *renderer = glGetString(GL_RENDERER); /* get renderer string */
    const GLubyte *version = glGetString(GL_VERSION);   /* version as a string */
    cout << "Renderer: " << renderer << endl;
    cout << "OpenGL version supported " << version << endl;


    //Inicializando paleta de cores
    colors.push_back(vec3(200, 191, 231));
    colors.push_back(vec3(174, 217, 224));
    colors.push_back(vec3(181, 234, 215));
    colors.push_back(vec3(255, 241, 182));
    colors.push_back(vec3(255, 188, 188));
    colors.push_back(vec3(246, 193, 199));
    colors.push_back(vec3(255, 216, 190));
    colors.push_back(vec3(220, 198, 224));
    colors.push_back(vec3(208, 230, 165));
    colors.push_back(vec3(183, 201, 226));

    // Normalizar as cores entre 0 e 1
    for (int i=0; i<colors.size(); i++)
    {
        colors[i].r /= 255.0;
        colors[i].g /= 255.0;
        colors[i].b /= 255.0;
    }

    // Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // Compilando e buildando o programa de shader
    GLuint shaderID = setupShader();

    glUseProgram(shaderID);

    // Enviando a cor desejada (vec4) para o fragment shader
    // Utilizamos a variáveis do tipo uniform em GLSL para armazenar esse tipo de info
    // que não está nos buffers
    GLint colorLoc = glGetUniformLocation(shaderID, "inputColor");
    GLint modelLoc = glGetUniformLocation(shaderID, "model");

    // Matriz de projeção paralela ortográfica
    mat4 projection = ortho(0.0f, (float)WIDTH, (float)HEIGHT, 0.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "projection"), 1, GL_FALSE, value_ptr(projection));

    // Loop da aplicação - "game loop"
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderID); // <-- Ativa seu shader!

        // Desenha os triângulos criados
        for (size_t i = 0; i < VAOs.size(); ++i) {
            glUniform4f(colorLoc, triangles[i].color.r, triangles[i].color.g, triangles[i].color.b, 1.0f);

            // Matriz de modelo identidade, pois os vértices já estão em coordenadas de tela
            glm::mat4 model = glm::mat4(1.0f);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, value_ptr(model));

            glBindVertexArray(VAOs[i]);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }

        glBindVertexArray(0);
        glfwSwapBuffers(window);
    }
    // Pede pra OpenGL desalocar os buffers
    //glDeleteVertexArrays(1, &VAO);
    // Você precisaria de um loop para desalocar todos os VAOs e VBOs em VAOs
    for (GLuint vao : VAOs) {
        glDeleteVertexArrays(1, &vao);
        // Assumindo que cada VAO tem um VBO associado e pode ser desalocado aqui
        // Isso pode ser mais complexo se VBOs forem compartilhados ou gerenciados separadamente.
    }

    // Finaliza a execução da GLFW, limpando os recursos alocados por ela
    glfwTerminate();
    return 0;
}

// Função de callback de teclado - só pode ter uma instância (deve ser estática se
// estiver dentro de uma classe) - É chamada sempre que uma tecla for pressionada
// ou solta via GLFW
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

// Esta função está basntante hardcoded - objetivo é compilar e "buildar" um programa de
//  shader simples e único neste exemplo de código
//  O código fonte do vertex e fragment shader está nos arrays vertexShaderSource e
//  fragmentShader source no iniçio deste arquivo
//  A função retorna o identificador do programa de shader
int setupShader()
{
    // Vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // Checando erros de compilação (exibição via log no terminal)
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                    << infoLog << std::endl;
    }
    // Fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // Checando erros de compilação (exibição via log no terminal)
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                    << infoLog << std::endl;
    }
    // Linkando os shaders e criando o identificador do programa de shader
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // Checando por erros de linkagem
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                    << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

GLuint createTriangle(float x0, float y0, float x1, float y1, float x2, float y2)
{
    GLuint VAO, VBO;

    // Coordenadas dos vértices do triângulo
    GLfloat vertices[] = {
        x0, y0, 0.0f,
        x1, y1, 0.0f,
        x2, y2, 0.0f
    };

    // 1. Crie e vincule o VAO primeiro
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // 2. Agora crie e vincule o VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 3. Configure os atributos do vértice
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // 4. Desvincule o VBO e depois o VAO 
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}

// Função para converter coordenadas de tela para as coordenadas do seu sistema de projeção
// Agora simplesmente retorna as coordenadas de tela, pois a projeção é ortográfica em pixels.
glm::vec2 screenToNDC(double xpos, double ypos) {
    return glm::vec2(xpos, ypos);
}

// Função adaptada para a cada 3 cliques armazenar as coordenadas e criar o triangulo
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        // Armazena as coordenadas de tela diretamente
        clickedPoints.push_back(glm::vec2(xpos, ypos));

        if (clickedPoints.size() == 3) {
            // Cria o triângulo com as coordenadas clicadas
            GLuint vao = createTriangle(
                clickedPoints[0].x, clickedPoints[0].y,
                clickedPoints[1].x, clickedPoints[1].y,
                clickedPoints[2].x, clickedPoints[2].y
            );
            VAOs.push_back(vao); // Adiciona o VAO à lista

            Triangle tri;
            // A posição e dimensões não são usadas para o desenho direto neste setup,
            // mas a cor é.
            tri.color = vec3(colors[iColor].r, colors[iColor].g, colors[iColor].b);
            iColor = (iColor + 1) % colors.size();
            triangles.push_back(tri); // Adiciona a struct Triangle à lista

            clickedPoints.clear(); // Limpa os pontos para o próximo triângulo
        }
    }
}