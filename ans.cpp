#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <ctime>
#include <list>



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
//    exit(EXIT_SUCCESS);
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
float ctrl=0,alt=0;
int exit_flag=0,start_flag=0,close_flag=0,pause_flag=0,flag_mirror=0;
int score=0,caught=0,miss=0,hit=0,dump=0,level=1;
long long int poi=0,f=0,poi2=0,f2=0;
float speed_var=0;
int mouse_pan=0;
int max_time=40;
int flag_shoot=0,flag_bullet=0;
double last_update_time = glfwGetTime(), current_time;
float brick_flag=0;
double update_shoot = glfwGetTime(),update_bullet = glfwGetTime(),update_exit,update_mirror = glfwGetTime();
double updatetime_fall = glfwGetTime(),fall_flag=0;
float zoom=0,pan=0,pany=0;
double xpos, ypos;
int miss_limit=10;

VAO *triangle, *rectangle;

glm::mat4 VP,MVP;


class Mirror{

  public:
    VAO *mirror;
    float x,y,rotation;

public: 
~Mirror()
{
  free(mirror);
}  
void createMirror()
{
  static const GLfloat vertex_buffer_data [] = {
    -10,-0.5,0,
    -10,0.5,0,
    10,0.5,0,

    10,0.5,0,
    10,-0.5,0,
    -10,-0.5,0,

  };

  static const GLfloat color_buffer_data [] = {
    0.5,0.5,0.5,
    0.5,0.5,0.5,
    0.5,0.5,0.5,

    0.5,0.5,0.5,
    0.5,0.5,0.5,
    0.5,0.5,0.5,
  };

  mirror= create3DObject(GL_TRIANGLES,6,vertex_buffer_data,color_buffer_data,GL_FILL);

}

void drawMirror()
{
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 transMirror = glm::translate (glm::vec3(x,y,0));
  glm::mat4 rotateMirror = glm::rotate((float)(rotation*M_PI/180.0f), glm::vec3(0,0,1));
  Matrices.model *=transMirror*rotateMirror;
  MVP= VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
  draw3DObject(mirror); 

}

}mirrors[4];



class Lasers {

public:
VAO *laser,*laser2,*lasercirc,*laser_click;
float laser_rot_dir,lasery_dir,l2x,l2y,laser_rot,lasery;
bool laser_rot_status,lasery_status;
int mouse_flag;

public:

Lasers()
{
  laser_rot_dir = 1;
  lasery_dir=1;  
  laser_rot=0;
  lasery=0;
  mouse_flag=0;
  laser_rot_status=false;
  lasery_status=false;  
}
~Lasers()
{
  free(laser);
  free(laser2);
  free(lasercirc);
  free(laser_click);
}
void createLaser()
{
  static const GLfloat vertex_buffer_data [] = {
    -100,0,0,
    -100,30,0,
    -90,20,0,

    -90,20,0,
    -90,10,0,
    -100,0,0,

 
  };

  static const GLfloat color_buffer_data [] = {
    0,0,0.57,
    0,0,0.57,
    0,0,0.57,

    0,0,0.57,
    0,0,0.57,
    0,0,0.57,

  };
  static const GLfloat color_buffer_data1 [] = {
    0,0,0.2,
    0,0,0.2,
    0,0,0.2,

    0,0,0.2,
    0,0,0.2,
    0,0,0.2,

  };
  laser = create3DObject(GL_TRIANGLES,6,vertex_buffer_data,color_buffer_data,GL_FILL);
  laser_click = create3DObject(GL_TRIANGLES,6,vertex_buffer_data,color_buffer_data1,GL_FILL);
}
void checkClick(double x,double y)
{
  if(-100<=x && x<=-90)
  {
    if(lasery<=y && lasery+30>=y)
      mouse_flag=1;
    else 
      mouse_flag=0;
  }
  else
    mouse_flag=0;
}


void createLaser2()
{
  l2x=-95;
  l2y=15;
  static const GLfloat vertex_buffer_data [] = {
 
    0,-2.5,0,
    0,2.5,0,
    15,2.5,0,

    15,2.5,0,
    15,-2.5,0,
    0,-2.5,0,

  };

  static const GLfloat color_buffer_data [] ={

    0,0,0.9,
    0,0,0.9,
    0,0,0.9,

    0,0,0.9,
    0,0,0.9,
    0,0,0.9,

  };
  laser2= create3DObject(GL_TRIANGLES,6,vertex_buffer_data,color_buffer_data,GL_FILL);

}

void createlasercirc()
{
  float radius=2.5;
  static GLfloat vertex_buffer_data [360*9];
  for(int i=0;i<360;i++)
  {
    vertex_buffer_data[9*i]=15;
    vertex_buffer_data[9*i+1]=0;
    vertex_buffer_data[9*i+2]=0;

    vertex_buffer_data[9*i+3]=15+radius*cos(i*M_PI/180);
    vertex_buffer_data[9*i+4]=radius*sin(i*M_PI/180);
    vertex_buffer_data[9*i+5]=0;

    vertex_buffer_data[9*i+6]=15+radius*cos(((i+1)%360)*M_PI/180);
    vertex_buffer_data[9*i+7]=radius*sin(((i+1)%360)*M_PI/180);
    vertex_buffer_data[9*i+8]=0;

  }

  static GLfloat color_buffer_data [360*9];
  for(int i=0;i<360;i++)
  {
    color_buffer_data[9*i]=0;
    color_buffer_data[9*i+1]=0;
    color_buffer_data[9*i+2]=0.9;

    color_buffer_data[9*i+3]=0;
    color_buffer_data[9*i+4]=0;
    color_buffer_data[9*i+5]=0.9;

    color_buffer_data[9*i+6]=0;
    color_buffer_data[9*i+7]=0;
    color_buffer_data[9*i+8]=0.9;

  }
  lasercirc= create3DObject(GL_TRIANGLES,360*3,vertex_buffer_data,color_buffer_data,GL_FILL);

}

void draw()
{
  if(lasery_dir==1 && lasery+30<75)
  {
      lasery+=1*lasery_status;
  }
  else if(lasery_dir==-1 && lasery>-60)
  {
      lasery-=1*lasery_status;
  }

  if(laser_rot_dir==1 && laser_rot<65)
  {
      laser_rot+=1*laser_rot_status;
  }
  else if(laser_rot_dir==-1 && laser_rot>-65)
  {
      laser_rot-=1*laser_rot_status;
  }

  glm::mat4 transLaser1 = glm::translate (glm::vec3(0,lasery,0));
  Matrices.model = glm::mat4(1.0f);
  Matrices.model *= transLaser1;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
  if(mouse_flag==0)
    draw3DObject(laser);
  else
    draw3DObject(laser_click);

  Matrices.model = glm::mat4(1.0f);
  glm::mat4 transLaser2 = glm::translate (glm::vec3(l2x,l2y+lasery,0));
  glm::mat4 rotateLaser2 = glm::rotate((float)(laser_rot*M_PI/180.0f), glm::vec3(0,0,1)); 
  Matrices.model *= transLaser2 * rotateLaser2;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
  draw3DObject(laser2);

  Matrices.model = glm::mat4(1.0f);
  glm::mat4 transLasercirc = glm::translate (glm::vec3(l2x,l2y+lasery,0));
  glm::mat4 rotateLasercirc = glm::rotate((float)(laser_rot*M_PI/180.0f), glm::vec3(0,0,1)); 
  Matrices.model *= transLasercirc * rotateLasercirc;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
  draw3DObject(lasercirc);
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 transLasercirc2= glm::translate (glm::vec3(l2x-15,l2y+lasery,0));
  Matrices.model *= transLasercirc2 ;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
  draw3DObject(lasercirc);

}

}Laser;

