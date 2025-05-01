#pragma once
#include <iostream>
#include <vector>
#include <GLES3/gl3.h>

#define MESH_COUNT 4

typedef enum ShapeType {
    TRIANGLE,
    MESH,
    SQUARE,
    CIRCLE,
    CUBE,
    SPHERE
} ShapeType;

class Shape {
public:
    ShapeType shape_type;
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    GLuint vertex_len;

    Shape(){

    }

    static Shape InitMesh(){
        Shape shape;
        shape.shape_type = ShapeType::MESH;

        std::vector<float> line_coords; //4 points are required for each mesh, and they all have X, Y coordinates
        std::vector<GLuint> indices;

        //there are 4 x, y points needed for a "mesh square"
        for (int i=4*2; i<4*2*(MESH_COUNT+1); i+=4*2){
            //things are divided by mesh_count*2, so the first 4 coordinate points won't be zeroes

            //up right coordinate x
            line_coords.push_back((float)((int)(i/(MESH_COUNT*2))));
            //up right coordinate z
            line_coords.push_back((float)((int)(i/(MESH_COUNT*2))));

            //down right coordinate x
            line_coords.push_back((float)((int)(i/(MESH_COUNT*2))));
            //down right coordinate z
            line_coords.push_back((float)-((int)(i/(MESH_COUNT*2))));

            //down left coordinate x
            line_coords.push_back((float)-((int)(i/(MESH_COUNT*2))));
            //down left coordinate z
            line_coords.push_back((float)-((int)(i/(MESH_COUNT*2))));

            //up left coordinate x
            line_coords.push_back((float)-((int)(i/(MESH_COUNT*2))));
            //up left coordinate z
            line_coords.push_back((float)((int)(i/(MESH_COUNT*2))));
        }

        for (GLuint i=0; i<MESH_COUNT*4; i++){
            indices.push_back(i);
            // std::cout << "current index: " << i << std::endl;
        }

        glGenVertexArrays(1, &shape.VAO);
        glBindVertexArray(shape.VAO);
        glGenBuffers(1, &shape.VBO);
        glBindBuffer(GL_ARRAY_BUFFER, shape.VBO);

        //MUST MULTIPLY BY THE LENGTH OF THE DATA TYPE, because this is how the size will be set internally
        glBufferData(GL_ARRAY_BUFFER, line_coords.size() * sizeof(float), line_coords.data(), GL_STATIC_DRAW);
        // std::cout << "line coords size: " << line_coords.size() << std::endl;

        // std::cout << "indices size: " << indices.size() << std::endl;
        //2 coordinates: x, z, tightly packed, without anything else
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glGenBuffers(1, &shape.EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shape.EBO);

        //MUST MULTIPLY BY THE LENGTH OF THE DATA TYPE, because at the time of drawing, we will also specify the data type
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

        shape.vertex_len = indices.size();

        return shape;
    }



};