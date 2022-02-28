
#include <iostream>
#include <cstdlib>
#include <glad/glad.h>
#include <GLFW/glfw3.h> // GLFW library

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>      // Image loading Utility functions
// GLM Math header inclusions

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

// Camera Class from learnopenGL
#include<camera.h>
#include "cylinder.h";
#include "Sphere.h"

using namespace std;

// Shader Program Macro
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
	const char* const WINDOW_TITLE = "Final Project"; // Macro for window title

	// Variables for window width and height
	const int WINDOW_WIDTH = 800;
	const int WINDOW_HEIGHT = 600;

	// Stores the GLdata relative to a given mesh
	struct GLMesh
	{
		GLuint vao; // Handle for the vertex array object
		GLuint vbo; // Handle for the vertex buffer objects
		GLuint nVertices; // Number of indices of the mesh
	};

	// Main GLFW window
	GLFWwindow* gWindow = nullptr;
	// Plane mesh data
	GLMesh gPlane;
	// Shader Program 
	GLuint gProgramId;
	// Texture for Table
	GLuint tableTexture, tableTextureSpecular;
	//Textures for book
	GLuint bookCover, bookCoverSpecular, bookPages, bookPagesSpecular;
	//Textuires for Candle
	GLuint candleTexture, candleSpecular;
	//Textures for potion bottle
	GLuint glassTexture, glassSpecular;
	// meshes for book 
	GLMesh gMeshBookBase;
	GLMesh gMeshBookPages;
	//Mesh for hat
	GLMesh hatMesh, hatTopMesh;
	// Texture for hat
	GLuint hatTexture;

	// camera
	Camera gCamera(glm::vec3(2.161944f, 3.725782f, 7.341916f), glm::vec3(0, 1, 0), -106.3f, -35.8501f);


	float gLastX = WINDOW_WIDTH / 2.0f;
	float gLastY = WINDOW_HEIGHT / 2.0f;
	bool gFristMouse = true;

	// timing 
	float gDeltaTime = 0.0f; // time between current frame and last frame
	float gLastFrame = 0.0f;



	// perspective switch 
	bool perspectiveSwitch = false;
}

/*
   Fucntion prototypes to:
   Initialize the program, set the window size,
   redraw graphics on the window when resized,
   and render graphics on the screen
*/

bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void UCreateMeshBookBase(GLMesh& mesh);
void UCreatePlaneMesh(GLMesh& mesh);
void UCreatePagesMesh(GLMesh& mesh);
void UCreateHatMesh(GLMesh& mesh);
void UCreateHatBaseMesh(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
glm::vec3 computeFaceNormal(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);
// Vertex Shader source code
const GLchar* vertexShaderSource = GLSL(440,
	layout(location = 0) in vec3 aPos;
	layout(location = 1) in vec3 aNormal;
	layout(location = 2) in vec2 aTexCoords;

	out vec3 FragPos;
	out vec3 Normal;
	out vec2 TexCoords;

	uniform mat4 model;
	uniform mat4 view;
	uniform mat4 projection;

void main()
{
	FragPos = vec3(model * vec4(aPos, 1.0));
	Normal = mat3(transpose(inverse(model))) * aNormal;
	TexCoords = aTexCoords;

	gl_Position = projection * view * vec4(FragPos, 1.0);
}
);

// Fragment Shader Source Code
const GLchar* fragmentShaderSource = GLSL(440,
	out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;    
    float shininess;
}; 

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoords;
  
uniform vec3 viewPos;
uniform Material material;
uniform Light light;
uniform PointLight pointLight;

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}
void main()
{
    // ambient
    vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;  
    
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb;  
        
    vec3 result = ambient + diffuse + specular;
	result += CalcPointLight(pointLight, norm, FragPos, viewDir);
    FragColor = vec4(result, 1.0);
} 
);

int main(int argc, char* argv[])
{
	if (!UInitialize(argc, argv, &gWindow))
		return EXIT_FAILURE;


	// Create Meshes
	UCreateMeshBookBase(gMeshBookBase);
	UCreatePagesMesh(gMeshBookPages);// Calls function to create the VBO
	UCreatePlaneMesh(gPlane); // create plane 
	UCreateHatBaseMesh(hatMesh);
	UCreateHatMesh(hatTopMesh);

	// Create the shader Program
	if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
		return EXIT_FAILURE;

	// Sets background color of the window to black
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	//Generate table texture
	const char* texFilename = "./Resources/table.jpg";
	if (!UCreateTexture(texFilename, tableTexture))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}

	//Generate table specualr map
	 texFilename = "./Resources/tablespecular.jpg";
	if (!UCreateTexture(texFilename, tableTextureSpecular))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}

	// Generate Book cover texture
	texFilename = "./Resources/spellbookcover.png";
	if (!UCreateTexture(texFilename, bookCover))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}

	// Generate Book cover specular map
	texFilename = "./Resources/spellbookcoverspecular.png";
	if (!UCreateTexture(texFilename, bookCoverSpecular))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}

	// Generate Spellbook pages texture
	texFilename = "./Resources/spellbookpages1.png";
	if (!UCreateTexture(texFilename, bookPages))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}

	// Generate Spellbook pages texture
	texFilename = "./Resources/spellbookpagesspecular.jpg";
	if (!UCreateTexture(texFilename, bookPagesSpecular))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}

	//Generate candle texture
	texFilename = "./Resources/candle.jpg";
	if (!UCreateTexture(texFilename, candleTexture))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}

	//Generate candle specular map
	texFilename = "./Resources/candlespecular.jpg";
	if (!UCreateTexture(texFilename, candleSpecular))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}

	//Generate glass texture
	texFilename = "./Resources/glass.png";
	if (!UCreateTexture(texFilename, glassTexture))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}

	//Generate glass specular map
	texFilename = "./Resources/glassspecular.png";
	if (!UCreateTexture(texFilename, glassSpecular))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}

	//Generate glass specular map
	texFilename = "./Resources/hat.png";
	if (!UCreateTexture(texFilename, hatTexture))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}
	glUseProgram(gProgramId);

	// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
	glUniform1i(glGetUniformLocation(gProgramId, "material.diffuse"), 0);
	glUniform1i(glGetUniformLocation(gProgramId, "material.specular"), 1);

	// Render loop
	while (!glfwWindowShouldClose(gWindow))
	{
		// Get frame timing
		float currentFrame = glfwGetTime();
		gDeltaTime = currentFrame - gLastFrame;
		gLastFrame = currentFrame;

		// Process input
		UProcessInput(gWindow);

		// Render Frame
		URender();

		// Poll GLFW events
		glfwPollEvents();
	}

	// Release mesh data after render loop ends
	UDestroyMesh(gMeshBookBase);
	UDestroyMesh(gMeshBookBase);
	UDestroyMesh(gMeshBookPages);
	UDestroyMesh(gPlane);
	UDestroyMesh(hatMesh);
	UDestroyMesh(hatTopMesh);
	

	// Release shader program after render loop ends
	UDestroyShaderProgram(gProgramId);

	// Destroy textures
	UDestroyTexture(tableTexture);
	UDestroyTexture(tableTextureSpecular);
	UDestroyTexture(bookCover);
	UDestroyTexture(bookCoverSpecular);
	UDestroyTexture(bookPages);
	UDestroyTexture(bookPagesSpecular);
	UDestroyTexture(candleSpecular);
	UDestroyTexture(candleTexture);
	UDestroyTexture(glassSpecular);
	UDestroyTexture(glassTexture);
	UDestroyTexture(hatTexture);


	exit(EXIT_SUCCESS); // Terminate program sucessfully
}

