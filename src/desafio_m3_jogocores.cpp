/* Hello Triangle - código adaptado de https://learnopengl.com/#!Getting-started/Hello-Triangle
 *
 * Adaptado por Rossana Baptista Queiroz
 * para a disciplina de Processamento Gráfico - Unisinos
 * Versão inicial: 7/4/2017
 * Última atualização em 13/08/2024
 *
 * Modificado por Marcos Krol Pacheco em 24/05/2025
 * Código adaptado para cumprimento do desafio do Módulo 03 - Jogo das cores
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

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);

// Protótipos das funções
GLuint createQuad();
int setupShader();
int setupGeometry();
void eliminarSimilares(float tolerancia);
bool quadradoEliminado();

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 800, HEIGHT = 600;

// Dimensão de linhas e colunas do GRID
const GLuint ROWS = 6, COLS = 8;
const GLuint QUAD_WIDTH = 100, QUAD_HEIGTH = 100;

// Distância máxima entre cores
const float dMax = sqrt(std::pow(-1, 2) + std::pow(-1, 2) + std::pow(-1, 2));

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded
const GLchar *vertexShaderSource = R"(
#version 400
layout (location = 0) in vec3 position;
uniform mat4 projection;
uniform mat4 model;
void main()	
{
	//...pode ter mais linhas de código aqui!
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

// Criado uma struct para armazenar a posição e cor do Quadrado
struct Quad
{
	vec3 position;
	vec3 dimensions;
	vec3 color;
	bool eliminated;
};

vector<Quad> quads;

vector<vec3> colors;
int iColor = 0;

// Criação de indice para identificar o quadrado selecionado
// e depois fazer o cálculo de distancia de cores
int iSelected = -1;
int pontos = 100;

// Criação da grid de quadrados
Quad grid[ROWS][COLS];

// Função MAIN
int main()
{
	// Sorteio de número para cores
	srand(glfwGetTime());

	// Inicialização da GLFW
	glfwInit();

	// Criação da janela GLFW
	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Jogo das cores -- Marcos.", nullptr, nullptr);
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
	const GLubyte *version = glGetString(GL_VERSION);	/* version as a string */
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	// Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	// Compilando e buildando o programa de shader
	GLuint shaderID = setupShader();

	GLuint VAO = createQuad();

	// Inicializar a grid

	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLS; j++)
		{
			Quad quad;
			// Variável para a posição inicial começar antes de zero, sem ela, a primeira linha e coluna ficam pela metade
			vec2 ini_pos = vec2(QUAD_WIDTH / 2, QUAD_HEIGTH / 2);
			quad.position = vec3(ini_pos.x + j * QUAD_WIDTH, ini_pos.y + i * QUAD_HEIGTH, 0);
			quad.dimensions = vec3(QUAD_WIDTH, QUAD_HEIGTH, 0);
			// Define cores aleatoriamento no padrão RGB
			float r, g, b;
			r = rand() % 256 / 255.00;
			g = rand() % 256 / 255.00;
			b = rand() % 256 / 255.00;
			quad.color = vec3(r, g, b);
			quad.eliminated = false;
			grid[i][j] = quad;
		}
	}

	glUseProgram(shaderID);

	// Enviando a cor desejada (vec4) para o fragment shader
	// Utilizamos a variáveis do tipo uniform em GLSL para armazenar esse tipo de info
	// que não está nos buffers
	GLint colorLoc = glGetUniformLocation(shaderID, "inputColor");

	// Matriz de projeção paralela ortográfica
	// mat4 projection = ortho(-10.0, 10.0, -10.0, 10.0, -1.0, 1.0);
	mat4 projection = ortho(0.0, 800.0, 600.0, 0.0, -1.0, 1.0);
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "projection"), 1, GL_FALSE, value_ptr(projection));

	// Loop da aplicação - "game loop"
	cout << "Bem-vindo ao jogo das cores!\n";
	while (!glfwWindowShouldClose(window))
	{
		// Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
		glfwPollEvents();

		// Limpa o buffer de cor
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // cor de fundo
		glClear(GL_COLOR_BUFFER_BIT);

		glLineWidth(10);
		glPointSize(20);

		glBindVertexArray(VAO); // Conectando ao buffer de geometria

		if (iSelected > -1)
		{
			// Tolerância de 20%
			eliminarSimilares(0.2);
		}

		if (quadradoEliminado())
		{
			std::cout << "Todos os quadrados foram eliminados!" << std::endl;
			cout << "Pontuacao final: " << pontos << std::endl;
			return 0;
		}

		for (int i = 0; i < ROWS; i++)
		{
			for (int j = 0; j < COLS; j++)
			{
				if (!grid[i][j].eliminated)
				{
					// Matriz de modelo: transformações na geometria (objeto)
					mat4 model = mat4(1); // matriz identidade

					// Translação
					model = translate(model, grid[i][j].position);

					// Escala
					model = scale(model, grid[i][j].dimensions);
					glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));

					glUniform4f(colorLoc, grid[i][j].color.r, grid[i][j].color.g, grid[i][j].color.b, 1.0f); // enviando cor para variável uniform inputColor
					// Chamada de desenho - drawcall
					// Poligono Preenchido - GL_TRIANGLE_STRIP
					// A primitiva GL_TRIANGLE_STRIP reaproveita os dois vertices para realizar o desenho do quadrado
					glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
				}
			}
		}
		glBindVertexArray(0); // Desconectando o buffer de geometria

		// Troca os buffers da tela
		glfwSwapBuffers(window);
	}
	// Finaliza a execução da GLFW, limpando os recursos alocados por ela
	glfwTerminate();
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

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		int x = xpos / QUAD_WIDTH;
		int y = ypos / QUAD_HEIGTH;
		// indice linear do quadrado selecionado
		iSelected = y * COLS + x;
	}
}

