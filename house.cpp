// CS370 Final Project
// Fall 2023

#define STB_IMAGE_IMPLEMENTATION
#include "../common/stb_image.h"	// Sean Barrett's image loader - http://nothings.org/
#include <stdio.h>
#include <vector>
#include "../common/vgl.h"
#include "../common/objloader.h"
#include "../common/utils.h"
#include "../common/vmath.h"
#include "lighting.h"
#define DEG2RAD (M_PI/180.0)

using namespace vmath;
using namespace std;

// Vertex array and buffer names
enum VAO_IDs {Cube, Octahedron, Sphere, Cylinder, NumVAOs};
enum ObjBuffer_IDs {PosBuffer, NormBuffer, TexBuffer, NumObjBuffers};
enum Color_Buffer_IDs {RedCube, WhiteCube, BlueOcta, GreenSphere, SomethingCylinder, NumColorBuffers};
enum LightBuffer_IDs {LightBuffer, NumLightBuffers};
enum MaterialBuffer_IDs {MaterialBuffer, NumMaterialBuffers};
enum MaterialNames {RedPlastic, GreenPlastic, BluePlastic, WhitePlastic};
enum Textures {Blank, NumTextures};

// Vertex array and buffer objects
GLuint VAOs[NumVAOs];
GLuint ObjBuffers[NumVAOs][NumObjBuffers];
GLuint ColorBuffers[NumColorBuffers];
GLuint LightBuffers[NumLightBuffers];
GLuint MaterialBuffers[NumMaterialBuffers];
GLuint TextureIDs[NumTextures];

// Number of vertices in each object
GLint numVertices[NumVAOs];

// Number of component coordinates
GLint posCoords = 4;
GLint normCoords = 3;
GLint texCoords = 2;
GLint colCoords = 4;

// Model files
const char * cubeFile = "../models/unitcube.obj";
const char * octaFile = "../models/octahedron.obj";
const char * sphereFile = "../models/sphere.obj";
const char * cylinderFile = "../models/cylinder.obj";

// Texture files
const char * blankFile = "../textures/blank.png";

// Camera
vec3 eye = {3.0f, 0.0f, 0.0f};
vec3 center = {0.0f, 0.0f, 0.0f};
vec3 up = {0.0f, 1.0f, 0.0f};
GLfloat azimuth = 0.0f;
GLfloat daz = 2.0f;
GLfloat elevation = 90.0f;
GLfloat del = 2.0f;
GLfloat radius = 2.0f;
GLfloat dr = 0.1f;

// Shader variables
// Default (color) shader program references
GLuint default_program;
GLuint default_vPos;
GLuint default_vCol;
GLuint default_proj_mat_loc;
GLuint default_cam_mat_loc;
GLuint default_model_mat_loc;
const char *default_vertex_shader = "../default.vert";
const char *default_frag_shader = "../default.frag";

// Lighting shader program reference
GLuint lighting_program;
GLuint lighting_vPos;
GLuint lighting_vNorm;
GLuint lighting_camera_mat_loc;
GLuint lighting_model_mat_loc;
GLuint lighting_proj_mat_loc;
GLuint lighting_norm_mat_loc;
GLuint lighting_lights_block_idx;
GLuint lighting_materials_block_idx;
GLuint lighting_material_loc;
GLuint lighting_num_lights_loc;
GLuint lighting_light_on_loc;
GLuint lighting_eye_loc;
const char *lighting_vertex_shader = "../lighting.vert";
const char *lighting_frag_shader = "../lighting.frag";

// Texture shader program reference
GLuint texture_program;
GLuint texture_vPos;
GLuint texture_vTex;
GLuint texture_proj_mat_loc;
GLuint texture_camera_mat_loc;
GLuint texture_model_mat_loc;
const char *texture_vertex_shader = "../texture.vert";
const char *texture_frag_shader = "../texture.frag";

// Global state
mat4 proj_matrix;
mat4 camera_matrix;
mat4 normal_matrix;
mat4 model_matrix;

vector<LightProperties> Lights;
vector<MaterialProperties> Materials;
GLuint numLights = 0;
GLint lightOn[8] = {0, 0, 0, 0, 0, 0, 0, 0};

// Global screen dimensions
GLint ww,hh;

// My constants
mat4 wall_scale_matrix = scale(7.025f, 2.0f, 1.0f);
mat4 obj_scale_matrix = scale(0.5f, 0.5f, 0.5f);
vec3 x_axis = { 1.0f, 0.0f, 0.0f };
vec3 y_axis = { 0.0f, 1.0f, 0.0f };
vec3 z_axis = { 0.0f, 0.0f, 1.0f };
int spotlight_index = 0;

