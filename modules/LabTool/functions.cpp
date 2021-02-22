#include "stdafx.h"
#pragma warning(disable : 4996)
#define SWRS_USES_HASH
/* Keymap addresses of both players */
#define SWRS_ADDR_1PKEYMAP 0x00898940
#define SWRS_ADDR_2PKEYMAP 0x0089912C

// Personal header
#include "functions.h"

/* UPDATE */
void update_position(Player *player) {
	player->position.x = ACCESS_FLOAT(player->p, CF_X_POS);
	player->position.y = ACCESS_FLOAT(player->p, CF_Y_POS);
	player->position.xspeed = ACCESS_FLOAT(player->p, CF_X_SPEED);
	player->position.yspeed = ACCESS_FLOAT(player->p, CF_Y_SPEED);
	player->position.gravity = ACCESS_FLOAT(player->p, CF_GRAVITY);
	player->position.direction = (int)ACCESS_CHAR(player->p, CF_DIR);
}

void update_playerinfo(Player *player, int add_bmgr_px) {
	player->p = ACCESS_PTR(g_pbattleMgr, add_bmgr_px);
	player->index = (int)ACCESS_CHAR(player->p, CF_CHARACTER_INDEX);

	player->x_pressed = ACCESS_INT(player->p, CF_PRESSED_X_AXIS);
	player->y_pressed = ACCESS_INT(player->p, CF_PRESSED_Y_AXIS);
	update_position(player);

	player->health = ACCESS_SHORT(player->p, CF_CURRENT_HEALTH);
	player->spirit = ACCESS_SHORT(player->p, CF_CURRENT_SPIRIT);

	player->framedata = ACCESS_PTR(player->p, CF_CURRENT_FRAME_DATA);
	player->frameflag = ACCESS_INT(player->framedata, FF_FFLAGS);
	player->current_sequence = ACCESS_SHORT(player->p, CF_CURRENT_SEQ);
	player->elapsed_in_subseq = ACCESS_SHORT(player->p, CF_ELAPSED_IN_SUBSEQ);

	player->card = ACCESS_CHAR(player->p, CF_CARD_SLOTS);
	player->untech = ACCESS_SHORT(player->p, CF_UNTECH);
}

/* POSITION */
Position init_pos(float custom_x) {
	Position pos;
	pos.x = custom_x;
	pos.y = 0.0f;
	pos.xspeed = 0.0f;
	pos.yspeed = 0.0f;
	pos.gravity = 0.0f;

	return pos;
}

Position custom_pos = init_pos(480.0f);
Position custom_pos2 = init_pos(800.0f);
Position LC_P1 = init_pos(LEFT_CORNER_P1);
Position LC_P2 = init_pos(LEFT_CORNER_P2);
Position LN_P1 = init_pos(LEFT_NEAR_P1);
Position LN_P2 = init_pos(LEFT_NEAR_P2);
Position MID_P1 = init_pos(MIDSCREEN_P1);
Position MID_P2 = init_pos(MIDSCREEN_P2);
Position RN_P1 = init_pos(RIGHT_NEAR_P1);
Position RN_P2 = init_pos(RIGHT_NEAR_P2);
Position RC_P1 = init_pos(RIGHT_CORNER_P1);
Position RC_P2 = init_pos(RIGHT_CORNER_P2);

Position save_checkpoint(Player *player) {
	Position pos;

	pos.x = player->position.x;
	pos.y = player->position.y;
	pos.xspeed = player->position.xspeed;
	pos.yspeed = player->position.yspeed;
	pos.gravity = player->position.gravity;

	return pos;
}

void set_position(Player *player, Position pos, int mode) {
	ACCESS_FLOAT(player->p, CF_X_POS) = pos.x;
	ACCESS_FLOAT(player->p, CF_Y_POS) = pos.y;

	if (mode == NONE) {
		ACCESS_FLOAT(player->p, CF_X_SPEED) = 0.0f;
		ACCESS_FLOAT(player->p, CF_Y_SPEED) = 0.0f;
	} else if (mode == CONSERVED) // will move you in the opposite x if you change sides.
	{
		ACCESS_FLOAT(player->p, CF_X_SPEED) = pos.xspeed;
		ACCESS_FLOAT(player->p, CF_Y_SPEED) = pos.yspeed;
	} else if (mode == CONTINUED) {
		// not much happens
	}
}

