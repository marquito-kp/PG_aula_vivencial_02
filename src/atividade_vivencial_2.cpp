/*
 * Hello Triangle - Código adaptado de:
 *   - https://learnopengl.com/#!Getting-started/Hello-Triangle
 *   - https://antongerdelan.net/opengl/glcontext2.html
 *
 * Adaptado por: Rossana Baptista Queiroz
 *
 * Modificado por Marcos Krol Pacheco em 21/06/2025
 *
 *
 */

#include <iostream>
#include <string>
#include <assert.h>
#include <cmath>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// STB_IMAGE
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

// Estrutura para criação de futuros sprites no main
struct Sprite
{
	GLuint VAO;
	GLuint textId;
	vec3 position;
	vec3 dimensions;
	float offsetx;
	float offsety;
	float z;
	float paralax;
};

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);

// Protótipos das funções
int setupShader();
int setupSprite();
int loadTexture(string filePath, int &width, int &heigth);

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 800, HEIGHT = 600;

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded
const GLchar *vertexShaderSource = R"(
 #version 400
 layout (location = 0) in vec3 position;
 layout (location = 1) in vec2 texc;
 uniform mat4 model, projection;

 out vec2 tex_coord;
 void main()
 {
	tex_coord = vec2(texc.s, 1.0 - texc.t);
	gl_Position = projection * model * vec4(position, 1.0);
 }
 )";

// Código fonte do Fragment Shader (em GLSL): ainda hardcoded
const GLchar *fragmentShaderSource = R"(
#version 400
in vec2 tex_coord;
uniform sampler2D tex_buff;
out vec4 color;
void main()
{
    color = texture(tex_buff, tex_coord);
}
 )";

Sprite knight;