void display();
void render_scene();
void build_geometry();
void build_solid_color_buffer(GLuint num_vertices, vec4 color, GLuint buffer);
void build_materials( );
void build_lights( );
void build_textures();
void load_model(const char * filename, GLuint obj);
void load_texture(const char * filename, GLuint texID, GLint magFilter, GLint minFilter, GLint sWrap, GLint tWrap, bool mipMap, bool invert);
void draw_color_obj(GLuint obj, GLuint color);
void draw_mat_object(GLuint obj, GLuint material);
void draw_tex_object(GLuint obj, GLuint texture);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow *window, int button, int action, int mods);

int main(int argc, char**argv)
{
	// Create OpenGL window
	GLFWwindow* window = CreateWindow("Think Inside The Box");
    if (!window) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return 1;
    } else {
        printf("OpenGL window successfully created\n");
    }

    // Store initial window size
    glfwGetFramebufferSize(window, &ww, &hh);

    // Register callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window,key_callback);
    glfwSetMouseButtonCallback(window, mouse_callback);

    // Load shaders and associate variables
    ShaderInfo default_shaders[] = { {GL_VERTEX_SHADER, default_vertex_shader},{GL_FRAGMENT_SHADER, default_frag_shader},{GL_NONE, NULL} };
    default_program = LoadShaders(default_shaders);
    default_vPos = glGetAttribLocation(default_program, "vPosition");
    default_vCol = glGetAttribLocation(default_program, "vColor");
    default_proj_mat_loc = glGetUniformLocation(default_program, "proj_matrix");
    default_cam_mat_loc = glGetUniformLocation(default_program, "camera_matrix");
    default_model_mat_loc = glGetUniformLocation(default_program, "model_matrix");

    // Load shaders
    // Load light shader
    ShaderInfo lighting_shaders[] = { {GL_VERTEX_SHADER, lighting_vertex_shader},{GL_FRAGMENT_SHADER, lighting_frag_shader},{GL_NONE, NULL} };
    lighting_program = LoadShaders(lighting_shaders);
    lighting_vPos = glGetAttribLocation(lighting_program, "vPosition");
    lighting_vNorm = glGetAttribLocation(lighting_program, "vNormal");
    lighting_proj_mat_loc = glGetUniformLocation(lighting_program, "proj_matrix");
    lighting_camera_mat_loc = glGetUniformLocation(lighting_program, "camera_matrix");
    lighting_norm_mat_loc = glGetUniformLocation(lighting_program, "normal_matrix");
    lighting_model_mat_loc = glGetUniformLocation(lighting_program, "model_matrix");
    lighting_lights_block_idx = glGetUniformBlockIndex(lighting_program, "LightBuffer");
    lighting_materials_block_idx = glGetUniformBlockIndex(lighting_program, "MaterialBuffer");
    lighting_material_loc = glGetUniformLocation(lighting_program, "Material");
    lighting_num_lights_loc = glGetUniformLocation(lighting_program, "NumLights");
    lighting_light_on_loc = glGetUniformLocation(lighting_program, "LightOn");
    lighting_eye_loc = glGetUniformLocation(lighting_program, "EyePosition");

    // Load texture shaders
    ShaderInfo texture_shaders[] = { {GL_VERTEX_SHADER, texture_vertex_shader},{GL_FRAGMENT_SHADER, texture_frag_shader},{GL_NONE, NULL} };
    texture_program = LoadShaders(texture_shaders);
    texture_vPos = glGetAttribLocation(texture_program, "vPosition");
    texture_vTex = glGetAttribLocation(texture_program, "vTexCoord");
    texture_proj_mat_loc = glGetUniformLocation(texture_program, "proj_matrix");
    texture_camera_mat_loc = glGetUniformLocation(texture_program, "camera_matrix");
    texture_model_mat_loc = glGetUniformLocation(texture_program, "model_matrix");

    // Create geometry buffers
    build_geometry();
    // Create material buffers
    build_materials();
    // Create light buffers
    build_lights();
    // Create textures
    build_textures();

    // Enable depth test
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    // Set Initial camera position
    GLfloat x, y, z;
    x = (GLfloat)(radius*sin(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
    y = (GLfloat)(radius*cos(elevation*DEG2RAD));
    z = (GLfloat)(radius*cos(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
    eye = vec3(x, y, z);

    // Start loop
    while ( !glfwWindowShouldClose( window ) ) {
    	// Draw graphics
        display();
        // Update other events like input handling
        glfwPollEvents();
        // Swap buffer onto screen
        glfwSwapBuffers( window );
    }

    // Close window
    glfwTerminate();
    return 0;

}

void display( )
{
    // Declare projection and camera matrices
    proj_matrix = mat4().identity();
    camera_matrix = mat4().identity();

	// Clear window and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Compute anisotropic scaling
    GLfloat xratio = 1.0f;
    GLfloat yratio = 1.0f;
    // If taller than wide adjust y
    if (ww <= hh)
    {
        yratio = (GLfloat)hh / (GLfloat)ww;
    }
        // If wider than tall adjust x
    else if (hh <= ww)
    {
        xratio = (GLfloat)ww / (GLfloat)hh;
    }

    // DEFAULT ORTHOGRAPHIC PROJECTION
    proj_matrix = ortho(-5.0f*xratio, 5.0f*xratio, -5.0f*yratio, 5.0f*yratio, -5.0f, 5.0f);

    // Set camera matrix
    camera_matrix = lookat(eye, center, up);

    // Render objects
	render_scene();

	// Flush pipeline
	glFlush();
}

void render_scene( ) {
    // Declare transformation matrices
    model_matrix = mat4().identity();
    mat4 scale_matrix = mat4().identity();
    mat4 rot_matrix = mat4().identity();
    mat4 trans_matrix = mat4().identity();

    // Set cube transformation matrix
    trans_matrix = translate(0.0f, 0.0f, 4.0f);
    rot_matrix = rotate(0.0f, z_axis);
    scale_matrix = wall_scale_matrix;
	model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    // Draw cube
    draw_mat_object(Cube, RedPlastic);
    //draw_color_obj(Cube, RedCube);

    trans_matrix = translate(4.0f, 0.0f, 0.0f);
    rot_matrix = rotate(90.0f, y_axis);
    scale_matrix = wall_scale_matrix;
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    // Draw cube
    draw_mat_object(Cube, RedPlastic);
    //draw_color_obj(Cube, RedCube);

    trans_matrix = translate(0.0f, 0.0f, -4.0f);
    rot_matrix = rotate(0.0f, z_axis);
    scale_matrix = wall_scale_matrix;
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    // Draw cube
    draw_mat_object(Cube, RedPlastic);
    //draw_color_obj(Cube, RedCube);

    trans_matrix = translate(-4.0f, 0.0f, 0.0f);
    rot_matrix = rotate(90.0f, y_axis);
    scale_matrix = wall_scale_matrix;
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    // Draw cube
    draw_mat_object(Cube, RedPlastic);
    //draw_color_obj(Cube, RedCube);

    // Draw floor
    trans_matrix = translate(0.0f, -1.0f, 0.0f);
    scale_matrix = scale(7.0f, 0.1f, 7.0f);
    model_matrix = trans_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    // Draw cube
    draw_mat_object(Cube, WhitePlastic);
    //draw_color_obj(Cube, WhiteCube);

    trans_matrix = translate(-1.0f, 0.0f, 0.0f);
    rot_matrix = rotate(90.0f, z_axis);
    scale_matrix = obj_scale_matrix;
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    // Draw octahedron
    draw_mat_object(Octahedron, RedPlastic);
    //draw_color_obj(Octahedron, BlueOcta);

    trans_matrix = translate(-1.0f, 0.0f, 2.0f);
    rot_matrix = rotate(90.0f, z_axis);
    scale_matrix = obj_scale_matrix;
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    // Draw sphere
    draw_mat_object(Sphere, GreenPlastic);
    //draw_color_obj(Sphere, GreenSphere);

    trans_matrix = translate(0.0f, 0.0f, -1.0f);
    rot_matrix = rotate(90.0f, z_axis);
    scale_matrix = obj_scale_matrix;
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    // Draw cylinder
    draw_mat_object(Cylinder, BluePlastic);
    //draw_color_obj(Cylinder, SomethingCylinder);
}

void build_geometry( )
{
    // Generate vertex arrays and buffers
    glGenVertexArrays(NumVAOs, VAOs);

    // Load models
    load_model(cubeFile, Cube);
    load_model(octaFile, Octahedron);
    load_model(sphereFile, Sphere);
    load_model(cylinderFile, Cylinder);

    // Generate color buffers
    glGenBuffers(NumColorBuffers, ColorBuffers);

    // Build color buffers
    // Define cube vertex colors (red)
    build_solid_color_buffer(numVertices[Cube], vec4(1.0f, 0.0f, 0.0f, 1.0f), RedCube);
    build_solid_color_buffer(numVertices[Cube], vec4(1.0f, 1.0f, 1.0f, 1.0f), WhiteCube);
    build_solid_color_buffer(numVertices[Octahedron], vec4(0.0f, 0.0f, 1.0f, 1.0f), BlueOcta);
    build_solid_color_buffer(numVertices[Sphere], vec4(0.0f, 1.0f, 0.0f, 1.0f), GreenSphere);
    build_solid_color_buffer(numVertices[Cylinder], vec4(1.0f, 1.0f, 0.0f, 1.0f), SomethingCylinder);
}

void build_materials( ) {
    // Add materials to Materials vector
    MaterialProperties redPlastic = {
            vec4(0.3f, 0.0f, 0.0f, 1.0f), //ambient
            vec4(0.6f, 0.0f, 0.0f, 1.0f), //diffuse
            vec4(0.8f, 0.6f, 0.6f, 1.0f), //specular
            32.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };

    MaterialProperties greenPlastic = {
            vec4(0.0f, 0.3f, 0.0f, 1.0f), //ambient
            vec4(0.0f, 0.6f, 0.0f, 1.0f), //diffuse
            vec4(0.6f, 0.8f, 0.6f, 1.0f), //specular
            32.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };

    MaterialProperties bluePlastic = {
            vec4(0.0f, 0.0f, 0.3f, 1.0f), //ambient
            vec4(0.0f, 0.0f, 0.6f, 1.0f), //diffuse
            vec4(0.6f, 0.6f, 0.8f, 1.0f), //specular
            32.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };

    MaterialProperties whitePlastic = {
            vec4(0.3f, 0.3f, 0.3f, 1.0f), //ambient
            vec4(0.6f, 0.6f, 0.6f, 1.0f), //diffuse
            vec4(0.8f, 0.8f, 0.8f, 1.0f), //specular
            32.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };

    Materials.push_back(redPlastic);
    Materials.push_back(greenPlastic);
    Materials.push_back(bluePlastic);
    Materials.push_back(whitePlastic);

    glGenBuffers(NumMaterialBuffers, MaterialBuffers);
    glBindBuffer(GL_UNIFORM_BUFFER, MaterialBuffers[MaterialBuffer]);
    glBufferData(GL_UNIFORM_BUFFER, Materials.size()*sizeof(MaterialProperties), Materials.data(), GL_STATIC_DRAW);
}

void build_lights( ) {
    // Add lights to Lights vector
    LightProperties whitePointLight = {
            POINT, //type
            {0.0f, 0.0f, 0.0f}, //pad
            vec4(0.0f, 0.0f, 0.0f, 1.0f), //ambient
            vec4(1.0f, 1.0f, 1.0f, 1.0f), //diffuse
            vec4(1.0f, 1.0f, 1.0f, 1.0f), //specular
            vec4(3.0f, 4.0f, 0.0f, 1.0f),  //position
            vec4(0.0f, 0.0f, 0.0f, 0.0f), //direction
            0.0f,   //cutoff
            0.0f,  //exponent
            {0.0f, 0.0f}  //pad2
    };

    Lights.push_back(whitePointLight);

    // Set numLights
    numLights = Lights.size();

    // Turn all lights on
    for (int i = 0; i < numLights; i++) {
        lightOn[i] = 1;
    }

    // Create uniform buffer for lights
    glGenBuffers(NumLightBuffers, LightBuffers);
    glBindBuffer(GL_UNIFORM_BUFFER, LightBuffers[LightBuffer]);
    glBufferData(GL_UNIFORM_BUFFER, Lights.size()*sizeof(LightProperties), Lights.data(), GL_STATIC_DRAW);
}

void build_textures( ) {

    // Create textures and activate unit 0
    glGenTextures( NumTextures,  TextureIDs);
    glActiveTexture( GL_TEXTURE0 );

    load_texture(blankFile, Blank, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT, GL_REPEAT, true, false);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    // ESC to quit
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, true);
    }

    if (key == GLFW_KEY_0 && action == GLFW_RELEASE) {
        lightOn[spotlight_index] = (lightOn[spotlight_index]+1)%2;
    }

    // Adjust azimuth
    if (key == GLFW_KEY_A) {
        azimuth += daz;
        if (azimuth > 360.0) {
            azimuth -= 360.0;
        }
    } else if (key == GLFW_KEY_D) {
        azimuth -= daz;
        if (azimuth < 0.0)
        {
            azimuth += 360.0;
        }
    }

    // Adjust elevation angle
    if (key == GLFW_KEY_W)
    {
        elevation += del;
        if (elevation > 179.0)
        {
            elevation = 179.0;
        }
    }
    else if (key == GLFW_KEY_S)
    {
        elevation -= del;
        if (elevation < 1.0)
        {
            elevation = 1.0;
        }
    }

    // Compute updated camera position
    GLfloat x, y, z;
    x = (GLfloat)(radius*sin(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
    y = (GLfloat)(radius*cos(elevation*DEG2RAD));
    z = (GLfloat)(radius*cos(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
    eye = vec3(x,y,z);

}

void mouse_callback(GLFWwindow *window, int button, int action, int mods){

}

#include "utilfuncs.cpp"