class Buckets{

public:
float bx,bx_dir,bcx,bcy,extra;
bool bx_status;
int mouse_flag;
VAO *basket,*bask_circ,*basket_click;

public:
  Buckets()
  {
    bx_dir=1;
    bx=0;
    mouse_flag=0;
    bx_status=false;
  }
  ~Buckets()
  {
    free(basket);
    free(bask_circ);
    free(basket_click);
  }
void createBaskRect()
{
  static const GLfloat vertex_buffer_data[] = {
      -60,-95,0,
      -60,-70,0,
      -40,-70,0,

      -40,-70,0,
      -40,-95,0,
      -60,-95,0,
  };
  static const GLfloat color_buffer_data[] = {
    1,0,0,
    1,0,0,
    1,0,0,

    1,0,0,
    1,0,0,
    1,0,0,
  };

  static const GLfloat color_buffer_data2[] = {
    0,1,0,
    0,1,0,
    0,1,0,

    0,1,0,
    0,1,0,
    0,1,0,
  };
  static const GLfloat color_buffer_data1[] = {
    0.4,0,0,
    0.4,0,0,
    0.4,0,0,

    0.4,0,0,
    0.4,0,0,
    0.4,0,0,
  };

  static const GLfloat color_buffer_data3[] = {
    0,0.4,0,
    0,0.4,0,
    0,0.4,0,

    0,0.4,0,
    0,0.4,0,
    0,0.4,0,
  };
  if(extra==0)
  {
  basket = create3DObject(GL_TRIANGLES,6,vertex_buffer_data,color_buffer_data,GL_FILL);
  basket_click = create3DObject(GL_TRIANGLES,6,vertex_buffer_data,color_buffer_data1,GL_FILL);
  }
  else
  {
    basket = create3DObject(GL_TRIANGLES,6,vertex_buffer_data,color_buffer_data2,GL_FILL);
    basket_click = create3DObject(GL_TRIANGLES,6,vertex_buffer_data,color_buffer_data3,GL_FILL);
  }
}


void checkClick(double x,double y)
{
  if(bx+extra-61<=x && bx+extra-39>=x)
  {
    if(-97<=y && y<=-68)
    {
      mouse_flag=1;
    }
    else
      mouse_flag=0;
  }
  else
    mouse_flag=0;
}

void createBaskCirc()
{
  if(extra==0)
  {
  bcx=-50;
  bcy=-70;
  }
  else
  {
  bcx=30;
  bcy=-70;
  }

  float radius=10;
  static GLfloat vertex_buffer_data [360*9];
  for(int i=0;i<360;i++)
  {
    vertex_buffer_data[9*i]=0;
    vertex_buffer_data[9*i+1]=0;
    vertex_buffer_data[9*i+2]=0;

    vertex_buffer_data[9*i+3]=radius*cos(i*M_PI/180);
    vertex_buffer_data[9*i+4]=radius*sin(i*M_PI/180);
    vertex_buffer_data[9*i+5]=0;

    vertex_buffer_data[9*i+6]=radius*cos(((i+1)%360)*M_PI/180);
    vertex_buffer_data[9*i+7]=radius*sin(((i+1)%360)*M_PI/180);
    vertex_buffer_data[9*i+8]=0;

  }

  static GLfloat color_buffer_data [360*9];
  for(int i=0;i<360;i++)
  {
    color_buffer_data[9*i]=1;
    color_buffer_data[9*i+1]=0.4;
    color_buffer_data[9*i+2]=0.4;

    color_buffer_data[9*i+3]=1;
    color_buffer_data[9*i+4]=0.4;
    color_buffer_data[9*i+5]=0.4;

    color_buffer_data[9*i+6]=1;
    color_buffer_data[9*i+7]=0.4;
    color_buffer_data[9*i+8]=0.4;

  }

  static GLfloat color_buffer_data2 [360*9];
  for(int i=0;i<360;i++)
  {
    color_buffer_data2[9*i]=0.4;
    color_buffer_data2[9*i+1]=1;
    color_buffer_data2[9*i+2]=0.4;

    color_buffer_data2[9*i+3]=0.4;
    color_buffer_data2[9*i+4]=1;
    color_buffer_data2[9*i+5]=0.4;

    color_buffer_data2[9*i+6]=0.4;
    color_buffer_data2[9*i+7]=1;
    color_buffer_data2[9*i+8]=0.4;

  }
  if(extra==0)
  bask_circ= create3DObject(GL_TRIANGLES,360*3,vertex_buffer_data,color_buffer_data,GL_FILL);
  else
  bask_circ= create3DObject(GL_TRIANGLES,360*3,vertex_buffer_data,color_buffer_data2,GL_FILL);

}

void draw()
{

  if(bx_dir==-1 && bx-60+extra>=-69)
  {
      bx-=1*bx_status;
  }
  else if(bx_dir==1 && bx-40+extra<=49)
  {
      bx+=1*bx_status;
  }

  glm::mat4 transBask1 = glm::translate (glm::vec3(bx+extra,0,0));
  Matrices.model = glm::mat4(1.0f);
  Matrices.model *= transBask1;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
  if(mouse_flag==0)
    draw3DObject(basket);
  else
    draw3DObject(basket_click);


  Matrices.model = glm::mat4(1.0f);
  glm::mat4 rotateBaskCirc1 = glm::rotate((float)(-80*M_PI/180.0f), glm::vec3(1,0,0)); 
  glm::mat4 transBaskCirc1 = glm::translate (glm::vec3(bcx+bx,bcy,0));
  Matrices.model *= transBaskCirc1*rotateBaskCirc1;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
  draw3DObject(bask_circ);
  Matrices.model = glm::mat4(1.0f);
  transBaskCirc1 = glm::translate (glm::vec3(bcx+bx,bcy-25,0));
  rotateBaskCirc1 = glm::rotate((float)(80*M_PI/180.0f), glm::vec3(1,0,0)); 
  Matrices.model *= transBaskCirc1*rotateBaskCirc1;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
  draw3DObject(bask_circ);  

}

}bucket[2];

VAO *rect1,*rect2;

void createRectangle()
{
  static const GLfloat vertex_buffer_data [] = {
    -2,-0.5,0,
    -2,0.5,0,
    2,0.5,0,

    2,0.5,0,
    2,-0.5,0,
    -2,-0.5,0,
  };

  static const GLfloat color_buffer_data [] = {
    0,0,0,
    0,0,0,
    0,0,0,

    0,0,0,
    0,0,0,
    0,0,0,
  };
  rect1 = create3DObject(GL_TRIANGLES,6,vertex_buffer_data,color_buffer_data,GL_FILL);

}

