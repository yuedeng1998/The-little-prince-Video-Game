// Header
#include "heart.hpp"

#include <cmath>

Texture Heart::heart_texture;

bool Heart::init()
{
	// Load shared texture
	const char *texturePath = textures_path("heart.png");
	if (!RenderManager::load_texture(texturePath, &heart_texture, this))
	{
		return false;
	}

	if (!RenderManager::set_buffers(&heart_texture, this, -0.02f, vec2{1, 1}))
	{
		return false;
	}

	// Loading shaders
	if (!effect.load_from_file(shader_path("textured.vs.glsl"), shader_path("textured.fs.glsl")))
		return false;

	motion.radians = 0.f;
	motion.speed = 380.f;

	// Setting initial values, scale is negative to make it face the opposite way
	// 1.0 would be as big as the original texture.
	// TODO: size is not consistent for the UI and heart being rendered
	physics.scale = {-0.1f, 0.1f};
	eaten = 0;

	// <<<<<<< HEAD
	is_collected = false;
	// =======
	return true;
}

// Releases all graphics resources
void Heart::destroy()
{
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteVertexArrays(1, &mesh.vao);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);
}

void Heart::update(float ms)
{
	//dy playable

	if (eaten != 0)
	{
		vec2 scale = vec2{(float)0.02 * eaten, (float)0.02 * eaten};
		eaten = ms / 5 + 1;
		if ((int)eaten % 10 == 0)
		{
			eaten = 1;
		}
		eaten = (int)eaten % 10;
		set_scale(scale);
	}
}

void Heart::draw(const mat3 &projection)
{
	RenderManager::draw_texture(projection, &heart_texture, this, motion.position, motion.radians, physics.scale);
}

vec2 Heart::get_position() const
{
	return motion.position;
}

void Heart::set_position(vec2 position)
{
	motion.position = position;
}

vec2 Heart::get_bounding_box() const
{
	// Returns the local bounding coordinates scaled by the current size of the heart
	// fabs is to avoid negative scale due to the facing direction.
	return {std::fabs(physics.scale.x) * heart_texture.width, std::fabs(physics.scale.y) * heart_texture.height};
}

void Heart::set_scale(vec2 newscale)
{
	physics.scale = newscale;
}

void Heart::set_eaten(int e)
{
	eaten = e;
	if (e == 0)
	{
		is_collected = false;
	}
	else
	{
		is_collected = true;
	}
}
