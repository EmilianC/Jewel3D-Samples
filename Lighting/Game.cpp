#include "Game.h"

#include <Jewel3D/Application/FileSystem.h>
#include <Jewel3D/Entity/Hierarchy.h>
#include <Jewel3D/Input/Input.h>
#include <Jewel3D/Math/Math.h>
#include <Jewel3D/Rendering/Camera.h>
#include <Jewel3D/Rendering/Light.h>
#include <Jewel3D/Rendering/Mesh.h>
#include <Jewel3D/Rendering/Rendering.h>
#include <Jewel3D/Rendering/Sprite.h>
#include <Jewel3D/Rendering/Text.h>
#include <Jewel3D/Resource/Material.h>
#include <Jewel3D/Resource/Model.h>
#include <Jewel3D/Utilities/Random.h>
#include <Jewel3D/Utilities/String.h>

Game::Game(ConfigTable& _config)
	: config(_config)
{
	// Keep aspect ratio in sync with the window's dimensions.
	onResized = [this](auto& e)
	{
		camera->Get<Camera>().SetAspectRatio(e.GetAspectRatio());
		cameraUI->Get<Camera>().SetBounds(Application.GetScreenViewport());

		frameBuffer->Resize(e.width, e.height);
		if (frameBufferResolve)
		{
			frameBufferResolve->Resize(e.width, e.height);
		}
	};

	onKeyPressed = [this](auto& e)
	{
		switch (e.key)
		{
		case Key::KeyBoard1:
		case Key::Numpad1:
			useNormalMap = !useNormalMap;
			UpdateSettings();
			break;

		case Key::KeyBoard2:
		case Key::Numpad2:
			useHeightMap = !useHeightMap;
			UpdateSettings();
			break;

		case Key::KeyBoard3:
		case Key::Numpad3:
			useEnvironmentMap = !useEnvironmentMap;
			UpdateSettings();
			break;

		case Key::KeyBoard4:
		case Key::Numpad4:
			useAdvancedLighting = !useAdvancedLighting;
			UpdateSettings();
			break;

		case Key::S:
			currentSkybox++;
			if (currentSkybox >= environmentMaps.size())
			{
				currentSkybox = 0;
			}
			UpdateSkybox();
			break;

		case Key::Space:
			light->Get<Light>().color = RandomColor() * RandomRange(5.0f, 125.0f);
			break;

		case Key::Escape:
			Application.Exit();
			return;
		}
	};
}

void Game::UpdateSkybox()
{
	mainRenderPass.textures.Add(environmentMaps[currentSkybox], 0);
	mainRenderPass.textures.Add(irradianceMaps[currentSkybox], 1);
	mainRenderPass.SetSkybox(environmentMaps[currentSkybox]);
}

void Game::UpdateSettings()
{
	for (Mesh& mesh : All<Mesh>())
	{
		mesh.variants.Switch("USE_NORMAL_MAP", useNormalMap);
		mesh.variants.Switch("USE_HEIGHT_MAP", useHeightMap);
		mesh.variants.Switch("USE_ENVIRONMENT_MAP", useEnvironmentMap);
		mesh.variants.Switch("USE_ADVANCED_LIGHTING", useAdvancedLighting);
	}

	text->Get<Text>().text = FormatString(
		"Normal Map (1)\n%s\nHeight Map (2)\n%s\nEnvironment Lighting (3)\n%s\nAdvanced Lighting (4)\n%s\nChange Skybox (S)",
		useNormalMap		? "  off  [on]" : " [off]  on ",
		useHeightMap		? "  off  [on]" : " [off]  on ",
		useEnvironmentMap	? "  off  [on]" : " [off]  on ",
		useAdvancedLighting ? "  off  [on]" : " [off]  on ");
}

void Game::CreateGround()
{
	auto model = Load<Model>("Models/Ground");
	auto material = Load<Material>("Materials/PBR/Stone");

	for (int x = -2; x <= 2; ++x)
	{
		for (int z = -2; z <= 2; ++z)
		{
			auto ent = rootEntity->Get<Hierarchy>().CreateChild();

			ent->Add<Mesh>(model, material);
			ent->position.x = x * 4.0f;
			ent->position.z = z * 4.0f;
		}
	}
}