void position_management(Player *p1, Player *p2) {
	if (toggle_keys.save_pos) {
		custom_pos = save_checkpoint(p1);
		custom_pos2 = save_checkpoint(p2);
		std::cout << std::endl
							<< " P1 position checkpoint: (" << std::fixed << std::setprecision(2) << custom_pos.x << ", " << std::fixed << std::setprecision(2)
							<< custom_pos.y << ")" << std::endl
							<< " P2 position checkpoint: (" << std::fixed << std::setprecision(2) << custom_pos2.x << ", " << std::fixed << std::setprecision(2)
							<< custom_pos2.y << ")" << std::endl
							<< std::endl;

		toggle_keys.save_pos = !toggle_keys.save_pos;
	}

	if (toggle_keys.reset_pos) {
		if (p1->y_pressed > 0) // DOWN
		{
			set_position(p1, MID_P1, NONE);
			set_position(p2, MID_P2, NONE);

			if (p1->x_pressed < 0) // LEFT+DOWN
			{
				set_position(p1, LN_P1, NONE);
				set_position(p2, LN_P2, NONE);
			} else if (p1->x_pressed > 0) // RIGHT+DOWN
			{
				set_position(p1, RN_P1, NONE);
				set_position(p2, RN_P2, NONE);
			}
		} else {
			if (p1->x_pressed < 0) // LEFT
			{
				set_position(p1, LC_P1, NONE);
				set_position(p2, LC_P2, NONE);
			} else if (p1->x_pressed > 0) // RIGHT
			{
				set_position(p1, RC_P1, NONE);
				set_position(p2, RC_P2, NONE);
			} else // NOTHING
			{
				if (custom_pos.y > 0) {
					ACCESS_SHORT(p1->p, CF_CURRENT_SEQ) = 9;
				}
				if (custom_pos2.y > 0) {
					ACCESS_SHORT(p2->p, CF_CURRENT_SEQ) = 9;
				} // ISSUE: find a way to reset the character to a air animation

				if ((custom_pos.x < custom_pos2.x && p1->position.x > p2->position.x) || (custom_pos.x > custom_pos2.x && p1->position.x < p2->position.x)) {
					p1->position.xspeed = -p1->position.xspeed;
				} // ISSUE: save the direction from ground to air reset

				set_position(p1, custom_pos, CONSERVED);
				set_position(p2, custom_pos2, CONSERVED);
			}
		}
		toggle_keys.reset_pos = !toggle_keys.reset_pos;
	}
}

/* FRAMECOUNT */
int firstToIdle;
std::string pos_or_neg;
void frameadvantage_count(Player *p1, Player *p2) {
	if (p1->current_sequence > 10 && p2->current_sequence > 10) {
		misc_states.frame_advantage = 0;
		misc_states.blockstring = true;
		pos_or_neg = "";
	}
	if ((p1->current_sequence <= 10 || p2->current_sequence <= 10) && misc_states.blockstring) {
		if (p1->current_sequence <= 10 && p2->current_sequence <= 10) {
			misc_states.blockstring = false;
			std::cout << " P1 is " << pos_or_neg << misc_states.frame_advantage << "F" << std::endl << std::endl;
		}
		if (pos_or_neg == "") {
			if (p1->current_sequence <= 10) {
				pos_or_neg = "+";
			} else if (p2->current_sequence <= 10) {
				pos_or_neg = "-";
			}
		}

		++misc_states.frame_advantage;
	}
}

void hjcadvantage_count(Player *p1, Player *p2) {
	if (p1->current_sequence > 10 && p2->current_sequence > 10 && !misc_states.hjc_blockstring) { // there was a frame both were acting, we entered a blockstring
		misc_states.hjc_advantage = 0;
		misc_states.hjc_blockstring = true;
	} else if (p1->current_sequence <= 10 && p2->current_sequence <= 10 && misc_states.hjc_blockstring) { // both went back to idle
		misc_states.hjc_advantage = 0;
		misc_states.hjc_blockstring = false;
	}
	if ((p1->current_sequence != 204 && p1->current_sequence >= 201 && p1->current_sequence <= 215)
		&& misc_states.hjc_blockstring) { // p2 is still blocking, p1 is high air

		if (p2->current_sequence < 10) { // if p2 now recovered, we display
			misc_states.hjc_blockstring = false;

			if (ACCESS_SHORT(p1->p, CF_ELAPSED_IN_SUBSEQ) < ACCESS_SHORT(p2->p, CF_ELAPSED_IN_SUBSEQ))
				std::cout << " P1 is [-" << misc_states.hjc_advantage << "F]" << std::endl << std::endl;
			else
				std::cout << " P1 is [+" << misc_states.hjc_advantage << "F]" << std::endl << std::endl;
		}
		++misc_states.hjc_advantage;
	}
}

void gap_count(Player *player) {
	if (player->frameflag & FF_GUARDING || !(player->frameflag & FF_GUARD_AVAILABLE)) {
		if (misc_states.isIdle != -1) {
			++misc_states.isIdle;
			if (misc_states.isIdle <= 30)
				std::cout << " Gap: " << misc_states.isIdle << "F" << std::endl;
		}
		misc_states.isIdle = -1;
	} else
		++misc_states.isIdle;
}

