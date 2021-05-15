#pragma once
#include <gemcutter/Application/Application.h>
#include <gemcutter/Entity/Entity.h>
#include <gemcutter/Input/Input.h>
#include <gemcutter/Rendering/RenderPass.h>
#include <gemcutter/Resource/ConfigTable.h>

using namespace gem;

class Game
{
public:
	Game(ConfigTable& config);

	bool Init();

	void Update();
	void Draw();

	void SpawnCrates(unsigned num);

	/* Events */
	Listener<KeyPressed> keyPress;
	Listener<Resize> resize;

	/* Rendering */
	RenderPass renderPass;
	RenderTarget::Ptr renderTarget;

	/* Scene */
	Entity::Ptr worldCamera = Entity::MakeNew("camera_world");
	Entity::Ptr worldRoot   = Entity::MakeNewRoot("root_world");

	/* Assets */
	ConfigTable& config;
};
