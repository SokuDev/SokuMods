#pragma once

/* Character class. */
#define CF_UNKNOWN 0x7D0 // short		// Dash time (?)
#define CF_X_POS 0x0EC // float			// x pos
#define CF_Y_POS 0x0F0 // float			// y pos
#define CF_X_SPEED 0x0F4 // float		// x speed
#define CF_Y_SPEED 0x0F8 // float		// y speed
#define CF_GRAVITY 0x100 // float		// gravity
#define CF_DIR 0x104 // char			// direction
#define CF_COLOR_B 0x110 // uchar		// blue
#define CF_COLOR_G 0x111 // uchar		// green
#define CF_COLOR_R 0x112 // uchar		// red
#define CF_COLOR_A 0x113 // uchar		// alpha
#define CF_SHADER_TYPE 0x114 // int
#define CF_SHADER_COLOR_B 0x118 // uchar
#define CF_SHADER_COLOR_G 0x119 // uchar
#define CF_SHADER_COLOR_R 0x11A // uchar
#define CF_SHADER_COLOR_A 0x11B // uchar
#define CF_SCALE_X 0x11C // float
#define CF_SCALE_Y 0x120 // float
#define CF_Z_ROTATION 0x12C // float
#define CF_CURRENT_SEQ 0x13C // short		//Displays in which animation sequence you are
#define CF_CURRENT_SUBSEQ 0x13E // short	//???
#define CF_CURRENT_FRAME 0x140 // short		//???
#define CF_ELAPSED_IN_FRAME 0x142 // short
#define CF_ELAPSED_IN_SUBSEQ 0x144 // int	//Displays at which frame of the anim seq you are at
#define CF_CURRENT_FRAME_DATA 0x158 // ptr
#define CF_CURRENT_SEQ_FRAMES 0x15C // ptr
#define CF_ENEMY 0x170 // ptr
#define CF_CURRENT_HEALTH 0x184 // short
#define CF_HIT_STATE 0x190 // int
#define CF_HIT_COUNT 0x194 // char
#define CF_HIT_STOP 0x196 // short
#define CF_ATTACK_BOX_COUNT 0x1CB // char
#define CF_HURT_BOX_COUNT 0x1CC // char
#define CF_HURT_BOXES 0x1D0 // rect[5]
#define CF_ATTACK_BOXES 0x220 // rect[5]
#define CF_ATTACK_BOXES_ROT 0x320 // altbox[5]
#define CF_HURT_BOXES_ROT 0x334 // altbox[5]
#define CF_CHARACTER_INDEX 0x34C // char
#define CF_PLAYER_INDEX 0x350 // char		// index of the character in the enum below
#define CF_BULLET_COUNTER 0x36C // short
#define CF_CURRENT_SPIRIT 0x49E // short	// spirit from 0 to 1000 (divided in 5 orbs)
#define CF_SPIRIT_REGEN_DELAY 0x4A2 // short
#define CF_TIME_STOP 0x4A8 // short			//blockstun/hitstun ???
#define CF_UNTECH 0x4BA // short
#define CF_DAMAGE_LIMIT 0x4BE // short
#define CF_CARD_SLOTS 0x5E6 // char
#define CF_CARDS_ARRAY 0x5E8 // ptr			//register the array and load it ?
#define CF_SKILL_LEVELS_1 0x6A4 // char[32]
#define CF_SKILL_LEVELS_2 0x6C4 // char[32]
#define CF_OBJ_LIST_MGR 0x6F8 // ptr
#define CF_PRESSED_X_AXIS 0x754 // int
#define CF_PRESSED_Y_AXIS 0x758 // int
#define CF_PRESSED_A 0x75C // int
#define CF_PRESSED_B 0x760 // int
#define CF_PRESSED_C 0x764 // int
#define CF_PRESSED_D 0x768 // int
#define CF_PRESSED_AB 0x76C // int
#define CF_PRESSED_BC 0x770 // int
#define CF_PRESSED_X_AXIS1 0x774 // int
#define CF_PRESSED_Y_AXIS1 0x778 // int
#define CF_PRESSED_A_1 0x77C // int
#define CF_PRESSED_B_1 0x780 // int
#define CF_PRESSED_C_1 0x784 // int
#define CF_PRESSED_D_1 0x788 // int
#define CF_PRESSED_AB_1 0x78C // int
#define CF_PRESSED_BC_1 0x790 // int
#define CF_PRESSED_COMBINATION 0x7C8 // int
/*
PRESSED COMBINATION CODES: (B version, multiply by two for C version)
421: 139296
214: 32
22: 536870912
236: 2
623: 514
*/
#define CF_CHARGE_ATTACK 0x7F4 // char
#define CF_DAMAGE_LIMITED 0x7F7 // bool