void createRectangle2()
{
  static const GLfloat vertex_buffer_data [] = {
    -7,-5,0,
    -7,5,0,
    7,5,0,

    7,5,0,
    7,-5,0,
    -7,-5,0,
  };

  static const GLfloat color_buffer_data [] = {
    0.5,0.5,1,
    0.5,0.5,1,
    0.5,0.5,1,

    0.5,0.5,1,
    0.5,0.5,1,
    0.5,0.5,1,
  };
  rect2 = create3DObject(GL_TRIANGLES,6,vertex_buffer_data,color_buffer_data,GL_FILL);

}

class Bricks {

public:
  VAO* brick;
  int val,val2,rem_flag,visit;
  float x,y;

public:
  Bricks()
  {
    rem_flag=0;
    visit=0;
  }
  ~Bricks()
  {
    free(brick);
  }

void createBrick()
{
  static const GLfloat vertex_buffer_data [] = {
    -1.5,0,0,
    -1.5,7,0,
    1.5,7,0,

    1.5,7,0,
    1.5,0,0,
    -1.5,0,0,
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0,
    1,0,0,
    1,0,0,

    1,0,0,
    1,0,0,
    1,0,0,
  };

  static const GLfloat color_buffer_data1 [] = {
    0,1,0,
    0,1,0,
    0,1,0,

    0,1,0,
    0,1,0,
    0,1,0,
  };

  static const GLfloat color_buffer_data2 [] = {
    0,0,0,
    0,0,0,
    0,0,0,

    0,0,0,
    0,0,0,
    0,0,0,
  };

  if(val2==0)
    brick= create3DObject(GL_TRIANGLES,6,vertex_buffer_data,color_buffer_data2,GL_FILL);
  else if(val2==1)
    brick = create3DObject(GL_TRIANGLES,6,vertex_buffer_data,color_buffer_data,GL_FILL);
  else
    brick = create3DObject(GL_TRIANGLES,6,vertex_buffer_data,color_buffer_data1,GL_FILL);
    rem_flag=0;
    visit=0;

}
void generateBlock()
{
    if(brick_flag==1)
    {
//        srand(time(NULL));
        val=(unsigned)rand()%2;
//        srand(time(NULL));
        val2=(unsigned)rand()%3;
    }
    if(val==0)
    {
//      srand(time(NULL));
      x=-65.0+40*(rand()*1.0/RAND_MAX);
      y=95;
    }
    else
    {
//      srand(time(NULL));
      x=5.0+40*(rand()*1.0/RAND_MAX);
      y=95;
    }
}

void checkBlock()
{
  int flag=0,flag2=0;
  if(fall_flag==1)
    y-=1.5+0.2*speed_var;
  if(rem_flag==1)
  {
    if(y<=-90)
    {
      free(brick);
      f++;
    }
    return;
  }
  if(y<=-72 && visit==0)
  {
    visit=1;
    flag2=1;

    if(val2==0)
    {
        if((bucket[0].bx-60)<=x && (bucket[0].bx-40)>=x)
          {
            if((bucket[1].bx+20)<=x && (bucket[1].bx+40)>=x)
             {
                dump++;
             }
            else
            {
              exit_flag=1;
              zoom=0;
              update_exit=max_time;
            }
  //          printf("firstif\n");
            flag=1;
            rem_flag=1;
          }
        else if((bucket[1].bx+20)<=x && (bucket[1].bx+40)>=x)
        {

            exit_flag=1;
            zoom=0;
            update_exit=max_time;
 //           printf("Secondif\n");
            flag=1;
            rem_flag=1;
        }

    }
    else if(val2==1)
    {
        if((bucket[0].bx-60)<=x && (bucket[0].bx-40)>=x)
          {
            if((bucket[1].bx+20)<=x && (bucket[1].bx+40)>=x)
              dump++;
            else
            {
              caught++;
              system("mpg123 -vC score.mp3 &");
            }
            flag=1;
            rem_flag=1;
          }
        else if((bucket[1].bx+20)<=x && (bucket[1].bx+40)>=x)
        {
          dump++;
          flag=1;
          rem_flag=1;
        }

    }
    else
    {
      if((bucket[1].bx+20)<=x && (bucket[1].bx+40)>=x)
      {
        if((bucket[0].bx-60)<=x && (bucket[0].bx-40)>=x)
          dump++;
        else
        {
          caught++;
          system("mpg123 -vC score.mp3 &");
        }
        flag=1;
        rem_flag=1;
      }
      else if((bucket[0].bx-60)<=x && (bucket[0].bx-40)>=x)
      {
        dump++;
        flag=1;
        rem_flag=1;
      }

    }

  } 
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 transMirror = glm::translate (glm::vec3(x,y,0));
  Matrices.model *=transMirror;
  MVP= VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
  draw3DObject(brick);
  if(y<=-88)
  {
    free(brick);
    f++;
  }
}


}block[1000];


class Bullets{
public:
  double x,y,rotation_angle,radius,axis_x,axis_y;
  VAO *bullet;
  int pre_flag,rem_flag;
  double x_n,y_n,c;

public:
  Bullets()
  {
    axis_x=15;
    axis_y=0;
    pre_flag=-1;
    rem_flag=0;
//    x=Laser.l2x;
//    y=Laser.l2y+Laser.lasery;
    radius=2.5;
//    rotation_angle=Laser.laser_rot;
  }
  ~Bullets()
  {
    free(bullet);
  }

  void createBullet(){
    x=Laser.l2x;
    y=Laser.l2y+Laser.lasery;
    rotation_angle=Laser.laser_rot;
    static GLfloat vertex_buffer_data [360*9];
    static GLfloat color_buffer_data [360*9];
    for(int i=0;i<360;i++)
    {
      vertex_buffer_data[9*i]=0;
      vertex_buffer_data[9*i+1]=0;
      vertex_buffer_data[9*i+2]=0;

      vertex_buffer_data[9*i+3]=radius*cos(i*M_PI/180);
      vertex_buffer_data[9*i+4]=radius*sin(i*M_PI/180);
      vertex_buffer_data[9*i+5]=0;

      vertex_buffer_data[9*i+6]=radius*cos(((i+1)%360)*M_PI/180);
      vertex_buffer_data[9*i+7]=radius*sin(((i+1)%360)*M_PI/180);
      vertex_buffer_data[9*i+8]=0;

    }

    for(int i=0;i<360;i++)
    {
      color_buffer_data[9*i]=0;
      color_buffer_data[9*i+1]=1;
      color_buffer_data[9*i+2]=1;

      color_buffer_data[9*i+3]=0;
      color_buffer_data[9*i+4]=1;
      color_buffer_data[9*i+5]=1;

      color_buffer_data[9*i+6]=0;
      color_buffer_data[9*i+7]=1;
      color_buffer_data[9*i+8]=1;

    }

    bullet=create3DObject(GL_TRIANGLES,360*3,vertex_buffer_data,color_buffer_data,GL_FILL);
    axis_x=15;
    axis_y=0;
    pre_flag=-1;
    rem_flag=0;
  }
  double distance(double v1,double v2)
  {
    return sqrt((v1*v1)+(v2*v2));
  }

