
// Header
#include "walls.hpp"

#include <cmath>
#include <string>

Texture Walls::walls_texture;
Texture Walls::trees_texture;
Texture Walls::water_texture;
bool Walls::change_textures;

bool Walls::init(int n, int current_level)
{
	std::string dir_path = data_path "/textures/";
	std::string basic_path = dir_path + std::to_string(current_level) + "/";

	std::string wall_path = basic_path + "wall.png";
	const char *wall = wall_path.c_str();

	std::string tree_path = basic_path + "tree.png";
	const char *tree = tree_path.c_str();

	std::string water_path = basic_path + "water.png";
	const char *water = water_path.c_str();

	if (n == 1)
	{
		t_t = 1;
		if (!walls_texture.is_valid() || change_textures)
		{
			if (!walls_texture.load_from_file(wall))
			{
				fprintf(stderr, "Failed to load walls texture!");
				return false;
			}
		}
	}
	else if (n == 2)
	{
		t_t = 2;
		if (!trees_texture.is_valid() || change_textures)
		{
			if (!trees_texture.load_from_file(tree))
			{
				fprintf(stderr, "Failed to load walls texture!");
				return false;
			}
		}
		m_particles.init();
		pop.init();
	}
	else if (n == 3)
	{
		t_t = 3;
		if (!water_texture.is_valid() || change_textures)
		{
			if (!water_texture.load_from_file(water))
			{

				fprintf(stderr, "Failed to load walls texture!");
				return false;
			}
		}
	}

	if (!RenderManager::set_buffers(&walls_texture, this, -0.01f, vec2{1, 1}))
	{
		return false;
	}

	// Loading shaders
	if (!effect.load_from_file(shader_path("textured.vs.glsl"), shader_path("textured.fs.glsl")))
		return false;

	motion.radians = 0.0f;
	motion.speed = 0.0f;

	// Setting initial values, scale is negative to make it face the opposite way
	// 1.0 would be as big as the original texture.
	physics.scale = {-1.0f, 1.2f};
	m_particles.init();
	checked = false;
	return true;
}

// Releases all graphics resources
void Walls::destroy()
{
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteVertexArrays(1, &mesh.vao);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);
}

void Walls::update(float ms)
{
	if (t_t == 2)
	{
		m_particles.update(ms);
		pop.update(ms);
	}
}

void Walls::draw(const mat3 &projection)
{
	if (t_t == 1)
	{
		RenderManager::draw_texture(projection, &walls_texture, this, motion.position, motion.radians, physics.scale);
	}
	else if (t_t == 3)
	{
		glBindTexture(GL_TEXTURE_2D, water_texture.id);
		RenderManager::draw_texture(projection, &water_texture, this, motion.position, motion.radians, physics.scale);
	}
	else if (t_t == 2)
	{
		glBindTexture(GL_TEXTURE_2D, trees_texture.id);
		RenderManager::draw_texture(projection, &trees_texture, this, motion.position, motion.radians, physics.scale);
		m_particles.draw(projection);
		pop.draw(projection);
	}
}

vec2 Walls::get_position() const
{
	return motion.position;
}

void Walls::set_position(vec2 position)
{
	motion.position = position;
}

vec2 Walls::get_bounding_box() const
{
	// Returns the local bounding coordinates scaled by the current size of the star
	// fabs is to avoid negative scale due to the facing direction.
	return {std::fabs(physics.scale.x) * walls_texture.width, std::fabs(physics.scale.y) * walls_texture.height};
}

void Walls::set_texture_change()
{
	change_textures = true;
}

void Walls::unset_texture_change()
{
	change_textures = false;
}

bool Walls::is_brarrel()
{
	return t_t == 2;
}

bool Walls::exploding()
{
	m_particles.spawn_pebble(motion.position, 5);
	pop.set_position(vec2{500, 500});
	pop.show_window();
	pop.set_empty();
	return true;
}

int Walls::get_points()
{
	if (t_t != 2)
	{
		return 0;
	}
	if (!checked)
	{
		checked = true;
		return pop.get_points();
	}
	return 0;
}