
//test

#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO {
	GLuint VertexArrayID;
	GLuint VertexBuffer;
	GLuint ColorBuffer;

	GLenum PrimitiveMode;
	GLenum FillMode;
	int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;

	// Create Vertex Array Object
	// Should be done after CreateWindow and before any other GL calls
	glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
	glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
	glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

	glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
	glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
	glVertexAttribPointer(
			0,                  // attribute 0. Vertices
			3,                  // size (x,y,z)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
	glVertexAttribPointer(
			1,                  // attribute 1. Color
			3,                  // size (r,g,b)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
	GLfloat* color_buffer_data = new GLfloat [3*numVertices];
	for (int i=0; i<numVertices; i++) {
		color_buffer_data [3*i] = red;
		color_buffer_data [3*i + 1] = green;
		color_buffer_data [3*i + 2] = blue;
	}

	return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
	// Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

	// Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

	// Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

	// Enable Vertex Attribute 1 - Color
	glEnableVertexAttribArray(1);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

	// Draw the geometry !
	glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/
int  pos_x, pos_y;
int  pos_z=1.5;
//int g1=rand() % 11 +1;
//int g2=rand() % 11 + 1;
int A[11]; // holes
int B[11]; // obstacles
int C[11]; //moving tiles
void r()
{
	for (int i=0; i<10; i++)
	{
		A[i] = rand() % 11 + 1;
		//	B[i] = rand() % 11 + 1;
		C[i] = rand() % 11 + 1; 

	}
}

void rand_obj()
{
	//srand((int)time(0));
	for (int i=0; i<10; i++)
	{
		B[i] = rand() % 11 + 1;
	}	
}

class Player
{
	//	VAO *cube;
	int x, y, z;
	public:
	int score;
	VAO *cube;
	void createCube(){
		static const GLfloat vertex_buffer_data[] = {

			-0.5f,-0.5f,-0.5f, // triangle 1 : begin

			-0.5f,-0.5f, 0.5f,

			-0.5f, 0.5f, 0.5f, // triangle 1 : end

			0.5f, 0.5f,-0.5f, // triangle 2 : begin

			-0.5f,-0.5f,-0.5f,

			-0.5f, 0.5f,-0.5f, // triangle 2 : end

			0.5f,-0.5f, 0.5f,

			-0.5f,-0.5f,-0.5f,

			0.5f,-0.5f,-0.5f,

			0.5f, 0.5f,-0.5f,

			0.5f,-0.5f,-0.5f,

			-0.5f,-0.5f,-0.5f,

			-0.5f,-0.5f,-0.5f,

			-0.5f, 0.5f, 0.5f,

			-0.5f, 0.5f,-0.5f,

			0.5f,-0.5f, 0.5f,

			-0.5f,-0.5f, 0.5f,

			-0.5f,-0.5f,-0.5f,

			-0.5f, 0.5f, 0.5f,

			-0.5f,-0.5f, 0.5f,

			0.5f,-0.5f, 0.5f,

			0.5f, 0.5f, 0.5f,

			0.5f,-0.5f,-0.5f,

			0.5f, 0.5f,-0.5f,

			0.5f,-0.5f,-0.5f,

			0.5f, 0.5f, 0.5f,

			0.5f,-0.5f, 0.5f,

			0.5f, 0.5f, 0.5f,

			0.5f, 0.5f,-0.5f,

			-0.5f, 0.5f,-0.5f,

			0.5f, 0.5f, 0.5f,

			-0.5f, 0.5f,-0.5f,

			-0.5f, 0.5f, 0.5f,

			0.5f, 0.5f, 0.5f,

			-0.5f, 0.5f, 0.5f,

			0.5f,-0.5f, 0.5f

		};
		static const GLfloat color_buffer_data[] = {
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1

		};

		cube = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
	}
	int get_x(){
		return x;
	}
	int get_y(){
		return y;
	}
	int get_z(){
		return z;
	}

	int set_x( int a){
		this -> x = a;
	}
	int set_y(int b){
		this -> y =b;
	}
	int get_score(){
		return score;
	}
};

Player player;




class Obstacle
{
	//	VAO *cube;
	int ox, oy, oz;

	public:
	VAO *cuboid;
	void createCuboid(){
		static const GLfloat vertex_buffer_data[] = {

			-0.5f,-0.5f,-0.5f, // triangle 1 : begin

			-0.5f,-0.5f, 0.5f,

			-0.5f, 0.5f, 0.5f, // triangle 1 : end

			0.5f, 0.5f,-0.5f, // triangle 2 : begin

			-0.5f,-0.5f,-0.5f,

			-0.5f, 0.5f,-0.5f, // triangle 2 : end

			0.5f,-0.5f, 0.5f,

			-0.5f,-0.5f,-0.5f,

			0.5f,-0.5f,-0.5f,

			0.5f, 0.5f,-0.5f,

			0.5f,-0.5f,-0.5f,

			-0.5f,-0.5f,-0.5f,

			-0.5f,-0.5f,-0.5f,

			-0.5f, 0.5f, 0.5f,

			-0.5f, 0.5f,-0.5f,

			0.5f,-0.5f, 0.5f,

			-0.5f,-0.5f, 0.5f,

			-0.5f,-0.5f,-0.5f,

			-0.5f, 0.5f, 0.5f,

			-0.5f,-0.5f, 0.5f,

			0.5f,-0.5f, 0.5f,

			0.5f, 0.5f, 0.5f,

			0.5f,-0.5f,-0.5f,

			0.5f, 0.5f,-0.5f,

			0.5f,-0.5f,-0.5f,

			0.5f, 0.5f, 0.5f,

			0.5f,-0.5f, 0.5f,

			0.5f, 0.5f, 0.5f,

			0.5f, 0.5f,-0.5f,

			-0.5f, 0.5f,-0.5f,

			0.5f, 0.5f, 0.5f,

			-0.5f, 0.5f,-0.5f,

			-0.5f, 0.5f, 0.5f,

			0.5f, 0.5f, 0.5f,

			-0.5f, 0.5f, 0.5f,

			0.5f,-0.5f, 0.5f

		};
		static const GLfloat color_buffer_data[] = {
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0,
			0,0,0
		};

		cuboid = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
	}

	int get_x()
	{
		return ox;
	}
	int get_y()
	{
		return oy;
	}
	int get_z()
	{
		return oz;
	}


};
Obstacle obstacle;




int top=0;
int tower=1;
int player_view, follow_view=0;
int helicopter=0;
float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
int up, down, lft, rght;
int spce, uspce, lspce, dspce, rspce;

float camera_rotation_angle;
int a, b, c;
int cura, curb;
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Function is called first on GLFW_PRESS.

	if (action == GLFW_RELEASE) {
		switch (key) {
			/*case GLFW_KEY_C:
			  rectangle_rot_status = !rectangle_rot_status;
			  break;
			  case GLFW_KEY_P:
			  triangle_rot_status = !triangle_rot_status;
			  break;
			  case GLFW_KEY_X:
			// do something ..
			break;*/
			case GLFW_KEY_UP:
				//				pos_y=player.get_y;
				uspce=1;
				if ( B[pos_x]==pos_y+1 || A[pos_x]==pos_y+1 || C[pos_x]==pos_y+1)
				{
					pos_x=0;
					pos_y=0;
					player.score-=10;
				}
				else if (pos_y+1==9 && pos_x==9)
				{
					pos_y+=1;
					player.score+=100;
					cout << "YOU WIN!" << endl;
					quit(window);
				}
				else if ( pos_y<9)
				{
					up=1;

					/*	pos_y+=1;

						player.set_y(pos_y);*/
					break;
				}
				else
					break;
			case GLFW_KEY_DOWN:
				//				pos_y=player.get_y;
				dspce=1;
				if ( B[pos_x]==pos_y-1 || A[pos_x]==pos_y-1 || C[pos_x]==pos_y-1)
				{
					pos_x=0;
					pos_y=0;
					player.score-=10;
				}
				else if (pos_y-1==9 && pos_x==9)
				{
					pos_y-=1;
					player.score+=100;
					cout << "YOU WIN!" << endl;
					quit(window);
				}
				else if (pos_y>0)
				{
					down=1;
					break;
				}
				else
					break;

			case GLFW_KEY_LEFT:
				lspce=1;
				if ( B[pos_x-1]==pos_y || A[pos_x-1]==pos_y || C[pos_x-1]==pos_y)
				{
					pos_x=0;
					pos_y=0;
					player.score-=10;
				}
				else if (pos_y==9 && pos_x-1==9)
				{
					pos_x-=1;
					player.score+=100;
					cout << "YOU WIN!" << endl;
					quit(window);
				}
				else if (pos_x>0)
				{
					lft=1;
					break;
				}
				else
					break;
			case GLFW_KEY_RIGHT:
				rspce=1;
				if ( B[pos_x+1]==pos_y || A[pos_x+1]==pos_y || C[pos_x+1]==pos_y)
				{
					player.score-=10;
					pos_x=0;
					pos_y=0;
				}
				else if (pos_y==9 && pos_x+1==9)
				{
					pos_x+=1;
					player.score+=100;
					cout << "YOU WIN!" << endl;
					quit(window);
				}
				else if(pos_x<9)
				{
					rght=1;
					break;
				}
				else
					break;
			case GLFW_KEY_SPACE:
				spce=1;
			case GLFW_KEY_T:
				tower=0;
				top=1;
				player_view=0;
				follow_view=0;
				helicopter=0;
				break;

			case GLFW_KEY_O:
				tower=1;
				top=0;
				player_view=0;
				follow_view=0;
				helicopter=0;
				break;

			case GLFW_KEY_P:
				player_view=1;
				tower=0;
				top=0;
				follow_view=0;
				helicopter=0;
				break;

			case GLFW_KEY_C:
				follow_view=1;
				tower=0;
				top=0;
				player_view=0;
				helicopter=0;
				break;

			case GLFW_KEY_H:
				follow_view=0;
				tower=0;
				top=0;
				player_view=0;
				helicopter=1;
				break;
			case GLFW_KEY_F:

				break;
			case GLFW_KEY_S:
				break;


			default:
				break;

		}
	}
	else if (action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_ESCAPE:
				quit(window);
				break;
			default:
				break;

				//			case GLFW_KEY_UP:
				//
		}
	}
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
			quit(window);
			break;
		default:
			break;
	}
}
int left_button;
int right_button;
/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
	switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			if (action == GLFW_PRESS)
				left_button=1;
			else
				left_button=0;
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			if (action == GLFW_PRESS) {
				right_button=1;
			}
			else
				right_button=0;
			break;
		default:
			break;
	}
}
int scroll_down=0;
int scroll_up=0;
int scroll_left=0;
int scroll_right=0;
void scroll ( GLFWwindow *window , double x, double y)
{
	float p,g;
	p=float(y)/4;
	g=float(x)/4;
	//cout << g;
	if ( p<0 )
	{
		scroll_down=1;
	}
	if (p>0 )
	{
		scroll_up=1;
	}
	if ( g>0 )
	{
		scroll_left=1;
	}
	if ( g<0 )
	{
		scroll_right=1;
	}
}