  int checkCollisionMirror()
  {
    int flag;
   double dis,val1,val2,d1,d2,d3;
    flag=-1;
    for(int i=0;i<4;i++)
    {
      d1=distance(20*cos(mirrors[i].rotation*M_PI/180),20*sin(mirrors[i].rotation*M_PI/180));
      val1=(sin(mirrors[i].rotation*M_PI/180))*((axis_x*cos(rotation_angle*M_PI/180)+x)-(mirrors[i].x-(10.0*cos(mirrors[i].rotation*M_PI/180))));
      val2=(cos(mirrors[i].rotation*M_PI/180))*((axis_x*sin(rotation_angle*M_PI/180)+y)-(mirrors[i].y-(10.0*sin(mirrors[i].rotation*M_PI/180))));
      c=-(val1-val2);
      x_n=((axis_x*cos(rotation_angle*M_PI/180)+x)+sin(mirrors[i].rotation*M_PI/180)*c);
      y_n=(axis_x*sin(rotation_angle*M_PI/180)+y)-cos(mirrors[i].rotation*M_PI/180)*c;
      d2=distance(x_n-(mirrors[i].x-(10.0*cos(mirrors[i].rotation*M_PI/180))),y_n-(mirrors[i].y-(10.0*sin(mirrors[i].rotation*M_PI/180))));
      d3=distance(x_n-(mirrors[i].x+(10.0*cos(mirrors[i].rotation*M_PI/180))),y_n-(mirrors[i].y+(10.0*sin(mirrors[i].rotation*M_PI/180))));
      dis=abs(c);
      if(dis<=radius)
      {
        if(abs(d1-(d2+d3))<0.001)
        {
        flag=i;
//        printf("%d %lf %lf\n",flag,dis,radius);
        break;
        }
      }
    }
    return flag;
  }

  bool intersects(int i)
  {
    if(block[i].rem_flag==1)
      return 0;
    float circleDistanceX = abs(x+axis_x*cos(rotation_angle*M_PI/180) - block[i].x);
    float circleDistanceY = abs(y+axis_x*sin(rotation_angle*M_PI/180) - (block[i].y+3.5));

    if (circleDistanceX > (1.5 + radius)) { return false; }
    if (circleDistanceY > (3.5 + radius)) { return false; }

    if (circleDistanceX <= (1.5)) { return true; } 
    if (circleDistanceY <= (3.5)) { return true; }

    float cornerDistance_sq = (circleDistanceX - 1.5)*(circleDistanceX - 1.5) +
                         (circleDistanceY - 3.5)*(circleDistanceY - 3.5);

    return (cornerDistance_sq <= (radius*radius));
  }


  void draw()
  {
    if(rem_flag==1)
    {
    if(x+axis_x*cos(rotation_angle*M_PI/180)<-100 || x+axis_x*cos(rotation_angle*M_PI/180)>71 || y+axis_x*sin(rotation_angle*M_PI/180)<-65 || y+axis_x*sin(rotation_angle*M_PI/180)>100 )
    {
//      free(bullet);
    }
    else
      axis_x+=2;
      return;
    }
    int flag;
    bool flag2=false;
    for(int i=f;i<poi;i++)
    {
      flag2=intersects(i%1000);
      if(flag2==true)
      {

        block[i%1000].rem_flag=1;
        if(block[i%1000].val2==0)
        {
          system("mpg123 -vC score.mp3 &");
          hit++;
        }
        else
        {
          miss++;
          if(miss>=miss_limit)
          {
            update_exit=max_time;
            exit_flag=1;
            zoom=0;
          }
        }
        break;
      }
    }
    if(flag2==true)
    {
      rem_flag=1;
      axis_x+=2;
    if(x+axis_x*cos(rotation_angle*M_PI/180)<-100 || x+axis_x*cos(rotation_angle*M_PI/180)>71 || y+axis_x*sin(rotation_angle*M_PI/180)<-65 || y+axis_x*sin(rotation_angle*M_PI/180)>100 )
    {
 //     free(bullet);
    }
      return;
    }
    flag=checkCollisionMirror();
    if(flag>-1)
    {
      system("mpg123 -vC mirror_collision.mp3 &");
      if(pre_flag==-1)
      {
        pre_flag=flag;
        x=((axis_x*cos(rotation_angle*M_PI/180)+x)+sin(mirrors[flag].rotation*M_PI/180)*c);
        y=(axis_x*sin(rotation_angle*M_PI/180)+y)-cos(mirrors[flag].rotation*M_PI/180)*c;
        rotation_angle=rotation_angle+2*(mirrors[flag].rotation-rotation_angle);
        axis_x=2.0;
       
      }
      else if(pre_flag!=flag)
      {
      pre_flag=flag;
      x=((axis_x*cos(rotation_angle*M_PI/180)+x)+sin(mirrors[flag].rotation*M_PI/180)*c);
      y=(axis_x*sin(rotation_angle*M_PI/180)+y)-cos(mirrors[flag].rotation*M_PI/180)*c;
      rotation_angle=rotation_angle+2*(mirrors[flag].rotation-rotation_angle);
      axis_x=2.0;

      }
    }
//    printf("%lf\n",rotation_angle);
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 transBullet1 = glm::translate (glm::vec3(axis_x,axis_y,0));
    glm::mat4 transBullet2 = glm::translate (glm::vec3(x,y,0));
    glm::mat4 rotateBullet = glm::rotate((float)(rotation_angle*M_PI/180.0f), glm::vec3(0,0,1));
    Matrices.model *=transBullet2 * rotateBullet * transBullet1;
    MVP= VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
    draw3DObject(bullet);
    axis_x+=2;
    if(x+axis_x*cos(rotation_angle*M_PI/180)<-100 || x+axis_x*cos(rotation_angle*M_PI/180)>71 || y+axis_x*sin(rotation_angle*M_PI/180)<-65 || y+axis_x*sin(rotation_angle*M_PI/180)>100 )
    {
//      free(bullet);
      rem_flag=1;
    }
  }

};