// Initialize GLFW, GLEW, and create window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
	// GLFW: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	stbi_set_flip_vertically_on_load(true);
#ifdef _APPLE_
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// GLFW: window creation

	* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
	if (*window == NULL)
	{
		cout << "Failed to create GLFW window" << endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(*window);
	glfwSetFramebufferSizeCallback(*window, UResizeWindow);
	glfwSetCursorPosCallback(*window, UMousePositionCallback);
	glfwSetScrollCallback(*window, UMouseScrollCallback);
	glfwSetKeyCallback(*window, key_callback);

	// tell GLFW to capture the mouse
	glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// Displays GPU openGL version
	cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;



	return true;
}

// Process all inpout: query GLFW wether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		gCamera.ProcessKeyboard(LEFT, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		gCamera.Position += (gCamera.MovementSpeed * gDeltaTime) * gCamera.Up;
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		gCamera.Position -= (gCamera.MovementSpeed * gDeltaTime) * gCamera.Up;

}

// glfw: whenever the window size is changed by the OS or user, this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// glfw: Whenver the mouse moves this callback is called
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (gFristMouse) {
		gLastX = xpos;
		gLastY = ypos;
		gFristMouse = false;
	}

	float xoffset = xpos - gLastX;
	float yoffset = gLastY - ypos;

	gLastX = xpos;
	gLastY = ypos;

	gCamera.ProcessMouseMovement(xoffset, yoffset);
	std::cout << "Yaw: " << gCamera.Yaw << endl;
	std::cout << "Pitch: " << gCamera.Pitch << endl;
	std::cout << "Position: " << to_string(gCamera.Position) << endl;
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	gCamera.MovementSpeed += yoffset;

	// Ensure that camera speed does not enter negatives, overflow, or go too fast or too slow.
	if (gCamera.MovementSpeed < 2.5f)
		gCamera.MovementSpeed = 2.5f;
	if (gCamera.MovementSpeed > 100.0f)
		gCamera.MovementSpeed = 100.0f;
}

//glfw: whenever key is pressed this callback is called, we do this so that the key is only accepted once on input instead of every frame when trying to swap camera projection
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		perspectiveSwitch = !perspectiveSwitch;
	}
}

// Implements the UCreateMeshBookBase function
void UCreateMeshBookBase(GLMesh& mesh)
{
	// Position and Color data
	GLfloat verts[] = {
		// Vertex Positions			// Normals		// Texture Coordinates
		//Top half first triangle
		 1.00f,  0.025f, -0.5f,	1.0f, 1.0f, 1.0f,	0.40f, 0.40f, // Top back Right
		 -1.00f,  0.025f, -0.5f,	1.0f, 1.0f, 1.0f,	0.10f, 0.40f, // Top back left 
		 1.00f,  0.025f,  0.5f,	1.0f, 1.0f, 1.0f,	0.40f, 0.10f, // Top front Right
		//Top half second Triangle
		  1.00f,  0.025f,  0.5f,	1.0f, 1.0f, 1.0f,	0.40f, 0.10f, // Top front Right
		   -1.00f,  0.025f, -0.5f,	1.0f, 1.0f, 1.0f,	0.10f, 0.40f, // Top back left 
		 -1.00f,  0.025f,  0.5f,	1.0f, 1.0f, 1.0f,	0.10f, 0.10f, // Top Front Left
		
		//Bottom half first triangle 
		 -1.00f,  0.0f, -0.5f,		1.0f, 1.0f, 1.0f,	1.0f, 1.0f, // Bottom back left 
		  1.00f,  0.0f, -0.5f,		1.0f, 1.0f, 1.0f,	0.0f, 1.00f, // Bottom back Right
		  1.00f,  0.0f,  0.5f,		1.0f, 1.0f, 1.0f,	0.00f, 0.00f, // Bottom front Right
		//Bottom half second Traingle
		  1.00f,  0.0f,  0.5f,		1.0f, 1.0f, 1.0f,	0.0f,  0.00f, // Bottom front Right
		 -1.00f,  0.0f,  0.5f,		1.0f, 1.0f, 1.0f,	1.0f,  0.0f, // Bottom Front Left
		 -1.00f,  0.0f, -0.5f,		1.0f, 1.0f, 1.0f,	1.0f,  1.0f, // Bottom back left 
		 //Left Side first triangle 
		  -1.00f,  0.025f,  0.5f,	1.0f, 1.0f, 1.0f,	0.25f, 1.0f, // Top Front Left
		  -1.00f,  0.025f, -0.5f,	1.0f, 1.0f, 1.0f,	0.20f, 1.0f, // Top back left 
		 -1.00f,  0.0f,  0.5f,		1.0f, 1.0f, 1.0f,	0.25f, 0.85f, // Bottom Front Left
		 //Left Side Second triangle
		  -1.00f,  0.0f,  0.5f,		1.0f, 1.0f, 1.0f,	0.25f, 0.85f, // Bottom Front Left
		  -1.00f,  0.025f, -0.5f,	1.0f, 1.0f, 1.0f,	0.20f, 1.0f, // Top back left 
		  -1.00f,  0.0f, -0.5f,		1.0f, 1.0f, 1.0f,	0.20f, 0.85f,  // Bottom back left
		  //Right Side first triangle
		   1.00f,  0.025f, -0.5f,	1.0f, 1.0f, 1.0f,	0.20f, 1.0f, // Top back right
		   1.00f,  0.025f,  0.5f,	1.0f, 1.0f, 1.0f,	0.25f, 1.0f, // Top Front right
		   1.00f,  0.0f,  0.5f,		1.0f, 1.0f, 1.0f,	0.25f, 0.85f, // Bottom Front right
		   //Right side second triangle
		   1.00f,  0.0f,  0.5f,		1.0f, 1.0f, 1.0f,	0.25f, 0.85f, // Bottom Front right
		   1.00f,  0.0f, -0.5f,		1.0f, 1.0f, 1.0f,	0.20f, 0.85f,  // Bottom back right
		   1.00f,  0.025f, -0.5f,	1.0f, 1.0f, 1.0f,	0.20f, 1.0f, // Top back right
		   //Back side first triangle
			1.00f,  0.0f, -0.5f,	1.0f, 1.0f, 1.0f,	0.20f, 0.85f,  // Bottom back right
			  -1.00f,  0.0f, -0.5f,	1.0f, 1.0f, 1.0f,	0.20f, 0.85f,  // Bottom back left
			 -1.00f,  0.025f, -0.5f,	1.0f, 1.0f, 1.0f,	0.20f, 1.0f, // Top back left 
			//Back Side second triangle 
			-1.00f,  0.025f, -0.5f,	1.0f, 1.0f, 1.0f,	0.20f, 1.0f, // Top back left 
			1.00f,  0.025f, -0.5f,	1.0f, 1.0f, 1.0f,	0.20f, 1.0f, // Top back right
			1.00f,  0.0f, -0.5f,	1.0f, 1.0f, 1.0f,	0.20f, 0.85f,  // Bottom back right
			//front side first triangle
			 -1.00f,  0.025f,  0.5f,	1.0f, 1.0f, 1.0f,	0.25f, 1.0f, // Top Front Left
			 -1.00f,  0.0f,  0.5f,		1.0f, 1.0f, 1.0f,	0.25f, 0.85f, // Bottom Front Left
			  1.00f,  0.0f,  0.5f,		1.0f, 1.0f, 1.0f,	0.25f, 0.85f, // Bottom Front right
			 // front side second triangle 
			 1.00f,  0.0f,  0.5f,		1.0f, 1.0f, 1.0f, 	0.25f, 0.85f, // Bottom Front right
			 1.00f,  0.025f,  0.5f,		1.0f, 1.0f, 1.0f,  	0.25f, 1.0f, // Top Front right
			 -1.00f,  0.025f,  0.5f,	1.0f, 1.0f, 1.0f,  	0.25f, 1.0f, // Top Front Left
	};


	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	const GLuint floatsPerVertex = 3;
	const GLuint floatsPerNormal = 3;
	const GLuint floatsPerUV = 2;
	const GLuint floatsPerEntry = (floatsPerVertex + floatsPerNormal + floatsPerUV);

	// Strides between vertex coordinates is 8 (x, y, z, r, g, b, a, s, t). A tightly packed stride is 0.
	GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);// The number of floats before each
	mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV)); // Calculate number of vertices from array

	// Calculate and update normals for reach vertex
	for (unsigned int i = 0; i < sizeof(verts)/sizeof(verts[0]); i += 3 * floatsPerEntry) {
        glm::vec3 A = glm::vec3(verts[i], verts[i+1],verts[i+2]);
        glm::vec3 B = glm::vec3(verts[i + floatsPerEntry], verts[i+1 + floatsPerEntry],verts[i+2 + floatsPerEntry]);
        glm::vec3 C = glm::vec3(verts[i + 2 * floatsPerEntry], verts[i + 1 + 2 * floatsPerEntry], verts[i + 2 + 2 * floatsPerEntry]);
        glm::vec3 normal = computeFaceNormal(A, B, C);
		
		for (int j = 0; j < 3; j++) {
			verts[i + 3 + j * floatsPerEntry] = normal.x;
			verts[i + 4 + j * floatsPerEntry] = normal.y;
			verts[i + 5 + j * floatsPerEntry] = -normal.z;
		}
    }

	glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
	glBindVertexArray(mesh.vao);

	// Create buffer for vertices
	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU


	// Create Vertex Attribute Pointers
	glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
	glEnableVertexAttribArray(2);
}

