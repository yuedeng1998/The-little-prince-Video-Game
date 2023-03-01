#pragma once
#include <set>
#include <vector>
#include <iostream>
#include <map>
#include "common.hpp"
#include "prince.hpp"
#include "enemy.hpp"
#include "bullet.hpp"

using namespace std;

class AttackManager {
public:
	void static enemies_update(vector<Enemy>* enemies, map<int, Bullet*> bullet_map, float m);
};