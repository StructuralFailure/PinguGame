#include "Collision.h"



void Collision_player_enemy(Entity* player, Entity* enemy) {
	/* this will get called twice in the collision loop.
	 * we only want it to run once.
         */
	static unsigned long ticks_last_handled = 0;
	unsigned long ticks = player->world->ticks;

	if (ticks == ticks_last_handled) {
		return;
	}

	/* check collision side and act accordingly. */


	ticks_last_handled = ticks;
}