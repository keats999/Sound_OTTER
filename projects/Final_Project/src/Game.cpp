#include "AudioEngine.h"
#include "GLFW/glfw3.h"

float gameTime = 0.0f;
// game states: 0 = start of game, 1 = instructions, 2 = difficulty seletion, 3 = game starting, 4 = game,  5 = game over (possibly score), 6 = restart
int gameState = 0;
int gameDifficulty = 0;
// 0 = left key, 1 = right key, 2 = space bar
bool wasPressed[] = { false, false, false };
/// each one is 1/16 to the right
/// Easy mode	- 0 = forward, 4 = right, 8 = backward, 12 = left
/// Normal mode	- 2 = forward-right, 6 = backward-right, 10 = backward-left, 14 = forward-left
/// Hard mode	- 1 = forward-forward-right, 3 = forward-right-right, 5 = backward-right-right, 7 = backward-backward-right, 9 = backward-backward-left, 11 = backward-left-left, 13 = forward-left-left, 15 = forward-forward-left
glm::vec3 spawnPoint[] = { glm::vec3(10.0f, 0.0f, 0.0f), glm::vec3(10.0f * cos(glm::radians(22.5f)), 0.0f, 10.0f * sin(glm::radians(22.5f))), glm::vec3(10.0f * cos(glm::radians(45.0f)), 0.0f, 10.0f * sin(glm::radians(45.0f))), glm::vec3(10.0f * cos(glm::radians(67.5f)), 0.0f, 10.0f * sin(glm::radians(67.5f))), glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(-10.0f * cos(glm::radians(67.5f)), 0.0f, 10.0f * sin(glm::radians(67.5f))), glm::vec3(-10.0f * cos(glm::radians(45.0f)), 0.0f, 10.0f * sin(glm::radians(45.0f))), glm::vec3(-10.0f * cos(glm::radians(22.5f)), 0.0f, 10.0f * sin(glm::radians(22.5f))), glm::vec3(-10.0f, 0.0f, 0.0f), glm::vec3(-10.0f * cos(glm::radians(22.5f)), 0.0f, -10.0f * sin(glm::radians(22.5f))), glm::vec3(-10.0f * cos(glm::radians(45.0f)), 0.0f, -10.0f * sin(glm::radians(45.0f))), glm::vec3(-10.0f * cos(glm::radians(67.5f)), 0.0f, -10.0f * sin(glm::radians(67.5f))), glm::vec3(0.0f, 0.0f, -10.0f), glm::vec3(10.0f * cos(glm::radians(67.5f)), 0.0f, -10.0f * sin(glm::radians(67.5f))), glm::vec3(10.0f * cos(glm::radians(45.0f)), 0.0f, -10.0f * sin(glm::radians(45.0f))), glm::vec3(10.0f * cos(glm::radians(22.5f)), 0.0f, -10.0f * sin(glm::radians(22.5f))) };
int currentForward = 0;
glm::vec3 enemyPos = spawnPoint[0];
int currentEnemySpawn = 4;
bool enemyActive = false;
float radarTimer = 0.0f;
float timeToReachPlayer = 0.0f;
GLFWwindow* gameWindow;

//------------------------------------------------------------------------
// Called before first update. Do any initial setup here.
//------------------------------------------------------------------------
void Init(GLFWwindow* currentWindow)
{
	gameWindow = currentWindow;

	srand(time(NULL));

	// Setup FMOD
	AudioEngine& engine = AudioEngine::Instance();
	engine.Init();
	engine.LoadBank("Master");
	engine.LoadBank("Master.strings");

	// Set music events
	AudioEvent& track1 = engine.CreateEvent("start of game", "event:/Start of Game");
	AudioEvent& track2 = engine.CreateEvent("instructions", "event:/Instructions");
	AudioEvent& track3 = engine.CreateEvent("difficulty selection", "event:/Difficulty Selection");
	AudioEvent& track4 = engine.CreateEvent("game starting", "event:/Game Starting");
	AudioEvent& track5 = engine.CreateEvent("pew pew", "event:/Shot");
	AudioEvent& track6 = engine.CreateEvent("explosion", "event:/Hit");
	AudioEvent& track7 = engine.CreateEvent("radar", "event:/Radar");
	AudioEvent& track8 = engine.CreateEvent("game over", "event:/Game Over");
	AudioEvent& track9 = engine.CreateEvent("restart", "event:/Restart");

	// Set parameters
	track3.SetParameter("Difficulty", gameDifficulty);

	// Get ref to Listener
	AudioListener& listener = engine.GetListener();
	listener.SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
	listener.SetUp(glm::vec3(0.0f, -1.0f, 0.0f));
	listener.SetForward(normalize(spawnPoint[0]));
}