// Implements the UCreatePlaneMesh function
void UCreatePlaneMesh(GLMesh& mesh)
{
	// Position and Color data
	GLfloat verts[] = {
		// Vertex Positions    // Normals	//Texture Coordinates 

		//First Traingle for Plane
		-5, 0, 2.5,			1.0f, 0.0f, 0.0f, 			0.0f, 0.0f, // front left vertex
		 5, 0, -2.5,		1.0f, 0.0f, 0.0f,			1.0f, 1.0f,// back right vertex
		-5, 0, -2.5,		1.0f, 0.0f, 0.0f, 			0.0f, 1.0f, // back left vertex
		

		 //Second Triangle for Plane
		 -5, 0, 2.5,		1.0f,0.0f, 0.0f, 			0.0f, 0.0f, // front left vertex
		 5, 0, 2.5,			1.0f, 0.0f, 0.0f, 			1.0f, 0.0f, // front right vertex
		5, 0, -2.5,			1.0f, 0.0f, 0.0f, 			1.0f, 1.0f// back right vertex

	};

	const GLuint floatsPerVertex = 3;
	const GLuint floatsPerNormal = 3;
	const GLuint floatsPerUV = 2;
	const GLuint floatsPerEntry = (floatsPerVertex + floatsPerNormal + floatsPerUV);

	// Strides between vertex coordinates is 9 (x, y, z, r, g, b, a). A tightly packed stride is 0.
	GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);// The number of floats before each
	mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV)); // Calculate number of vertices from array

	// Calculate and update normals for reach vertex
	for (unsigned int i = 0; i < sizeof(verts)/sizeof(verts[0]); i += 3 * floatsPerEntry) {
        glm::vec3 A = glm::vec3(verts[i], verts[i+1],verts[i+2]);
        glm::vec3 B = glm::vec3(verts[i + floatsPerEntry], verts[i+1 + floatsPerEntry],verts[i+2 + floatsPerEntry]);
        glm::vec3 C = glm::vec3(verts[i + 2 * floatsPerEntry], verts[i + 1 + 2 * floatsPerEntry], verts[i + 2 + 2 * floatsPerEntry]);
        glm::vec3 normal = computeFaceNormal(A, B, C);
	
		for (int j = 0; j < 3; j++) {
			verts[i + 3 + j * floatsPerEntry] = normal.x;
			verts[i + 4 + j * floatsPerEntry] = normal.y;
			verts[i + 5 + j * floatsPerEntry] = -normal.z;
		}
    }

	glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
	glBindVertexArray(mesh.vao);

	// Create buffer: for the vertex data
	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU


	// Create Vertex Attribute Pointers
	glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
	glEnableVertexAttribArray(2);
}

