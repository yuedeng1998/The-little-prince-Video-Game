#include "game_object.hpp"
//#include "collision_manager.hpp"

bool GameObject::init()
{
	//CollisionManager::get_instance().register_collision_game_object(this);
	return true;
}
/*
void GameObject::register_game_object(GameObject* gameObject) {
	m_game_objects.insert(gameObject);
}*/

void GameObject::destroy()
{
	//CollisionManager::get_instance().unregister_collision_game_object(this);
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteVertexArrays(1, &mesh.vao);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);
}

vec2 GameObject::get_position() const
{
	return motion.position;
}

void GameObject::set_position(vec2 position)
{
	motion.position = position;
}

void GameObject::set_rotation(float radians)
{
	motion.radians = radians;
}
/*
vec2 GameObject::get_bounding_box() const
{
	// Returns the local bounding coordinates scaled by the current size of the prince
	// fabs is to avoid negative scale due to the facing direction.
	return { std::fabs(physics.scale.x) * m_texture->width, std::fabs(physics.scale.y) * m_texture->height };
}*/