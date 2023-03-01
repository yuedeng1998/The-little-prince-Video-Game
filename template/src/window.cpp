// Header
#include "window.hpp"

#include <cmath>

Texture Window::window_texture;

bool Window::init()
{
    num = rand() % 5;
    empty = num == 0;
    // std::string file = "window.png";
    // std::string dir_path = data_path "/textures/";

    // std::string file_path = dir_path + file;

    // const char *c = file_path.c_str();

    // // Load shared key texture
    // if (!window_texture.is_valid())
    // {
    //     if (!window_texture.load_from_file(c))
    //     {
    //         fprintf(stderr, "Failed to load window texture!");
    //         return false;
    //     }
    // }

    const char *texturePath = textures_path("window.png");
    if (!RenderManager::load_texture(texturePath, &window_texture, this))
    {
        return false;
    }

    if (!RenderManager::set_buffers(&window_texture, this, -0.04f, vec2{1, 2}))
    {
        return false;
    }

    // // The position corresponds to the center of the texture.
    // float wr = window_texture.width * 0.5f;
    // float hr = window_texture.height * 0.5f;

    // TexturedVertex vertices[4];
    // vertices[0].position = {-wr, +hr, -0.01f};
    // vertices[0].texcoord = {0.f, 1.f};
    // vertices[1].position = {+wr, +hr, -0.01f};
    // vertices[1].texcoord = {
    //     1.f,
    //     1.f / 2,
    // };
    // vertices[2].position = {+wr, -hr, -0.01f};
    // vertices[2].texcoord = {1.f, 0.f};
    // vertices[3].position = {-wr, -hr, -0.01f};
    // vertices[3].texcoord = {0.f, 0.f};

    // // Counterclockwise as it's the default opengl front winding direction.
    // uint16_t indices[] = {0, 3, 1, 1, 3, 2};

    // // Clearing errors
    // gl_flush_errors();

    // // Vertex Buffer creation
    // glGenBuffers(1, &mesh.vbo);
    // glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(TexturedVertex) * 4, vertices, GL_STATIC_DRAW);

    // // Index Buffer creation
    // glGenBuffers(1, &mesh.ibo);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * 6, indices, GL_STATIC_DRAW);

    // // Vertex Array (Container for Vertex + Index buffer)
    // glGenVertexArrays(1, &mesh.vao);
    // if (gl_has_errors())
    //     return false;

    // Loading shaders
    if (!effect.load_from_file(shader_path("window.vs.glsl"), shader_path("num.fs.glsl")))
        return false;

    physics.scale = {-1.5f, 0.8f};
    motion.position = vec2{200, 150};
    if (num != 0)
    {
        // star.init();
        num_stars.init();
    }
    count_down = 0;
    // float M_PI = 3.14159265359;
    motion.radians = M_PI;

    return true;
}

// Releases all graphics resources
void Window::destroy()
{
    // for (auto &star : stars)
    //     star.destroy();
    if (num != 0)
    {
        // star.destroy();
        num_stars.destroy();
    }
    glDeleteBuffers(1, &mesh.vbo);
    glDeleteBuffers(1, &mesh.ibo);
    glDeleteVertexArrays(1, &mesh.vao);

    glDeleteShader(effect.vertex);
    glDeleteShader(effect.fragment);
    glDeleteShader(effect.program);
}

void Window::update(float ms)
{

    // star.update(ms);
    count_down--;
    if (empty && count_down <= 0)
    {
        num = 0;
    }
}

void Window::draw(const mat3 &projection)
{
    if (count_down > 0)
    {
        glEnable(GL_DEPTH_TEST);
        Entity *entity = this;
        Texture *texture = &window_texture;

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
        // glDisable(GL_DEPTH_TEST);

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
        float oy = 1.0;
        if (num == 0)
        {
            oy = 1.0 / 2.0;
        }
        float offset_in[2] = {0.0, oy};
        glUniform3fv(color_uloc, 1, color);
        glUniform2fv(offset_uloc, 1, offset_in);
        glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float *)&projection);

        // Drawing!
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
        // RenderManager::draw_texture(projection, &window_texture, this, motion.position, motion.radians, physics.scale);

        if (num != 0)
        {
            // star.draw(projection);
            // vec2 star_p = vec2{motion.position.x - 70, motion.position.y + 40};
            // star.set_position(star_p);
            // star.change_state(4);
            num_stars.draw(projection);
            vec2 points = vec2{motion.position.x + 30, motion.position.y + 40};
            num_stars.set_position(points);
            num_stars.set_point(num);
        }
        glDisable(GL_DEPTH_TEST);
    }
}

vec2 Window::get_position() const
{
    return motion.position;
}

void Window::set_position(vec2 position)
{
    motion.position = position;
}

void Window::set_empty()
{
    empty = true;
}

void Window::show_window()
{
    count_down = 100;
}

bool Window::set_buffer()
{
    float wr = window_texture.width * 0.5f;
    float hr = window_texture.height * 0.5f;

    TexturedVertex vertices[4];
    vertices[0].position = {-wr, +hr, -0.01f};
    vertices[0].texcoord = {0.f, 1.f / 2};
    vertices[1].position = {+wr, +hr, -0.01f};
    vertices[1].texcoord = {
        1.f,
        1.f / 2,
    };
    vertices[2].position = {+wr, -hr, -0.01f};
    vertices[2].texcoord = {1.f, 0.f};
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

int Window::get_points()
{
    return num;
}