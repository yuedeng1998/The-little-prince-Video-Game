// Header
#include "arrow.hpp"

#include <cmath>

Texture Arrow::arrow_texture;

bool Arrow::init()
{
    const char *texturePath = textures_path("arrow.png");
    if (!RenderManager::load_texture(texturePath, &arrow_texture, this))
    {
        return false;
    }

    if (!RenderManager::set_buffers(&arrow_texture, this, -0.001f, vec2{1, 1}))
    {
        return false;
    }

    // Loading shaders
    if (!effect.load_from_file(shader_path("textured.vs.glsl"), shader_path("textured.fs.glsl")))
        return false;

    motion.radians = 3.14159265359;
    motion.speed = 380.f;

    // set initial pos for first key
    motion.position = {1000.f, 400.f};

    // Setting initial values, scale is negative to make it face the opposite way
    // 1.0 would be as big as the original texture.
    physics.scale = {-0.3f, 0.3f};
    moving = 3;

    return true;
}

// Releases all graphics resources
void Arrow::destroy()
{
    glDeleteBuffers(1, &mesh.vbo);
    glDeleteBuffers(1, &mesh.ibo);
    glDeleteVertexArrays(1, &mesh.vao);

    glDeleteShader(effect.vertex);
    glDeleteShader(effect.fragment);
    glDeleteShader(effect.program);
}

void Arrow::update(float ms)
{
    motion.radians = 3.14159265359;
    float step = motion.speed * (ms / 10000);

    // if ((int)moving % 10 == 0)
    // {
    //     moving = 1;
    // }
    // moving = (int)moving % 10;
    //physics.scale = scale;

    // moving = (int)moving % 10;

    if (motion.position.x > 1100)
    {
        motion.position.x = 1090;
        moving = -moving;
    }
    if (motion.position.x < 980)
    {
        motion.position.x = 982;
        moving = -moving;
    }
    motion.position.x += moving * step;

    // motion.position.y += move.y;
}

void Arrow::draw(const mat3 &projection)
{
    RenderManager::draw_texture(projection, &arrow_texture, this, motion.position, motion.radians, physics.scale);
}

vec2 Arrow::get_position() const
{
    return motion.position;
}

void Arrow::set_position(vec2 position)
{
    motion.position = position;
}

vec2 Arrow::get_bounding_box() const
{
    // Returns the local bounding coordinates scaled by the current size of the key
    // fabs is to avoid negative scale due to the facing direction.
    return {std::fabs(physics.scale.x) * arrow_texture.width, std::fabs(physics.scale.y) * arrow_texture.height};
}