void GetKeyboardInput()
{
	AudioEngine& engine = AudioEngine::Instance();
	AudioEvent& difficultySelection = engine.GetEvent("difficulty selection");
	AudioEvent& shot = engine.GetEvent("pew pew");
	AudioEvent& hit = engine.GetEvent("explosion");
	AudioListener& listener = engine.GetListener();

	if (glfwGetKey(gameWindow, GLFW_KEY_RIGHT) == GLFW_PRESS && !wasPressed[1])
	{
		if (gameState == 2)
		{
			if (++gameDifficulty >= 3) gameDifficulty = 0;
			if (difficultySelection.isPlaying()) difficultySelection.Stop();
			difficultySelection.SetParameter("Difficulty", gameDifficulty);
			difficultySelection.Play();
		}
		else if (gameState == 4)
		{
			currentForward += int(pow(2, 2 - gameDifficulty) + 0.01f);
			if (currentForward > 15) currentForward -= 16;
			listener.SetForward(normalize(spawnPoint[currentForward]));
		}
		wasPressed[1] = true;
	}
	else if (glfwGetKey(gameWindow, GLFW_KEY_RIGHT) == GLFW_RELEASE)
	{
		wasPressed[1] = false;
	}
	if (glfwGetKey(gameWindow, GLFW_KEY_LEFT) == GLFW_PRESS && !wasPressed[0])
	{
		if (gameState == 2)
		{
			if (--gameDifficulty <= -1) gameDifficulty = 2;
			if (difficultySelection.isPlaying()) difficultySelection.Stop();
			difficultySelection.SetParameter("Difficulty", gameDifficulty);
			difficultySelection.Play();
		}
		else if (gameState == 4)
		{
			currentForward -= int(pow(2, 2 - gameDifficulty) + 0.01f);
			if (currentForward < 0) currentForward += 16;
			listener.SetForward(normalize(spawnPoint[currentForward]));
		}
		wasPressed[0] = true;
	}
	else if (glfwGetKey(gameWindow, GLFW_KEY_LEFT) == GLFW_RELEASE)
	{
		wasPressed[0] = false;
	}
	if (glfwGetKey(gameWindow, GLFW_KEY_ENTER) == GLFW_PRESS)
	{
		if (gameState == 0)
		{
			gameState = 1;
			gameTime = 0.0f;
		}
		if (gameState == 2)
		{
			gameState = 3;
			gameTime = 0.0f;
		}
		if (gameState == 6)
		{
			gameState = 3;
			gameTime = 0.0f;
		}
	}
	if (glfwGetKey(gameWindow, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		if (gameState == 4  && !wasPressed[2])
		{
			if (shot.isPlaying()) shot.Stop();
			shot.Play();
			if (currentEnemySpawn == currentForward)
			{
				if (hit.isPlaying()) hit.Stop();
				hit.Play();
				enemyActive = false;
				timeToReachPlayer /= 1.05f;
			}
		}
		wasPressed[2] = true;
	}
	else if (glfwGetKey(gameWindow, GLFW_KEY_SPACE) == GLFW_RELEASE)
	{
		wasPressed[2] = false;
	}
	if (glfwGetKey(gameWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		if (gameState == 6 || gameState == 0)
		{
			glfwSetWindowShouldClose(gameWindow, true);
		}
	}
}

glm::vec3 lerp(float t)
{
	return (1.0f - t) * spawnPoint[currentEnemySpawn];
}
float Magnitude(glm::vec3 vector)
{
	return sqrt(vector.x * vector.x + vector.z * vector.z);
}

//------------------------------------------------------------------------
// Update your game here. 
//------------------------------------------------------------------------
void Update(float deltaTime)
{
	// Increment game time
	gameTime += deltaTime;

	// Delare sound variables
	AudioEngine& engine = AudioEngine::Instance();
	AudioEvent& startOfGame = engine.GetEvent("start of game");
	AudioEvent& instructions = engine.GetEvent("instructions");
	AudioEvent& dificultySelection = engine.GetEvent("difficulty selection");
	AudioEvent& gameStarting = engine.GetEvent("game starting");
	AudioEvent& radar = engine.GetEvent("radar");
	AudioEvent& gameOver = engine.GetEvent("game over");
	AudioEvent& restart = engine.GetEvent("restart");
	AudioListener& listener = engine.GetListener();

	if (gameState == 0) // Start of game
	{
		//play the appropiate sound event while bug proofing other events
		if (!startOfGame.isPlaying() && gameTime <= 0.5f) startOfGame.Play();
		if (instructions.isPlaying()) instructions.Stop();
		if (dificultySelection.isPlaying()) dificultySelection.Stop();
		if (gameStarting.isPlaying()) gameStarting.Stop();
		if (gameOver.isPlaying()) gameOver.Stop();
		if (restart.isPlaying()) restart.Stop();

		//check keyboard to start or quit the game
		GetKeyboardInput();
	}
	else if (gameState == 1) // Instructions
	{
		//play the appropiate sound event while bug proofing other events
		if (startOfGame.isPlaying()) startOfGame.Stop();
		if (!instructions.isPlaying() && gameTime <= 0.5f) instructions.Play();
		if (dificultySelection.isPlaying()) dificultySelection.Stop();
		if (gameStarting.isPlaying()) gameStarting.Stop();
		if (gameOver.isPlaying()) gameOver.Stop();
		if (restart.isPlaying()) restart.Stop();

		//move to the next sound event
		if (gameTime >= 16.937f)
		{
			gameState = 2;

			gameTime = 0.0f;
		}
	}
	else if (gameState == 2) // Difficulty selection
	{
		//play the appropiate sound event while bug proofing other events
		if (startOfGame.isPlaying()) startOfGame.Stop();
		if (instructions.isPlaying()) instructions.Stop();
		if (!dificultySelection.isPlaying() && gameTime <= 0.5f) dificultySelection.Play();
		if (gameStarting.isPlaying()) gameStarting.Stop();
		if (gameOver.isPlaying()) gameOver.Stop();
		if (restart.isPlaying()) restart.Stop();

		//check keyboard to change difficulty and move to the next sound event
		GetKeyboardInput();
	}
	else if (gameState == 3) // Game starting
	{
		//play the appropiate sound event while bug proofing other events
		if (startOfGame.isPlaying()) startOfGame.Stop();
		if (instructions.isPlaying()) instructions.Stop();
		if (dificultySelection.isPlaying()) dificultySelection.Stop();
		if (!gameStarting.isPlaying() && gameTime <= 0.5f) gameStarting.Play();
		if (gameOver.isPlaying()) gameOver.Stop();
		if (restart.isPlaying()) restart.Stop();

		//move to the next sound event
		if (gameTime >= 2.384f)
		{
			gameState = 4;
			timeToReachPlayer = 20.0f;
		}
	}
	else if (gameState == 4) // Game
	{
		if (!enemyActive)
		{
			int tempRand = 0;

			if (gameDifficulty == 0)
				tempRand = (rand() % 4) * 4;
			else if (gameDifficulty == 1)
				tempRand = (rand() % 8) * 2;
			else
				tempRand = rand() % 16;

			enemyPos = spawnPoint[tempRand];
			currentEnemySpawn = tempRand;

			gameTime = 0;
			enemyActive = true;

			radar.SetPosition(enemyPos);
			if (radar.isPlaying()) radar.Stop();
			radar.Play();
			radarTimer = Magnitude(enemyPos) / 4.0f;
		}
		else
		{
			if (gameTime >= timeToReachPlayer)
			{
				gameTime = 0.0f;
				enemyActive = false;
				gameState = 5;
			}
			enemyPos = lerp(gameTime / timeToReachPlayer);

			radarTimer -= deltaTime;
			if (radarTimer <= 0.0f)
			{
				radar.SetPosition(enemyPos);
				if (radar.isPlaying()) radar.Stop();
				radar.Play();
				radarTimer = Magnitude(enemyPos) / 4.0f;
			}
		}

		GetKeyboardInput();
	}
	else if (gameState == 5) // Game over
	{
		//play the appropiate sound event while bug proofing other events
		if (startOfGame.isPlaying()) startOfGame.Stop();
		if (instructions.isPlaying()) instructions.Stop();
		if (dificultySelection.isPlaying()) dificultySelection.Stop();
		if (gameStarting.isPlaying()) gameStarting.Stop();
		if (!gameOver.isPlaying() && gameTime <= 0.5f) gameOver.Play();
		if (restart.isPlaying()) restart.Stop();

		//move to the next sound event
		if (gameTime >= 2.541f)
		{
			gameState = 6;

			gameTime = 0.0f;
		}
	}
	else if (gameState == 6) // Restart
	{
		//play the appropiate sound event while bug proofing other events
		if (startOfGame.isPlaying()) startOfGame.Stop();
		if (instructions.isPlaying()) instructions.Stop();
		if (dificultySelection.isPlaying()) dificultySelection.Stop();
		if (gameStarting.isPlaying()) gameStarting.Stop();
		if (gameOver.isPlaying()) gameOver.Stop();
		if (!restart.isPlaying() && gameTime <= 1.0f) restart.Play();

		//check keyboard to restart or quit
		GetKeyboardInput();
	}
	else
	{
		gameState = 0;
	}

	engine.Update();
}

//------------------------------------------------------------------------
// Add your shutdown code here.
//------------------------------------------------------------------------
void Shutdown()
{
	AudioEngine::Instance().Shutdown();
}