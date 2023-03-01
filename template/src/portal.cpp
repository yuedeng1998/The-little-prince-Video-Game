// Header
#include "portal.hpp"

#include <cmath>

bool Portal::init()
{
    // The position corresponds to the center of the texture.
    motion.radians = 0.f;
    motion.speed = 0.f;
    is_verticle = true;
    physics.scale = {20.f, 40.f};

    vec3 green = {0 / 256.f, 256 / 256.f, 0 / 256.f};
    vec3 yellow = {155.f / 256.f, 135.f / 256.f, 12.f / 256.f};
    vec3 white = {1.f,1.f,1.f};
    Vertex vertice;
    constexpr float z = -0.1;
    constexpr int NUM_SEGMENTS = 12;

    // inner circle
    for (int i = 0; i < NUM_SEGMENTS; i++)
    {
        vertice.position = {(float)std::cos(M_PI * 2.0 * float(i) / (float)NUM_SEGMENTS),
                            (float)std::sin(M_PI * 2.0 * float(i) / (float)NUM_SEGMENTS), z};
        vertice.color = yellow;
        m_vertices.emplace_back(vertice);

        m_indices.emplace_back(i);
        m_indices.emplace_back((i+1)%NUM_SEGMENTS);
        m_indices.emplace_back(NUM_SEGMENTS);
    }
    // index 12 (NUM_SEGMENTS)
    vertice.position = {0,0, z};
    vertice.color = white;
    m_vertices.emplace_back(vertice);

    // outer triangle; index starting at 13
    for (int i = 0; i < NUM_SEGMENTS; i++)
    {
        float y = (float)std::sin(M_PI * 2.0 * float(i+0.5f) / (float)NUM_SEGMENTS);
        if (y>0)
            y=-0.5f;
        else
            y=+0.5f;
        vertice.position = {(float)std::cos(M_PI * 2.0 * float(i+0.5f) / (float)NUM_SEGMENTS),
                            y, z};
        vertice.color = green;
        m_vertices.emplace_back(vertice);

        m_indices.emplace_back(i);
        m_indices.emplace_back((i+1)%NUM_SEGMENTS);
        m_indices.emplace_back(NUM_SEGMENTS+1+i);
    }

    // Clearing errors
    gl_flush_errors();

    // Vertex Buffer creation
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * m_vertices.size(), m_vertices.data(), GL_STATIC_DRAW);

    // Index Buffer creation
    glGenBuffers(1, &mesh.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * m_indices.size(), m_indices.data(), GL_STATIC_DRAW);

    // Vertex Array (Container for Vertex + Index buffer)
    glGenVertexArrays(1, &mesh.vao);
    if (gl_has_errors())
        return false;

    // Loading shaders
    if (!effect.load_from_file(shader_path("coloured.vs.glsl"), shader_path("coloured.fs.glsl")))
        return false;

    return true;
}

// Releases all graphics resources
void Portal::destroy()
{
    glDeleteBuffers(1, &mesh.vbo);
    glDeleteBuffers(1, &mesh.ibo);
    glDeleteVertexArrays(1, &mesh.vao);

    glDeleteShader(effect.vertex);
    glDeleteShader(effect.fragment);
    glDeleteShader(effect.program);
}

void Portal::update()
{
    if (physics.scale.y < 10.f)
        is_verticle = false;
    else if (physics.scale.y > 20.f)
        is_verticle = true;

    if (is_verticle)
    {
        physics.scale.x = physics.scale.x * 1.01f;
        physics.scale.y = physics.scale.y / 1.01f;
    }
    else
    {
        physics.scale.x = physics.scale.x / 1.01f;
        physics.scale.y = physics.scale.y * 1.01f;
    }
    motion.radians += 0.01;
}

void Portal::draw(const mat3 &projection)
{
    // Transformation code, see Rendering and Transformation in the template specification for more info
    // Incrementally updates transformation matrix, thus ORDER IS IMPORTANT
    transform.begin();
    transform.translate(motion.position);
    transform.rotate(motion.radians);
    transform.scale(physics.scale);
    transform.end();

    // Setting shaders
    glUseProgram(effect.program);

    // Enabling alpha channel for textures
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    // Getting uniform locations for glUniform* calls
    GLint transform_uloc = glGetUniformLocation(effect.program, "transform");
    GLint projection_uloc = glGetUniformLocation(effect.program, "projection");

    // Setting vertices and indices
    glBindVertexArray(mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);

    // Input data location as in the vertex buffer
    GLint in_position_loc = glGetAttribLocation(effect.program, "in_position");
    GLint in_color_uloc = glGetAttribLocation(effect.program, "in_color");
    glEnableVertexAttribArray(in_position_loc);
    glEnableVertexAttribArray(in_color_uloc);
    glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
    glVertexAttribPointer(in_color_uloc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)sizeof(vec3));

    // Setting uniform values to the currently bound program
    glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float *)&transform.out);
    glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float *)&projection);

    GLint size = 0;
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
    GLsizei num_indices = size / sizeof(uint16_t);

    // Drawing!
    glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
}

vec2 Portal::get_position() const
{
    return motion.position;
}

void Portal::set_position(vec2 position)
{
    motion.position = position;
}

vec2 Portal::get_bounding_box() const
{
    // Returns the local bounding coordinates scaled by the current size of the portal
    // fabs is to avoid negative scale due to the facing direction.
    return {physics.scale.x, physics.scale.y};
}

void Portal::get_tblr_points(vec2 *top, vec2 *bottom, vec2 *left, vec2 *right)
{
    for (Vertex v : m_vertices)
    {
        vec3 pos = mul(transform.out, vec3({v.position.x, v.position.y, 1.0}));
        if (pos.x < left->x)
        {
            left->x = pos.x;
            left->y = pos.y;
        }
        if (pos.x > right->x)
        {
            right->x = pos.x;
            right->y = pos.y;
        }
        if (pos.y < top->y)
        {
            top->x = pos.x;
            top->y = pos.y;
        }
        if (pos.y > bottom->y)
        {
            bottom->x = pos.x;
            bottom->y = pos.y;
        }
    }
}