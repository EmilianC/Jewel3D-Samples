#include "Game.h"

#include <gemcutter/Application/Application.h>
#include <gemcutter/Entity/Hierarchy.h>
#include <gemcutter/GUI/Button.h>
#include <gemcutter/GUI/Image.h>
#include <gemcutter/GUI/Screen.h>
#include <gemcutter/Input/Input.h>
#include <gemcutter/Math/Math.h>
#include <gemcutter/Rendering/Camera.h>
#include <gemcutter/Rendering/Rendering.h>
#include <gemcutter/Resource/Font.h>

Game::Game(ConfigTable& _config)
	: config(_config)
{
	onResized = [this](const Resize& event)
	{
		camera->Get<Camera>().SetOrthographic(Application.GetScreenViewport(), -1.0f, 1.0f);
	};
}

bool Game::Init()
{
	// Setup Camera.
	camera->Add<Camera>().SetOrthographic(Application.GetScreenViewport(), -1.0f, 1.0f);

	// Setup up renderer.
	mainRenderPass.SetCamera(camera);

	// Setup widgets.
	screen->Add<Screen>();
	root = screen->Get<Widget>().CreateChildWidget();

	auto backGround = root->Get<Hierarchy>().CreateChild();
	backGround->Add<Image>(Load<Texture>("Textures/GUI/PanelDark"));

	auto panel = root->Get<Hierarchy>().CreateChild();
	auto& image = panel->Add<Image>(Load<Texture>("Textures/GUI/Panel"));

	image.top.anchor = 0.1f;
	image.right.anchor = 0.1f;
	image.left.anchor = 0.6f;
	image.bottom.anchor = 0.1f;

	{
		auto child = panel->Get<Hierarchy>().CreateChild();
		auto& button = child->Add<Button>(
			Load<Font>("Fonts/georgia"),
			"Click",
			Load<Texture>("Textures/GUI/ButtonIdle"),
			Load<Texture>("Textures/GUI/ButtonHover"),
			Load<Texture>("Textures/GUI/ButtonPressed")
		);

		button.pressedSound = Load<Sound>("Sounds/Snap");
		button.top.anchor = 0.55f;
		button.right.anchor = 0.1f;
		button.left.anchor = 0.1f;
		button.bottom.anchor = 0.1f;
		button.textScale = 0.5f;
	}

	{
		auto child = panel->Get<Hierarchy>().CreateChild();
		auto& button = child->Add<Button>(
			Load<Font>("Fonts/georgia"),
			"Wiggle",
			Load<Texture>("Textures/GUI/ButtonIdle"),
			Load<Texture>("Textures/GUI/ButtonHover"),
			Load<Texture>("Textures/GUI/ButtonPressed")
		);

		button.top.anchor = 0.1f;
		button.right.anchor = 0.1f;
		button.left.anchor = 0.1f;
		button.bottom.anchor = 0.55f;
		button.textScale = 0.5f;

		button.onClick = [this]()
		{
			wiggleRoot = !wiggleRoot;
		};
	}

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
	elapsed += deltaTime;

	// Toggle fullscreen.
	if (Input.IsDown(Key::F))
	{
		if (canFullScreen)
		{
			Application.SetFullscreen(!Application.IsFullscreen());
			canFullScreen = false;
		}
	}
	else
	{
		canFullScreen = true;
	}

	// Toggle window border.
	if (Input.IsDown(Key::B))
	{
		if (canChangeBorder)
		{
			Application.SetBordered(!Application.IsBordered());
			canChangeBorder = false;
		}
	}
	else
	{
		canChangeBorder = true;
	}

	// Toggle window resize.
	if (Input.IsDown(Key::R))
	{
		if (canResize)
		{
			Application.SetResizable(!Application.IsResizable());
			canResize = false;
		}
	}
	else
	{
		canResize = true;
	}

	// Set resolution.
	if (Input.IsDown(Key::KeyBoard1))
	{
		if (canChangeResolution)
		{
			Application.SetResolution(800, 600);
			canChangeResolution = false;
		}
	}
	else if (Input.IsDown(Key::KeyBoard2))
	{
		if (canChangeResolution)
		{
			Application.SetResolution(1280, 720);
			canChangeResolution = false;
		}
	}
	else if (Input.IsDown(Key::KeyBoard3))
	{
		if (canChangeResolution)
		{
			Application.SetResolution(1600, 900);
			canChangeResolution = false;
		}
	}
	else if (Input.IsDown(Key::KeyBoard4))
	{
		if (canChangeResolution)
		{
			Application.SetResolution(1920, 1080);
			canChangeResolution = false;
		}
	}
	else
	{
		canChangeResolution = true;
	}

	if (wiggleRoot)
	{
		auto& widget = root->Get<Widget>();

		widget.top.anchor    = sin(elapsed) / 10.0f + 0.1f;
		widget.bottom.anchor = sin(elapsed + ToRadian(90.0f)) / 10.0f + 0.1f;
		widget.left.anchor   = sin(elapsed + ToRadian(270.0f)) / 10.0f + 0.1f;
		widget.right.anchor  = sin(elapsed + ToRadian(180.0f)) / 10.0f + 0.1f;
	}
	else
	{
		root->Get<Widget>().FitToParent();
	}

	// Engine systems and components are updated here.
	Application.UpdateEngine();
}

void Game::Draw()
{
	ClearBackBuffer();

	mainRenderPass.Render(*screen);
}