Bullets blt[1000];
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.

    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_A:
                Laser.laser_rot_dir=1;
                Laser.laser_rot_status=false;
                break;
            case GLFW_KEY_D:
                Laser.laser_rot_dir=-1;
                Laser.laser_rot_status=false;
                break;
            case GLFW_KEY_S:
                Laser.lasery_dir=1;
                Laser.lasery_status=false;
                break;
            case GLFW_KEY_F:
                Laser.lasery_dir=-1;
                Laser.lasery_status=false;
                break;
            case GLFW_KEY_N:
                if(speed_var<7)
                  speed_var+=1;
                break;
            case GLFW_KEY_M:
                if(speed_var>-3)
                  speed_var-=1;
                break;
            case GLFW_KEY_LEFT_CONTROL:
                ctrl=0;
                bucket[0].bx_status=false;
                break;
            case GLFW_KEY_RIGHT_CONTROL:
                ctrl=0;
                bucket[0].bx_status=false;
                break;
            case GLFW_KEY_LEFT_ALT:
                alt=0;
                bucket[1].bx_status=false;
                break;
            case GLFW_KEY_RIGHT_ALT:
                alt=0;
                bucket[1].bx_status=false;
                break;
            case GLFW_KEY_LEFT:
                if(ctrl==1)
                {
                bucket[0].bx_dir=-1;
                bucket[0].bx_status=false;                  
                }
                else if(alt==1)
                {
                bucket[1].bx_dir=-1;
                bucket[1].bx_status=false;                  
                }
                else
                {
                  if(pan>-zoom)
                    pan-=2;
                }
                break;
            case GLFW_KEY_RIGHT:
                if(ctrl==1)
                {
                bucket[0].bx_dir=1;
                bucket[0].bx_status=false;                  
                }
                else if(alt==1)
                {
                bucket[1].bx_dir=1;
                bucket[1].bx_status=false;                  
                }
                else
                {
                  if(pan<zoom)
                    pan+=2;
                }
                break;
            case GLFW_KEY_UP:
              if(zoom<50)
                zoom+=2;
                break;
            case GLFW_KEY_DOWN:
              if(zoom>=2)
                zoom-=2;
                break;
            case GLFW_KEY_C:
              if(pany<=zoom-2)
                pany+=2;
              break;
            case GLFW_KEY_X:
              if(pany>=-zoom+2)
                pany-=2;
              break;
            case GLFW_KEY_SPACE:
//              blt = new Bullets;
            if(flag_shoot==1)
            {
              system("mpg123 -vC bullet_fire.mp3 &");
              blt[poi2].createBullet();
              poi2++;
              flag_shoot=0;
            }
              break;
            case GLFW_KEY_ENTER:
              if(exit_flag==1)
                start_flag=1;
              break;
            case GLFW_KEY_P:
              pause_flag=(pause_flag+1)%2;
              break;
            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_A:
                Laser.laser_rot_dir=1;
                Laser.laser_rot_status=true;
                break;
            case GLFW_KEY_D:
                Laser.laser_rot_dir=-1;
                Laser.laser_rot_status=true;
                break;
            case GLFW_KEY_S:
                Laser.lasery_dir=1;
                Laser.lasery_status=true;
                break;
            case GLFW_KEY_F:
                Laser.lasery_dir=-1;
                Laser.lasery_status=true;
                break;
            case GLFW_KEY_N:
                break;
            case GLFW_KEY_M:
                break;
            case GLFW_KEY_LEFT_CONTROL:
                ctrl=1;
                break;
            case GLFW_KEY_RIGHT_CONTROL:
                ctrl=1;
                break;
            case GLFW_KEY_LEFT_ALT:
                alt=1;
                break;
            case GLFW_KEY_RIGHT_ALT:
                alt=1;
                break;
            case GLFW_KEY_LEFT:
                if(ctrl==1)
                {
                bucket[0].bx_dir=-1;
                bucket[0].bx_status=true;                  
                }
                else if(alt==1)
                {
                bucket[1].bx_dir=-1;
                bucket[1].bx_status=true;                  
                }
                break;
            case GLFW_KEY_RIGHT:
                if(ctrl==1)
                {
                bucket[0].bx_dir=1;
                bucket[0].bx_status=true;                  
                }
                else if(alt==1)
                {
                bucket[1].bx_dir=1;
                bucket[1].bx_status=true;                  
                }
                break;
            case GLFW_KEY_UP:
                break;
            case GLFW_KEY_DOWN:
                break;
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            default:
                break;
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

double x_g,y_g,val;

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_RELEASE)
            {
                if(Laser.mouse_flag==1)
                  Laser.mouse_flag=0;
                for(int i=0;i<2;i++)
                {
                  if(bucket[i].mouse_flag==1)
                    bucket[i].mouse_flag=0;
                }
            }
            else if(action == GLFW_PRESS)
            {
                glfwGetCursorPos(window, &xpos, &ypos);
                x_g=(xpos-400)*1.0/4;
                y_g=(300-ypos)*1.0/3;
                Laser.checkClick((xpos-400)*1.0/4,(300-ypos)*1.0/3);
//                printf("%lf %lf %d \n",x,y,Laser.mouse_flag);
                if(Laser.mouse_flag==0)
                {
                  for(int i=0;i<2;i++)
                  {
                    bucket[i].checkClick((xpos-400)*1.0/4,(300-ypos)*1.0/3);
                    if(bucket[i].mouse_flag==1)
                      break;
                  }
                  if(bucket[0].mouse_flag==0 && bucket[1].mouse_flag==0)
                  {

                    if(exit_flag==0)
                    {
                      if(x_g>=-98 && x_g<=-84 && y_g>=88 && y_g<=98)
                      {
                        if(pause_flag==0)
                          pause_flag=1;
                      }
                      else if(x_g>=-98 && x_g<=-84 && y_g>=76 && y_g<=86)
                      {
                        if(pause_flag==1)
                          pause_flag=0;
                        else
                        {
                          exit_flag=1;
                          start_flag=1;
                        }

                      }
                      else
                      {
                        if(x_g<=Laser.l2x && y_g>=(Laser.l2y+Laser.lasery))
                        {
                          val=90;
                        }
                        else if(x_g<=Laser.l2x && y_g<(Laser.l2y+Laser.lasery))
                        {
                          val=-90;
                        }
                        else
                        {
                        val=atan((y_g-Laser.l2y-Laser.lasery)*1.0/(x_g-Laser.l2x));
                        val=val*180/M_PI;
                        }

                        if(val<=65 && val>=-65)
                          Laser.laser_rot=val;
                        else if(val>65)
                          Laser.laser_rot=65;
                        else if(val<-65)
                         Laser.laser_rot=-65;
                        if(flag_shoot==1)
                        {
                          system("mpg123 -vC bullet_fire.mp3 &");
                          blt[poi2].createBullet();
                          poi2++;
                          flag_shoot=0;
                        }
                      }

                    }
                    else if(exit_flag==1 && start_flag==0)
                    {
                      if(x_g>=-6 && x_g<=8 && y_g>=-18 && y_g<=-8)
                      {
                        start_flag=1;
                      }
                    }

                  }

                }


            }
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_RELEASE) 
            {
              mouse_pan=0;
            }
            else if(action==GLFW_PRESS)
            {
              glfwGetCursorPos(window, &xpos, &ypos);
              x_g=(xpos-400)*1.0/4;
              y_g=(300-ypos)*1.0/3;
              mouse_pan=1;
            }
            break;

        default:
            break;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  if(yoffset>0 && zoom<50)
    zoom+=yoffset*2;
  if(yoffset<0 && zoom>=2)
    zoom+=yoffset*2;
  if(xoffset>0)
  {
     if(pan<zoom)
         pan+=2;
      else if(pan>zoom)
        pan=zoom;

  }
  else if(xoffset<0)
  {
    if(pan>-zoom)
      pan-=2;
    else if(pan<-zoom)
    {
      pan=-zoom;
    }

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
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(-100.0f+zoom*2, 100.0f-zoom*2, -100.0f+zoom*2, 100.0f-zoom*2, 0.1f, 500.0f);
}