// Implements the UCreatePagesMesh function
void UCreatePagesMesh(GLMesh& mesh)
{
	// Position and Color data
	GLfloat verts[] = {
		// Vertex Positions    // Normals				//Texture Coordinates 

		//Left side top
		// 
		//First Traingle top left first 
		 -.65,  1.0,  -0.5,		1.0f, 0.0f, 0.0f,		0.17f, 1.0f,// back right vertex
		 -1, 0.75,  -0.5,		1.0f, 0.0f, 0.0f,		 0.0f, 1.0f, // back left vertex
		 -1, 0.75,   0.5,		1.0f, 0.0f, 0.0f,		 0.0f, 0.0f, // front left vertex
		//Second Trangle top left first part
		  -.65,  1.0,   0.5,		1.0f, 0.0f, 0.0f,		0.17f, 0.0f, // front right vertex
		   -.65,  1.0,  -0.5,		1.0f, 0.0f, 0.0f,		0.17f, 1.0f,// back right vertex
		  -1, 0.75,   0.5,		1.0f, 0.0f, 0.0f,		 0.0f, 0.0f, // front left vertex
		 //First Traingle top left second part
		 -.65, 1.0,  -0.5,		1.0f, 0.0f, 0.0f,		0.17f, 1.0f,// back left vertex
		 -.65, 1.0,   0.5,		1.0f, 0.0f, 0.0f,		0.17f, 0.0f, // front left vertex
		 -.35, 1.0,   0.5,		1.0f, 0.0f, 0.0f,		0.30f, 0.0f, // front right vertex
		 //Second Traingle top left second part
		 -.35, 1.0,   0.5,		1.0f, 0.0f, 0.0f,		0.30f, 0.0f, // front right vertex
		 -.35, 1.0,  -0.5,		1.0f, 0.0f, 0.0f,		0.30f, 1.0f,// back right vertex
		 -.65, 1.0,  -0.5,		1.0f, 0.0f, 0.0f,		0.17f, 1.0f,// back left vertex
		 //First Triangle top left third part
		 -0, 0.75,  -0.5,		1.0f, 0.0f, 0.0f,		0.5f,  1.0f, // back right vertex
		  -.35, 1.0,  -0.5,		1.0f, 0.0f, 0.0f,		0.30f, 1.0f,// back left vertex
		 -.35, 1.0,   0.5,		1.0f, 0.0f, 0.0f,		0.30f, 0.0f, // front left vertex
		//Second Traingle top left third part
		   -.35,  1.0,  0.5,		1.0f, 0.0f, 0.0f,		0.30f, 0.0f, // front left vertex
		   -0, 0.75,  0.5,		1.0f, 0.0f, 0.0f,		0.5f,  0.0f, // front right vertex
		    -0, 0.75, -0.5,		1.0f, 0.0f, 0.0f,		0.5f,  1.0f, // back right vertex

		 //Right side top
		// 
		//First Traingle top left first part
		   1,  0.75, 0.5,		1.0f, 0.0f, 0.0f,		1.00f, 0.0f, // front right vertex
		   1,  0.75, -0.5,		1.0f, 0.0f, 0.0f,		1.00f, 1.0f, // back right vertex
		 .65,   1.0, -0.5,		1.0f, 0.0f, 0.0f,		0.85f, 1.0f, // back left vertex
		//Second Trangle top left first part
		 .65,  1.0,  -0.5,		1.0f, 0.0f, 0.0f,		0.85f, 1.0f, // back left vertex
		 .65,  1.0,   0.5,		1.0f, 0.0f, 0.0f,		0.85f, 0.0f, // front left vertex
		   1, 0.75,   0.5,		1.0f, 0.0f, 0.0f,		1.00f, 0.0f, // front right vertex
		//First Traingle top left second part
		 .65,  1.0,  -0.5,		1.0f, 0.0f, 0.0f,		0.85f, 1.0f, // back right vertex
		  .35,  1.0,   0.5,		1.0f, 0.0f, 0.0f,		0.67f, 0.0f, // front left vertex
		 .65,  1.0,   0.5,		1.0f, 0.0f, 0.0f,		0.85f, 0.0f, // front right vertex
		
		 //Second Traingle top left second part
		 .65,  1.0,  -0.5,		1.0f, 0.0f, 0.0f,		0.85f, 1.0f, // back right vertex
		 .35,  1.0,  -0.5,		1.0f, 0.0f, 0.0f,		0.67f, 1.0f, // back left vertex
		.35,  1.0,   0.5,		1.0f, 0.0f, 0.0f,		0.67f, 0.0f,// front left vertex
		 //First Triangle top left third part
		 .35,  1.0,   0.5,		1.0f, 0.0f, 0.0f,		0.67,  0.0f,  // front right vertex
		 .35,  1.0,  -0.5,		1.0f, 0.0f, 0.0f,		0.67f, 1.0f, // back right vertex
		   0, 0.75,  -0.5,		1.0f, 0.0f, 0.0f,		0.5f, 1.0f, // back left vertex
		 //Second Traingle top left third part
		   0, 0.75,  -0.5,		1.0f, 0.0f, 0.0f,		0.5f, 1.0f, // back left vertex
		   0, 0.75,   0.5,		1.0f, 0.0f, 0.0f,		0.5f, 0.0f, // front left vertex
		 .35,  1.0,   0.5,		1.0f, 0.0f, 0.0f,		0.67f, 0.0f, // front right vertex

		 //Bottom face
		 //
		 //First Traingle 
		  1,  0.0,  0.5,		1.0f, 0.0f, 0.0f,		0.0f, 0.0f, // front left vertex
		  -1,  0.0, -0.5,		1.0f, 0.0f, 0.0f,		1.0f, 1.0f, // back right vertex
		  1,  0.0, -0.5,		1.0f, 0.0f, 0.0f,		0.0f, 1.0f, // back left vertex
		 
		 //Second Traingle 
		 -1,  0.0, -0.5,		1.0f, 0.0f, 0.0f,		1.0f, 1.0f, // back right vertex
		  1,  0.0,  0.5,		1.0f, 0.0f, 0.0f,		0.0f, 0.0f, // front left vertex
		 -1,  0.0,  0.5,		1.0f, 0.0f, 0.0f,		1.0f, 0.0f, // front right vertex
		 

		  //Left Face
		  //
		  //First Triangle
		 -1,  0.0,   0.5,		1.0f, 0.0f, 0.0f,		0.0f, 0.0f, // front left bottom vertex
		  -1, 0.75,  -0.5,		1.0f, 0.0f, 0.0f,		0.5f, 1.0f, // back left top vertex
		 -1,  0.0,  -0.5,		1.0f, 0.0f, 0.0f,		0.0f, 1.0f, // back left bottom vertex
		
		 // Second Triangle
		 -1, 0.75,  -0.5,		1.0f, 0.0f, 0.0f,		0.5f, 1.0f, // back left top vertex
		 -1,  0.0,   0.5,		1.0f, 0.0f, 0.0f,		0.0f, 0.0f, // front left bottom vertex
		 -1, 0.75,   0.5,		1.0f, 0.0f, 0.0f,		0.5f, 0.0f, // front left top vertex
		 

		 //Right
		 //
		 //First Triangle
		 1,  0.0,   0.5,		1.0f, 0.0f, 0.0f,		0.0f, 0.0f, // front left bottom vertex
		  1,  0.0,  -0.5,		1.0f, 0.0f, 0.0f,		0.0f, 1.0f, // back left bottom vertex
		  1, 0.75,  -0.5,		1.0f, 0.0f, 0.0f,		0.5f, 1.0f, // back left top vertex
		
		
		 // Second Triangle
		 1, 0.75,  -0.5,		1.0f, 0.0f, 0.0f,		0.5f, 1.0f, // back left top vertex
		  1, 0.75,   0.5,		1.0f, 0.0f, 0.0f,		0.5f, 0.0f, // front left top vertex
		  1,  0.0,   0.5,		1.0f, 0.0f, 0.0f,		0.0f, 0.0f, // front left bottom vertexh
		
		

		 //Front Face
		 //
		 //First block first triangle 
		 -1,    0.0,   0.5,		1.0f, 0.0f, 0.0f,		0.0f, 0.0f, // front left bottom vertex
		 -.65,  1.0,   0.5,		1.0f, 0.0f, 0.0f,		0.17f, 1.0f, // front right top vertex
		 -1,   0.75,   0.5,		1.0f, 0.0f, 0.0f,		0.0f, 1.0f, // front left top vertex
		 
		 //First Block Second Triangle
		 -.65,  1.0,   0.5,		1.0f, 0.0f, 0.0f,		0.17f, 1.0f, // front right top vertex
			  -1,    0.0,   0.5,		1.0f, 0.0f, 0.0f,		0.0f, 0.0f, // front left bottom vertex
		 -.65,  0.0,   0.5,		1.0f, 0.0f, 0.0f,		0.17f, 0.0f, // front right bottom vertex
		
		//Second Block First Triangle
		-.65, 1.0, 0.5,			1.0f, 0.0f, 0.0f,		0.17f, 1.0f, // front left top vertex
		-.65, 0.0, 0.5,			1.0f, 0.0f, 0.0f,		0.17f, 0.0f, // front left bottom vertex
		-.35, 0.0, 0.5,			1.0f, 0.0f, 0.0f,		0.30f, 0.0f, // front right bottom vertex
		//Second block second Triangle
		-.35, 0.0, 0.5,			1.0f, 0.0f, 0.0f,		0.30f, 0.0f, // front right bottom vertex
		-.35, 1.0, 0.5,			1.0f, 0.0f, 0.0f,		0.30f, 1.0f, // front right top vertex
		-.65, 1.0, 0.5,			1.0f, 0.0f, 0.0f,		0.17f, 1.0f, // front left top vertex
		//Third Block first triangle
		-.35, 0.0, 0.5,			1.0f, 0.0f, 0.0f,		0.30f, 0.0f, // front left bottom vertex
		-0, 0.75, 0.5,			1.0f, 0.0f, 0.0f,		0.50f, 1.0f, // front right top vertex
		-.35, 1.0, 0.5,			1.0f, 0.0f, 0.0f,		0.30f, 1.0f, // front left top vertex
		 
		//Third Block second Triangle
		 -0, 0.75, 0.5,			1.0f, 0.0f, 0.0f,		0.50f, 1.0f, // front right top vertex
		-.35, 0.0, 0.5,			1.0f, 0.0f, 0.0f,		0.30f, 0.0f, // front left bottom vertex
		 -0,  0.0, 0.5,			1.0f, 0.0f, 0.0f,		0.50f, 0.0f, // front right bottom vertex
	
		//
		// Right Side Front face
		//First block first triangle 
		 1,   0.0, 0.5,			1.0f, 0.0f, 0.0f,		0.0f, 0.0f, // front left bottom vertex
			  
		 1,  0.75, 0.5,			1.0f, 0.0f, 0.0f,		0.0f, 1.0f, // front left top vertex
			  .65,  1.0, 0.5,			1.0f, 0.0f, 0.0f,		0.17f, 1.0f, // front right top vertex
		
		 //First Block Second Triangle
		.65,  1.0,   0.5,		1.0f, 0.0f, 0.0f,		0.17f, 1.0f, // front right top vertex
			 
		.65,  0.0,   0.5,		1.0f, 0.0f, 0.0f,		0.17f, 0.0f, // front right bottom vertex
			  1,  0.0,   0.5,		1.0f, 0.0f, 0.0f,		0.0f, 0.0f, // front left bottom vertex
		 
		//Second Block First Triangle
		.65, 1.0,	0.5,		1.0f, 0.0f, 0.0f,		0.17f, 1.0f, // front left top vertex
		.35, 0.0, 0.5,			1.0f, 0.0f, 0.0f,		0.30f, 0.0f, // front right bottom vertex
		.65, 0.0, 0.5,			1.0f, 0.0f, 0.0f,		0.17f, 0.0f, // front left bottom vertex
		
		//Second block second Triangle
		.35, 0.0, 0.5,			1.0f, 0.0f, 0.0f,		0.30f, 0.0f, // front right bottom vertex
		.65, 1.0, 0.5,			1.0f, 0.0f, 0.0f,		0.17f, 1.0f, // front left top vertex
		.35, 1.0, 0.5,			1.0f, 0.0f, 0.0f,		0.30f, 1.0f, // front right top vertex
		
		//Third Block first triangle
		.35, 0.0, 0.5,			1.0f, 0.0f, 0.0f,		0.30f, 0.0f, // front left bottom vertex
		.35, 1.0, 0.5,			1.0f, 0.0f, 0.0f,		0.30f, 1.0f, // front left top vertex
		 0, 0.75, 0.5,			1.0f, 0.0f, 0.0f,		0.50f, 1.0f, // front right top vertex
		//Third Block second Triangle
		 0, 0.75, 0.5,			1.0f, 0.0f, 0.0f,		0.50f, 1.0f, // front right top vertex
		0,  0.0, 0.5,			1.0f, 0.0f, 0.0f,		0.50f, 0.0f, // front right bottom vertex
		.35, 0.0, 0.5,			1.0f, 0.0f, 0.0f,		0.30f, 0.0f, // front left bottom vertex
		

		// back Face
		//left side
		 //
		 //First block first triangle 
		 -1,  0.0,  -0.5,		1.0f, 0.0f, 0.0f,		0.0f, 0.0f, // front left bottom vertex
		 -1, 0.75,  -0.5,		1.0f, 0.0f, 0.0f,		0.0f, 1.0f, // front left top vertex
		 -.65,  1.0, -0.5,		1.0f, 0.0f, 0.0f,		0.17f, 1.0f, // front right top vertex
		 //First Block Second Triangle
		 -.65,  1.0,  -0.5,		1.0f, 0.0f, 0.0f,		0.17f, 1.0f, // front right top vertex
		 -.65,  0.0,  -0.5,		1.0f, 0.0f, 0.0f,		0.17f, 0.0f, // front right bottom vertex
		 -1,  0.0,  -0.5,		1.0f, 0.0f, 0.0f,		0.0f, 0.0f, // front left bottom vertex
		//Second Block First Triangle
		-.65, 1.0, -0.5,		1.0f, 0.0f, 0.0f,		0.17f, 1.0f, // front left top vertex
		-.35, 0.0, -0.5,		1.0f, 0.0f, 0.0f,		0.30f, 0.0f, // front right bottom vertex
		-.65, 0.0, -0.5,		1.0f, 0.0f, 0.0f,		0.17f, 0.0f, // front left bottom vertex
		
		//Second block second Triangle
		-.35, 0.0, -0.5,		1.0f, 0.0f, 0.0f,		0.30f, 0.0f, // front right bottom vertex
		-.65, 1.0, -0.5,		1.0f, 0.0f, 0.0f,		0.17f, 1.0f, // front left top vertex
		-.35, 1.0, -0.5,		1.0f, 0.0f, 0.0f,		0.30f, 1.0f, // front right top vertex
	
		//Third Block first triangle
		-.35, 0.0, -0.5,		1.0f, 0.0f, 0.0f,		0.30f, 0.0f, // front left bottom vertex
		-.35, 1.0, -0.5,		1.0f, 0.0f, 0.0f,		0.30f, 1.0f, // front left top vertex
		 -0, 0.75, -0.5,		1.0f, 0.0f, 0.0f,		0.50f, 1.0f, // front right top vertex
		//Third Block second Triangle
		 -0, 0.75, -0.5,		1.0f, 0.0f, 0.0f,		0.50f, 1.0f, // front right top vertex
		 -0,  0.0, -0.5,		1.0f, 0.0f, 0.0f,		0.50f, 0.0f, // front right bottom vertex
		-.35, 0.0, -0.5,		1.0f, 0.0f, 0.0f,		0.30f, 0.0f, // front left bottom vertex
		//
		// Right Side back face
		//First block first triangle 
		 1,  0.0,   -0.5,		1.0f, 0.0f, 0.0f,		0.0f, 0.0f, // front left bottom vertex
		.65,  1.0,   -0.5,		1.0f, 0.0f, 0.0f,		0.17f, 1.0f, // front right top vertex
		 1, 0.75,   -0.5,		1.0f, 0.0f, 0.0f,		0.0f, 1.0f, // front left top vertex
		 
		 //First Block Second Triangle
		 .65,  1.0,   -0.5,		1.0f, 0.0f, 0.0f,		0.17f, 1.0f, // front right top vertex
		  1,  0.0,   -0.5,		1.0f, 0.0f, 0.0f,		0.0f, 0.0f, // front left bottom vertex
		 .65,  0.0,   -0.5,		1.0f, 0.0f, 0.0f,		0.17f, 0.0f, // front right bottom vertex
		
		//Second Block First Triangle
		.65, 1.0, -0.5,			1.0f, 0.0f, 0.0f,		0.17f, 1.0f, // front left top vertex
		.65, 0.0, -0.5,			1.0f, 0.0f, 0.0f,		0.17f, 0.0f, // front left bottom vertex
		.35, 0.0, -0.5,			1.0f, 0.0f, 0.0f,		0.30f, 0.0f, // front right bottom vertex
		//Second block second Triangle
		.35, 0.0, -0.5,			1.0f, 0.0f, 0.0f,		0.30f, 0.0f, // front right bottom vertex
		.35, 1.0, -0.5,			1.0f, 0.0f, 0.0f,		0.30f, 1.0f, // front right top vertex
		.65, 1.0, -0.5,			1.0f, 0.0f, 0.0f,		0.17f, 1.0f, // front left top vertex
		//Third Block first triangle
		.35, 0.0, -0.5,			1.0f, 0.0f, 0.0f,		0.30f, 0.0f, // front left bottom vertex
		0, 0.75, -0.5,			1.0f, 0.0f, 0.0f,		0.50f, 1.0f, // front right  top vertex
		.35, 1.0, -0.5,			1.0f, 0.0f, 0.0f,		0.30f, 1.0f, // front left top vertex
		
		//Third Block second Triangle
		 0, 0.75, -0.5,			1.0f, 0.0f, 0.0f,		0.50f, 1.0f, // front right top vertex
		.35, 0.0, -0.5,			1.0f, 0.0f, 0.0f,		0.30f, 0.0f, // front left bottom vertex
		0,  0.0, -0.5,			1.0f, 0.0f, 0.0f,		0.50f, 0.0f, // front right bottom vertex
			
			
		
	
	};

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	const GLuint floatsPerVertex = 3;
	const GLuint floatsPerNormal= 3;
	const GLuint floatsPerUV = 2;
	const GLuint floatsPerEntry = (floatsPerVertex + floatsPerNormal + floatsPerUV);

	// Strides between vertex coordinates is 9 (x, y, z, r, g, b, a). A tightly packed stride is 0.
	GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);// The number of floats before each
	mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));// calculate number of vertices from array

	// Calculate and update normals for reach vertex
	for (unsigned int i = 0; i < sizeof(verts)/sizeof(verts[0]); i += 3 * floatsPerEntry) {
        glm::vec3 A = glm::vec3(verts[i], verts[i+1],verts[i+2]);
        glm::vec3 B = glm::vec3(verts[i + floatsPerEntry], verts[i+1 + floatsPerEntry],verts[i+2 + floatsPerEntry]);
        glm::vec3 C = glm::vec3(verts[i + 2 * floatsPerEntry], verts[i + 1 + 2 * floatsPerEntry], verts[i + 2 + 2 * floatsPerEntry]);
        glm::vec3 normal = computeFaceNormal(A, B, C);
		//cout << "normals: " << normal.x << " " << normal.y << " " << normal.z << endl;
		for (int j = 0; j < 3; j++) {
			verts[i + 3 + j * floatsPerEntry] = normal.x;
			cout << " Normal " << verts[i + 3 + j * floatsPerEntry] << " ";
			verts[i + 4 + j * floatsPerEntry] = normal.y;
			cout << " Normal " << verts[i + 4 + j * floatsPerEntry] << " ";
			verts[i + 5 + j * floatsPerEntry] = normal.z;
			cout << " Normal " << verts[i + 5 + j * floatsPerEntry] << endl;
		}
    }

	glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
	glBindVertexArray(mesh.vao);

	// Create buffer: for the vertex data
	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

	

	// Create Vertex Attribute Pointers
	glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
	glEnableVertexAttribArray(2);
}

