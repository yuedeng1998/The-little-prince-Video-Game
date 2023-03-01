#pragma once

#include "common.hpp"
#include "render_manager.hpp"

class Points : public Entity
{
    // Shared between all fish, no need to load one for each instance
    static Texture num_texture;

public:
    // Creates all the associated render resources and default transform
    bool init();

    // Releases all the associated resources
    void destroy();

    // Update fish
    // ms represents the number of milliseconds elapsed from the previous update() call
    void update(float points);

    // Renders the fish
    // projection is the 2D orthographic projection matrix
    void draw(const mat3 &projection) override;

    // Sets the new fish position
    void set_position(vec2 position);

    bool set_buffer();
    void set_point(int i);

private:
    float offset[2];
    float prince_points;
};
