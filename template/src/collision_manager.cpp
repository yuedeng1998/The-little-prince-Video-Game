#include "collision_manager.hpp"
#include "game_object.hpp"
#include "prince.hpp"
#include "star.hpp"
#include "walls.hpp"
#include "heart.hpp"
#include "enemy.hpp"
#include "key.hpp"
#include "portal.hpp"
#include "common.hpp"
#include "bullet.hpp"

#include <string>
#include <algorithm>
#include <cmath>

// Simple bounding box collision check
// This is a SUPER APPROXIMATE check that puts a circle around the bounding boxes and sees
// if the center point of either object is inside the other's bounding-box-circle. You don't
// need to try to use this technique.
bool CollisionManager::prince_collides_with_enemy(Prince *prince, Enemy *enemy)
{
    float dx = prince->motion.position.x - enemy->get_position().x;
    float dy = prince->motion.position.y - enemy->get_position().y;
    float d_sq = dx * dx + dy * dy;
    float other_r = std::max(enemy->get_bounding_box().x, enemy->get_bounding_box().y);
    float my_r = std::max(prince->physics.scale.x, prince->physics.scale.y);
    float r = std::max(other_r, my_r);
    r *= 0.6f;
    if (d_sq < r * r)
    {
        return true;
    }
    return false;
}

bool CollisionManager::prince_collides_with_portal(Prince *prince, Portal &portal)
{
    vec2 bb_tl = sub(portal.get_position(), portal.get_bounding_box());
    vec2 bb_br = add(portal.get_position(), portal.get_bounding_box());
    bool is_in_bb = (portal.get_position().x > bb_tl.x) && (portal.get_position().y > bb_tl.y) &&
                    (portal.get_position().x < bb_br.x) && (portal.get_position().y < bb_br.y);
    if (is_in_bb)
    {
        float max = std::numeric_limits<float>::max();
        float min = std::numeric_limits<float>::min();
        vec2 top = {max, max}, bottom = {min, min}, left = {max, max}, right = {min, min};
        vec2 offset = {50.f, 50.f};
        portal.get_tblr_points(&top, &bottom, &left, &right);
        vec2 prince_bb_tl = sub(prince->get_position(), prince->get_bounding_box());
        prince_bb_tl = add(prince_bb_tl, offset);
        float pl = prince_bb_tl.x, pt = prince_bb_tl.y;
        vec2 prince_bb_br = add(prince->get_position(), prince->get_bounding_box());
        prince_bb_br = sub(prince_bb_br, offset);
        float pr = prince_bb_br.x, pb = prince_bb_br.y;

        if (left.x > pl && left.x < pr && left.y > pt && left.y < pb ||
            right.x > pl && right.x < pr && right.y > pt && right.y < pb ||
            top.x > pl && top.x < pr && top.y > pt && top.y < pb ||
            bottom.x > pl && bottom.x < pr && bottom.y > pt && bottom.y < pb)
        {
            return true;
        }
    }
    return false;
}

bool CollisionManager::prince_collides_with_heart(Prince *prince, Heart &heart)
{
    float dx = prince->motion.position.x - heart.get_position().x;
    float dy = prince->motion.position.y - heart.get_position().y;
    float d_sq = dx * dx + dy * dy;
    float other_r = std::max(heart.get_bounding_box().x, heart.get_bounding_box().y);
    float my_r = std::max(prince->physics.scale.x, prince->physics.scale.y);
    float r = std::max(other_r, my_r);
    r *= 0.9f;
    if (d_sq < r * r)
        return true;
    return false;
}

bool CollisionManager::prince_collides_with_key(Prince *prince, Key &key)
{
    float dx = prince->motion.position.x - key.get_position().x;
    float dy = prince->motion.position.y - key.get_position().y;
    float d_sq = dx * dx + dy * dy;
    float other_r = std::max(key.get_bounding_box().x, key.get_bounding_box().y);
    float my_r = std::max(prince->physics.scale.x, prince->physics.scale.y);
    float r = std::max(other_r, my_r);
    r *= 0.6f;
    if (d_sq < r * r)
        return true;
    return false;
}