// Implements the UCreatePagesMesh function
void UCreateHatBaseMesh(GLMesh& mesh)
{
	// Position and Color data
	GLfloat verts[] = {
		// Vertex Positions    // Normals				//Texture Coordinates 

		//Left side top
		// 
		//First Traingle top left first 
		 0,  0,  0,		1.0f, 0.0f, 0.0f,		0.5f, 1.0f, // top point
		 -0.578, 0,  1,		1.0f, 0.0f, 0.0f,		 0.0f, 0.0f, //  left vertex
		 0.578, 0,   1,		1.0f, 0.0f, 0.0f,		 1.0f, 0.0f, // right vertex

	};

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	const GLuint floatsPerVertex = 3;
	const GLuint floatsPerNormal= 3;
	const GLuint floatsPerUV = 2;
	const GLuint floatsPerEntry = (floatsPerVertex + floatsPerNormal + floatsPerUV);

	// Strides between vertex coordinates is 9 (x, y, z, r, g, b, a). A tightly packed stride is 0.
	GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);// The number of floats before each
	mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));// calculate number of vertices from array

	// Calculate and update normals for reach vertex
	for (unsigned int i = 0; i < sizeof(verts)/sizeof(verts[0]); i += 3 * floatsPerEntry) {
        glm::vec3 A = glm::vec3(verts[i], verts[i+1],verts[i+2]);
        glm::vec3 B = glm::vec3(verts[i + floatsPerEntry], verts[i+1 + floatsPerEntry],verts[i+2 + floatsPerEntry]);
        glm::vec3 C = glm::vec3(verts[i + 2 * floatsPerEntry], verts[i + 1 + 2 * floatsPerEntry], verts[i + 2 + 2 * floatsPerEntry]);
        glm::vec3 normal = computeFaceNormal(A, B, C);
		//cout << "normals: " << normal.x << " " << normal.y << " " << normal.z << endl;
		for (int j = 0; j < 3; j++) {
			verts[i + 3 + j * floatsPerEntry] = normal.x;
			cout << " Normal " << verts[i + 3 + j * floatsPerEntry] << " ";
			verts[i + 4 + j * floatsPerEntry] = normal.y;
			cout << " Normal " << verts[i + 4 + j * floatsPerEntry] << " ";
			verts[i + 5 + j * floatsPerEntry] = normal.z;
			cout << " Normal " << verts[i + 5 + j * floatsPerEntry] << endl;
		}
    }

	glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
	glBindVertexArray(mesh.vao);

	// Create buffer: for the vertex data
	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

	

	// Create Vertex Attribute Pointers
	glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
	glEnableVertexAttribArray(2);
}

