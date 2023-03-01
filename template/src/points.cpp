// Header
#include "points.hpp"

#include <cmath>

Texture Points::num_texture;

bool Points::init()
{
    const char *texturePath = textures_path("numb.png");
    if (!RenderManager::load_texture(texturePath, &num_texture, this))
    {
        return false;
    }

    if (!RenderManager::set_buffers(&num_texture, this, -0.01, vec2{5, 2}))
    {
        return false;
    }

    // Loading shaders
    if (!effect.load_from_file(shader_path("num.vs.glsl"), shader_path("num.fs.glsl")))
        return false;

    motion.radians = 3.14159265359;
    motion.speed = 380.f;

    // set initial pos for first key
    motion.position = {100.f, 100.f};

    // Setting initial values, scale is negative to make it face the opposite way
    // 1.0 would be as big as the original texture.
    physics.scale = {-0.08f, 0.12f};
    offset[0] = 0.0;
    offset[1] = 1.0;
    prince_points = 0;

    return true;
}

// Releases all graphics resources
void Points::destroy()
{
    glDeleteBuffers(1, &mesh.vbo);
    glDeleteBuffers(1, &mesh.ibo);
    glDeleteVertexArrays(1, &mesh.vao);

    glDeleteShader(effect.vertex);
    glDeleteShader(effect.fragment);
    glDeleteShader(effect.program);
}

void Points::update(float i)
{

    if (i != prince_points)
    {
        if (i >= 0 && i <= 4)
        {
            offset[0] = i;
            offset[1] = 1;
        }
        else
        {
            offset[0] = i - 5;
            offset[1] = 2;
        }
        prince_points = i;
    }
}

void Points::draw(const mat3 &projection)
{
    glEnable(GL_DEPTH_TEST);
    Entity *entity = this;
    Texture *texture = &num_texture;

    transform.begin();
    transform.translate(motion.position);
    entity->transform.rotate(motion.radians);
    entity->transform.scale(physics.scale);
    entity->transform.end();
    // Setting shaders
    glUseProgram(entity->effect.program);

    // Enabling alpha channel for textures
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    // Getting uniform locations for glUniform* calls
    GLint transform_uloc = glGetUniformLocation(entity->effect.program, "transform");
    GLint color_uloc = glGetUniformLocation(entity->effect.program, "fcolor");
    GLint projection_uloc = glGetUniformLocation(entity->effect.program, "projection");
    GLint offset_uloc = glGetUniformLocation(entity->effect.program, "offset");

    // Setting vertices and indices
    glBindVertexArray(entity->mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, entity->mesh.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, entity->mesh.ibo);

    // Input data location as in the vertex buffer
    GLint in_position_loc = glGetAttribLocation(entity->effect.program, "in_position");
    GLint in_texcoord_loc = glGetAttribLocation(entity->effect.program, "in_texcoord");
    glEnableVertexAttribArray(in_position_loc);
    glEnableVertexAttribArray(in_texcoord_loc);
    glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void *)0);
    glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void *)sizeof(vec3));

    // Enabling and binding texture to slot 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->id);

    // Setting uniform values to the currently bound program
    glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float *)&entity->transform.out);
    float color[] = {1.f, 1.f, 1.f};
    float ox = offset[0] / 5.0;
    float oy = offset[1] / 2.0;
    float offset_in[2] = {ox, oy};
    glUniform3fv(color_uloc, 1, color);
    glUniform2fv(offset_uloc, 1, offset_in);
    glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float *)&projection);

    // Drawing!
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
    glDisable(GL_DEPTH_TEST);
}

bool Points::set_buffer()
{
    float wr = num_texture.width * 0.5f;
    float hr = num_texture.height * 0.5f;

    TexturedVertex vertices[4];
    vertices[0].position = {-wr, +hr, -0.01f};
    vertices[0].texcoord = {0.f, 1.f / 2};
    vertices[1].position = {+wr, +hr, -0.01f};
    vertices[1].texcoord = {
        1.f / 5,
        1.f / 2,
    };
    vertices[2].position = {+wr, -hr, -0.01f};
    vertices[2].texcoord = {1.f / 5, 0.f};
    vertices[3].position = {-wr, -hr, -0.01f};
    vertices[3].texcoord = {0.f, 0.f};

    // Counterclockwise as it's the default opengl front winding direction.
    uint16_t indices[] = {0, 3, 1, 1, 3, 2};

    // Clearing errors
    gl_flush_errors();

    // Vertex Buffer creation
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TexturedVertex) * 4, vertices, GL_STATIC_DRAW);

    // Index Buffer creation
    glGenBuffers(1, &mesh.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * 6, indices, GL_STATIC_DRAW);

    // Vertex Array (Container for Vertex + Index buffer)
    glGenVertexArrays(1, &mesh.vao);
    if (gl_has_errors())
        return false;

    return true;
}
void Points::set_position(vec2 position)
{
    motion.position = position;
}

void Points::set_point(int i)
{
    if (i >= 0 && i <= 4)
    {
        offset[0] = i;
        offset[1] = 1;
    }
    else
    {
        offset[0] = i - 4;
        offset[1] = 2;
    }
}
