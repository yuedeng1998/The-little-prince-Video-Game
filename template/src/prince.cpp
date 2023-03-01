// Header
#include "prince.hpp"
#include "common.hpp"
// internal
#include "enemy.hpp"
#include "portal.hpp"
#include "star.hpp"
#include "heart.hpp"
#include "walls.hpp"
#include "key.hpp"

// stlib
#include <string>
#include <algorithm>
#include <cmath>

Texture Prince::prince_texture;

bool Prince::init()
{
    m_vertices.clear();
    m_indices.clear();
    m_light_up_countdown_ms = 0.f;

    m_lives = 3;
    m_points = 0;

    // Load shared texture
    const char *texturePath = textures_path("little_prince.png");
    if (!RenderManager::load_texture(texturePath, &prince_texture, this))
    {
        return false;
    }

    float spritesheet_width = 4.0f;
    float spritesheet_height = 4.0f;

    m_sprite_width = prince_texture.width / spritesheet_width;
    m_sprite_height = prince_texture.height / spritesheet_height;

    spritesheet.init(&prince_texture, {spritesheet_width, spritesheet_height}, this);
    spritesheet.set_data(this, 0);

    // Setting initial values
    motion.position = {200.f, 355.f};
    motion.radians = 0.f;
    motion.speed = 0.f;
    motion.max_speed = 300.f;
    m_velocity = {0.f, 0.f};

    physics.scale = {-0.75f, 0.75f};

    // keeps track of number of lives the prince has, prince starts with 3 lives by default
    m_lives = 3;
    return true;
}

// Called on each frame by World::update()
void Prince::update(float ms)
{
    if (m_lives > 0)
    {
        m_velocity = add(m_velocity, mul(motion.acceleration, ms / 1000));
        if (len(motion.acceleration) > 0.001f)
        {
            if (len(m_velocity) > motion.max_speed)
                m_velocity = mul(normalize(m_velocity), motion.max_speed);
            move(mul(m_velocity, (ms / 1000)));
        }
        else
        {
            velocity_to_zero(ms);
            move(mul(m_velocity, (ms / 1000)));
        }
    }
}

void Prince::draw(const mat3 &projection)
{
    animate();
    RenderManager::draw_texture(projection, &prince_texture, this, motion.position, motion.radians, physics.scale);
}

void Prince::animate()
{
    // total frames in the animation
    int total_frames = 4;
    // the index the frame starts at
    int frame_index;
    float animation_speed = 0.2f;

    // if is_moving
    if (len(m_velocity) > 0.f)
    {
        if (std::abs(m_velocity.x) > std::abs(m_velocity.y))
        {
            if (m_velocity.x < 0.f)
            {
                frame_index = 8;
            }
            else if (m_velocity.x > 0.f)
            {
                frame_index = 12;
            }
        }
        else
        {
            if (m_velocity.y > 0.f)
            {
                frame_index = 0;
            }
            else if (m_velocity.y < 0.f)
            {
                frame_index = 4;
            }
        }
        m_last_frame_index = frame_index;
        m_animation_time += animation_speed;
    }
    else
    {
        frame_index = m_last_frame_index;
    }

    // Apply animation
    frame_index = frame_index + (int)m_animation_time % total_frames;

    spritesheet.update_data(this, frame_index);
}

// When the prince moves to the next screen, reset the offset so that the prince
// doesn't appear too far on the edge of the screen
// pos corresponds to the direction that the prince is too far
// 0: too far up, 1: too far down, 2: too far left, 3: too far right,
// default: reset the prince's position to the top left corner
void Prince::reset_position(int pos)
{
    switch (pos)
    {
    case 0:
        motion.position = {motion.position.x, 750.f};
        break;
    case 1:
        motion.position = {motion.position.x, 50.f};
        break;
    case 2:
        motion.position = {1150.f, motion.position.y};
        break;
    case 3:
        motion.position = {50.f, motion.position.y};
        break;
    case 4:
        //setting init values for tutorial level;
        motion.position = {100.f, 355.f};
        break;
    default:
        motion.position = {150.f, 150.f};
        break;
    }
}

void Prince::move(vec2 off)
{
    motion.position.x += off.x;
    motion.position.y += off.y;
}

// Called when the prince collides with a enemy
void Prince::minus_live()
{
    --m_lives;
}

void Prince::velocity_to_zero(float ms)
{
    float fraction = 300.f;
    if (m_velocity.x > 0.f)
        m_velocity.x = std::max(m_velocity.x - fraction * ms / 1000, 0.f);
    else if (m_velocity.x < 0.f)
        m_velocity.x = std::min(m_velocity.x + fraction * ms / 1000, 0.f);
    if (m_velocity.y > 0.f)
        m_velocity.y = std::max(m_velocity.y - fraction * ms / 1000, 0.f);
    else if (m_velocity.y < 0.f)
        m_velocity.y = std::min(m_velocity.y + fraction * ms / 1000, 0.f);
}

// Called when the prince collides with a heart
void Prince::add_life()
{
    ++m_lives;
}

int Prince::get_lives()
{
    return m_lives;
}

bool Prince::is_alive() const
{
    return m_lives > 0;
}

bool Prince::is_extra_lives()
{
    return m_lives > 1;
}

void Prince::kill()
{
    m_lives = 0;
}

vec2 Prince::get_bounding_box() const
{
    // Returns the local bounding coordinates scaled by the current size of the prince
    // fabs is to avoid negative scale due to the facing direction.
    return {std::fabs(physics.scale.x) * m_sprite_width, std::fabs(physics.scale.y) * m_sprite_height};
}

void Prince::set_lives(int num)
{
    m_lives = num;
}
