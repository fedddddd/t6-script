#include <stdinc.hpp>

namespace game
{
	gamemode current = gamemode::none;

	namespace environment
	{
		bool t6mp()
		{
			return current == gamemode::multiplayer;
		}

		bool t6zm()
		{
			return current == gamemode::zombies;
		}
	}

	void init()
	{
		current = reinterpret_cast<const char*>(0xC2F028) == "multiplayer"s
			? gamemode::multiplayer 
			: gamemode::zombies;
	}

	scr_entref_t Scr_GetEntityIdRef(unsigned int entId)
	{
		scr_entref_t entref;

		const auto v2 = &game::scr_VarGlob[0].objectVariableValue[entId];
		
		entref.entnum = v2->entnum;
		entref.classnum = v2->classnum >> 8;

		return entref;
	}
}
