#include<iostream>
#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <cmath>
#include "ShaderClass.hpp"
#include "VAO.hpp"
#include "EBO.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <../include/stb_image.h>

VAO* VAO1;
Shader* shaderProgram;

void MainLoop(){
    glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
    // Clean the back buffer and assign the new color to it
    glClear(GL_COLOR_BUFFER_BIT);
    // Tell OpenGL which Shader Program we want to use
    glUseProgram(shaderProgram->ID);
    // Bind the VAO so OpenGL knows to use it
    glBindVertexArray(VAO1->ID);
    // Draw primitives, number of indices, datatype of indices, index of indices
    glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_INT, 0);


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
	{		//coordinates									//colors
		-0.5f, -0.5f * float(sqrt(3)) * 1 / 3, 0.0f,     0.2f, 0.6f,  0.3f, // Lower left corner
		0.5f, -0.5f * float(sqrt(3)) * 1 / 3, 0.0f,     0.2f, 0.8f,  0.3f, // Lower right corner
		0.0f,  0.5f * float(sqrt(3)) * 2 / 3, 0.0f,     0.5f, 0.2f,  0.6f, // Upper corner
	   -0.25f, 0.5f * float(sqrt(3)) * 1 / 6, 0.0f,     0.9f, 0.45f, 0.17f, // Inner left
		0.25f, 0.5f * float(sqrt(3)) * 1 / 6, 0.0f,     0.9f, 0.45f, 0.17f, // Inner right
		0.0f, -0.5f * float(sqrt(3)) * 1 / 3, 0.0f,     1.0f, 1.0f,  0.4f  // Inner down
	};

	// Indices for vertices order
	GLuint indices[] =
	{
		0, 3, 5, // Lower left triangle
		3, 2, 4, // Upper triangle
		5, 4, 1 // Lower right triangle
	};

	shaderProgram = new Shader("./shaders/default.vert", "./shaders/default.frag");

	VAO1 = new VAO();
	VAO1->Bind();
	
	VBO VBO1(vertices, sizeof(vertices));

	EBO EBO1(indices, sizeof(indices));

	// VAO1->LinkVBO(VBO1, 0);
	VAO1->LinkAttrib(VBO1, 0, 3, GL_FLOAT, sizeof(float)*6, (void*)0);
	VAO1->LinkAttrib(VBO1, 1, 3, GL_FLOAT, sizeof(float)*6, (void*)(sizeof(float)*3));

	emscripten_set_main_loop(MainLoop, 0, 1);


	VAO1->Unbind();
	VBO1.Unbind();
	EBO1.Unbind();
	shaderProgram->Delete();

	return 0;
}