// Header
#include "heart_grey.hpp"

#include <cmath>

Texture Heart_Grey::heart_grey_texture;

bool Heart_Grey::init()
{
	// Load shared texture
	const char *texturePath = textures_path("heart_grey.png");
	if (!RenderManager::load_texture(texturePath, &heart_grey_texture, this))
	{
		return false;
	}

	if (!RenderManager::set_buffers(&heart_grey_texture, this, -0.02f, vec2{1, 1}))
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
	return true;
}

// Releases all graphics resources
void Heart_Grey::destroy()
{
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteVertexArrays(1, &mesh.vao);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);
}

void Heart_Grey::update(float ms)
{
}

void Heart_Grey::draw(const mat3 &projection)
{
	RenderManager::draw_texture(projection, &heart_grey_texture, this, motion.position, motion.radians, physics.scale);
}

void Heart_Grey::set_position(vec2 position)
{
	motion.position = position;
}

void Heart_Grey::set_scale(vec2 newscale)
{
	physics.scale = newscale;
}
