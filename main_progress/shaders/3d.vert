#version 300 es
layout(location=0) in vec3 basePos;
layout(location=1) in vec2 inputTexCoord;
uniform vec3 translation;
uniform vec3 rotation;
uniform vec3 scaling;
uniform mat4 mvp_matrix;
uniform mat4 cam_matrix;

out vec2 TexCoord;

mat4 rotationMatrix(vec3 rotation_angles) {
    vec3 eulerAngles = radians(rotation_angles);
    float cx = cos(eulerAngles.x);
    float sx = sin(eulerAngles.x);
    float cy = cos(eulerAngles.y);
    float sy = sin(eulerAngles.y);
    float cz = cos(eulerAngles.z);
    float sz = sin(eulerAngles.z);
    
    mat4 rotX = mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, cx, -sx, 0.0,
        0.0, sx, cx, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
    
    mat4 rotY = mat4(
        cy, 0.0, sy, 0.0,
        0.0, 1.0, 0.0, 0.0,
        -sy, 0.0, cy, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
    
    mat4 rotZ = mat4(
        cz, -sz, 0.0, 0.0,
        sz, cz, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
    
    return rotZ * rotY * rotX;
}

void main() {
    // Create scaling matrix
    mat4 scale_matrix = mat4(
        scaling.x, 0.0, 0.0, 0.0,
        0.0, scaling.y, 0.0, 0.0,
        0.0, 0.0, scaling.z, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
    
    // Create translation matrix
    mat4 translation_matrix = mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        translation.x, translation.y, translation.z, 1.0
    );
    
    // Get rotation matrix
    mat4 rotation_matrix = rotationMatrix(rotation);
    
    // Create model matrix (scale -> rotate -> translate)
    mat4 model_matrix = translation_matrix * rotation_matrix * scale_matrix;
    
    // Apply transformations
    gl_Position = cam_matrix * model_matrix * vec4(basePos, 1.0);
    TexCoord = inputTexCoord;
}