bool untight_check(Player *player) {
	// RIGHTBLOCK
	if (player->elapsed_in_subseq == VERYLIGHT_RB_TIME - 1 && (player->current_sequence == STAND_VL_RB || player->current_sequence == CROUCH_VL_RB))
		return true;
	else if (player->elapsed_in_subseq == LIGHT_RB_TIME - 1 && (player->current_sequence == STAND_L_RB || player->current_sequence == CROUCH_L_RB))
		return true;
	else if (player->elapsed_in_subseq == MEDIUM_RB_TIME - 1 && (player->current_sequence == STAND_M_RB || player->current_sequence == CROUCH_M_RB))
		return true;
	else if (player->elapsed_in_subseq == HEAVY_RB_TIME - 1 && (player->current_sequence == STAND_H_RB || player->current_sequence == CROUCH_H_RB))
		return true;
	// AIRBLOCK
	else if (player->elapsed_in_subseq == AIR_B_TIME - 1 && player->current_sequence == AIRBLOCK)
		return true;
	// WRONGBLOCK
	else if (player->elapsed_in_subseq == VERYLIGHT_WB_TIME - 1 && player->current_sequence == STAND_VL_WB)
		return true;
	else if (player->elapsed_in_subseq == LIGHT_WB_TIME - 1 && (player->current_sequence == STAND_L_WB || player->current_sequence == CROUCH_L_WB))
		return true;
	else if (player->elapsed_in_subseq == MEDIUM_WB_TIME - 1 && (player->current_sequence == STAND_M_WB || player->current_sequence == CROUCH_M_WB))
		return true;
	else
		return false;
}

void is_tight(Player *player) {
	if (player->current_sequence >= 143 && player->current_sequence <= 165) {
		if (misc_states.untight_nextframe) { // bug in which the first frame of the first hit is considered untight
			std::cout << " Gap: 0F (mashable)" << std::endl;
		}

		misc_states.untight_nextframe = false;
		misc_states.untight_nextframe = untight_check(player);
	} else {
		misc_states.untight_nextframe = false;
	}
}

/* MISCELLANEOUS */
void state_display(Player *player) {
	int transparency = -1;
	int red = -1;
	int blue = -1;
	int green = -1;

	if (toggle_keys.display_states) {
		if (player->frameflag & FF_GRAZE)
			transparency = 100;
		if (player->frameflag & FF_MELEE_INVINCIBLE) {
			red = 60;
			green = 60;
		} else if (player->frameflag & FF_CH_ON_HIT) {
			blue = 60;
			green = 60;
		} else if (!(player->frameflag & FF_GUARD_AVAILABLE)) {
			red = 148;
			green = 0;
			blue = 211;
		}

		ACCESS_CHAR(player->p, CF_COLOR_R) = red;
		ACCESS_CHAR(player->p, CF_COLOR_B) = blue;
		ACCESS_CHAR(player->p, CF_COLOR_G) = green;
		ACCESS_CHAR(player->p, CF_COLOR_A) = transparency;
	} else {
		ACCESS_CHAR(player->p, CF_COLOR_R) = red;
		ACCESS_CHAR(player->p, CF_COLOR_B) = blue;
		ACCESS_CHAR(player->p, CF_COLOR_G) = green;
		ACCESS_CHAR(player->p, CF_COLOR_A) = transparency;
	}
}

void set_health(Player *player, short health) {
	ACCESS_SHORT(player->p, CF_CURRENT_HEALTH) = health;
}

void set_spirit(Player *player, short spirit, short delay) {
	ACCESS_SHORT(player->p, CF_CURRENT_SPIRIT) = spirit; // 1000 divided by 200
	ACCESS_SHORT(player->p, CF_SPIRIT_REGEN_DELAY) = delay;
}

void reset_skills(Player *player) {
	int nb_skills = 4;

	if (ACCESS_CHAR(player->p, CF_CHARACTER_INDEX) == PATCHOULI)
		nb_skills = 5;

	if (toggle_keys.reset_skills) {
		for (int i = 0; i < nb_skills; ++i) {
			ACCESS_CHAR(player->p, CF_SKILL_LEVELS_1 + i) = 0;
			ACCESS_CHAR(player->p, CF_SKILL_LEVELS_2 + i) = 0;
		}

		for (int i = nb_skills; i < 32; ++i) {
			ACCESS_CHAR(player->p, CF_SKILL_LEVELS_1 + i) = -1;
			ACCESS_CHAR(player->p, CF_SKILL_LEVELS_2 + i) = -1;
		}

		toggle_keys.reset_skills = !toggle_keys.reset_skills;
		std::cout << " Skills have been reset to default and lv0." << std::endl;
	}
}
