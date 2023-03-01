#pragma once
#include "common.hpp"

class RenderManager : public Entity
{

public:
	bool static load_texture(const char *texturePath, Texture *texture, Entity *entity);

	void static draw_texture(const mat3 &projection, Texture *texture, Entity *entity, vec2 position, float rotation, vec2 scale);

	bool static set_buffers(Texture *texture, Entity *entity, float zvalue, vec2 offset);
};
