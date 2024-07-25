#pragma once
#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <vector>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include <vector>
#include <iostream>
#include "glm/glm.hpp"

int mapRange(float value, float inMin, float inMax, int outMin, int outMax);
void print_vector(std::vector<int> rhs);
void print_vector_vector(std::vector<std::vector<int>> rhs);

// << operator for glm::vec3
std::ostream& operator<<(std::ostream& os, const glm::vec3& vec);

class Utility {
public:
    // ————— METHODS ————— //
    static GLuint load_texture(const char* filepath);
    static void draw_text(ShaderProgram* program, GLuint font_texture_id, std::string text, float screen_size, float spacing, glm::vec3 position);
};