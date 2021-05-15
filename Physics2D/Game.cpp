#include "Game.h"

#include <gemcutter/Entity/Hierarchy.h>
#include <gemcutter/Entity/Name.h>
#include <gemcutter/Physics/2D/PhysicsWorld.h>
#include <gemcutter/Physics/2D/RigidBody.h>
#include <gemcutter/Rendering/Camera.h>
#include <gemcutter/Rendering/Rendering.h>
#include <gemcutter/Rendering/Sprite.h>
#include <gemcutter/Resource/Material.h>
#include <gemcutter/Sound/SoundListener.h>
#include <gemcutter/Utilities/Random.h>

Game::Game(ConfigTable& _config)
	: config(_config)
{
	keyPress = [this](const KeyPressed& e)
	{
		if (e.key == Key::F8)
		{
			PhysicsWorld.SetDebugEnabled(!PhysicsWorld.GetDebugEnabled());
		}
		else if (e.key == Key::Space)
		{
			SpawnCrates(50);
		}
	};

	resize = [this](const Resize& e)
	{
		float worldWidth  = static_cast<float>(e.width) * 0.01f;
		float worldHeight = static_cast<float>(e.height) * 0.01f;

		worldCamera->Get<Camera>().SetOrthographic(-worldWidth, worldWidth, worldHeight, -worldHeight, -100.0f, 100.0f);
		renderTarget->Resize(e.width, e.height);
	};
}

bool Game::Init()
{
	// Setup Cameras, it is also the listener for all sound in the scene.
	worldCamera->Add<SoundListener, Camera>();
	worldCamera->position.y = 5.0f;

	renderTarget = RenderTarget::MakeNew(Application.GetScreenWidth(), Application.GetScreenHeight(), 1, false);
	renderTarget->InitTexture(0, TextureFormat::sRGBA_8, TextureFilter::Point);
	if (!renderTarget->Validate())
		return false;

	// Set up renderer.
	renderPass.SetCamera(worldCamera);
	renderPass.SetTarget(renderTarget);

	// Setup background color to cornflower blue.
	SetClearColor(0.35f, 0.7f, 0.9f, 0.0f);

	auto mat = Material::MakeNew();
	mat->shader = Load<Shader>("Shaders/Default/Sprite");
	mat->textures.Add(Load<Texture>("Textures/Block"));

	Entity::Ptr floor = worldRoot->Get<Hierarchy>().CreateChild();
	floor->position = vec3(0.0f, -8.0f, 0.0f);
	floor->scale = vec3(100.0f, 15.0f, 1.0f);
	floor->Add<RigidBody2d>(RigidBodyType::Static).SetBox();
	floor->Add<Sprite>(Alignment::Center, false, mat);
	floor->Add<Name>("floor");

	SpawnCrates(50);

	return true;
}

void Game::Update()
{
	if (Input.IsDown(Key::Escape))
	{
		Application.Exit();
		return;
	}

	float deltaTime = Application.GetDeltaTime();

	PhysicsWorld.Update(deltaTime);

	Application.UpdateEngine();
}

void Game::Draw()
{
	ClearBackBuffer();
	renderTarget->Clear();

	renderPass.Render(*worldRoot);

	// Phsyics Debug draw needs to know what camera/target is in use manually.
	worldCamera->Get<Camera>().Bind();
	renderTarget->Bind();
	PhysicsWorld.DrawDebug();

	renderTarget->CopyColorToBackBuffer(0);
}

void Game::SpawnCrates(unsigned num)
{
	auto& hierarchy = worldRoot->Get<Hierarchy>();

	auto mat = Material::MakeNew();
	mat->shader = Load<Shader>("Shaders/Default/Sprite");
	mat->textures.Add(Load<Texture>("Textures/Crate"));

	for (unsigned i = 0; i < num; ++i)
	{
		Entity::Ptr entity = hierarchy.CreateChild();
		entity->position.x = RandomRange(-20.0f, 20.0f);
		entity->position.y = static_cast<float>(i) + 1.0f;
		entity->scale.x = RandomRange(0.5f, 2.0f);
		entity->scale.y = RandomRange(0.5f, 2.0f);
		entity->Add<Sprite>(Alignment::Center, false, mat);

		auto& rb = entity->Add<RigidBody2d>(RigidBodyType::Dynamic);
		rb.SetBox();
	}
}