bool Game::Init()
{
	// Load game assets.
	auto ballModel = Load<Model>("Models/Orb");
	auto font = Load<Font>("Fonts/editundo");
	auto spriteMaterial = Load<Material>("Materials/Lightbulb");
	auto textMaterial = Load<Material>("Materials/Text");
	auto ironMaterial = Load<Material>("Materials/PBR/RustedIron");

	// Load all skyboxes.
	DirectoryData skyboxDir;
	ParseDirectory(skyboxDir, RootAssetDirectory + "/Textures/Skyboxes");
	for (auto& file : skyboxDir.files)
	{
		if (!CompareLowercase(ExtractFileExtension(file), ".texture"))
			continue;

		environmentMaps.push_back(Load<Texture>("Textures/Skyboxes/" + file));
		irradianceMaps. push_back(Load<Texture>("Textures/Skyboxes/IrradianceMaps/" + file));
	}

	// Setup the main object.
	ball->Add<Mesh>(ballModel, ironMaterial);
	ball->position.y += 2.75f;
	ball->scale = vec3(14.0f);

	// Create the UI text.
	text->Add<Text>(font, "", textMaterial);
	text->position.y = 180.0f;
	text->scale = vec3(0.3f);

	// Setup Lighting.
	light->Add<Sprite>(Alignment::Center, true, spriteMaterial);
	light->Add<Light>(vec3(25.0f, 23.0f, 21.0f));
	light->position = vec3(-4.0f, 2.0f, -3.0f);
	light->scale = vec3(0.5f);

	// Setup cameras.
	camera->Add<Camera>(66.0f, Application.GetAspectRatio(), 0.1f, 100.0f);
	camera->LookAt(vec3(0.0f, 6.0f, 6.0f), vec3(0.0f, 1.0f, 0.0f));

	cameraUI->Add<Camera>().SetOrthographic(Application.GetScreenViewport(), -1.0f, 1.0f);

	// Setup scene.
	rootEntity->Get<Hierarchy>().AddChild(ball);
	rootLight->Get<Hierarchy>().AddChild(light);
	cameraParent->Get<Hierarchy>().AddChild(camera);

	// Set up renderer.
	MSAA_Level = config.GetInt("MSAA");
	frameBuffer = RenderTarget::MakeNew(Application.GetScreenWidth(), Application.GetScreenHeight(), 1, true, MSAA_Level);
	frameBuffer->InitTexture(0, TextureFormat::sRGB_8, TextureFilter::Point);
	if (!frameBuffer->Validate())
		return false;

	if (MSAA_Level > 1)
	{
		frameBufferResolve = frameBuffer->MakeResolve();
	}

	mainRenderPass.SetCamera(camera);
	mainRenderPass.SetTarget(frameBuffer);
	mainRenderPass.buffers.Add(light->Get<Light>().GetBuffer(), 0);

	transparentRenderPass.SetCamera(camera);
	transparentRenderPass.SetTarget(frameBuffer);

	UIRenderPass.SetCamera(cameraUI);
	UIRenderPass.SetTarget(frameBuffer);

	// Load the ground and set the remaining scene parameters.
	CreateGround();
	UpdateSkybox();
	UpdateSettings();

	return true;
}

void Game::Update()
{
	float deltaTime = Application.GetDeltaTime();

	// Camera controls.
	if (Input.IsDown(Key::Up))		angleX -= deltaTime * 25.0f;
	if (Input.IsDown(Key::Down))	angleX += deltaTime * 25.0f;
	if (Input.IsDown(Key::Left))	angleY -= deltaTime * 30.0f;
	if (Input.IsDown(Key::Right))	angleY += deltaTime * 30.0f;
	angleX = Clamp(angleX, -45.0f, 42.5f);

	cameraParent->rotation = quat::Identity;
	cameraParent->RotateX(angleX);
	cameraParent->RotateY(angleY);

	// Rotate the light around the scene.
	rootLight->RotateY(deltaTime * -16.0f);

	// Engine systems and components are updated here.
	Application.UpdateEngine();
}

void Game::Draw()
{
	ClearBackBufferDepth();
	frameBuffer->Clear();

	mainRenderPass.Render(*rootEntity);
	transparentRenderPass.Render(*rootLight);
	UIRenderPass.Render(*text);

	if (MSAA_Level > 1)
	{
		frameBuffer->ResolveMultisamplingColor(*frameBufferResolve);
		frameBufferResolve->CopyColorToBackBuffer(0);
	}
	else
	{
		frameBuffer->CopyColorToBackBuffer(0);
	}
}