// Implements the UCreatePagesMesh function
void UCreateHatMesh(GLMesh& mesh)
{
	// Position and Color data
	GLfloat verts[] = {
		// Vertex Positions    // Normals				//Texture Coordinates 

		//Left side top
		// 
		//First Traingle top left first 
		 0,  2.5,  0,		1.0f, 0.0f, 0.0f,		0.5f, 1.0f, // top point
		 -0.475, 0,  .75,		1.0f, 0.0f, 0.0f,		 0.0f, 0.0f, //  left vertex
		 0.475, 0,   .75,		1.0f, 0.0f, 0.0f,		 1.0f, 0.0f, // right vertex

	};

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	const GLuint floatsPerVertex = 3;
	const GLuint floatsPerNormal= 3;
	const GLuint floatsPerUV = 2;
	const GLuint floatsPerEntry = (floatsPerVertex + floatsPerNormal + floatsPerUV);

	// Strides between vertex coordinates is 9 (x, y, z, r, g, b, a). A tightly packed stride is 0.
	GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);// The number of floats before each
	mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));// calculate number of vertices from array

	// Calculate and update normals for reach vertex
	for (unsigned int i = 0; i < sizeof(verts)/sizeof(verts[0]); i += 3 * floatsPerEntry) {
        glm::vec3 A = glm::vec3(verts[i], verts[i+1],verts[i+2]);
        glm::vec3 B = glm::vec3(verts[i + floatsPerEntry], verts[i+1 + floatsPerEntry],verts[i+2 + floatsPerEntry]);
        glm::vec3 C = glm::vec3(verts[i + 2 * floatsPerEntry], verts[i + 1 + 2 * floatsPerEntry], verts[i + 2 + 2 * floatsPerEntry]);
        glm::vec3 normal = computeFaceNormal(A, B, C);
		//cout << "normals: " << normal.x << " " << normal.y << " " << normal.z << endl;
		for (int j = 0; j < 3; j++) {
			verts[i + 3 + j * floatsPerEntry] = normal.x;
			cout << " Normal " << verts[i + 3 + j * floatsPerEntry] << " ";
			verts[i + 4 + j * floatsPerEntry] = normal.y;
			cout << " Normal " << verts[i + 4 + j * floatsPerEntry] << " ";
			verts[i + 5 + j * floatsPerEntry] = normal.z;
			cout << " Normal " << verts[i + 5 + j * floatsPerEntry] << endl;
		}
    }

	glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
	glBindVertexArray(mesh.vao);

	// Create buffer: for the vertex data
	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

	

	// Create Vertex Attribute Pointers
	glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
	glEnableVertexAttribArray(2);
}