// Função MAIN
int main()
{

	// Inicialização da GLFW
	glfwInit();

	// Ativa a suavização de serrilhado (MSAA) com 8 amostras por pixel
	glfwWindowHint(GLFW_SAMPLES, 8);

	// Essencial para computadores da Apple
	// #ifdef __APPLE__
	//	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	// #endif

	// Criação da janela GLFW
	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Ola Triangulo! -- Rossana", nullptr, nullptr);
	if (!window)
	{
		std::cerr << "Falha ao criar a janela GLFW" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Fazendo o registro da função de callback para a janela GLFW
	glfwSetKeyCallback(window, key_callback);

	// GLAD: carrega todos os ponteiros d funções da OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Falha ao inicializar GLAD" << std::endl;
		return -1;
	}

	// Obtendo as informações de versão
	const GLubyte *renderer = glGetString(GL_RENDERER); /* get renderer string */
	const GLubyte *version = glGetString(GL_VERSION);	/* version as a string */
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	// Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, WIDTH, HEIGHT);

	// Compilando e buildando o programa de shader
	GLuint shaderID = setupShader();

	glUseProgram(shaderID); // Reseta o estado do shader para evitar problemas futuros

	double prev_s = glfwGetTime();	// Define o "tempo anterior" inicial.
	double title_countdown_s = 0.1; // Intervalo para atualizar o título da janela com o FPS.

	float colorValue = 0.0;

	// Ativando o primeiro buffer de textura do OpenGL
	glActiveTexture(GL_TEXTURE0);

	// Criando a variável uniform pra mandar a textura pro shader
	glUniform1i(glGetUniformLocation(shaderID, "tex_buff"), 0);

	// Ativação do canal alfa - Transparencia
	glEnable(GL_BLEND);								   // Habilita a transparência
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Seta a função de transparência

	// Habilitar a profundidade
	glEnable(GL_DEPTH_TEST); // habilita o teste de profundidade
	glDepthFunc(GL_ALWAYS);	 //  testa a cada ciclo

	// Variáveis para textura
	int imgWidth, imgHeight;

	// Gerando um buffer simples, com a geometria de um quadrado para o Sprite

	knight.VAO = setupSprite();
	knight.position = vec3(400.0, 100.0, 0.0);
	knight.textId = loadTexture("../assets/sprites/knight.png", imgWidth, imgHeight);
	knight.dimensions = vec3(imgWidth, imgHeight, 0.0);

	Sprite hillsTrees;
	hillsTrees.VAO = setupSprite();
	hillsTrees.position = vec3(400.0, 300.0, 0.0);
	hillsTrees.textId = loadTexture("../assets/layers/hills&trees.png", imgWidth, imgHeight);
	hillsTrees.dimensions = vec3(WIDTH, HEIGHT, 0.0);
	hillsTrees.paralax = 0.3;

	Sprite ruinsBg;
	ruinsBg.VAO = setupSprite();
	ruinsBg.position = vec3(400.0, 300.0, 0.0);
	ruinsBg.textId = loadTexture("../assets/layers/ruins_bg.png", imgWidth, imgHeight);
	ruinsBg.dimensions = vec3(WIDTH, HEIGHT, 0.0);
	ruinsBg.paralax = 0.2;

	Sprite ruins;
	ruins.VAO = setupSprite();
	ruins.position = vec3(400.0, 300.0, 0.0);
	ruins.textId = loadTexture("../assets/layers/ruins.png", imgWidth, imgHeight);
	ruins.dimensions = vec3(WIDTH, HEIGHT, 0.0);
	ruins.paralax = 0.5;

	Sprite ruins2;
	ruins2.VAO = setupSprite();
	ruins2.position = vec3(400.0, 300.0, 0.0);
	ruins2.textId = loadTexture("../assets/layers/ruins2.png", imgWidth, imgHeight);
	ruins2.dimensions = vec3(WIDTH, HEIGHT, 0.0);
	ruins2.paralax = 0.4;

	Sprite sky;
	sky.VAO = setupSprite();
	sky.position = vec3(400.0, 300.0, 0.0);
	sky.textId = loadTexture("../assets/layers/sky.png", imgWidth, imgHeight);
	sky.dimensions = vec3(WIDTH, HEIGHT, 0.0);
	sky.paralax = 0.1;

	Sprite statue;
	statue.VAO = setupSprite();
	statue.position = vec3(400.0, 300.0, 0.0);
	statue.textId = loadTexture("../assets/layers/statue.png", imgWidth, imgHeight);
	statue.dimensions = vec3(WIDTH, HEIGHT, 0.0);
	statue.paralax = 0.7;

	Sprite stonesGrass;
	stonesGrass.VAO = setupSprite();
	stonesGrass.position = vec3(400.0, 300.0, 0.0);
	stonesGrass.textId = loadTexture("../assets/layers/stones&grass.png", imgWidth, imgHeight);
	stonesGrass.dimensions = vec3(WIDTH, HEIGHT, 0.0);
	stonesGrass.paralax = 1.0;

	// Loop da aplicação - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		// Este trecho de código é totalmente opcional: calcula e mostra a contagem do FPS na barra de título
		{
			double curr_s = glfwGetTime();		// Obtém o tempo atual.
			double elapsed_s = curr_s - prev_s; // Calcula o tempo decorrido desde o último frame.
			prev_s = curr_s;					// Atualiza o "tempo anterior" para o próximo frame.

			// Exibe o FPS, mas não a cada frame, para evitar oscilações excessivas.
			title_countdown_s -= elapsed_s;
			if (title_countdown_s <= 0.0 && elapsed_s > 0.0)
			{
				double fps = 1.0 / elapsed_s; // Calcula o FPS com base no tempo decorrido.

				// Cria uma string e define o FPS como título da janela.
				char tmp[256];
				sprintf(tmp, "Sprites! -- Marcos.\tFPS %.2lf", fps);
				glfwSetWindowTitle(window, tmp);

				title_countdown_s = 0.1; // Reinicia o temporizador para atualizar o título periodicamente.
			}
		}

		// Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
		glfwPollEvents();

		// Limpa o buffer de cor
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // cor de fundo
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLineWidth(10);
		glPointSize(20);

		mat4 projection = ortho(0.0, 800.0, 0.0, 600.0, -1.0, 1.0);
		glUniformMatrix4fv(glGetUniformLocation(shaderID, "projection"), 1, GL_FALSE, value_ptr(projection));

		mat4 model = mat4(1);
		// Desenhando os layers
		Sprite layers[] = {sky, ruinsBg, hillsTrees, ruins2, ruins, statue, stonesGrass};
		int lenghtLayers = (sizeof(layers) / sizeof(layers[0]));

		// for (int i = 0; i < lenghtLayers; i++)
		// {
		// 	model = mat4(1.0);
		// 	model = translate(model, layers[i].position + vec3(-knight.position.x * layers[i].paralax,0.0,0.0));
		// 	model = rotate(model, radians(0.0f), vec3(0.0, 0.0, 1.0));
		// 	model = scale(model, layers[i].dimensions);
		// 	glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));
		// 	glBindVertexArray(layers[i].VAO);
		// 	glBindTexture(GL_TEXTURE_2D, layers[i].textId);
		// 	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		// }

		// Neste for ele irá repetir a imagem para o efeito paralax
		for (int i = 0; i < lenghtLayers; i++)
		{
			float parallaxOffset = -knight.position.x * layers[i].paralax;

			float repeatWidth = 800.0;
			float offsetX = fmod(parallaxOffset,repeatWidth);

			if (offsetX < 0.0)
				offsetX	+= repeatWidth;

			for (int j = -2; j <= 2; j++)
			{
				model = mat4(1.0);
				model = translate(model, layers[i].position + vec3(j * repeatWidth + offsetX, 0.0, 0.0));
				model = rotate(model, radians(0.0f), vec3(0.0, 0.0, 1.0));
				model = scale(model, layers[i].dimensions);
				glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));
				glBindVertexArray(layers[i].VAO);
				glBindTexture(GL_TEXTURE_2D, layers[i].textId);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}
		}

		// Desenhando o Knight
		model = mat4(1.0);
		model = translate(model, knight.position);
		model = scale(model, knight.dimensions);
		glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));
		glBindVertexArray(knight.VAO);				 // Conectando ao buffer de geometria
		glBindTexture(GL_TEXTURE_2D, knight.textId); // Conectando ao buffer de textura
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		// glBindVertexArray(0); // Desnecessário aqui, pois não há múltiplos VAOs

		// Troca os buffers da tela
		glfwSwapBuffers(window);
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

	// Variavés para mexer a tela

	float cameraSpeed = 5.0;

	// Lógica para deslocamento da tela
	if (action == GLFW_KEY_SPACE || action == GLFW_REPEAT)
	{
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
			knight.position.x += cameraSpeed;
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
			knight.position.x -= cameraSpeed;
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			knight.position.y += cameraSpeed;
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			knight.position.y -= cameraSpeed;
	}
	// Irá manter o personagem andando na tela
	if (knight.position.x > 800.0)
		knight.position.x = 0.0;
	if (knight.position.x < 0.0)
		knight.position.x = 800.0;
	if (knight.position.y > 800.0)
		knight.position.y = 0.0;
	if (knight.position.y < 0.0)
		knight.position.x = 800.0;
	// Irá limitar a movimentação no eixo y
	if (knight.position.y < 100.0)
		knight.position.y = 100.0;
	if (knight.position.y > 300.0)
		knight.position.y = 300.0;
}

