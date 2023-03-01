#pragma once

#include "common.hpp"
#include "render_manager.hpp"
//#include "collision_manager.hpp"

#include <vector>
#include <iostream>
#include <set>

class GameObject : public Entity 
{
public: 
	virtual ~GameObject() { GameObject::destroy(); };
	
	virtual bool init();

	// register GameObject
	//void register_game_object(GameObject* gameObject);

	virtual void destroy();
	
	// subclasses instantiate the instance
	//void draw(const mat3& projection) = 0;

	// Returns the current GameObject's position
	vec2 get_position() const;

	// Sets the new GameObject's position
	void set_position(vec2 position);

	// Sets the current GameObject's rotation
	void set_rotation(float radians);

	// Returns the local bounding coordinates scaled by the current size of the texture
	// fabs is to avoid negative scale due to the facing direction.
	//vec2 get_bounding_box() const;

protected:
	// pointer to the current texture
	//Texture* m_texture;

	std::set<GameObject*> m_game_objects;
};