// Função para criar o quadrado a partir de dois triângulos
// Para criação da 2º metade do triangulo será replicado o v1 e v2 e criado o v3

GLuint createQuad()
{
	GLuint VAO;

	GLfloat vertices[] = {
		// x    y    z
		-0.5, 0.5, 0.0,	 // v0
		-0.5, -0.5, 0.0, // v1
		0.5, 0.5, 0.0,	 // v2
		0.5, -0.5, 0.0,	 // v3
	};

	GLuint VBO;
	// Geração do identificador do VBO
	glGenBuffers(1, &VBO);
	// Faz a conexão (vincula) do buffer como um buffer de array
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// Envia os dados do array de floats para o buffer da OpenGl
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Geração do identificador do VAO (Vertex Array Object)
	glGenVertexArrays(1, &VAO);
	// Vincula (bind) o VAO primeiro, e em seguida  conecta e seta o(s) buffer(s) de vértices
	// e os ponteiros para os atributos
	glBindVertexArray(VAO);
	// Para cada atributo do vertice, criamos um "AttribPointer" (ponteiro para o atributo), indicando:
	//  Localização no shader * (a localização dos atributos devem ser correspondentes no layout especificado no vertex shader)
	//  Numero de valores que o atributo tem (por ex, 3 coordenadas xyz)
	//  Tipo do dado
	//  Se está normalizado (entre zero e um)
	//  Tamanho em bytes
	//  Deslocamento a partir do byte zero
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *)0);
	glEnableVertexAttribArray(0);

	// Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice
	// atualmente vinculado - para que depois possamos desvincular com segurança
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
	glBindVertexArray(0);

	return VAO;
}

// Função irá realizar a lógica de comparação para eliminação das cores similares
// ou seja, irá selecionar uma cor padrão (C) e ira compara-la no for com O, utilizando
// a formula de distância de cores



void eliminarSimilares(float tolerancia)
{
	int x = iSelected % COLS;
	int y = iSelected / COLS;
	vec3 C = grid[y][x].color;
	int quadRemovidos = 0; // Variavel que conta os quadrados removidos
	if (grid[y][x].eliminated == true)
	{
		std::cout << "Quadrado ja foi removido!" << std::endl;
	}
	else
	{
		for (int i = 0; i < ROWS; i++)
		{
			for (int j = 0; j < COLS; j++)
			{
				// Variável está usando o exemplo da formulá de distancia de cores
				// utilizado no material
				vec3 O = grid[i][j].color;
				float d = sqrt(std::pow(C.r - O.r, 2) + std::pow(C.g - O.g, 2) + std::pow(C.b - O.b, 2));
				float dd = d / dMax;
				// Verificando os similares
				if (dd <= tolerancia && !grid[i][j].eliminated)
				{
					grid[i][j].eliminated = true;
					quadRemovidos++;
				}
			}
		}
		if (quadRemovidos>1)
		{
			pontos -= 1;
			std::cout << "Voce perdeu 1 ponto" << std::endl;
		}
		else
		{
			pontos -= 10;
			std::cout << "Voce perdeu 10 pontos." << std::endl;
		}


		// Imprime quantos quadrados forma removidos
		cout << "Quadrados removidos: " << quadRemovidos << std::endl;
		cout << "Pontuação atual: " << pontos << std::endl;
	}
	iSelected = -1;
}

// Função verifica se a matriz foi compleatamente eliminada
bool quadradoEliminado()
{
	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLS; j++)
		{
			if (!grid[i][j].eliminated)
			{
				return false;
			}
		}
	}
	return true;
}