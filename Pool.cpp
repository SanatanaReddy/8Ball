#define GL_SILENCE_DEPRECATION
#define GLEW_STATIC
#include "Ball.h"
#include "TwoDVector.h"
#include "Cue.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "jpeglib.h"
#include <cstdio>

const char *vertexSource = "#version 330 core\n"
    "layout (location = 0) in vec3 position;\n"
    "layout (location = 1) in vec3 norm;\n"
    "layout (location = 2) in vec2 texCoor;\n"
    "layout (location = 3) in float mixCoefficient;\n"
    "out vec2 textureCoordinate;\n"
    "out vec3 fragPosition;\n"
    "out vec3 normal;\n"
    "out float mixCoef;\n"
    "uniform mat4 rotation;\n"
    "uniform mat4 model;\n"
    "uniform mat4 mvp;\n"
    "void main()\n"
    "{\n"
    "   mixCoef = mixCoefficient;\n"
    "   textureCoordinate = texCoor;\n"
    "   fragPosition = vec3(model * vec4(position,1.0f));\n"
    "   normal = normalize(vec3( rotation * vec4(norm,1.0f)));\n"
    "   gl_Position =  mvp * vec4(position,1.0f);\n"
    "}\0";

const char *fragmentSource = "#version 330 core\n"
    "in vec3 fragPosition;\n"
    "in vec3 normal;\n"
    "in vec2 textureCoordinate;\n"
    "in float mixCoef;\n"
    "out vec4 outColor;\n"
    "uniform vec3 lightColor;\n"
    "uniform vec3 color;\n"
    "uniform sampler2D currentTexture;\n"
    "uniform vec3 lightPosition;\n"
    "uniform vec3 cameraPosition;\n"
    "uniform float specularCoefficient;\n"
    "void main()\n"
    "{\n"
    "   float ambientStrength = 0.1;\n"
    "   vec3 lightDirection = normalize(lightPosition -  fragPosition);\n"
    "   float diffuseStrength = max(dot(normal,lightDirection),0.0);\n"
    "   vec3 cameraDirection = normalize(cameraPosition - fragPosition);\n"
    "   vec3 reflection = reflect(-lightDirection, normal);\n"
    "   float specularStrength = pow(max(dot(cameraDirection,reflection),0.0),32.0);\n"
    "   vec4 texColor = mix(texture(currentTexture, textureCoordinate),vec4(color,1.0),mixCoef);\n"
    "   vec3 result = (ambientStrength + diffuseStrength + (specularCoefficient * specularStrength)) * lightColor * vec3(texColor);\n"
    "   outColor = vec4(result,1.0);\n"
    "}\0";

    
int readjpeg(const char *name, int* xres, int* yres, unsigned char **imgdata) {
  FILE * ifp;
  struct jpeg_decompress_struct cinfo; 
  struct jpeg_error_mgr jerr;          
  JSAMPROW row_pointer[1];             
  int row_stride;                      
  if ((ifp = fopen(name, "rb")) == NULL) {
    return 1; 
  }
  cinfo.err = jpeg_std_error(&jerr); 
  jpeg_create_decompress(&cinfo);     
  jpeg_stdio_src(&cinfo, ifp);       
  jpeg_read_header(&cinfo, TRUE);    
  jpeg_start_decompress(&cinfo);     
  *xres = cinfo.output_width;        
  *yres = cinfo.output_height;       
  row_stride = cinfo.output_width * cinfo.output_components;
  *imgdata = (unsigned char *) malloc(row_stride * cinfo.output_height);
  while (cinfo.output_scanline < cinfo.output_height) {
    row_pointer[0] = &((*imgdata)[(cinfo.output_scanline)*row_stride]);
    jpeg_read_scanlines(&cinfo, row_pointer, 1);
  }
  jpeg_finish_decompress(&cinfo);   
  jpeg_destroy_decompress(&cinfo);  
  fclose(ifp); 
  return 0;
}

bool ballMoving( const std::vector<Ball>& balls){
    bool moving = false;
    std::vector<Ball>::const_iterator iter = balls.begin();
    while(!moving && iter != balls.end())
    {
        if( iter->Velocity.x != 0 || iter->Velocity.y != 0)
        {
            moving = true;
        }
        ++iter;
    }
    return moving;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        void *data = glfwGetWindowUserPointer(window);
        Cue *cp = static_cast<Cue *>(data);
        if(cp->visible && !(cp->shooting))
        {
            cp->shooting = true;
            cp->velocity = fmax(-10.0 * (cp->distanceToCue - cp->length), -19.0);
        }
    }
}

