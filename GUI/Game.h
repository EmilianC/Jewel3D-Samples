#pragma once
#include <gemcutter/Application/Application.h>
#include <gemcutter/Entity/Entity.h>
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

	/* Rendering */
	RenderPass mainRenderPass;

	/* Events */
	Listener<Resize> onResized;

	/* Scene */
	Entity::Ptr camera = Entity::MakeNew();
	Entity::Ptr screen = Entity::MakeNewRoot();
	Entity::Ptr root = Entity::MakeNewRoot();

	/* Assets */
	ConfigTable& config;

	float elapsed            = 0.0f;
	bool wiggleRoot          = false;

	bool canFullScreen		 = true;
	bool canChangeBorder	 = true;
	bool canResize			 = true;
	bool canChangeResolution = true;
};
