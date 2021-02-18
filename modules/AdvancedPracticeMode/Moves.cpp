//
// Created by PinkySmile on 18/02/2021.
//

#include <array>
#include "Moves.hpp"

namespace Practice
{
	std::array<std::array<std::string, 16>, SokuLib::CHARACTER_RANDOM> movesNames{{
		{ //CHARACTER_REIMU
			"Hakurei Amulet",
			"Cautionary Border",
			"Dimensional Rift",
			"Ascension Kick",

			"Youkai Buster",
			"Binding Border",
			"Demon Sealing Dimensional Rift",
			"Sliding Ascension Kick",

			"Spread Amulet",
			"Permanent Border",
			"Instant Dimensional Rift",
			"Rain Dance"
		},
		{ //CHARACTER_MARISA
			"Witch Leyline",
			"Miasma Sweep",
			"Grand Stardust",
			"Meteonic Debris",

			"Radial Strike",
			"Bosky Sweeper",
			"Devildom Torch",
			"Narrow Spark",

			"Up Sweeper",
			"Stellar Missile",
			"Magical Reusable Bomb",
			"Green Spread",
		},
		{ //CHARACTER_SAKUYA
			"Crossup Magic",
			"Bounce No-bounce",
			"Magic Star Sword",
			"Vanishing Everything",

			"Propelling Silver",
			"Square Ricochet",
			"Sense of Thrown Edge",
			"Perfect Maid",

			"Dancing Star Sword",
			"Misdirection",
			"Parallel Brain",
			"Time Paradox",
		},
		{ //CHARACTER_ALICE
			"Doll Placement",
			"Volatile Doll",
			"Doll Arrangement",
			"Doll Activation",

			"Returning Dolls",
			"Doll Cremation",
			"Thousand Spear Dolls",
			"SP Doll",

			"Doll Ambush",
			"Edo High-Explosive Pawn",
			"Doll Archers",
			"Seeker Wire",
		},
		{ //CHARACTER_PATCHOULI
			"Summer Red",
			"Winter Element",
			"Spring Wind",
			"Autumn Edge",
			"Dew Spear",

			"Summer Flame",
			"Condensed Bubble",
			"Flash of Spring",
			"Autumn Blade",
			"Emerald City",

			"Wipe Moisture",
			"Sticky Bubbles",
			"Static Green",
			"Force Lasher",
			"Diamond Hardness",
		},
		{ //CHARACTER_YOUMU
			"Netherworld Reflection Slash",
			"Crescent Moon Slash",
			"Slash of Life and Death",
			"Medium's Bind",

			"Lotus Stance Cut",
			"Intolerant Avici",
			"Heart Carving Cut",
			"Wicked Soul",

			"Phosphoric Slash",
			"Insightful Sword",
			"Slashing a Flower Upon One's Head",
			"Strange Half-Body",
		},
		{ //CHARACTER_REMILIA
			"Demon Lord Walk",
			"Servant Flier",
			"Demon Lord Cradle",
			"Demon Lord Arrow",

			"Vampire Claw",
			"Chain Gang",
			"Rocket Kick Upper",
			"Ceiling Fear",

			"Trickster Devil",
			"Demon's Dinner Fork",
			"Vampire Kiss",
			"Stigmanizer",
		},
		{ //CHARACTER_YUYUKO
			"Ghostly Butterfly",
			"Light of the Undead",
			"Eternal Wandering",
			"Dance of the Butterfly Dream",

			"Spirit that Died Well",
			"Lance of the Swallowtail Crest",
			"Spirit Luring Nectar",
			"Reverse Screens",

			"Sphere Bloom",
			"Land of Death",
			"Gifts to the Deceased",
			"Sense of Elegance",
		},
		{ //CHARACTER_YUKARI
			"Open, Lamented Box of Urashima",
			"Ghostly Butterfly Hidden in Zen Temple",
			"Ride the Waves, Fight the Ocean",
			"Illusional Rift",

			"Bound to the Grave",
			"Illusion Manji Parasol",
			"Universe of Matter and Antimatter",
			"Flesh Dismantler",

			"Bewitching Bait",
			"Border between Brains and Feet",
			"Eye of Changes",
			"Wings of the Chimera",
		},
		{ //CHARACTER_SUIKA
			"Spectre -Dense-",
			"Gnome -Dense-",
			"Spectre -Thin-",
			"Gathering Oni",

			"Oni Spirit Bomb",
			"Gnome -Thin-",
			"Unpleasant Mist",
			"Thin Oni",

			"Foot Bellows",
			"Fire Oni",
			"Ignis Fatuus",
			"Kidnapping Oni",
		},
		{ //CHARACTER_REISEN
			"Mind Explosion",
			"Illusionary Blast",
			"Ultrared Field",
			"Disbelief Aspect",

			"Mind Bending",
			"Eyesight Cleaning",
			"Ultraviolet Field",
			"Disorder Eye",

			"Mond Dropping",
			"Ripple Vision",
			"Undersense Break",
			"Acura Spectral",
		},
		{ //CHARACTER_AYA
			"Gust Fan",
			"Domination Dash",
			"Wind Standing of the Tengu",
			"Pelting in the Night",

			"Gale Fan",
			"Graceful Dash",
			"Tengu's Downburst",
			"Wind Sickle Veiling",

			"Wind from the Maple Fan",
			"Tengu Felling",
			"Tengu Drum",
			"Tengu's Pebbles",
		},
		{ //CHARACTER_KOMACHI
			"Spirits of the Firm",
			"Bound Spirits of the Earth",
			"Ritual of Ecstacy",
			"Floating Spirits of the Indolent Dead",

			"Scythe of the Reaper",
			"Wind on the Last Journey",
			"The Endless Way",
			"Lonely Bound Spirit",

			"A Human Spirit Passing By",
			"Boat on the Sanzu",
			"Taste of Death",
			"Scythe of Exorcism",
		},
		{ //CHARACTER_IKU
			"Dragon Fish's Strike",
			"Viels like Water",
			"Dragon Fish's Wrath",
			"Electrostatic Guided Missile",

			"Dragon God's Strike",
			"Viels like Wind",
			"Dragon God's Wrath",
			"Dragon God's Lightning Flash",

			"Dragonfish, the Able Swimmer",
			"The Smite of the Goddess",
			"Whiskers of the Dragon God",
			"Dragon's Eye",
		},
		{ //CHARACTER_TENSHI
			"Sword of the Earth",
			"Pillars of Divine Punishment",
			"Beams of Non-Perception",
			"Sword of Non-Perception",

			"Six Earthquakes -Signs-",
			"Guarding Keystones",
			"Heaven and Earth Press",
			"Sword of Scarlet Perception",

			"Sword of Fate",
			"Awakening of the Earth Spirits",
			"Scarlet Sword Temperament",
			"Sky Attack",
		},
		{ //CHARACTER_SANAE
			"Wind-Call",
			"Omikuji Bomb",
			"Sky-God Summon: Attack",
			"Earth-God Summon: Shield",

			"Wave-Call",
			"Sky Serpent",
			"Sky-God Summon: Wind",
			"Earth-God Summon: Iron Ring",

			"Falling Stars",
			"Cobalt Spread",
			"Sky-God Summon: Onbashira",
			"Earth-God Summon: Trap",
		},
		{ //CHARACTER_CIRNO
			"Icicle Shots",
			"Midsummer Snowman",
			"Little Iceberg",
			"Freeze Touch Me",

			"Frost Pillars",
			"Ice Charge",
			"Icicle Bomb",
			"Icicle Lance",

			"Freezing Light",
			"Ice Kick",
			"Frozen Technique",
			"Icicle Sword",
		},
		{ //CHARACTER_MEILING
			"Spiral-Light Steps",
			"Red Cannon",
			"Yellow Quake Kick",
			"Fragrant Wave",

			"Searing Red Fist",
			"Red Energy Release",
			"Earth Dragon Wave",
			"Water Taichi Fist",

			"Kick of Showering Brilliance",
			"Tiger Chi Release",
			"Sky Dragon Kick",
			"Colorful Rain",
		},
		{ //CHARACTER_UTSUHO
			"Flare Up",
			"Ground Melt",
			"Break Sun",
			"Shooting Star",

			"Rocket Dive",
			"Hell Geyser",
			"Shooting sun",
			"Retro Atomic Model",

			"Melting Pummel Kick",
			"Radiant Blade",
			"Hell Wave Cannon",
			"Vengeful Nuclear Spirits",
		},
		{ //CHARACTER_SUWAKO
			"Ancient Geyser",
			"Toad God",
			"Lake of Great Earth",
			"Native Scourge",

			"Frog God",
			"Ancient Metal Ring",
			"Ancient Jade",
			"Blighted Earth",

			"Rain-Calling frog",
			"Ms Longlimbs",
			"Stone Frog God",
			"Mishaguji Scourge",
		}
	}};
}