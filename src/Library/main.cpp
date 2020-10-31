//
// Created by Gegel85 on 31/10/2020
//

#include <discord.h>
#include <iostream>

int main()
{
	discord::Core *core;

	discord::Core::Create(ClientID, DiscordCreateFlags_Default, &core);
	discord::Activity activity{};

	//discord::ActivityManager::
	activity.SetState("Testing");
	activity.SetDetails("Fruit Loops");

	auto &assets = activity.GetAssets();

	assets.SetLargeImage("cover");

	core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
		auto code = static_cast<unsigned>(result);

		if (code)
			std::cerr << "Error: " << code << std::endl;
	});
	while (true)
		core->RunCallbacks();
}