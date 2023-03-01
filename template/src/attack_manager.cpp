#include"attack_manager.hpp"

void AttackManager::enemies_update(vector<Enemy>* enemies, map<int, Bullet*> bullet_map, float m){
for (auto enemy : *enemies) {
		Bullet * bullet = bullet_map[enemy.get_key()];
		if (enemy.get_stop_and_attack()) {
			bullet->set_shooting(true);
			if (!bullet->get_runnning()) {
				bullet->set_position(enemy.get_position());
				bullet->set_rotation(enemy.get_angle());
			}
		}
		else {
		}
		bullet->update(m);
	}
}
