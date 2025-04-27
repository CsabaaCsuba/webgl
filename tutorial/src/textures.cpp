#include<iostream>
#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <cmath>
#include "ShaderClass.hpp"
#include "VAO.hpp"
#include "EBO.hpp"
#include "Texture.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <../include/stb_image.h>

VAO* VAO1;
Shader* shaderProgram;
Texture* popCat;
GLuint uniformID;

void MainLoop(){
    glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
    // Clean the back buffer and assign the new color to it
    glClear(GL_COLOR_BUFFER_BIT);
    // Tell OpenGL which Shader Program we want to use
    shaderProgram->Activate();

    glUniform1f(uniformID, 0.5f);

    popCat->Bind();
    // Bind the VAO so OpenGL knows to use it
    VAO1->Bind();
    // Draw primitives, number of indices, datatype of indices, index of indices
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    //this might not work
    eglSwapBuffers(eglGetCurrentDisplay(), eglGetCurrentSurface(EGL_DRAW));
}


int main()
{
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.majorVersion = 2; // WebGL 2.0 corresponds to OpenGL ES 3.0
    attrs.minorVersion = 0;

    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context("#canvas", &attrs);
    emscripten_webgl_make_context_current(context);

    double canvasWidth, canvasHeight;
    EMSCRIPTEN_RESULT res = emscripten_get_element_css_size("#canvas", &canvasWidth, &canvasHeight);

	glViewport(0, 0, (GLsizei)canvasWidth, (GLsizei)canvasHeight);

	// Vertices coordinates
	GLfloat vertices[] =
	{		//coordinates       //colors            //texture coordinate description
        -0.5f, -0.5f, 0.0f,     1.0f, 0.0f, 0.0f,	0.0f, 0.0f, // Lower left corner
        -0.5f,  0.5f, 0.0f,     0.0f, 1.0f, 0.0f,	0.0f, 1.0f, // Upper left corner
         0.5f,  0.5f, 0.0f,     0.0f, 0.0f, 1.0f,	1.0f, 1.0f, // Upper right corner
         0.5f, -0.5f, 0.0f,     1.0f, 1.0f, 1.0f,	1.0f, 0.0f  // Lower right corner
	};

	// Indices for vertices order
	GLuint indices[] =
	{
		0, 2, 1,
        0, 3, 2
	};

	shaderProgram = new Shader("./shaders/texture.vert", "./shaders/texture.frag");

	VAO1 = new VAO();
	VAO1->Bind();
	
	VBO VBO1(vertices, sizeof(vertices));

	EBO EBO1(indices, sizeof(indices));

	// VAO1->LinkVBO(VBO1, 0);
	VAO1->LinkAttrib(VBO1, 0, 3, GL_FLOAT, sizeof(float)*8, (void*)0);
	VAO1->LinkAttrib(VBO1, 1, 3, GL_FLOAT, sizeof(float)*8, (void*)(sizeof(float)*3));
    VAO1->LinkAttrib(VBO1, 2, 2, GL_FLOAT, sizeof(float)*8, (void*)(sizeof(float)*6));

    VAO1->Unbind();
	VBO1.Unbind();
	EBO1.Unbind();

    uniformID = glGetUniformLocation(shaderProgram->ID, "scale");

    popCat = new Texture("./textures/pop_cat.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
    popCat->texUnit(shaderProgram, "tex0", 0); 

	emscripten_set_main_loop(MainLoop, 0, 1);
	
    VAO1->Delete();
	VBO1.Delete();
	EBO1.Delete();
    popCat->Delete();
	shaderProgram->Delete();

	return 0;
}