//Render function - Renders the scene every frame
void URender()
{

	// Enable z-depth 
	glEnable(GL_DEPTH_TEST);

	// Clear the frame and z buffers
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);

	// Bind textures for draw
	glBindTexture(GL_TEXTURE_2D, bookCover);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bookCoverSpecular);

	// Scales object
	glm::mat4 scale = glm::scale(glm::vec3(1.5f, 1.0f, 2.5f));
	// Rotate shape 
	glm::mat4 rotation = glm::rotate(glm::radians(10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	// Place object
	glm::mat4 translation = glm::translate(glm::vec3(-1.5f, 0.01f, 0.90f));
	// Model matrix: transformations are applied right-to-left order
	glm::mat4 model = translation * rotation * scale;

	// Transforms the camera: move the camera back (z axis)
	glm::mat4 view = gCamera.GetViewMatrix();
	glm::mat4 projection;

	// Create perspective projection
	if (!perspectiveSwitch)
	{

		projection = glm::perspective(45.0f, (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
	}

	else
	{
		projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);
	}

	// Select shader program to be used
	glUseProgram(gProgramId);

	// Retrives and passes transfrom matrices to the shader program
	GLint modelLoc = glGetUniformLocation(gProgramId, "model");
	GLint viewLoc = glGetUniformLocation(gProgramId, "view");
	GLint projLoc = glGetUniformLocation(gProgramId, "projection");

	// Retrives and passes Lighting information to shader
	GLint lightPosition = glGetUniformLocation(gProgramId, "light.position");
	GLint lightAmbient = glGetUniformLocation(gProgramId, "light.ambient");
	GLint lightDiffuse = glGetUniformLocation(gProgramId, "light.diffuse");
	GLint lightSpecular = glGetUniformLocation(gProgramId, "light.specular");
	GLint materialShininess = glGetUniformLocation(gProgramId, "material.shininess");
	GLint pointLightPosition = glGetUniformLocation(gProgramId, "pointLight.position");
	GLint pointLightAmbient = glGetUniformLocation(gProgramId, "pointLight.ambient");
	GLint pointLightDiffuse = glGetUniformLocation(gProgramId, "pointLight.diffuse");
	GLint pointLightSpecular = glGetUniformLocation(gProgramId, "pointLight.specular");
	GLint pointLightConstant = glGetUniformLocation(gProgramId, "pointLight.constant");
	GLint pointLightLinear = glGetUniformLocation(gProgramId, "pointLight.linear");
	GLint pointLightquadratic = glGetUniformLocation(gProgramId, "pointLight.quadratic");

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	

	glUniform3f(glGetUniformLocation(gProgramId,"viewPos"), gCamera.Position.x,gCamera.Position.y, gCamera.Position.z);
	glUniform3f(lightPosition, 0.0f, 5.0f, -5.0f);
	glUniform3f(lightAmbient, 0.2f, 0.2f, 0.2f);
	glUniform3f(lightDiffuse, 0.4f, 0.4f, 0.4f);
	glUniform3f(lightSpecular, 1.0f, 1.0f, 1.0f);

	 // point light 1
        glUniform3f(pointLightPosition, -2.0f, 1.5f, -1.3f);
		glUniform3f(pointLightAmbient,0.97f, 0.6426f, 0.1843f);
		glUniform3f(pointLightDiffuse, 0.97f, 0.6426f, 0.1843f);
		glUniform3f(pointLightSpecular, 0.97f, 0.6426f, 0.1843f);
		glUniform1f(pointLightConstant, 1.0f);
        glUniform1f(pointLightLinear, 0.35f);
        glUniform1f(pointLightquadratic, 0.44f);

	glUniform1f(materialShininess, 128.0f);

	// Draw Book Pages
	// 
	// Activate the VBO contained within the mesh's VAO
	glBindVertexArray(gMeshBookBase.vao);

	//Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, gMeshBookBase.nVertices);

	// Scales object
	scale = glm::scale(glm::vec3(1.4f, 0.25f, 2.4f));
	// Rotate shape
	rotation = glm::rotate(glm::radians(10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	// Place object
	translation = glm::translate(glm::vec3(-1.5f, 0.026f, 0.90f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;

	//Send updated model information for next object
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	// Activate the VBO contained within the mesh's VAO
	glBindVertexArray(gMeshBookPages.vao);

	// Bind texture for draw
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bookPages);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bookPagesSpecular);

	glUniform1f(materialShininess, 4.0f);

	//Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, gMeshBookPages.nVertices);


	// Draw the Table
	// 
	// Object remains same size
	scale = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));
	// object remains unrotated
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// Place object slightly below y = 0 to avoid drawing issues with items sitting on the table 
	translation = glm::translate(glm::vec3(0.0f, -0.001f, 0.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;

	

	//Send updated model information for object being drawn
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	// Activate the VBO contained within the mesh's VAO
	glBindVertexArray(gPlane.vao);

	// Bind texture for draw
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tableTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tableTextureSpecular);

	
	glUniform1f(materialShininess, 16.0f);

	//Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, gPlane.nVertices);

	//Mesh for Candle
	static_meshes_3D::Cylinder candle(0.25f, 20, 1.25, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), true, true, candleTexture, candleSpecular);
	// Draw Candle 1
	// 
	// Object remains same size
	scale = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));
	// object remains unrotated
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// Place object slightly below y = 0 to avoid drawing issues with items sitting on the table 
	translation = glm::translate(glm::vec3(-2.5f, .63f, -0.9f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;

	//Send updated model information for object being drawn
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	
	// Bind texture for draw
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, candleTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, candleSpecular);

	candle.render();

	// Draw Candle 2
	// 
	// Object remains same size
	scale = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));
	// object remains unrotated
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// Place object slightly below y = 0 to avoid drawing issues with items sitting on the table 
	translation = glm::translate(glm::vec3(-2.0f, .63f, -1.3f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;

	//Send updated model information for object being drawn
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	
	candle.render();
	

	

	glUniform1f(materialShininess, 126.0f);
	// Draw potion bottle base
	// 
	// Object remains same size
	scale = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));
	// object remains unrotated
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// Place object slightly below y = 0 to avoid drawing issues with items sitting on the table 
	translation = glm::translate(glm::vec3(-1.9f, 0.355f, -0.68f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;

	//Send updated model information for object being drawn
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	Sphere s(0.35f, 20, 20);
	s.Draw();

	//Mesh for neck
	static_meshes_3D::Cylinder neck(0.1f, 20, 0.25f, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), true, true, glassTexture, glassSpecular);
	// Draw potion bottle neck
	// 
	// Object remains same size
	scale = glm::scale(glm::vec3(1.1f, 1.0f, 1.0f));
	// object remains unrotated
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// Place object slightly below y = 0 to avoid drawing issues with items sitting on the table 
	translation = glm::translate(glm::vec3(-1.9f, 0.82f, -0.68f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;

	//Send updated model information for object being drawn
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	neck.render();
	

	// Draw the Hat
	// 
	// Object remains same size
	scale = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));
	// object remains unrotated
	rotation = glm::rotate(0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
	// Place object slightly below y = 0 to avoid drawing issues with items sitting on the table 
	translation = glm::translate(glm::vec3(1.5f, -0.00f, 0.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniform1f(materialShininess, 4.0f);
	

	//Send updated model information for object being drawn
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	// Activate the VBO contained within the mesh's VAO
	glBindVertexArray(hatMesh.vao);

	// Bind texture for draw
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hatTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, hatTexture);

	for (int i = 0; i < 6; i++)
	{

	rotation = glm::rotate(glm::radians(60.0f * i), glm::vec3(0.0f, 1.0f, 0.0f));
	model = translation * rotation * scale;
	//Send updated model information for object being drawn
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, hatMesh.nVertices);
	}

	glBindVertexArray(hatTopMesh.vao);
	
	for (int i = 0; i < 6; i++)
	{

	rotation = glm::rotate(glm::radians(60.0f * i), glm::vec3(0.0f, 1.0f, 0.0f));
	model = translation * rotation * scale;
	//Send updated model information for object being drawn
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, hatTopMesh.nVertices);
	}
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);
	// glfw: swap buffers and poll IO events
	glfwSwapBuffers(gWindow); // Flips the back buffer with the front buffer

}