#define CF_X_OFFSET 0x88C // short
#define CF_Y_OFFSET 0x88E // short

#define CF_SP_DOLL_COUNT 0x890 // short
#define CF_DOLL_COUNT 0x892 // short

/* Projectile class. */
#define PF_IS_ACTIVE 0x34C // int
#define PF_INIT_ARGS 0x35C // void *
#define PF_CUSTOM_FIELD 0x390 // int
#define PF_PARENT 0x398 // void *

/* ProjectileManager class. */
#define PMF_OBJ_PROJ_OFS 0x54 // int

/* Frame class. */
#define FF_DAMAGE 0x1C // short
#define FF_SPIRIT_DAMAGE 0x22 // short
#define FF_FFLAGS 0x4C // int
#define FF_AFLAGS 0x50 // int
#define FF_COLLISION_BOX 0x54 // rect<short>
#define FF_HURT_BOX_COUNT 0x5C // int
#define FF_HURT_BOXES 0x60 // rect<short>
#define FF_ATTACK_BOX_COUNT 0x5C // int
#define FF_ATTACK_BOXES 0x60 // rect<short>

/* Frame flags. */
#define FF_STAND 0x1
#define FF_CROUCH 0x2
#define FF_AIRBORNE 0x4
#define FF_DOWN 0x8
#define FF_GUARD_AVAILABLE 0x10
#define FF_CANCELLEABLE 0x20
#define FF_CH_ON_HIT 0x40
#define FF_SUPERARMOR 0x80
#define FF_EXTENDED_ARMOR 0x100
#define FF_GUARD_POINT 0x200
#define FF_GRAZE 0x400
#define FF_GUARDING 0x800
#define FF_GRAB_INVINCIBLE 0x1000
#define FF_MELEE_INVINCIBLE 0x2000
#define FF_PROJECTILE_INVINCIBLE 0x4000
#define FF_INV_AIRBORNE 0x8000
#define FF_INV_MID_BLOW 0x10000
#define FF_INV_LOW_BLOW 0x20000
#define FF_INV_SHOOT 0x40000
#define FF_REFLECTION_PROJECTILE 0x80000
#define FF_FLIP_VELOCITY 0x100000
#define FF_HIGH_JUMP_CANCELLABLE 0x200000
#define FF_UNK400000 0x400000
#define FF_UNK800000 0x800000
#define FF_ATK_AS_HIT 0x1000000

/* Attack flags. */
#define AF_MID_HIT 0x2
#define AF_LOW_HIT 0x4
#define AF_AIR_BLOCKABLE 0x8
#define AF_GRAZABLE 0x400000

/* Characters in order. */
enum {
	REIMU,
	MARISA,
	SAKUYA,
	ALICE,
	PATCHOULI,
	YOUMU,
	REMILIA,
	YUYUKO,
	YUKARI,
	SUIKA,
	REISEN,
	AYA,
	KOMACHI,
	IKU,
	TENSHI,
	SANAE,
	CIRNO,
	MEILING,
	UTSUHO,
	SUWAKO,
	NAMAZU
};

/* Accessors. */
#define ACCESS_TO_FIELD_IMP(object, offset) (reinterpret_cast<PBYTE>(object) + (offset))
#define ACCESS_FIELD(object, type, offset) *(reinterpret_cast<type *>(ACCESS_TO_FIELD_IMP(object, offset)))
#define ACCESS_INT(object, offset) ACCESS_FIELD(object, int, offset)
#define ACCESS_PTR(object, offset) ACCESS_FIELD(object, void *, offset)
#define ACCESS_CHAR(object, offset) ACCESS_FIELD(object, char, offset)
#define ACCESS_UCHAR(object, offset) ACCESS_FIELD(object, unsigned char, offset)
#define ACCESS_SHORT(object, offset) ACCESS_FIELD(object, short, offset)
#define ACCESS_FLOAT(object, offset) ACCESS_FIELD(object, float, offset)
#define FIELD_ADDRESS(object, offset) reinterpret_cast<void *>(ACCESS_TO_FIELD_IMP(object, offset))

template<typename Dest> typename std::decay_t<Dest> &GetField(void *object, unsigned int offset) {
	return *(reinterpret_cast<std::decay_t<Dest> *>(reinterpret_cast<PBYTE>(object) + (offset)));
}

template<typename Dest> void ReadField(void *object, unsigned int offset, Dest &output) {
	output = GetField<std::decay_t<Dest>>(object, offset);
}
