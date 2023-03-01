// Header
#include "key.hpp"

#include <cmath>

Texture Key::key_texture;

bool Key::init()
{
    std::string file = "key.png";
    std::string dir_path = data_path "/textures/";

    std::string file_path = dir_path + file;

    const char *c = file_path.c_str();

    // Load shared key texture
    if (!key_texture.is_valid())
    {
        if (!key_texture.load_from_file(c))
        {
            fprintf(stderr, "Failed to load key texture!");
            return false;
        }
    }

    if (!RenderManager::set_buffers(&key_texture, this, -0.01f, vec2{1, 1}))
    {
        return false;
    }

    // Loading shaders
    if (!effect.load_from_file(shader_path("textured.vs.glsl"), shader_path("textured.fs.glsl")))
        return false;

    motion.radians = 0.f;
    motion.speed = 380.f;
    is_collected = false;

    // set initial pos for first key
    // motion.position = {100.f, 100.f};

    // Setting initial values, scale is negative to make it face the opposite way
    // 1.0 would be as big as the original texture.
    physics.scale = {-0.4f, 0.4f};

    return true;
    // >>>>>>> min-play-steph
}

// Releases all graphics resources
void Key::destroy()
{
    glDeleteBuffers(1, &mesh.vbo);
    glDeleteBuffers(1, &mesh.ibo);
    glDeleteVertexArrays(1, &mesh.vao);

    glDeleteShader(effect.vertex);
    glDeleteShader(effect.fragment);
    glDeleteShader(effect.program);
}

void Key::update()
{
}

void Key::draw(const mat3 &projection)
{
    RenderManager::draw_texture(projection, &key_texture, this, motion.position, motion.radians, physics.scale);
}

vec2 Key::get_position() const
{
    return motion.position;
}

void Key::set_position(vec2 position)
{
    motion.position = position;
}

vec2 Key::get_bounding_box() const
{
    // Returns the local bounding coordinates scaled by the current size of the key
    // fabs is to avoid negative scale due to the facing direction.
    return {std::fabs(physics.scale.x) * key_texture.width, std::fabs(physics.scale.y) * key_texture.height};
}

void Key::set_is_collected(bool b)
{
    is_collected = b;
}
bool Key::get_is_collected()
{
    return is_collected;
}