// Deletes Meshes
void UDestroyMesh(GLMesh& mesh)
{
	glDeleteVertexArrays(1, &mesh.vao);
	glDeleteBuffers(1, &mesh.vbo);
}


// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
	// Compilation and linkage error reporting
	int success = 0;
	char infoLog[512];

	// Create a Shader program object.
	programId = glCreateProgram();

	// Create the vertex and fragment shader objects
	GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

	// Retrive the shader source
	glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
	glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

	// Compile the vertex shader, and print compilation errors (if any)
	glCompileShader(vertexShaderId); // compile the vertex shader
	// check for shader compile errors
	glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

		return false;
	}

	glCompileShader(fragmentShaderId); // compile the fragment shader
	// check for shader compile errors
	glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

		return false;
	}

	// Attached compiled shaders to the shader program
	glAttachShader(programId, vertexShaderId);
	glAttachShader(programId, fragmentShaderId);

	glLinkProgram(programId);   // links the shader program
	// check for linking errors
	glGetProgramiv(programId, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

		return false;
	}

	glUseProgram(programId);    // Uses the shader program

	return true;
}

// Deletes shader program
void UDestroyShaderProgram(GLuint programId)
{
	glDeleteProgram(programId);
}

bool UCreateTexture(const char* filename, GLuint& textureId)
{
	int width, height, channels;
	unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
	if (image)
	{
		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);

		// Set the texture wrapping parameters.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// Set texture filtering parameters.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (channels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		else if (channels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			cout << "Not implemented to handle image with " << channels << " channels" << endl;
			return false;
		}

		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture.

		return true;
	}

	// Error loading the image
	return false;
}

// Destorys texture 
void UDestroyTexture(GLuint textureId)
{
	glDeleteTextures(1, &textureId);
}

//Computes Normal for Triangle with vertices p1, p2, p3
glm::vec3 computeFaceNormal(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
    // Uses p2 as a new origin for p1,p3
    auto a = p3 - p2;
    auto b = p1 - p2;
    // Compute the cross product a X b to get the face normal
    return glm::normalize(glm::cross(a, b));
}