// Esta função está bastante hardcoded - objetivo é compilar e "buildar" um programa de
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

// Esta função está bastante harcoded - objetivo é criar os buffers que armazenam a
// geometria de um triângulo
// Apenas atributo coordenada nos vértices
// 1 VBO com as coordenadas, VAO com apenas 1 ponteiro para atributo
// A função retorna o identificador do VAO
int setupSprite()
{
	// Aqui setamos as coordenadas x, y e z do triângulo e as armazenamos de forma
	// sequencial, já visando mandar para o VBO (Vertex Buffer Objects)
	// Cada atributo do vértice (coordenada, cores, coordenadas de textura, normal, etc)
	// Pode ser arazenado em um VBO único ou em VBOs separados
	GLfloat vertices[] = {
		// s e t são coordenadas de textura
		// x   y     z    s    t
		-0.5, 0.5, 0.0, 0.0, 1.0,  // v0
		-0.5, -0.5, 0.0, 0.0, 0.0, // v1
		0.5, 0.5, 0.0, 1.0, 1.0,   // v2
		0.5, -0.5, 0.0, 1.0, 0.0,  // v3
	};

	GLuint VBO, VAO;
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

	// CONFIGURAÇÃO DO VAO
	// Ponteiro pro atributo 0 - Posição - coordenadas x, y, z
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *)0);
	glEnableVertexAttribArray(0);

	// Ponteiro pro atributo 1 - Coordenada de textura - componentes s, t
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice
	// atualmente vinculado - para que depois possamos desvincular com segurança
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
	glBindVertexArray(0);

	return VAO;
}

// Função de carregar a textura
int loadTexture(string filePath, int &width, int &height)
{
	GLuint texID;

	// Gera o identificador da textura na memória
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	int nrChannels;

	unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);

	if (data)
	{
		if (nrChannels == 3) // jpg, bmp
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		else // png
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, 0);

	return texID;
}