bool CollisionManager::prince_collides_with_star(Prince *prince, Star &star)
{
    vec2 prince_bounding_box = prince->get_bounding_box();
    vec2 star_bounding_box = star.get_bounding_box();
    vec2 star_pos = star.get_position();

    float prince_off_set_x = prince_bounding_box.x / 2;
    float prince_off_set_y = prince_bounding_box.y / 2;

    float prince_right = prince->motion.position.x + prince_off_set_x;
    float prince_left = prince->motion.position.x - prince_off_set_x;
    float prince_top = prince->motion.position.y - prince_off_set_y;
    float prince_bottom = prince->motion.position.y + prince_off_set_y;

    float star_off_set_x = star_bounding_box.x / 2;
    float star_off_set_y = star_bounding_box.y / 2;

    float star_right = star_pos.x + star_off_set_x;
    float star_left = star_pos.x - star_off_set_x;
    float star_top = star_pos.y - star_off_set_y;
    float star_bottom = star_pos.y + star_off_set_y;

    if (prince_top <= star_bottom && prince_bottom >= star_top)
    {
        if (prince_left <= star_right && prince_right >= star_left)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool CollisionManager::prince_collides_with_wall(Prince *prince, Walls &wall, float ms) {
    vec2 nvel = add(prince->m_velocity, mul(prince->motion.acceleration, ms / 1000));
    if (len(prince->motion.acceleration) > 0.001f) {
        if (len(nvel) > prince->motion.max_speed)
            nvel = mul(normalize(nvel), prince->motion.max_speed);
    }
    vec2 npos = add(prince->motion.position, mul(nvel, (ms / 1000)));


    vec2 prince_bounding_box = prince->get_bounding_box();
    vec2 wall_bounding_box = wall.get_bounding_box();
    vec2 wall_pos = wall.get_position();

    float prince_off_set_x = prince_bounding_box.x / 2;
    float prince_off_set_y = prince_bounding_box.y / 2;

    float prince_right = npos.x + prince_off_set_x;
    float prince_left = npos.x - prince_off_set_x;
    float prince_top = npos.y - prince_off_set_y;
    float prince_bottom = npos.y + prince_off_set_y;

    float wall_off_set_x = wall_bounding_box.x / 2;
    float wall_off_set_y = wall_bounding_box.y / 2;

    float wall_right = wall_pos.x + wall_off_set_x;
    float wall_left = wall_pos.x - wall_off_set_x;
    float wall_top = wall_pos.y - wall_off_set_y;
    float wall_bottom = wall_pos.y + wall_off_set_y;

    if (prince_top <= wall_bottom && prince_bottom >= wall_top)
    {
        if (prince_left <= wall_right && prince_right >= wall_left)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool CollisionManager::prince_collides_with_bullet(Prince *prince, Bullet *bullet)
{
    float dx = prince->motion.position.x - bullet->get_position().x;
    float dy = prince->motion.position.y - bullet->get_position().y;
    float d_sq = dx * dx + dy * dy;
    float other_r = std::max(bullet->get_bounding_box().x, bullet->get_bounding_box().y);
    float my_r = std::max(prince->physics.scale.x, prince->physics.scale.y);
    float r = std::max(other_r, my_r);
    r *= 0.6f;
    if (d_sq < r * r)
        return true;
    return false;
}

bool CollisionManager::bullet_collides_with_wall(Bullet *bullet, Walls &wall)
{
    vec2 bullet_bounding_box = bullet->get_bounding_box();
    vec2 wall_bounding_box = wall.get_bounding_box();
    vec2 wall_pos = wall.get_position();

    float bullet_off_set_x = bullet_bounding_box.x / 2;
    float bullet_off_set_y = bullet_bounding_box.y / 2;

    float bullet_right = bullet->motion.position.x + bullet_off_set_x;
    float bullet_left = bullet->motion.position.x - bullet_off_set_x;
    float bullet_top = bullet->motion.position.y - bullet_off_set_y;
    float bullet_bottom = bullet->motion.position.y + bullet_off_set_y;

    float wall_off_set_x = wall_bounding_box.x / 2;
    float wall_off_set_y = wall_bounding_box.y / 2;

    float wall_right = wall_pos.x + wall_off_set_x;
    float wall_left = wall_pos.x - wall_off_set_x;
    float wall_top = wall_pos.y - wall_off_set_y;
    float wall_bottom = wall_pos.y + wall_off_set_y;

    if (bullet_top <= wall_bottom && bullet_bottom >= wall_top)
    {
        if (bullet_left <= wall_right && bullet_right >= wall_left)
        {
            if (wall.is_brarrel())
                wall.exploding();
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool CollisionManager::bullet_collides_with_boundry(Bullet *bullet)
{
    vec2 bullet_bounding_box = bullet->get_bounding_box();

    float bullet_off_set_x = bullet_bounding_box.x / 2;
    float bullet_off_set_y = bullet_bounding_box.y / 2;

    float bullet_right = bullet->motion.position.x + bullet_off_set_x;
    float bullet_left = bullet->motion.position.x - bullet_off_set_x;
    float bullet_top = bullet->motion.position.y - bullet_off_set_y;
    float bullet_bottom = bullet->motion.position.y + bullet_off_set_y;
    return bullet_top < 0 || bullet_bottom > 800 || bullet_left < 0 || bullet_right > 1200;
}

bool CollisionManager::bullet_collides_with_enemy(Bullet &bullet, Enemy &enemy)
{
    float dx = bullet.motion.position.x - enemy.get_position().x;
    float dy = bullet.motion.position.y - enemy.get_position().y;
    float d_sq = dx * dx + dy * dy;
    float other_r = std::max(enemy.get_bounding_box().x, enemy.get_bounding_box().y);
    float my_r = std::max(bullet.physics.scale.x, bullet.physics.scale.y);
    float r = std::max(other_r, my_r);
    r *= 0.6f;
    if (d_sq < r * r)
        return true;
    return false;
}

/*
void CollisionManager::register_collision_game_object(GameObject* gameObject)
{
	if (registeredGameObjects.find(gameObject) != registeredGameObjects.end())
	{
		unregister_collision_game_object(gameObject);
	}
	registeredGameObjects.insert(gameObject);
}

// unregister game object in Collision Manager
void CollisionManager::unregister_collision_game_object(GameObject* gameObject) 
{
	registeredGameObjects.erase(gameObject);
}

// return registered game objects
std::set<GameObject*> CollisionManager::get_collision_game_objects()
{
	return registeredGameObjects;
}*/
