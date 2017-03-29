#include "Game.h"

#include <Jewel3D/Application/Application.h>
#include <Jewel3D/Input/Input.h>
#include <Jewel3D/Rendering/Camera.h>
#include <Jewel3D/Rendering/Material.h>
#include <Jewel3D/Rendering/Light.h>
#include <Jewel3D/Rendering/Mesh.h>
#include <Jewel3D/Math/Matrix.h>

Game::Game(ConfigTable &config)
	: config(config)
{
}

bool Game::Init()
{
	// Prepare ground object.
	auto model = Load<Model>("Models/Ground.model");
	auto texture = Load<Texture>("Textures/Ground.png");
	lambertShadow = Load<Shader>("Shaders/LambertShadow.shader");
	if (!model || !texture || !lambertShadow) return false;
	
	ground->Add<Material>(lambertShadow, texture);
	ground->Add<Mesh>(model);
	ground->scale = vec3(1.15f, 1.15f, 1.15f);

	// Prepare shack object.
	model = Load<Model>("Models/Shack.model");
	texture = Load<Texture>("Textures/Shack.png");
	if (!model || !texture) return false;

	shack->Add<Material>(lambertShadow, texture);
	shack->Add<Mesh>(model);
	shack->scale = vec3(1.33f, 1.33f, 1.33f);

	// Setup Scene.
	rootNode->AddChild(ground);
	rootNode->AddChild(shack);

	// Setup Light.
	shadowCamera->Add<Camera>(-20.0f, 20.0f, 20.0f, -20.0f, -100.0f, 100.0f);
	shadowCamera->Add<Light>(vec3(1.0f), Light::Type::Directional);
	shadowCamera->LookAt(vec3(2.0f, 1.0f, 0.5f), vec3(0.0f));

	lambertShadow->buffers.Add(shadowCamera->Get<Light>().GetBuffer(), 0);
	viewToShadow = lambertShadow->buffers[2]->MakeHandle<mat4>("ViewToShadow");

	// Setup Camera.
	mainCamera->Add<Camera>(60.0f, Application.GetAspectRatio(), 1.0f, 10000.0f);
	mainCamera->position = vec3(0.0f, 15.0f, 40.0f);
	mainCamera->RotateX(-20.0f);

	// Setup up renderer.
	shadowMap->Init(2048, 2048, 0, true);
	if (!shadowMap->Validate()) return false;

	auto shadowPassShader = Shader::MakeNew();
	if (!shadowPassShader->LoadPassThrough()) return false;

	shadowRenderPass.SetCamera(shadowCamera);
	shadowRenderPass.SetShader(shadowPassShader);
	shadowRenderPass.SetTarget(shadowMap);

	mainRenderPass.textures.Add(shadowMap->GetDepthTexture(), 1);
	mainRenderPass.SetCamera(mainCamera);

	// Set background color to black.
	SetClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	return true;
}

void Game::Update()
{
	if (Input.IsDown(Key::Escape))
	{
		Application.Exit();
		return;
	}

	rootNode->RotateY(Application.GetDeltaTime() * -12.0f);

	// Keep shadow direction and ViewSpace to shadowMap matrix up to date.
	viewToShadow = 
		mat4(0.5, 0.0, 0.0, 0.5,
			0.0, 0.5, 0.0, 0.5,
			0.0, 0.0, 0.5, 0.5,
			0.0, 0.0, 0.0, 1.0) * shadowCamera->Get<Camera>().GetViewProjMatrix() * mainCamera->GetWorldTransform();

	// Engine systems and components are updated here.
	Application.UpdateEngine();
}

void Game::Draw()
{
	ClearBackBuffer();
	shadowMap->Clear();

	shadowRenderPass.Render(*rootNode);
	mainRenderPass.Render(*rootNode);
}
