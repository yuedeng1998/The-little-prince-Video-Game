#pragma once

#include "common.hpp"

class SpriteSheet
{

public:

	void init(Texture* texture, const vec2 dimensions, Entity* entity);

	bool set_data(Entity* entity, int frame);

	void update_data(Entity* entity, int frame);

	Texture* texture;
	vec2 dimensions;

};