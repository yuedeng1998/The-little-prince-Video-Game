#pragma once

#include <vector>

#include "common.hpp"

class Particles : public Entity
{
public:
    // Data structure for pebble contains information needed
    // to render and simulate a basic pebble (apart from mesh.vbo),
    // we will use this layout to pass information from m_pebbles to the pipeline.
    struct Particle
    {
        float life = 0.0f; // remove pebble when its life reaches 0
        vec2 position;
        vec2 velocity;
        float radius;
        float radiusy;
    };

    // Creates all the associated render resources
    bool init();

    // Releases all associated resources
    void destroy();

    // Updates all pebbles
    // ms represents the number of milliseconds elapsed from the previous update() call
    void update(float ms);

    // Renders the pebbles
    // projection is the 2D orthographic projection matrix
    void draw(const mat3 &projection) override;

    // Spawn new pebble
    void spawn_pebble(vec2 position, float radius);

    int num_left();

private:
    GLuint m_instance_vbo;             // vbo for instancing pebbles
    std::vector<Particle> m_particles; // vector of pebbles
    int num_p;
    int pcolor;
};