void configureVertexAttributeObject( unsigned int vao, unsigned int vbo, unsigned int ebo, int datasize, int indexsize,float vertices[], const unsigned int* indices){ 
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, datasize, vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexsize, indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2,2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*) (6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3,1, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*) (8* sizeof(float)));
    glEnableVertexAttribArray(3);
    glBindVertexArray(0);
}

void configureVertexAttributeObject( unsigned int vao, unsigned int vbo, unsigned int ebo, const std::vector<float>& vertices, const std::vector<unsigned int>& indices){ 
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2,2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*) (6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3,1, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*) (8* sizeof(float)));
    glEnableVertexAttribArray(3);
    glBindVertexArray(0);
}

TwoDVector startingLocation( int x, int y){
    TwoDVector ret( 13.0 + 1.732 * Ball::radius * x, 5.0 + Ball::radius * y);
    return ret;
}


int main(){

    //Create sphere model vertex data.
    int numberOfPoints = 20;
    int layers = 20;
    std::vector<float> sphereVertices;
    sphereVertices.push_back(0.0);
    sphereVertices.push_back(0.0);
    sphereVertices.push_back(-1.0);
    sphereVertices.push_back(0.0);
    sphereVertices.push_back(0.0);
    sphereVertices.push_back(-1.0);
    sphereVertices.push_back(0.0);
    sphereVertices.push_back(1.0);
    sphereVertices.push_back(0.0);
    for(int row = 1; row < layers; ++row)
    {
        float height = -1.0 + 2.0 * ((float) row/(float)layers);
        float radius = sqrt(1 - height * height);
        float startingfraction = 0;
        for(int point = 0; point < numberOfPoints; ++point)
        {
            for(int j = 1; j <=2; ++j)
            {
                sphereVertices.push_back(std::cos(((float)point/numberOfPoints + startingfraction)*6.2831) * radius);
                sphereVertices.push_back(std::sin(((float)point/numberOfPoints + startingfraction)*6.2831) * radius);
                sphereVertices.push_back(height);
            }
            sphereVertices.push_back( 0.1 + 0.8* ((float)point/numberOfPoints + startingfraction));
            sphereVertices.push_back(0.75 - (height + 1.0)/4.0);
            if((point == 0 || point == numberOfPoints -1) && (height < 0.57 && height > -0.57))
            {
                sphereVertices.push_back(1.0);
            }
            else
            {
                sphereVertices.push_back(0.0);
            }
        }
    }
    sphereVertices.push_back(0.0);
    sphereVertices.push_back(0.0);
    sphereVertices.push_back(1.0);
    sphereVertices.push_back(0.0);
    sphereVertices.push_back(0.0);
    sphereVertices.push_back(1.0);
    sphereVertices.push_back(0.0);
    sphereVertices.push_back(0.0);
    sphereVertices.push_back(0.0);

    //Create sphere model index data.
    std::vector<unsigned int> sphereIndices;
    for(int i = 1; i < numberOfPoints; ++i)
    {
        sphereIndices.push_back(0);
        sphereIndices.push_back(i);
        sphereIndices.push_back(i+1);
    }    
    sphereIndices.push_back(0);
    sphereIndices.push_back(numberOfPoints);
    sphereIndices.push_back(1);
    for(int rowindex = 0; rowindex < layers - 2; ++rowindex)
    {   
        for(int i = 1; i < numberOfPoints; ++i)
        {
            sphereIndices.push_back( numberOfPoints * (rowindex) + i);   
            sphereIndices.push_back( numberOfPoints * (rowindex) + i + 1);   
            sphereIndices.push_back( numberOfPoints * (rowindex + 1) + i);   
            sphereIndices.push_back( numberOfPoints * (rowindex) + i + 1);   
            sphereIndices.push_back( numberOfPoints * (rowindex + 1) + i);   
            sphereIndices.push_back( numberOfPoints * (rowindex + 1) + i + 1);
        }
        sphereIndices.push_back( numberOfPoints * (rowindex) + 1);
        sphereIndices.push_back( numberOfPoints * (rowindex + 1) + 1);   
        sphereIndices.push_back( numberOfPoints * (rowindex + 2));   
        sphereIndices.push_back( numberOfPoints * (rowindex) + 1);   
        sphereIndices.push_back( numberOfPoints * (rowindex + 1));   
        sphereIndices.push_back( numberOfPoints * (rowindex + 2));   
    }
    for(int p = (layers - 2) * numberOfPoints + 1; p < (layers - 1) * numberOfPoints; ++p)
    {
        sphereIndices.push_back(p);
        sphereIndices.push_back(p+1);
        sphereIndices.push_back( (layers - 1) * numberOfPoints + 1);
    }
    sphereIndices.push_back( (layers - 2) * numberOfPoints + 1);
    sphereIndices.push_back( (layers - 1) * numberOfPoints);
    sphereIndices.push_back( (layers - 1) * numberOfPoints + 1);
    
    //Create hole model vertex data.
    float holeRadius = 0.35;
    float cornerOffset = -0.15;
    float edgeOffset = 0.35/3.0;
    std::vector<float> holeVertices;
    int pointsOnCircle = 20;
    holeVertices.push_back(0.0);
    holeVertices.push_back(0.0);
    holeVertices.push_back(0.0);
    holeVertices.push_back(0.0);
    holeVertices.push_back(0.0);
    holeVertices.push_back(1.0);
    holeVertices.push_back(0.0);
    holeVertices.push_back(0.0);
    holeVertices.push_back(1.0);
    for(int i = 0; i < pointsOnCircle; ++i)
    {
        holeVertices.push_back(std::cos( (float)i/pointsOnCircle * 6.2832));
        holeVertices.push_back(std::sin( (float)i/pointsOnCircle * 6.2832));
        holeVertices.push_back(0.0);
        holeVertices.push_back(0.0);
        holeVertices.push_back(0.0);
        holeVertices.push_back(1.0);
        holeVertices.push_back(0.0);
        holeVertices.push_back(0.0);
        holeVertices.push_back(1.0);
    }

    //Create hole model index data.
    std::vector<unsigned int> holeIndices;
    for(unsigned int i = 1; i < pointsOnCircle; ++i)
    {
        holeIndices.push_back(0);
        holeIndices.push_back(i);
        holeIndices.push_back(i+1);
    }
    holeIndices.push_back(0);
    holeIndices.push_back(pointsOnCircle);
    holeIndices.push_back(1);

    //Create hole location vector.
    std::vector<TwoDVector> holeLocations;
    holeLocations.push_back(TwoDVector( -1.0 * cornerOffset , -1.0 * cornerOffset));
    holeLocations.push_back(TwoDVector(9.5, -1.0 * edgeOffset));
    holeLocations.push_back(TwoDVector(19.0 + cornerOffset, -1.0 * cornerOffset));
    holeLocations.push_back(TwoDVector(-1.0 * cornerOffset , 10.0 +  cornerOffset));
    holeLocations.push_back(TwoDVector(9.5, 10.0 + edgeOffset));
    holeLocations.push_back(TwoDVector(19.0 + cornerOffset, 10.0 + cornerOffset));

    //Create table vertex data.  
    float tableVertices[] = {
        0.0,  0.0,  0.0,    0.0, 0.0, 1.0,   0.0, 0.0,  0.8, 
        19.0, 0.0,  0.0,    0.0, 0.0, 1.0,   19.0, 0.0,  0.8,
        19.0, 10.0, 0.0,    0.0, 0.0, 1.0,   19.0, 10.0,  0.8,
        0.0,  10.0, 0.0,    0.0, 0.0, 1.0,   0.0, 10.0,  0.8,
        0.0,  0.0,  0.0,    1.0, 0.0, 0.0,   1.0, 1.0,  0.0,
        0.0,  10.0, 0.0,    1.0, 0.0, 0.0,   1.0, 0.0,  0.0,
        0.0,  0.0,  0.5,    1.0, 0.0, 0.0,   0.0, 0.0,  0.0,
        0.0,  10.0, 0.5,    1.0, 0.0, 0.0,   1.0, 0.0,  0.0,
    };


    // Create table index data.
    unsigned int tableIndices[] = {
        0, 1, 2,
        0, 2, 3,
        4, 5, 6,
        5, 6, 7,
    };
    //Create pool cue vertex data.
    float cueVertices[] = {
        0.02, -0.013, 0.0,   0.0, 0.0, 1.0,   0.806, 0.483,  0.0,
        1.0, -0.004, 0.0,   0.0, 0.0, 1.0,   0.017, 0.5,  0.0,
        1.0, 0.004,  0.0,   0.0, 0.0, 1.0,   0.017, 0.515,  0.0,
        0.02, 0.013,  0.0,   0.0, 0.0, 1.0,   0.806, 0.53,  0.0,
        0.0, -0.009, 0.0, 0.0, 0.0, 1.0, 0.806, 0.53, 0.0,
        0.0, 0.009, 0.0, 0.0, 0.0, 1.0,0.806,0.53,0.0,
    };

    //Create pool cue index data.
    unsigned int cueIndices[] = {
        0, 1, 2,
        0, 2, 3,
        0, 4, 5, 
        0, 5, 3,
    };

    // Create bumper vertex data. 
    float chd = sqrt( holeRadius * holeRadius - cornerOffset * cornerOffset) - cornerOffset; //CornerHoleDistance
    float ehd= sqrt( holeRadius * holeRadius - edgeOffset * edgeOffset); //EdgeHoleDistance
    float bt = 0.15; //Bumper Thickness
    float bumperCorners[] = {
        chd, 0, chd + bt, bt, 9.5 - ehd - 0.5 * bt, bt, 9.5 - ehd, 0,
        9.5 + ehd, 0, 9.5 + ehd + 0.5 * bt, bt, 19.0-chd - bt, bt, 19.0 - chd, 0,
        19.0, chd, 19.0 - bt, chd + bt, 19.0 - bt, 10.0 - chd-bt, 19.0, 10.0-chd,
        19.0 - chd, 10.0, 19.0- chd - bt, 10.0 -bt, 9.5 + ehd + 0.5 * bt, 10.0-bt, 9.5 + ehd, 10.0,
        9.5 - ehd, 10.0, 9.5 - ehd - 0.5*bt, 10.0 - bt, chd + bt, 10.0 - bt,chd, 10.0,
        0.0, 10.0 - chd, bt, 10.0 - chd - bt, bt, chd + bt, 0.0, chd, 
    };
    std::vector<float> bumperVertices;
    for(unsigned int i = 0; i < sizeof(bumperCorners)/sizeof(float)- 1; i+=2)
    {
        bumperVertices.push_back(bumperCorners[i]);
        bumperVertices.push_back(bumperCorners[i+1]);
        bumperVertices.push_back(0.1);
        bumperVertices.push_back(0.0);
        bumperVertices.push_back(0.0);
        bumperVertices.push_back(1.0);
        bumperVertices.push_back(0.0);
        bumperVertices.push_back(0.0);
        bumperVertices.push_back(1.0);
    } 
    std::vector<TwoDVector> bumperEdges;
    for(unsigned int i = 0; i < sizeof(bumperCorners)/sizeof(float) - 7; i += 8)
    {   
        for(unsigned int j = i; j < i +5;j+=2)
        {
            bumperEdges.push_back(TwoDVector(bumperCorners[j], bumperCorners[j+1]));
            bumperEdges.push_back(TwoDVector(bumperCorners[j+2], bumperCorners[j+3]));
        }
    }

    //Create bumper index data.
    unsigned int bumperElements[] = {
        0,1,2,
        0,2,3,
        4,5,6,
        4,6,7,
        8,9,10,
        8,10,11,
        12,13,14,
        12,14,15,
        16,17,18,
        16,18,19,
        20,21,22,
        20,22,23,
    };
    std::vector<unsigned int> bumperIndices(std::begin(bumperElements),std::end(bumperElements));

    //Create wood vertex data.
    float woodVertices[] = {
        -0.5,  -0.5,  -0.01,    0.0, 0.0, 1.0,   0.0, 0.0,  0.0, 
        19.5, -0.5,  -0.01,    0.0, 0.0, 1.0,   1.0, 0.0,  0.0,
        19.5, 10.5, -0.01,    0.0, 0.0, 1.0,   1.0, 1.0,  0.0,
        -0.5,  10.5, -0.01,    0.0, 0.0, 1.0,   0.0, 1.0,  0.0,
    };
    
    //Create wood index data.
    unsigned int woodIndices[] = {
        0,1,2,
        0,2,3,
    };
  
    //Create Ball objects.
    TwoDVector vel(0.0,0.0);
    TwoDVector pos(9.5,5.0);
    Ball cueBall(vel, pos, 0, 0.98 ,0.95,0.92);
    std::vector<Ball> balls;
    balls.push_back(cueBall);
    balls.push_back( Ball(TwoDVector( 0.0,0.0 ), startingLocation(0,0), 1, 1.0 , 0.87, 0.16 ));
    balls.push_back( Ball(TwoDVector( 0.0,0.0 ), startingLocation(1,-1), 2, 0.145, 0.184, 0.67 ));
    balls.push_back( Ball(TwoDVector( 0.0,0.0 ), startingLocation(4,4),3, 1.0, 0.0, 0.01 ));
    balls.push_back( Ball(TwoDVector( 0.0,0.0 ), startingLocation(4, 0), 4, 0.406, 0.0, 0.407 ));
    balls.push_back( Ball(TwoDVector( 0.0,0.0 ), startingLocation(2,0), 5, 1.0, 0.3, 0.01 ));
    balls.push_back( Ball(TwoDVector( 0.0,0.0 ), startingLocation(2,2), 6, 0.01, 0.35, 0.18 ));
    balls.push_back( Ball(TwoDVector( 0.0,0.0 ), startingLocation(3,-3), 7, 0.63, 0.0, 0.01 ));
    balls.push_back( Ball(TwoDVector( 0.0,0.0 ), startingLocation(3,-1), 8, 0.0, 0.0, 0.0 ));
    balls.push_back( Ball(TwoDVector( 0.0,0.0 ), startingLocation(3,1),9, 1.0, 0.87, 0.16 ));
    balls.push_back( Ball(TwoDVector( 0.0,0.0 ), startingLocation(3,3), 10, 0.145, 0.184, 0.67 ));
    balls.push_back( Ball(TwoDVector( 0.0,0.0 ), startingLocation(4,-4), 11, 1.0, 0.0, 0.01 ));
    balls.push_back( Ball(TwoDVector( 0.0,0.0 ), startingLocation(4,-2), 12, 0.406, 0.0, 0.407 ));
    balls.push_back( Ball(TwoDVector( 0.0,0.0 ), startingLocation(2,-2),13, 1.0, 0.3, 0.01 ));
    balls.push_back( Ball(TwoDVector( 0.0,0.0 ), startingLocation(4,2), 14, 0.01, 0.35, 0.18 ));
    balls.push_back( Ball(TwoDVector( 0.0,0.0 ), startingLocation(1,1), 15, 0.63, 0.0, 0.01 ));
    
    //Create Cue object.
    Cue* cuePointer;
    Cue poolCue;
    cuePointer = &poolCue;

    //Initialize glfw with window specifications.
    glfwInit();                                                            
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

    //Create glfw window.
    GLFWwindow* window = glfwCreateWindow(1425, 750, "OpenGL", nullptr, nullptr); 
    glfwMakeContextCurrent(window);
    glViewport(0,0,1425,750);
    glfwSetWindowUserPointer(window, cuePointer);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    
    //Initialize glew.
    glewExperimental = GL_TRUE;                                     
    glewInit(); 
    glEnable(GL_DEPTH_TEST);

    //Load texture data.
    std::vector<unsigned char**> textureData;
    std::vector<int*> textureWidths;
    std::vector<int*> textureHeights;
    std::vector<std::string> textureFilenames;
    textureFilenames.push_back("BallCue.jpg");
    for(int i = 1; i < 16; ++i)
    {
        textureFilenames.push_back( "Ball" + std::to_string(i) + std::string(".jpg"));
    }
    textureFilenames.push_back("PoolTable.jpeg");
    textureFilenames.push_back("PoolCue.jpg");
    textureFilenames.push_back("Wood.jpg");
    for(int i =0; i< textureFilenames.size(); ++i)
    { 
        textureData.push_back( new(unsigned char*));
        textureWidths.push_back(new int);
        textureHeights.push_back(new int);
    }
    for(int i = 0; i < textureFilenames.size(); ++i)
    {
        readjpeg( textureFilenames[i].c_str(), textureWidths[i], textureHeights[i], textureData[i]);
    }

    //Configure texture objects.
    GLuint textures[textureFilenames.size()];
    glGenTextures(textureFilenames.size(), textures);
    for( int i = 0; i < textureFilenames.size(); ++i)
    {   
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, *(textureWidths[i]), *(textureHeights[i]), 0, GL_RGB, GL_UNSIGNED_BYTE, *(textureData[i]));
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    
    //Free texture data pointers.
    for(int i = 0; i< textureFilenames.size(); ++i)
    {
         delete(textureData[i]);
         delete(textureWidths[i]);
         delete(textureHeights[i]);
    }

    //Create and compile vertex shader object.
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    //Create and compile fragment shader object.
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);
 
    //Create and link shader program.
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    //Create buffer objects.
    GLuint vaos[6], vbos[6], ebos[6];
    glGenVertexArrays(6, vaos);
    glGenBuffers(6, vbos);
    glGenBuffers(6, ebos);

    //Configure Attribute Objects.
    configureVertexAttributeObject(vaos[0], vbos[0], ebos[0],sphereVertices, sphereIndices);
    configureVertexAttributeObject(vaos[1], vbos[1], ebos[1],sizeof(tableVertices), sizeof(tableIndices), tableVertices, tableIndices);
    configureVertexAttributeObject(vaos[2], vbos[2], ebos[2], sizeof(woodVertices), sizeof(woodIndices), woodVertices, woodIndices); 
    configureVertexAttributeObject(vaos[3], vbos[3], ebos[3], bumperVertices, bumperIndices);
    configureVertexAttributeObject(vaos[4], vbos[4], ebos[4], sizeof(cueVertices), sizeof(cueIndices), cueVertices, cueIndices);
    configureVertexAttributeObject(vaos[5], vbos[5], ebos[5], holeVertices, holeIndices);

    //Locate shader uniforms.
    GLuint mvpLoc = glGetUniformLocation(shaderProgram, "mvp");
    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLuint rotationLoc = glGetUniformLocation(shaderProgram, "rotation");
    GLuint colorLoc = glGetUniformLocation(shaderProgram, "color");
    GLuint lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
    GLuint lightPositionLoc = glGetUniformLocation(shaderProgram, "lightPosition");
    GLuint cameraPositionLoc = glGetUniformLocation(shaderProgram, "cameraPosition");
    GLuint specularCoefficientLoc = glGetUniformLocation(shaderProgram, "specularCoefficient");
  
    //Set shader uniforms. 
    glUniform3f(lightColorLoc, 1.0, 1.0, 1.0);
    glUniform3f(cameraPositionLoc,9.5,5.0,15.0);
    glUniform3f(lightPositionLoc, 9.5,5.0, 5.0);

    //Create transformation matrices. 
    glm::mat4 ballScale = glm::scale(glm::mat4(1.0f),glm::vec3(0.25f,0.25f,0.25f));
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(-9.5f, -5.0f, -15.0f));
    glm::mat4 projection = glm::ortho(-9.5f * 1.2f, 9.5f * 1.2f, -5.0f * 1.2f, 5.0f * 1.2f, 0.1f, 100.0f);
    glm::mat4 tableMVP = projection * view;
    glm::mat4 tableModel = glm::mat4(1.0f);
    glm::mat4 tableRotation = glm::mat4(1.0f);
    glm::mat4 cueScale = glm::scale(glm::mat4(1.0f), glm::vec3(8.0,8.0,8.0));
    glm::mat4 holeScale = glm::scale(glm::mat4(1.0f), glm::vec3(0.35,0.35,0.35));
    glm::mat4 holeHorizontalTrans = glm::translate(glm::mat4(1.0f), glm::vec3( (19.0 + 0.35)/2.0, 0.0,0.0));
    std::vector<glm::mat4> holeModels;
    for(int i = 0; i < 6; ++i)
    {
        holeModels.push_back( glm::translate(glm::mat4(1.0f), glm::vec3( holeLocations[i].x, holeLocations[i].y, 0.01)) * holeScale);
    }

    //Initialize frame varibles.    
    float lastTimeStamp = 0;
    double* xMouse = new double;
    double* yMouse = new double;

    //Initialize game state variables.
    bool gameWon = false;
    int winner = -1;
    int currentPlayer = 1;
    int playerAssignements[] = { -1, -1};
    bool cueBallReset = false;
    bool goodShot = false;
    bool badShot = false;
    bool previousFrameMoving = false;
    bool firstStationaryFrame = true;
    int numberOfBalls[] = {7,7};
    const char title1[] = "PLAYER 1";
    const char title2[] = "PLAYER 2";
    const char gamewin1[] = "PLAYER 1 HAS WON";
    const char gamewin2[] = "PLAYER 2 HAS WON";
    const char* titles[] = {title1,title2};
    const char* gamewins[] = {gamewin1, gamewin2};

   
    while(!glfwWindowShouldClose(window) && !gameWon)
    { 
        if(glfwGetTime() > lastTimeStamp + 0.01 )
	{  
            double time = glfwGetTime() - lastTimeStamp;
            if(poolCue.visible)
            {
                if(poolCue.shooting)
                {
                    poolCue.applyVelocity(time);
                    poolCue.processCollision(balls[0]);
                }
                else
                {
                    glfwGetCursorPos(window, xMouse, yMouse);
                    poolCue.updatePosition( balls[0].Position, TwoDVector(9.5 - 9.5 * 1.2 + *xMouse/1425.0 * 1.2 * 19.0, 5.0 - 5.0* 1.2 + (750 - *yMouse)/750 *1.2 * 10.0));
                }
            }
             
            if(ballMoving(balls))
            {
                for(std::vector<Ball>::iterator iter = balls.begin(); iter != balls.end(); ++iter)
                {
                    iter->applyVelocity(time);
                    iter->processCollisionWithBumpers(bumperEdges);
                } 
                for(std::vector<Ball>::iterator i = balls.begin(); i != balls.end() - 1; ++i)
                {
                    for(std::vector<Ball>::iterator j = i + 1; j != balls.end(); ++j)
                    {
                        processCollision(*i,*j);
                    }
                }
                std::vector<Ball>::iterator iter = balls.begin();
                while( iter != balls.end())
                {
                    if(iter->inPocket(holeLocations, holeRadius))
                    {
                        int ballnum = iter->ballNumber;
                        if( ballnum != 8 && ballnum != 0)
                        {
                            int ballcode = ((ballnum >= 1 && ballnum <= 7) ? 0 : 1);
                            numberOfBalls[ballcode] -= 1;
                            if(playerAssignements[0] == -1)
                            {
                                playerAssignements[currentPlayer] = ballcode;
                                playerAssignements[1-currentPlayer] = 1-ballcode;
                            }
                            if( playerAssignements[currentPlayer] == ballcode)
                            {
                                goodShot = true;
                            }
                            else
                            {
                                badShot = true;
                            } 
                        }
                        if(ballnum == 8)
                        {
                           gameWon = true;
                           if( playerAssignements[currentPlayer] != -1 && numberOfBalls[playerAssignements[currentPlayer]] == 0)
                           {
                               winner = currentPlayer;
                           }
                           else
                           {
                               winner = 1 - currentPlayer;
                           }
                        }
                        if(ballnum == 0)
                        {
                            cueBallReset = true; 
                            badShot = true;
                        }
                        balls.erase(iter);
                    }
                    else { ++iter;}
                } 
            }
            if( previousFrameMoving && !ballMoving(balls))
            {
                firstStationaryFrame = true;
            }
            if(firstStationaryFrame)
            {
                
                if(cueBallReset)
                {
                    balls.insert( balls.begin(),Ball( TwoDVector(0.0,0.0), TwoDVector(5.0,5.0), 0,1.0,0.9,0.8));
                    cueBallReset = false;
                }
                if(!poolCue.visible)
                {
                    poolCue.visible = true;
                    poolCue.velocity = 0.0;
                    poolCue.distanceToCue = 5.0;
                }
                if( badShot || !goodShot)
                {
                    currentPlayer = 1 - currentPlayer;
                }
                glfwSetWindowTitle(window, titles[currentPlayer]);
                goodShot = false;
                badShot = false;
                firstStationaryFrame = false;
            }
            previousFrameMoving = ballMoving(balls);
            lastTimeStamp = glfwGetTime();
            
            //Prepare window to render.
            glUseProgram(shaderProgram);
            glClearColor(0.0f,0.0f,0.0f, 1.0);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

            //Draw table.
            glUniform3f(colorLoc, 0.12,0.31,0.16);   
            glUniformMatrix4fv(mvpLoc, 1 , GL_FALSE, glm::value_ptr(tableMVP));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(tableModel));
            glUniformMatrix4fv(rotationLoc, 1, GL_FALSE, glm::value_ptr(tableRotation));
            glUniform1f(specularCoefficientLoc, 0.0f);
            glBindTexture(GL_TEXTURE_2D, textures[16]);
            glBindVertexArray(vaos[1]);
            glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);

            //Draw wood.
            glUniform3f(colorLoc, 0.15, 0.05,0.0);
            glBindTexture(GL_TEXTURE_2D, textures[18]);
            glBindVertexArray(vaos[2]);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            //Draw bumpers.
            glUniform3f(colorLoc, 1.0,0.0,0.0);
            glBindVertexArray(vaos[3]);
            glUniform3f(colorLoc, 0.08,0.2,0.11);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

            //Draw holes.
            glBindVertexArray(vaos[5]);
            glUniform3f(colorLoc, 0.05,0.05,0.05);
            for( std::vector<glm::mat4>::iterator iter = holeModels.begin(); iter != holeModels.end(); ++iter)
            {
                glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(projection * view * (*iter)));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr((*iter)));
                glDrawElements(GL_TRIANGLES, holeIndices.size(), GL_UNSIGNED_INT, 0);
            }

            //Draw cue.
            if(poolCue.visible)
            {
                glm::vec3 cuePosition = glm::vec3( balls[0].Position.x - std::cos(poolCue.angle)*(poolCue.distanceToCue), balls[0].Position.y - std::sin(poolCue.angle) * (poolCue.distanceToCue), 0.25);
                glm::mat4 cueTranslation = glm::translate(glm::mat4(1.0f), cuePosition);
                glm::mat4 cueRotation = glm::rotate(glm::mat4(1.0f),poolCue.angle, glm::vec3(0.0,0.0,1.0));
                glm::mat4 cueModel = cueTranslation * cueRotation * cueScale;
                glm::mat4 cueMVP = projection * view * cueModel;
                glUniform3f(colorLoc, 0.4,0.0,0.0);
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(cueModel));
                glUniformMatrix4fv(rotationLoc, 1, GL_FALSE, glm::value_ptr(cueRotation));
                glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(cueMVP));
                glBindTexture(GL_TEXTURE_2D, textures[17]);
                glBindVertexArray(vaos[4]);
                glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
            }
            
            //Draw balls.
            for(std::vector<Ball>::iterator iter = balls.begin(); iter != balls.end(); ++iter)
            {   
                glm::mat4 ballTranslation = glm::translate(glm::mat4(1.0f), glm::vec3(iter->Position.x, iter->Position.y, 0.25f));
                glm::mat4 ballRotation = glm::mat4_cast(iter->rotation);
                glm::mat4 ballModel = ballTranslation * ballScale * ballRotation ;
                glm::mat4 ballMVP = projection * view * ballModel;
                glUniformMatrix4fv(rotationLoc, 1, GL_FALSE, glm::value_ptr(ballRotation));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(ballModel));
                glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(ballMVP));
                glUniform3f(colorLoc, iter->red, iter->green,iter->blue);
                glUniform1f(specularCoefficientLoc, 0.7f);
                glBindTexture(GL_TEXTURE_2D, textures[iter->ballNumber]);
                glBindVertexArray(vaos[0]);
                glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);
            }
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
        if(gameWon)
        {
            while(glfwGetTime() - lastTimeStamp < 2.0)
            {}
        }
        
    }
    if(gameWon)
    {
        std::cout<< gamewins[winner] << std::endl;
    }
    //Free array and buffer objects.
    glDeleteVertexArrays(6,vaos);
    glDeleteBuffers(6, vbos);
    glDeleteBuffers(6, ebos);

    glfwTerminate();
    return 0;
}