float camera_rotation_angle = 90;

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw_rect(float x,float y,float rotation)
{
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 rotateRect = glm::rotate((float)(rotation*M_PI/180.0f), glm::vec3(0,0,1)); 
    glm::mat4 transRect = glm::translate (glm::vec3(x,y,0));
    Matrices.model *= transRect*rotateRect;
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
    draw3DObject(rect1);

}

void draw_boxes(int flag)
{
  glm::mat4 rotateRect,transRect;
  float x,y;
  if(flag==0)
  {
    x=-91;
    y=93;

    Matrices.model = glm::mat4(1.0f);
    rotateRect = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); 
    transRect = glm::translate (glm::vec3(x,y,0));
    Matrices.model *= transRect*rotateRect;
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
    draw3DObject(rect2);

    draw_rect(x-2,y,90);
    draw_rect(x+2,y,90);

  }
  else if(flag==1)
  {
    x=-91;
    y=81;

    Matrices.model = glm::mat4(1.0f);
    rotateRect = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); 
    transRect = glm::translate (glm::vec3(x,y,0));
    Matrices.model *= transRect*rotateRect;
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
    draw3DObject(rect2);
    draw_rect(x-2,y,90);
    draw_rect(x-2+2*cos(30.0*M_PI/180),y+1,-30);
    draw_rect(x-2+2*cos(30.0*M_PI/180),y-2*cos(60.0*M_PI/180),30);

  }
  else if(flag==2)
  {
    x=1;
    y=-13;

    Matrices.model = glm::mat4(1.0f);
    rotateRect = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); 
    transRect = glm::translate (glm::vec3(x,y,0));
    Matrices.model *= transRect*rotateRect;
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
    draw3DObject(rect2);
    draw_rect(x-2,y,90);
    draw_rect(x-2+2*cos(30.0*M_PI/180),y+1,-30);
    draw_rect(x-2+2*cos(30.0*M_PI/180),y-2*cos(60.0*M_PI/180),30);

  }
}

void draw_score(int flag)
{
  int value,value2,shift=6,flag2=0,i;
  i=0;
  float x=95,y;
  if(flag==0||flag==2)
  {
  value=score;
  y=70;
  if(value<0)
  {
    flag2=1;
    value=-1*value;
  }
  }
  else if(flag==1)
  {
    value=miss_limit-miss;
    y=50;
  }
  else if(flag==3)
  {
    value=level;
    y=30;
  }

  if(flag==2)
  {
    x=25;
    y=0;
  }
  do
  {
    value2=value%10;
    value=value/10;
    if(value2==2||value2==3||value2==5||value2==6||value2==8||value2==9||value2==0)
    {
      draw_rect(x-shift*i,y,0);
    }
    if(value2==2||value2==6||value2==8||value2==0)
    {
      draw_rect(x-2-shift*i,y+2,90);
    }
    if(value2==4||value2==5||value2==6||value2==8||value2==9||value2==0)
    {
      draw_rect(x-2-shift*i,y+6,90);
    }
    if(value2==2||value2==3||value2==5||value2==6||value2==8||value2==9||value2==7||value2==0)
    {
      draw_rect(x-shift*i,y+8,0);
    }
    if(value2==1||value2==2||value2==3||value2==4||value2==7||value2==8||value2==9||value2==0)
    {
      draw_rect(x+2-shift*i,y+6,90);
    }
    if(value2==1||value2==3||value2==4||value2==5||value2==6||value2==7||value2==8||value2==9||value2==0)
    {
      draw_rect(x+2-shift*i,y+2,90);
    }
    if(value2==2||value2==3||value2==4||value2==5||value2==6||value2==8||value2==9)
    {
      draw_rect(x-shift*i,y+4,0);
    }
  i++;

  }while(value>0);

  if(flag2==1)
  {
      value=-1*value;
      draw_rect(x-shift*i,y+4,0);

  }
}

void draw_gameover()
{
  float x,y;
  x=22.5;
  y=20;

  draw_rect(x-2,y+2,90);
  draw_rect(x-2,y+6,90);
  draw_rect(x,y+4,0);
  draw_rect(x,y+8,0);
  draw_rect(x+2,y+6,90);
  draw_rect(x-0,y+2,-45);

  draw_rect(x-5,y,0);
  draw_rect(x-5,y+4,0);
  draw_rect(x-5,y+8,0);
  draw_rect(x-5-2,y+2,90);
  draw_rect(x-5-2,y+6,90);

  draw_rect(x-5*2-1+2*sin(20.0*M_PI/180),y+2*cos(20.0*M_PI/180),70);
  draw_rect(x-5*2-1+6*sin(20.0*M_PI/180),y+6*cos(20.0*M_PI/180),70);
  draw_rect(x-5*2-1-2*sin(20.0*M_PI/180),y+2*cos(20.0*M_PI/180),-70);
  draw_rect(x-5*2-1-6*sin(20.0*M_PI/180),y+6*cos(20.0*M_PI/180),-70);

  draw_rect(x-5*3-1,y,0);
  draw_rect(x-5*3-2-1,y+2,90);
  draw_rect(x-5*3-2-1,y+6,90);
  draw_rect(x-5*3-1,y+8,0);
  draw_rect(x-5*3+2-1,y+2,90);
  draw_rect(x-5*3+2-1,y+6,90); 

  draw_rect(x-5*5,y,0);
  draw_rect(x-5*5,y+4,0);
  draw_rect(x-5*5,y+8,0);
  draw_rect(x-5*5-2,y+2,90);
  draw_rect(x-5*5-2,y+6,90);

  draw_rect(x-5*6-2+1,y+2,90);
  draw_rect(x-5*6-2+1,y+6,90);
  draw_rect(x-5*6+2,y+2,90);
  draw_rect(x-5*6+2,y+6,90);
  draw_rect(x-5*6-1-6*sin(20.0*M_PI/180)+3,y+6*cos(20.0*M_PI/180),-70);
  draw_rect(x-5*6-1+6*sin(20.0*M_PI/180),y+6*cos(20.0*M_PI/180),70);

  draw_rect(x-5*7-2*sin(20.0*M_PI/180),y+6*cos(20.0*M_PI/180),70);
  draw_rect(x-5*7-6*sin(20.0*M_PI/180),y+2*cos(20.0*M_PI/180),70);
  draw_rect(x-5*7+2*sin(20.0*M_PI/180),y+6*cos(20.0*M_PI/180),-70);
  draw_rect(x-5*7+6*sin(20.0*M_PI/180),y+2*cos(20.0*M_PI/180),-70);
  draw_rect(x-5*7,y+2,0);

  draw_rect(x-5*8-1,y,0);
  draw_rect(x-5*8,y+3,0);
  draw_rect(x-5*8-1,y+8,0);
  draw_rect(x-5*8-2-1,y+2,90);
  draw_rect(x-5*8-2-1,y+6,90);
  draw_rect(x-5*8+2-1,y+1,90);



}

VAO *boarder;
void createLine()
{
  static const GLfloat vertex_buffer_data [] = {
    -100,-0.5,0, // vertex 1
    -100,0.5,0, // vertex 2
    100, 0.5,0, // vertex 3

    100, 0.5,0, // vertex 3
    100, -0.5,0, // vertex 4
    -100,-0.5,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0,0,0, // color 1
    0,0,0, // color 2
    0,0,0, // color 3

    0,0,0, // color 3
    0,0,0, // color 4
    0,0,0  // color 1
  };
  // create3DObject creates and returns a handle to a VAO that can be used later
  boarder = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);

}

