#include "spritesheet.hpp"

void SpriteSheet::init(Texture* texture, const vec2 dimensions, Entity* entity)
{
	// set texture and spritesheet dimensions
	this->texture = texture;
	this->dimensions = dimensions;

}

bool SpriteSheet::set_data(Entity* entity, int frame)
{
	float spritesheet_width = texture->width / dimensions.x;
	float spritesheet_height = texture->height / dimensions.y;
	float x_offset = 1 / dimensions.x;
	float y_offset = 1 / dimensions.y;

	float wr = spritesheet_width * 0.5f;
	float hr = spritesheet_height * 0.5;

	int x_frame = frame % (int) dimensions.x;
	int y_frame = (int)(frame / (int)dimensions.x % (int)dimensions.y);

	TexturedVertex vertices[4];

	vertices[0].position = { -wr, +hr, -0.01f };
	vertices[0].texcoord = { x_offset * x_frame, y_offset * (y_frame + 1) };
	vertices[1].position = { +wr, +hr, -0.01f };
	vertices[1].texcoord = { x_offset * (x_frame + 1), y_offset * (y_frame + 1) };
	vertices[2].position = { +wr, -hr, -0.01f };
	vertices[2].texcoord = { x_offset * (x_frame + 1), y_offset * y_frame };
	vertices[3].position = { -wr, -hr, -0.01f };
	vertices[3].texcoord = { x_offset * x_frame, y_offset * y_frame };

	// Counterclockwise as it's the default opengl front winding direction.
	uint16_t indices[] = { 0, 3, 1, 1, 3, 2 };

	// Clearing errors
	gl_flush_errors();

	// Vertex Buffer creation
	glGenBuffers(1, &entity->mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, entity->mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TexturedVertex) * 4, vertices, GL_STATIC_DRAW);

	// Index Buffer creation
	glGenBuffers(1, &entity->mesh.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, entity->mesh.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * 6, indices, GL_STATIC_DRAW);

	// Vertex Array (Container for Vertex + Index buffer)
	glGenVertexArrays(1, &entity->mesh.vao);
	if (gl_has_errors())
		return false;

	// Loading shaders
	if (!entity->effect.load_from_file(shader_path("textured.vs.glsl"), shader_path("textured.fs.glsl")))
		return false;

	return true;

}

void SpriteSheet::update_data(Entity* entity, int frame)
{
	float spritesheet_width = texture->width / dimensions.x;
	float spritesheet_height = texture->height / dimensions.y;
	float x_offset = 1 / dimensions.x;
	float y_offset = 1 / dimensions.y;

	float wr = spritesheet_width * 0.5f;
	float hr = spritesheet_height * 0.5;

	int x_frame = frame % (int)dimensions.x;
	int y_frame = (int)(frame / (int)dimensions.x % (int)dimensions.y);

	TexturedVertex vertices[4];

	vertices[0].position = { -wr, +hr, -0.01f };
	vertices[0].texcoord = { x_offset * x_frame, y_offset * (y_frame + 1) };
	vertices[1].position = { +wr, +hr, -0.01f };
	vertices[1].texcoord = { x_offset * (x_frame + 1), y_offset * (y_frame + 1) };
	vertices[2].position = { +wr, -hr, -0.01f };
	vertices[2].texcoord = { x_offset * (x_frame + 1), y_offset * y_frame };
	vertices[3].position = { -wr, -hr, -0.01f };
	vertices[3].texcoord = { x_offset * x_frame, y_offset * y_frame };

	gl_flush_errors();
	glBindBuffer(GL_ARRAY_BUFFER, entity->mesh.vbo);
	// glBufferSubData redefine some or all of the data store for the specified buffer object
	// (swap sprite texture)
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(TexturedVertex) * 4, vertices);
}