void cursormove(GLFWwindow* window, double x, double y)
{
	float cura = a;
	float curb = b;
	if ( helicopter==1 && left_button==1)
	{
		float a1=(float)x/4.0;
		if ( a1<0)
			camera_rotation_angle -=1;
		else if( a1>0)
			camera_rotation_angle +=1;

		a=10*cos(camera_rotation_angle*M_PI/180.0f);
		b=-10*sin(camera_rotation_angle*M_PI/180.0f);
	}
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
	int fbwidth=width, fbheight=height;
	/* With Retina display on Mac OS X, GLFW's FramebufferSize
	   is different from WindowSize */
	glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
	// Perspective projection for 3D views
	Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 1.0f, 500.0f);

	// Ortho projection for 2D views
	//Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}
//int pos_x, pos_y, pos_z;

VAO *triangle, *rectangle, *cube;

// Creates the triangle object used in this sample code
void createTriangle ()
{
	/* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

	/* Define vertex array as used in glBegin (GL_TRIANGLES) */
	static const GLfloat vertex_buffer_data [] = {
		0, 1,0, // vertex 0
		-1,-1,0, // vertex 1
		1,-1,0, // vertex 2
	};

	static const GLfloat color_buffer_data [] = {
		1,0,0, // color 0
		0,1,0, // color 1
		0,0,1, // color 2
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}

// Creates the rectangle object used in this sample code
void createRectangle ()
{
	// GL3 accepts only Triangles. Quads are not supported
	static const GLfloat vertex_buffer_data [] = {
		-1.2,-1,0, // vertex 1
		1.2,-1,0, // vertex 2
		1.2, 1,0, // vertex 3

		1.2, 1,0, // vertex 3
		-1.2, 1,0, // vertex 4
		-1.2,-1,0  // vertex 1
	};

	static const GLfloat color_buffer_data [] = {
		1,0,0, // color 1
		0,0,1, // color 2
		0,1,0, // color 3

		0,1,0, // color 3
		0.3,0.3,0.3, // color 4
		1,0,0  // color 1
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);


}

void createCube()
{
	static const GLfloat vertex_buffer_data[] = {

		-0.5f,-0.5f,-9.5f, // triangle 1 : begin

		-0.5f,-0.5f, 0.5f,

		-0.5f, 0.5f, 0.5f, // triangle 1 : end

		0.5f, 0.5f,-9.5f, // triangle 2 : begin

		-0.5f,-0.5f,-9.5f,

		-0.5f, 0.5f,-9.5f, // triangle 2 : end

		0.5f,-0.5f, 0.5f,

		-0.5f,-0.5f,-9.5f,

		0.5f,-0.5f,-9.5f,

		0.5f, 0.5f,-9.5f,

		0.5f,-0.5f,-9.5f,

		-0.5f,-0.5f,-9.5f,

		-0.5f,-0.5f,-9.5f,

		-0.5f, 0.5f, 0.5f,

		-0.5f, 0.5f,-9.5f,

		0.5f,-0.5f, 0.5f,

		-0.5f,-0.5f, 0.5f,

		-0.5f,-0.5f,-9.5f,

		-0.5f, 0.5f, 0.5f,

		-0.5f,-0.5f, 0.5f,

		0.5f,-0.5f, 0.5f,

		0.5f, 0.5f, 0.5f,

		0.5f,-0.5f,-9.5f,

		0.5f, 0.5f,-9.5f,

		0.5f,-0.5f,-9.5f,

		0.5f, 0.5f, 0.5f,

		0.5f,-0.5f, 0.5f,

		0.5f, 0.5f, 0.5f,

		0.5f, 0.5f,-9.5f,

		-0.5f, 0.5f,-9.5f,

		0.5f, 0.5f, 0.5f,

		-0.5f, 0.5f,-9.5f,

		-0.5f, 0.5f, 0.5f,

		0.5f, 0.5f, 0.5f,

		-0.5f, 0.5f, 0.5f,

		0.5f,-0.5f, 0.5f

	};
	static const GLfloat color_buffer_data[] = {

		0.583f,  0.771f,  0.014f,

		0.609f,  0.115f,  0.436f,

		0.327f,  0.483f,  0.844f,

		0.822f,  0.569f,  0.201f,

		0.435f,  0.602f,  0.223f,

		0.310f,  0.747f,  0.185f,

		0.597f,  0.770f,  0.761f,

		0.559f,  0.436f,  0.730f,

		0.359f,  0.583f,  0.152f,

		0.483f,  0.596f,  0.789f,

		0.559f,  0.861f,  0.639f,

		0.195f,  0.548f,  0.859f,

		0.014f,  0.184f,  0.576f,

		0.771f,  0.328f,  0.970f,

		0.406f,  0.615f,  0.116f,

		0.676f,  0.977f,  0.133f,

		0.971f,  0.572f,  0.833f,

		0.140f,  0.616f,  0.489f,

		0.997f,  0.513f,  0.064f,

		0.945f,  0.719f,  0.592f,

		0.543f,  0.021f,  0.978f,

		0.279f,  0.317f,  0.505f,

		0.167f,  0.620f,  0.077f,

		0.347f,  0.857f,  0.137f,

		0.055f,  0.953f,  0.042f,

		0.714f,  0.505f,  0.345f,

		0.783f,  0.290f,  0.734f,

		0.722f,  0.645f,  0.174f,

		0.302f,  0.455f,  0.848f,

		0.225f,  0.587f,  0.040f,

		0.517f,  0.713f,  0.338f,

		0.053f,  0.959f,  0.120f,

		0.393f,  0.621f,  0.362f,

		0.673f,  0.211f,  0.457f,

		0.820f,  0.883f,  0.371f,

		0.982f,  0.099f,  0.879f

	};

	cube = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
}

//float camera_rotation_angle;
float rectangle_rotation = 0;
float triangle_rotation = 0;
int  d, e, f, g, h, i;
float k;
int hula;
int temp;/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
	if (up==1)
	{
		pos_y+=1;
		player.set_y(pos_y);
		up=0;
	}
	if (down==1)
	{
		pos_y-=1;
		player.set_y(pos_y);
		down=0;
	}
	if (lft==1)
	{
		pos_x-=1;
		player.set_x(pos_x);
		lft=0;
	}
	if (rght==1)
	{
		pos_x+=1;
		player.set_x(pos_x);
		rght=0;
	}


	if (spce==1 && uspce==1 && pos_y+2<=10)
	{
		if ( A[pos_x]==pos_y+2 || B[pos_x]==pos_y+2)
		{
			pos_x=0;
			pos_y=0;
			player.score-=10;
		}
		else 
		{
			pos_y+=2;
			uspce=0;
			spce=0;
		}
	}

	if (spce==1 && dspce==1 && pos_y-2>=0)
	{
		if ( A[pos_x]==pos_y-2 || B[pos_x]==pos_y-2)
		{
			pos_x=0;
			pos_y=0;
			player.score-=10;
		}
		else
		{
			pos_y-=2;
			spce=0;
			dspce=0;
		}
	}

	if (spce==1 && lspce==1 && (pos_x-2)>=0)
	{
		if ( A[pos_x-2]==pos_y || B[pos_x-2]==pos_y )
		{
			pos_x=0;
			pos_y=0;
			player.score-=10;
		}
		else
		{
			pos_x-=2;
			lspce=0;
			spce=0;
		}
	}

	if (spce==1 && rspce==1 && (pos_x+2)<=10)
	{
		if ( A[pos_x+2]==pos_y || B[pos_x+2]==pos_y)
		{
			pos_x=0;
			pos_y=0;
			player.score-=10;
			//		break;
		}
		else
		{
			pos_x+=2;
			spce=0;
			rspce=0;
		}
		//break;
	}
	if ( tower==1)
		camera_rotation_angle=120;
	else if( top==1)
		camera_rotation_angle=60;
	else if ( helicopter ==1 )
		camera_rotation_angle = 45;
	// clear the color and depth in the frame buffer
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// use the loaded shader program
	// Don't change unless you know what you are doing
	glUseProgram (programID);

	// Eye - Location of camera. Don't change unless you are sure!!
	/*	if (tower==1)
		{
		glm::vec3 eye ( 10*cos(camera_rotation_angle*M_PI/180.0f), -10*sin(camera_rotation_angle*M_PI/180.0f), 7 );
		}
		else if (top==1)
		{*/
	
	if (scroll_right==1)
	{
		cout << "in " << endl;
		camera_rotation_angle--;
		cout << camera_rotation_angle << endl;
	}
	if (scroll_left ==1 )
	{
		camera_rotation_angle+=1;
	}

	

	
	if (tower==1)
	{
		a=10*cos(camera_rotation_angle*M_PI/180.0f);
		b=-10*sin(camera_rotation_angle*M_PI/180.0f);
		c=7;
	}
	else if (top==1)
	{
		a=5;
		b=5;
		c=7;
	}
	else if (player_view==1)
	{
		a=pos_x;
		b=pos_y;
		c=3;

	}
	else if(follow_view==1)
	{
		a=pos_x;
		b=pos_y-3;
		c=4;
	}
	else if (helicopter==1)
	{
		a=10*cos(camera_rotation_angle*M_PI/180.0f);
		b=-10*sin(camera_rotation_angle*M_PI/180.0f);
		c=7;
	}
	if ( scroll_up==1)
		c--;
	if (scroll_down==1)
		c++;
	glm::vec3 eye ( a,b,c);
	//glm::vec3 eye ( 5,5,7);
	//	}
	//glm::vec3 eye ( 10*cos(camera_rotation_angle*M_PI/180.0f), -10*sin(camera_rotation_angle*M_PI/180.0f), 7 );
	// Target - Where is the camera looking at.  Don't change unless you are sure!!
	/*	if (tower==1)
		{
		glm::vec3 target (4, 0, 0);
		}
		else if ( top==1)
		{*/
	if (player_view==1)
	{
		g=pos_x;
		h=pos_y+1;
		i=3;
	}
	else if(follow_view==1)
	{
		g=pos_x;
		h=pos_y;
		i=4;
	}
	else
	{
		g=5;
		h=5;
		i=0;
	}
	glm::vec3 target (g,h,i);
	//	}
	// Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
	//	if ( tower==1 || player_view==1)
	//glm::vec3 up (0, 0, 1);
	if ( top==1)
	{
		d=0;
		e=1;
		f=0;
	}

	else
	{
		d=0;
		e=0;
		f=1;
	}
	glm::vec3 up (d,e,f);
	// Compute Camera matrix (view)
	Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
	//  Don't change unless you are sure!!
	//Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

	// Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
	//  Don't change unless you are sure!!
	glm::mat4 VP = Matrices.projection * Matrices.view;

	// Send our transformation to the currently bound shader, in the "MVP" uniform
	// For each model you render, since the MVP will be different (at least the M part)
	//  Don't change unless you are sure!!
	glm::mat4 MVP;	// MVP = Projection * View * Model

	// Load identity to model matrix
	/*	Matrices.model = glm::mat4(1.0f);

	// Render your scene 

	glm::mat4 translateTriangle = glm::translate (glm::vec3(-2.0f, 0.0f, 0.0f)); // glTranslatef
	glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
	glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
	Matrices.model *= triangleTransform; 
	MVP = VP * Matrices.model; // MVP = p * V * M

	//  Don't change unless you are sure!!
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	// draw3DObject draws the VAO given to it using current MVP matrix
	//draw3DObject(triangle);
	 */
	// Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
	// glPopMatrix ();
	/*Matrices.model = glm::mat4(1.0f);

	  glm::mat4 translateRectangle = glm::translate (glm::vec3(2, 0, 0));        // glTranslatef
	  glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	  Matrices.model *= (translateRectangle * rotateRectangle);
	  MVP = VP * Matrices.model;
	  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	// draw3DObject draws the VAO given to it using current MVP matrix
	//draw3DObject(rectangle);
	 */

	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateCube = glm::translate (glm::vec3(pos_x, pos_y, pos_z));        // glTranslatef
	//glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	Matrices.model *= translateCube;
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	// draw3DObject draws the VAO given to it using current MVP matrix
	draw3DObject(player.cube);

	int i, j;
	//r();

	for (i=0; i<10; i++)
	{
		//g1=rand() % 11 + 1; 
		for (j=0; j<10; j++)
		{
			if (A[i]!=j && C[i]!=j) //A gives holes
			{	
				Matrices.model = glm::mat4(1.0f);
				glm::mat4 translateCube = glm::translate (glm::vec3(i, j, 0));        // glTranslatef
				//glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
				Matrices.model *= translateCube;
				MVP = VP * Matrices.model;
				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

				// draw3DObject draws the VAO given to it using current MVP matrix
				draw3DObject(cube);
			}
			if (C[i]==j && C[i]!=B[i])
			{

				Matrices.model = glm::mat4(1.0f);
				glm::mat4 translateCube = glm::translate (glm::vec3(i, j, k));        // glTranslatef
				//glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)

				Matrices.model *= translateCube;
				MVP = VP * Matrices.model;
				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

				// draw3DObject draws the VAO given to it using current MVP matrix
				draw3DObject(cube);
			}
		}
	}
	if (hula==1 &&  k<3)
	{
		k=k+0.5;
		//hula=1;
	}
	if (k==3)
		hula=0;
	if ( hula==0 && k>-3)
	{
		k=k-0.5;
		//hula=0;
	}
	if (k<=-3)
	{	
		hula=1;
	}

	for (i=0; i<10; i++)
	{
		for (j=0; j<10; j++)
		{
			if (B[i]==j)
			{




				Matrices.model = glm::mat4(1.0f);
				glm::mat4 translateCuboid = glm::translate (glm::vec3(i, j, 1.5));        // glTranslatef
				//glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
				Matrices.model *= translateCuboid;
				MVP = VP * Matrices.model;
				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

				// draw3DObject draws the VAO given to it using current MVP matrix
				draw3DObject(obstacle.cuboid);

			}
		}
	}

	// Increment angles
	float increments = 1;

	//camera_rotation_angle++; // Simulating camera rotation
	triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
	rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;



}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
	GLFWwindow* window; // window desciptor/handle

	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glfwSwapInterval( 1 );

	/* --- register callbacks with GLFW --- */

	/* Register function to handle window resizes */
	/* With Retina display on Mac OS X GLFW's FramebufferSize
	   is different from WindowSize */
	glfwSetFramebufferSizeCallback(window, reshapeWindow);
	glfwSetWindowSizeCallback(window, reshapeWindow);

	/* Register function to handle window close */
	glfwSetWindowCloseCallback(window, quit);

	/* Register function to handle keyboard input */
	glfwSetKeyCallback(window, keyboard);      // general keyboard input
	glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

	/* Register function to handle mouse click */
	glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

	glfwSetScrollCallback(window, scroll);
	glfwSetCursorPosCallback(window, cursormove);
	return window;

	return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
	r();
	rand_obj();
	/* Objects should be created before any other gl function and shaders */
	// Create the models
	//createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	//createRectangle ();
	createCube();
	player.createCube();
	obstacle.createCuboid();
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

	// Background color of the scene
	glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

	cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 1000;
	int height = 1000;

	GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

	double last_update_time = glfwGetTime(), current_time;

	/* Draw in loop */
	while (!glfwWindowShouldClose(window)) {

		// OpenGL Draw commands
		draw();

		// Swap Frame Buffer in double buffering
		glfwSwapBuffers(window);

		// Poll for Keyboard and mouse events
		glfwPollEvents();

		// Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
		current_time = glfwGetTime(); // Time in seconds
		if ((current_time - last_update_time) >= 6) { // atleast 0.5s elapsed since last frame
			// do something every 0.5 seconds ..
			rand_obj();
			last_update_time = current_time;
		}
	}
	int score;
	score=player.score;
	cout << "SCORE: " << score << endl;
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
