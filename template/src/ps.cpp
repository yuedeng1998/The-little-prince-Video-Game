#define _USE_MATH_DEFINES

// Header
#include "ps.hpp"

#include <cmath>
#include <iostream>
#include <vector>

static const int MAX_PEBBLES = 25;
constexpr int NUM_SEGMENTS = 12;

bool Particles::init()
{
    std::vector<GLfloat> screen_vertex_buffer_data;
    constexpr float z = -0.1;

    for (int i = 0; i < NUM_SEGMENTS; i++)
    {
        screen_vertex_buffer_data.push_back(std::cos(M_PI * 2.0 * float(i) / (float)NUM_SEGMENTS));
        screen_vertex_buffer_data.push_back(std::sin(M_PI * 2.0 * float(i) / (float)NUM_SEGMENTS));
        screen_vertex_buffer_data.push_back(z);

        screen_vertex_buffer_data.push_back(std::cos(M_PI * 2.0 * float(i + 1) / (float)NUM_SEGMENTS));
        screen_vertex_buffer_data.push_back(std::sin(M_PI * 2.0 * float(i + 1) / (float)NUM_SEGMENTS));
        screen_vertex_buffer_data.push_back(z);

        screen_vertex_buffer_data.push_back(0);
        screen_vertex_buffer_data.push_back(0);
        screen_vertex_buffer_data.push_back(z);
    }

    // Clearing errors
    gl_flush_errors();

    // Vertex Buffer creation
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, screen_vertex_buffer_data.size() * sizeof(GLfloat), screen_vertex_buffer_data.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &m_instance_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_instance_vbo);
    if (gl_has_errors())
        return false;

    // Loading shaders
    if (!effect.load_from_file(shader_path("ps.vs.glsl"), shader_path("ps.fs.glsl")))
        return false;
    pcolor = 0;
    return true;
}

// Releases all graphics resources
void Particles::destroy()
{
    glDeleteBuffers(1, &mesh.vbo);
    glDeleteBuffers(1, &m_instance_vbo);

    glDeleteShader(effect.vertex);
    glDeleteShader(effect.fragment);
    glDeleteShader(effect.program);

    m_particles.clear();
}

void Particles::update(float ms)
{
    float min_dis_to_salmon = 0;
    auto particle = m_particles.begin();
    while (particle != m_particles.end())
    {
        float stepx = -1.0 * particle->velocity.x * (ms / 10000);
        float stepy = -1.0 * particle->velocity.y * (ms / 10000);
        particle->position.x += stepx;
        particle->position.y += stepy;
        //printf("ms is %f", ms);
        particle->velocity.y += -0.98 * ms;
        particle->life--;
        if (particle->life == 0)
        {
            particle = m_particles.erase(particle);
        }
        else
            ++particle;
    }
    pcolor += 0.1;
    num_p = m_particles.size();
}

int Particles::num_left()
{
    return num_p;
}

void Particles::spawn_pebble(vec2 position, float radius)
{

    vec2 inital = position;
    for (int i = 0; i < 100; i++)
    {
        Particle p;
        {
            p.position = inital;
            p.position.y -= 30;
            float mx, my;
            float stepx = rand() % 25;
            float stepy = rand() % 5;
            float x = pow(-1, stepx) * (50 + (stepx + 1) * 20.f);
            float y = 300 + 10 * i + stepy * 100;
            p.velocity = {x, y};
            p.life = 30 + 0.005 * i;

            p.radius = sqrt(i);
            p.radiusy = 5;
            // p.color = stepy * 0.05;
            m_particles.emplace_back(p);
        }
    }
}

void Particles::draw(const mat3 &projection)
{
    // Setting shaders
    glUseProgram(effect.program);

    // Enabling alpha channel for textures
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    // Getting uniform locations
    GLint projection_uloc = glGetUniformLocation(effect.program, "projection");
    GLint color_uloc = glGetUniformLocation(effect.program, "color");

    // Pebble color
    pcolor += 0.1;
    float b = (0.4f * pcolor);
    float color[] = {0.4f, 0.0f, b};
    glUniform3fv(color_uloc, 1, color);
    glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float *)&projection);

    // Draw the screen texture on the geometry
    // Setting vertices
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);

    // Mesh vertex positions
    // Bind to attribute 0 (in_position) as in the vertex shader
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glVertexAttribDivisor(0, 0);

    // Load up pebbles into buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_instance_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_particles.size() * sizeof(Particle), m_particles.data(), GL_DYNAMIC_DRAW);

    // Pebble translations
    // Bind to attribute 1 (in_translate) as in vertex shader
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid *)offsetof(Particle, position));
    glVertexAttribDivisor(1, 1);

    // Pebble radii
    // Bind to attribute 2 (in_scale) as in vertex shader
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid *)offsetof(Particle, radius));
    glVertexAttribDivisor(2, 1);

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid *)offsetof(Particle, radiusy));
    glVertexAttribDivisor(3, 1);

    // Draw using instancing
    // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glDrawArraysInstanced.xhtml
    glDrawArraysInstanced(GL_TRIANGLES, 0, NUM_SEGMENTS * 3, m_particles.size());

    // Reset divisor
    glVertexAttribDivisor(1, 0);
    glVertexAttribDivisor(2, 0);
}