void draw_level()
{
  float x,y;
  x=95;
  y=40;

  draw_rect(x+1,y,0);
  draw_rect(x-2+1,y+2,90);
  draw_rect(x-2+1,y+6,90);

  draw_rect(x-5+1,y,0);
  draw_rect(x-5+1,y+4,0);
  draw_rect(x-5+1,y+8,0);
  draw_rect(x-5-2+1,y+2,90);
  draw_rect(x-5-2+1,y+6,90);

  draw_rect(x-5*2+2*sin(20.0*M_PI/180),y+2*cos(20.0*M_PI/180),70);
  draw_rect(x-5*2+6*sin(20.0*M_PI/180),y+6*cos(20.0*M_PI/180),70);
  draw_rect(x-5*2-2*sin(20.0*M_PI/180),y+2*cos(20.0*M_PI/180),-70);
  draw_rect(x-5*2-6*sin(20.0*M_PI/180),y+6*cos(20.0*M_PI/180),-70);

  draw_rect(x-5*3,y,0);
  draw_rect(x-5*3,y+4,0);
  draw_rect(x-5*3,y+8,0);
  draw_rect(x-5*3-2,y+2,90);
  draw_rect(x-5*3-2,y+6,90);

  draw_rect(x-5*4,y,0);
  draw_rect(x-5*4-2,y+2,90);
  draw_rect(x-5*4-2,y+6,90);

}
void draw_scoretext(int flag_option)
{
  float x,y;
  if(flag_option==0)
  {
    x=95;
    y=85;
  }
  else if(flag_option==1)
  {
    x=0;
    y=0;
    /*draw score */
  }
  draw_rect(x,y,0);
  draw_rect(x,y+4,0);
  draw_rect(x,y+8,0);
  draw_rect(x-2,y+2,90);
  draw_rect(x-2,y+6,90);

  draw_rect(x-5-2,y+2,90);
  draw_rect(x-5-2,y+6,90);
  draw_rect(x-5,y+4,0);
  draw_rect(x-5,y+8,0);
  draw_rect(x-5+2,y+6,90);
  draw_rect(x-5-0,y+2,-45);

  draw_rect(x-5*2,y,0);
  draw_rect(x-5*2-2,y+2,90);
  draw_rect(x-5*2-2,y+6,90);
  draw_rect(x-5*2,y+8,0);
  draw_rect(x-5*2+2,y+2,90);
  draw_rect(x-5*2+2,y+6,90);

  draw_rect(x-5*3,y,0);
  draw_rect(x-5*3-2,y+2,90);
  draw_rect(x-5*3-2,y+6,90);
  draw_rect(x-5*3,y+8,0);

  draw_rect(x-5*4,y,0);
  draw_rect(x-5*4+2,y+2,90);
  draw_rect(x-5*4,y+4,0);
  draw_rect(x-5*4-2,y+6,90);
  draw_rect(x-5*4,y+8,0);



  if(flag_option==0)
  {
    y=60;
  draw_rect(x,y,0);
  draw_rect(x+2,y+2,90);
  draw_rect(x,y+4,0);
  draw_rect(x-2,y+6,90);
  draw_rect(x,y+8,0);

  draw_rect(x-5,y,0);
  draw_rect(x-5,y+4,0);
  draw_rect(x-5,y+8,0);
  draw_rect(x-5-2,y+2,90);
  draw_rect(x-5-2,y+6,90);

  draw_rect(x-5*2-1+2*sin(20.0*M_PI/180),y+2*cos(20.0*M_PI/180),70);
  draw_rect(x-5*2-1+6*sin(20.0*M_PI/180),y+6*cos(20.0*M_PI/180),70);
  draw_rect(x-5*2-1-2*sin(20.0*M_PI/180),y+2*cos(20.0*M_PI/180),-70);
  draw_rect(x-5*2-1-6*sin(20.0*M_PI/180),y+6*cos(20.0*M_PI/180),-70);

  draw_rect(x-5*3-2,y+2,90);
  draw_rect(x-5*3-2,y+6,90);

  draw_rect(x-5*4,y,0);
  draw_rect(x-5*4-2,y+2,90);
  draw_rect(x-5*4-2,y+6,90);

  }


}


