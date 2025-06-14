/* Ceasar Moya-Cobian | Student ID: 300274872 | CSCI 173 | Assignment 4 - Model Loader with VBO */
// Note: Current implementation assumes .obj file using triangles and does not have vt values
#include <string.h>
#define GLEW_STATIC
#include <GL/glew.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <SOIL.h>

#define PI 3.14159

using namespace std;

bool WireFrame= false;
float i =0;
const GLfloat light_ambient[]  = { 0.0f, 0.0f, 0.0f, 1.0f };
const GLfloat light_diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_position[] = { 2.0f, 5.0f, 5.0f, 0.0f };

const GLfloat mat_ambient[]    = { 0.7f, 0.7f, 0.7f, 1.0f };
const GLfloat mat_diffuse[]    = { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat mat_specular[]   = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat high_shininess[] = { 100.0f };

    float xpos =0;
    float ypos =0;
    float Wwidth,Wheight;

    float rotateX, rotateY;
    float eyeZ;

struct Model{   // each object will use this struct to contain its own information
    vector<GLfloat> coords;
    vector<GLfloat> textures;
    vector<GLfloat> normals;
    vector<GLuint> faces;
    vector<GLuint> indices;
    vector<GLfloat> interleaved;    // vector structure {v, v, v, vt, vt, vn, vn, vn} per vertex
    GLuint vbo, ebo;
    GLfloat scale;
    void interleaveData();
};

void Model::interleaveData(){
    for(int i = 0; i < faces.size(); i+=2){
        GLfloat vertexX;
        GLfloat vertexY;
        GLfloat vertexZ;
        GLfloat textureX;
        GLfloat textureY;
        GLfloat normalX;
        GLfloat normalY;
        GLfloat normalZ;
        vertexX = coords[faces[i] * 3];
        vertexY = coords[faces[i] * 3 + 1];
        vertexZ = coords[faces[i] * 3 + 2];
        textureX = textures[faces[i] * 2];
        textureY = textures[faces[i] * 2 + 1];
        normalX = normals[faces[i+1] * 3];
        normalY = normals[faces[i+1] * 3 + 1];
        normalZ = normals[faces[i+1] * 3 + 2];
        interleaved.push_back(vertexX);
        interleaved.push_back(vertexY);
        interleaved.push_back(vertexZ);
        interleaved.push_back(textureX);
        interleaved.push_back(textureY);
        interleaved.push_back(normalX);
        interleaved.push_back(normalY);
        interleaved.push_back(normalZ);

        indices.push_back(indices.size());
    }
}

Model model1, model2, model3;
Model* activeModel = &model1;   // pointer that will point to the model that should be displayed


string obj1 = "models/ateneam.obj";     // no vt
string obj2 = "models/elephal.obj";     // no vt
string obj3 = "models/leaves.obj";      // no vt

GLuint tex;

// Variables below are for scaling the model to fit the view port
float xMin, xMax;
float yMin, yMax;
float zMin, zMax;


// Variables below are for quaternion rotation
    GLfloat matrixX[16];    // all of these matrices are 4x4
    GLfloat matrixY[16];
    GLfloat matrixZ[16];

    float x, y, z, w;


void createVBO(Model &model){
    glGenBuffers(1, &model.vbo);    // create VBO for vertex data
    glGenBuffers(1, &model.ebo);    // create EBO for indices

    size_t interleavedSize = model.interleaved.size() * sizeof(GLfloat);    // total size of interleaved data
    size_t indicesSize = model.indices.size() * sizeof(GLuint);             // total size of indices

    glBindBuffer(GL_ARRAY_BUFFER, model.vbo);
    glBufferData(GL_ARRAY_BUFFER, interleavedSize, &model.interleaved[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0); // resetting buffer

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize, &model.indices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // resetting buffer
}


void drawVBO(Model &model){
    glBindBuffer(GL_ARRAY_BUFFER, model.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.ebo);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    GLsizei stride = 8 * sizeof(GLfloat);   // each vertex has 8 float values (3 vertex coordinates + 2 vertex textures + 3 vertex normals)

    glVertexPointer(3, GL_FLOAT, stride, (void*)(0));                     // offset 0
    glTexCoordPointer(2, GL_FLOAT, stride, (void*)(3 * sizeof(GLfloat))); // offset after 3 floats
    glNormalPointer(GL_FLOAT, stride, (void*)(5 * sizeof(GLfloat)));      // offset after 5 floats

    glDrawElements(GL_TRIANGLES, model.indices.size(), GL_UNSIGNED_INT, (void*)0);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


void getMax(Model &model){
    xMin = model.coords[0];
    xMax = model.coords[0];
    yMin = model.coords[1];
    yMax = model.coords[1];
    zMin = model.coords[2];
    zMax = model.coords[2];
    for(int i = 0; i < model.coords.size()/3; i++){
        if(xMin > model.coords[i*3]){
            xMin = model.coords[i*3];
        }
        else if(xMax < model.coords[i*3]){
            xMax = model.coords[i*3];
        }
        if(yMin > model.coords[i*3+1]){
            yMin = model.coords[i*3+1];
        }
        else if(yMax < model.coords[i*3+1]){
            yMax = model.coords[i*3+1];
        }
        if(zMin > model.coords[i*3+2]){
            zMin = model.coords[i*3+2];
        }
        else if(zMax < model.coords[i*3+2]){
            zMax = model.coords[i*3+2];
        }
    }
    double temp = pow(xMax-xMin, 2) + pow(yMax-yMin, 2) + pow(zMax-zMin, 2);
    double d = sqrt(temp);
    model.scale = 5/d;
}


void readObjFile(string fileName, Model &model){
    bool texturesPresent = false;   // bool to determine whether obj has VT
    ifstream iFile(fileName);
    string currLine;
    if(iFile.is_open()){
        cout << "File "<< fileName << " was opened" << endl;
        while(getline(iFile, currLine)){
            istringstream singleLine(currLine);
            string check;
            singleLine >> check;
            if(check == "v"){
                GLfloat vertX;
                GLfloat vertY;
                GLfloat vertZ;
                singleLine >> vertX >> vertY >> vertZ;
                model.coords.push_back(vertX);
                model.coords.push_back(vertY);
                model.coords.push_back(vertZ);
            }
            else if(check == "vt"){
                texturesPresent = true;
                GLfloat texX;
                GLfloat texY;
                singleLine >> texX >> texY;
                model.textures.push_back(texX);
                model.textures.push_back(texY);
            }
            else if(check == "vn"){
                GLfloat normalX;
                GLfloat normalY;
                GLfloat normalZ;
                singleLine >> normalX >> normalY >> normalZ;
                model.normals.push_back(normalX);
                model.normals.push_back(normalY);
                model.normals.push_back(normalZ);
            }
            else if(check == "f"){
                if(texturesPresent){
                    // we know vt exists but not sure if quads or triangles
                    char slash = '/';
                    int counting = count(currLine.begin(), currLine.end(), slash);
                    if(counting == 6){
                        // we established it is triangles
                        for(int i = 0; i < 3; i++){
                            GLuint f1;
                            GLuint f2;
                            GLuint f3;
                            char placeholder;
                            singleLine >> f1 >> placeholder >> f2 >> placeholder >> f3;
                            // NEED TO ACCOUNT FOR FACE INDICES STARTING AT 1 WHILE ARRAY/VECTOR INDICES START AT 0
                            model.faces.push_back(f1-1);
                            model.faces.push_back(f2-1);
                            model.faces.push_back(f3-1);
                        }
                    }
                    else{
                        // we established it is quads
                        for(int i = 0; i < 4; i++){
                            GLuint f1;
                            GLuint f2;
                            GLuint f3;
                            char placeholder;
                            singleLine >> f1 >> placeholder >> f2 >> placeholder >> f3;
                            // NEED TO ACCOUNT FOR FACE INDICES STARTING AT 1 WHILE ARRAY/VECTOR INDICES START AT 0
                            model.faces.push_back(f1-1);
                            model.faces.push_back(f2-1);
                            model.faces.push_back(f3-1);
                        }
                    }
                }
                else{
                    // we know vt does NOT exist, but not sure if quads or triangles
                    char slash = '/';
                    int counting = count(currLine.begin(), currLine.end(), slash);
                    if(counting == 6){
                        // we established that it is triangles
                        for(int i = 0; i < 3; i++){
                            GLuint f1;
                            GLuint f2;
                            char placeholder;
                            singleLine >> f1 >> placeholder >> placeholder >> f2;
                            // NEED TO ACCOUNT FOR FACE INDICES STARTING AT 1 WHILE ARRAY/VECTOR INDICES START AT 0
                            model.faces.push_back(f1-1);
                            model.faces.push_back(f2-1);
                        }
                    }
                    else{
                        // we established it is quads
                        for(int i = 0; i < 4; i++){
                            GLuint f1;
                            GLuint f2;
                            char placeholder;
                            singleLine >> f1 >> placeholder >> placeholder >> f2;
                            // NEED TO ACCOUNT FOR FACE INDICES STARTING AT 1 WHILE ARRAY/VECTOR INDICES START AT 0
                            model.faces.push_back(f1-1);
                            model.faces.push_back(f2-1);
                        }
                    }
                }
                //cout << endl;
            }
        }
        iFile.close();
        cout << "Closed .obj file" << endl;
        getMax(model);

        // Generate planar UVs using X and Y
        for (int i = 0; i < model.coords.size(); i += 3) {
            float x = model.coords[i];
            float y = model.coords[i + 1];

            float u = (x - xMin) / (xMax - xMin);
            float v = (y - yMin) / (yMax - yMin);

            model.textures.push_back(u);
            model.textures.push_back(v);
        }

        cout << "This is how many v: " << model.coords.size() / 3 << endl;
        cout << "This is how many vt: " << model.textures.size() / 2 << endl;
        cout << "This is how many vn: " << model.normals.size() / 3 << endl;
        cout << "This is how many f: " << model.faces.size() / 6 << endl;       // expects f values as v//vn v//vn v//vn
    }

    else{
        cout << "Couldn't open file" << endl;
    }
}

/* GLUT callback Handlers */

static void resize(int width, int height)
{
     double Ratio;

     Wwidth = (float)width;
     Wheight = (float)height;

     Ratio= (double)width /(double)height;

    glViewport(0,0,(GLsizei) width,(GLsizei) height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

	gluPerspective (45.0f,Ratio,0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

 }

void textureLoader(char* fileName, GLuint &handle){
    int width, height;
    unsigned char *image;
    glBindTexture(GL_TEXTURE_2D, handle);   // activate handle to the texture space
    image = SOIL_load_image(fileName, &width, &height, 0, SOIL_LOAD_RGBA);

    if(!image){
        cout << "Image file not found" << endl;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    SOIL_free_image_data(image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
 }

void createFromAxis(GLfloat X, GLfloat Y, GLfloat Z, GLfloat degree){
    GLfloat angle = (degree/180) * PI;

    GLfloat result = sin(angle/2.0);
    w = cos(angle/2.0);
    x = X * result;
    y = Y * result;
    z = Z * result;
}

void createMatrix(GLfloat *matrixQ){
    matrixQ[0] = 1 - 2*(y*y + z*z);
    matrixQ[1] = 2*(x*y + z*w);
    matrixQ[2] = 2*(x*z - y*w);
    matrixQ[3] = 0.0;

    matrixQ[4] = 2*(x*y - z*w);
    matrixQ[5] = 1 - 2*(x*x + z*z);
    matrixQ[6] = 2*(z*y + x*w);
    matrixQ[7] = 0.0;

    matrixQ[8] = 2*(x*z + y*w);
    matrixQ[9] = 2*(y*z - x*w);
    matrixQ[10] = 1 - 2*(x*x + y*y);
    matrixQ[11] = 0.0;

    matrixQ[12] = 0.0;
    matrixQ[13] = 0.0;
    matrixQ[14] = 0.0;
    matrixQ[15] = 1.0;

}

static void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(0,0,eyeZ,0.0,0.0,0.0,0.0,1.0,0.0);

    if(WireFrame)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);		//Draw Our Mesh In Wireframe Mesh
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);		//Toggle WIRE FRAME

    // your code here
    createFromAxis(1, 0, 0, rotateX);
    createMatrix(matrixX);

    createFromAxis(0, 1, 0, rotateY);
    createMatrix(matrixY);

    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, tex);
    glTranslatef(0, -1, 2);
    glScalef(activeModel->scale, activeModel->scale, activeModel->scale);
    glMultMatrixf(matrixX);
    glMultMatrixf(matrixY);
    drawVBO(*activeModel);
    glPopMatrix();

    glutSwapBuffers();
}


static void key(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 27 :
        case 'q':
            exit(0);
            break;

	  case 'w':
		WireFrame =!WireFrame;
	       break;

      case '1':
        activeModel = &model1;
        break;

      case '2':
        activeModel = &model2;
        break;

      case '3':
        activeModel = &model3;
        break;
    }
}

void Specialkeys(int key, int x, int y)
{
    switch(key)
    {
    case GLUT_KEY_LEFT:
            rotateY -= 10.0;
            break;

        case GLUT_KEY_RIGHT:
            rotateY += 10.0;
            break;

        case GLUT_KEY_UP:
            rotateX -= 10.0;
            break;

        case GLUT_KEY_DOWN:
            rotateX += 10.0;
            break;

        case GLUT_KEY_END:
            if(eyeZ > 1){
                eyeZ -= 1;
            }
            break;

        case GLUT_KEY_HOME:
            if(eyeZ < 70){
                eyeZ += 1;
            }
            break;
   }
  glutPostRedisplay();
}


static void idle(void)
{
    // Use parametric equation with t increment for xpos and y pos
    // Don't need a loop
    glutPostRedisplay();
}



void mouse(int btn, int state, int x, int y){

    float scale = 100*(Wwidth/Wheight);

    switch(btn){
        case GLUT_LEFT_BUTTON:

        if(state==GLUT_DOWN){

               // get new mouse coordinates for x,y
               // use scale to match right
            }
            break;
    }
     glutPostRedisplay();
};



static void init(void)
{
    glewInit();
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);

    glClearColor(0.5f, 0.5f, 1.0f, 0.0f);                 // assign a color you like

    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);

    glEnable(GL_DEPTH_TEST);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glShadeModel(GL_SMOOTH);

    glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);

    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_LIGHTING);

    glGenTextures(1, &tex);
    glEnable(GL_TEXTURE_2D);
    textureLoader("images/texture3.jpg", tex);

    rotateX = 0;
    rotateY = 0;
    eyeZ = 10;

    readObjFile(obj1, model1);
    model1.interleaveData();
    createVBO(model1);

    readObjFile(obj2, model2);
    model2.interleaveData();
    createVBO(model2);

    readObjFile(obj3, model3);
    model3.interleaveData();
    createVBO(model3);

}

/* Program entry point */

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);

    glutInitWindowSize(800,600);
    glutInitWindowPosition(0,0);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

    glutCreateWindow("Assignment 4 - Model Loader with VBO");
    init();
    glutReshapeFunc(resize);
    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutKeyboardFunc(key);
    glutSpecialFunc(Specialkeys);

    glutIdleFunc(idle);

    glutMainLoop();

    return EXIT_SUCCESS;
}