void draw (double x,double y)
{
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  if(pan>zoom)
    pan=zoom;
  else if(pan<-zoom)
    pan=-zoom;
  if(pany>zoom)
    pany=zoom;
  else if(pany<-zoom)
    pany=-zoom;


  if(mouse_pan==1)
  {
    if(pan+(x-x_g)>zoom)
    {
      pan=zoom;
      x_g=x;
    }
    else if((pan+(x-x_g))<=zoom && (pan+(x-x_g))>=-zoom )
    {
      pan=pan+(x-x_g);
      x_g=x;
    }
    else if(pan+(x-x_g)<-zoom)
    {
      pan=-zoom;
      x_g=x;
    }
    if(pany+(y-y_g)>zoom)
    {
     pany=zoom;
     y_g=y;
    }
    else if((pany+(y-y_g))<=zoom && (pany+(y-y_g))>=-zoom)
    {
      pany=pany+(y-y_g);
      y_g=y;
    }
    else if(pany+(y-y_g)<-zoom)
    {

      pany=-zoom;
      y_g=y;
    }

  }
  Matrices.view = glm::lookAt(glm::vec3( 0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

//  Matrices.view = glm::lookAt(glm::vec3( pan,pany,3), glm::vec3(pan,pany,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane
  Matrices.projection = glm::ortho(-100.0f+zoom+pan, 100.0f-zoom+pan, -100.0f+zoom+pany, 100.0f-zoom+pany, 0.1f, 500.0f);

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
    // MVP = Projection * View * Model

  // Load identity to model matrix
 
  /* Render your scene */

  // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
  // glPopMatrix ();
/*  if(Laser.mouse_flag==1)
    Laser.checkClick(x,y);
    for(int i=0;i<2;i++)
    {
      if(bucket[i].mouse_flag==1)
        bucket[i].checkClick(x,y);
    }*/

    if(Laser.mouse_flag==1)
    {
      if(Laser.lasery+30+(y-y_g)>75)
      {
        Laser.lasery=75-30;
        y_g=y;
      }
      else if((Laser.lasery+30+(y-y_g))<=75 && (Laser.lasery+(y-y_g))>=-60)
      {
        Laser.lasery+=(y-y_g);
        y_g=y;
      }
      else if((Laser.lasery+(y-y_g))<-60)
      {
        Laser.lasery=-60;
        y_g=y;
      }

    }
    else
    {
    for(int i=0;i<2;i++)
    {
      if(bucket[i].mouse_flag==1)
      {
        if((bucket[i].bx-40+bucket[i].extra+(x-x_g))>50)
        {
          bucket[i].bx=90-bucket[i].extra;
          x_g=x;
        }
        else if((bucket[i].bx-40+bucket[i].extra+(x-x_g))<=50 && (bucket[i].bx-60+bucket[i].extra+(x-x_g))>=-70)
        {
          bucket[i].bx=bucket[i].bx+(x-x_g);
          x_g=x;
        }
        else if(x<-70)
        {
          bucket[i].bx=-10-bucket[i].extra;
          x_g=x;
        }
      }
    }

    }
long long int r;
  score = 4*hit + 3*caught - miss;

  if(score > ((speed_var+1)*50) )
  {
    if(speed_var<7)
    {
      system("mpg123 -vC level_up.mp3 &");
      level++;
      speed_var++;
    }
  }

  if(exit_flag==1)
  {
      if(start_flag==1)
      {
        exit_flag=0;
        score=0;
        miss=0;
        caught=0;
        Laser.laser_rot=0;
        Laser.lasery=0;
        pause_flag=0;
        speed_var=0;
        zoom=0;
        level=1;

        mirrors[0].x=-10;mirrors[0].y=85;mirrors[0].rotation=-30;
        mirrors[1].x=-10;mirrors[1].y=0;mirrors[1].rotation=65;
        mirrors[2].x=60;mirrors[2].y=70;mirrors[2].rotation=-50;
        mirrors[3].x=60;mirrors[3].y=-50;mirrors[3].rotation=50;

        last_update_time=glfwGetTime();
        update_shoot=glfwGetTime();
        update_bullet=glfwGetTime();
        updatetime_fall=glfwGetTime();
        for(int i=0;i<2;i++)
        {
          bucket[i].bx=0;
        }
        hit=0;
        dump=0;
        for(int i=f;i<poi;i++)
        {
          free(block[i].brick);
        }
        f=0;
        poi=0;
        for(int i=f2;i<poi2;i++)
        {
          free(blt[i].bullet);
        }
        f2=0;
        poi2=0;
        start_flag=0;
        return;
      }
      else
      {
      draw_boxes(2);
      draw_scoretext(1);
      draw_score(2);
      draw_gameover();
      }
      return;
  }

  draw_boxes(0);
  draw_boxes(1);
  draw_scoretext(0);
  draw_level();
  draw_score(3);
  draw_score(0);
  draw_score(1);

  if(flag_mirror==1)
  {
    for(int i=0;i<4;i++)
    {
      mirrors[i].rotation=mirrors[i].rotation+0.025*(2+speed_var/2);
    }
    flag_mirror=0;
  }

        /*Boarder Creation*/

    Matrices.model = glm::mat4(1.0f);
    glm::mat4 rotateBoarder = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); 
    glm::mat4 transBoarder = glm::translate (glm::vec3(0,-65.5,0));
    Matrices.model *= transBoarder*rotateBoarder;
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
    draw3DObject(boarder);

    Matrices.model = glm::mat4(1.0f);
    rotateBoarder = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1)); 
    transBoarder = glm::translate (glm::vec3(71,0,0));
    Matrices.model *= transBoarder*rotateBoarder;
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID,1,GL_FALSE,&MVP[0][0]);
    draw3DObject(boarder);



          /*Laser Movement*/
              Laser.draw();


     /* Bullet Movement */ 

  if(flag_bullet==1)
  {

  for(int i=f2;i<poi2;i++)
    blt[i%1000].draw();

  r=f2;
  for(int i=f2;i<poi2;i++)
  {
    if((blt[i%1000].rem_flag)==1)
      r++;
    else
      break;
  }
//  printf("Second %lld %lld\n",r,f2);
  f2=r;
  flag_bullet=0;
  }
 

    /* brick generation */

  if(brick_flag==1 )
  {
    block[poi%1000].generateBlock();
    block[poi%1000].createBrick();
    brick_flag=0;
    poi++;
  }

  for(int i=f;i<(poi);i++)
  {
     block[i%1000].checkBlock();
  }
  fall_flag=0;


       /*Baskets Movement*/

  for(int i=0;i<2;i++)
  {
    if(ctrl==1)
      bucket[(i+1)%2].draw();
    else
      bucket[i].draw();
  }


    /* Mirror Placement*/
  
  for(int i=0;i<4;i++)
  {
    mirrors[i].drawMirror();
  }
  //camera_rotation_angle++; // Simulating camera rotation
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
//        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Brick Breaker", NULL, NULL);

    if (!window) {
        glfwTerminate();
//        exit(EXIT_FAILURE);
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
    glfwSetScrollCallback(window, scroll_callback);

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
    // Create the models
  bucket[1].extra=80;
  bucket[0].extra=0;
  for(int i=0;i<2;i++)
  {
    bucket[i].createBaskRect();
    bucket[i].createBaskCirc();
  }
  Laser.createLaser();
  Laser.createLaser2();
  Laser.createlasercirc();
  for(int i=0;i<4;i++)
  {
    mirrors[i].createMirror();
  }

  mirrors[0].x=-10;mirrors[0].y=85;mirrors[0].rotation=-30;
  mirrors[1].x=-10;mirrors[1].y=0;mirrors[1].rotation=65;
  mirrors[2].x=60;mirrors[2].y=70;mirrors[2].rotation=-50;
  mirrors[3].x=60;mirrors[3].y=-50;mirrors[3].rotation=50;

  createRectangle();
  createRectangle2();
  createLine();
    // Create and compile our GLSL program from the shaders
    programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
    // Get a handle for our "MVP" uniform
    Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

    
    reshapeWindow (window, width, height);

    // Background color of the scene
    glClearColor (1.0f, 1.0f, 1.0f, 0.0f); // R, G, B, A
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
    int width = 800;
    int height = 600;
    double x,y;

    GLFWwindow* window = initGLFW(width, height);

    initGL (window, width, height);

//    glfwGetCursorPos(window, &xpos, &ypos);
    // Draw in loop 
    while (!glfwWindowShouldClose(window)) {

        // OpenGL Draw commands
        glfwGetCursorPos(window, &xpos, &ypos);
        x=(xpos-400)*1.0/4;
        y=(300-ypos)*1.0/3;
/*        if(exit_flag==1)
        {
          update_exit--;
          if(update_exit==0)
          {
            quit(window);
            break;
          }

        }*/

//        printf("%lf\n %lf\n",xpos,ypos);
        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        if(pause_flag==1)
          continue;
//      cout << '\a'<<flush;        

        draw(x,y);

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 1-speed_var*0.05) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
            brick_flag=1;
            last_update_time = current_time;
        }
        if ((current_time - update_mirror) >= 0.075-speed_var*0.001) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
            flag_mirror=1;
            update_mirror = current_time;
        }        
        if ((current_time - update_shoot) >= 1) { // atleast 0.5s elapsed since last frame
            flag_shoot=1;
            update_shoot = current_time;
        }
        if ((current_time - update_bullet) >= 0.01) { // atleast 0.5s elapsed since last frame
            flag_bullet=1;
            update_bullet = current_time;
        }
        if ((current_time - updatetime_fall) >= 0.075-speed_var*0.001) { // atleast 0.5s elapsed since last frame
            fall_flag=1;
            updatetime_fall = current_time;
        }

    }

    glfwTerminate();
//    exit(EXIT_SUCCESS);
}
