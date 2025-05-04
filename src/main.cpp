#ifdef _WIN32
#include <windows.h>
#endif

#include <glm/glm.hpp>
#include <string>
#include <fstream>
#include <vector>
#include <cstdlib>  // for rand() and srand()
#include <ctime>    // for time()
#include "Canis/Canis.hpp"
#include "Canis/Entity.hpp"
#include "Canis/Graphics.hpp"
#include "Canis/Window.hpp"
#include "Canis/Shader.hpp"
#include "Canis/Debug.hpp"
#include "Canis/IOManager.hpp"
#include "Canis/InputManager.hpp"
#include "Canis/Camera.hpp"
#include "Canis/Model.hpp"
#include "Canis/World.hpp"
#include "Canis/Editor.hpp"
#include "Canis/FrameRateManager.hpp"

using namespace glm;

// git restore .
// git fetch
// git pull

// 3d array
std::vector<std::vector<std::vector<unsigned int>>> map = {};

// declaring functions
void SpawnLights(Canis::World &_world);
void LoadMap(std::string _path);
void Rotate(Canis::World &_world, Canis::Entity &_entity, float _deltaTime);
void AnimateFire(Canis::World &_world, Canis::Entity &_entity, float _deltaTime);
void RandomizeGrassAndFlowers(int startY, int endY, int startX, int endX, float grassChance, float flowerChance);
void SetupRandomVegetation();

// Fire animation parameters
const int FIRE_FRAME_COUNT = 31;  // Number of fire frames (1-31)
std::vector<Canis::GLTexture> fireTextures;
float fireAnimTimer = 0.0f;
int currentFireFrame = 0;
const float FIRE_ANIM_SPEED = 0.05f;  // Seconds per frame

#ifdef _WIN32
#define main SDL_main
extern "C" int main(int argc, char* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    Canis::Init();
    Canis::InputManager inputManager;
    Canis::FrameRateManager frameRateManager;
    frameRateManager.Init(60);

    /// SETUP WINDOW
    Canis::Window window;
    window.MouseLock(true);

    unsigned int flags = 0;

    if (Canis::GetConfig().fullscreen)
        flags |= Canis::WindowFlags::FULLSCREEN;

    window.Create("Hello Graphics", Canis::GetConfig().width, Canis::GetConfig().heigth, flags);
    /// END OF WINDOW SETUP

    Canis::World world(&window, &inputManager, "assets/textures/lowpoly-skybox/");
    SpawnLights(world);

    Canis::Editor editor(&window, &world, &inputManager);

    Canis::Graphics::EnableAlphaChannel();
    Canis::Graphics::EnableDepthTest();

    /// SETUP SHADER
    Canis::Shader shader;
    shader.Compile("assets/shaders/hello_shader.vs", "assets/shaders/hello_shader.fs");
    shader.AddAttribute("aPosition");
    shader.Link();
    shader.Use();
    shader.SetInt("MATERIAL.diffuse", 0);
    shader.SetInt("MATERIAL.specular", 1);
    shader.SetFloat("MATERIAL.shininess", 64);
    shader.SetBool("WIND", false);
    shader.UnUse();

    Canis::Shader grassShader;
    grassShader.Compile("assets/shaders/hello_shader.vs", "assets/shaders/hello_shader.fs");
    grassShader.AddAttribute("aPosition");
    grassShader.Link();
    grassShader.Use();
    grassShader.SetInt("MATERIAL.diffuse", 0);
    grassShader.SetInt("MATERIAL.specular", 1);
    grassShader.SetFloat("MATERIAL.shininess", 64);
    grassShader.SetBool("WIND", true);
    grassShader.SetFloat("WINDEFFECT", 0.2);
    grassShader.UnUse();

    // Flat shader for blocks with different textures on different faces
    Canis::Shader flatShader;
    flatShader.Compile("assets/shaders/block_flat.vs", "assets/shaders/block_flat.fs");
    flatShader.AddAttribute("aPosition");
    flatShader.AddAttribute("aNormal");
    flatShader.AddAttribute("aTexCoords");
    flatShader.Link();
    flatShader.Use();
    flatShader.SetInt("uSideTex", 0);    // side texture unit
    flatShader.SetInt("uTopTex", 1);     // top texture unit
    flatShader.SetInt("uBottomTex", 2);  // bottom texture unit
    
    // Add lighting uniforms to flatShader
    // Set lighting properties for the flat shader
    flatShader.SetFloat("MATERIAL.shininess", 64.0f);
    flatShader.UnUse();

    // Fire shader setup - simplified
    Canis::Shader fireShader;
    fireShader.Compile("assets/shaders/fire_shader.vs", "assets/shaders/fire_shader.fs");
    fireShader.AddAttribute("aPosition");
    fireShader.AddAttribute("aNormal");
    fireShader.AddAttribute("aUV");
    fireShader.Link();
    fireShader.Use();
    fireShader.SetInt("MATERIAL.diffuse", 0);          // Now using single texture
    fireShader.SetInt("MATERIAL.specular", 1);         // Specular map
    fireShader.SetFloat("MATERIAL.shininess", 32.0f);  // Lower shininess for fire
    fireShader.UnUse();
    /// END OF SHADER

    /// Load Image
    Canis::GLTexture glassTexture = Canis::LoadImageGL("assets/textures/glass.png", true);
    Canis::GLTexture grassTexture = Canis::LoadImageGL("assets/textures/grass.png", false);
    Canis::GLTexture flowerTexture = Canis::LoadImageGL("assets/textures/blue_orchid.png", false);

    Canis::GLTexture woodplankTexture = Canis::LoadImageGL("assets/textures/oak_planks.png", true);

    Canis::GLTexture houseTexture = Canis::LoadImageGL("assets/textures/house.png", true);

    
    // Dirt block textures
    Canis::GLTexture dirtSideTex = Canis::LoadImageGL("assets/textures/grass_block_side.png", false);
    Canis::GLTexture dirtTopTex = Canis::LoadImageGL("assets/textures/grass_block_top.png", false);
    Canis::GLTexture dirtBottomTex = Canis::LoadImageGL("assets/textures/dirt_bottom.png", false); // Changed to dirt.png for bottom

    Canis::GLTexture brickblock = Canis::LoadImageGL("assets/textures/bricks.png", true);

    Canis::GLTexture textureSpecular = Canis::LoadImageGL("assets/textures/container2_specular.png", true);

    // Load fire textures with verification
    fireTextures.clear(); // Make sure vector is empty
    for (int i = 1; i <= FIRE_FRAME_COUNT; i++) {
        std::string path = "assets/textures/fire_textures/fire_" + std::to_string(i) + ".png";
        Canis::GLTexture fireTexture = Canis::LoadImageGL(path, true); // Enable transparency for fire
        
        // Verify the texture loaded correctly
        if (fireTexture.id == 0) {
            Canis::Log("Failed to load fire texture: " + path);
        } else {
            Canis::Log("Successfully loaded fire texture: " + path);
        }
        
        fireTextures.push_back(fireTexture);
    }
    /// End of Image Loading

    /// Load Models
    Canis::Model cubeModel = Canis::LoadModel("assets/models/cube.obj");
    Canis::Model grassModel = Canis::LoadModel("assets/models/plants.obj");
    Canis::Model fireModel = Canis::LoadModel("assets/models/fire.obj");
    /// END OF LOADING MODEL

    // Load Map into 3d array
    LoadMap("assets/maps/level.map");
    
    // Add this line to randomize grass and flowers in the specified region
    SetupRandomVegetation();

    // Loop map and spawn objects
    for (int y = 0; y < map.size(); y++)
    {
        for (int x = 0; x < map[y].size(); x++)
        {
            for (int z = 0; z < map[y][x].size(); z++)
            {
                Canis::Entity entity;
                entity.active = true;

                switch (map[y][x][z])
                {
                case 1: // places a glass block
                    entity.tag = "glass";
                    entity.albedo = &glassTexture;
                    entity.specular = &textureSpecular;
                    entity.model = &cubeModel;
                    entity.shader = &shader;
                    entity.transform.position = vec3(x + 0.0f, y + 0.0f, z + 0.0f);
                    world.Spawn(entity);
                    break;
                case 2: // places a grass block
                    entity.tag = "grass";
                    entity.albedo = &grassTexture;
                    entity.specular = &textureSpecular;
                    entity.model = &grassModel;
                    entity.shader = &grassShader;
                    entity.transform.position = vec3(x + 0.0f, y + 0.0f, z + 0.0f);
                    entity.Update = &Rotate;
                    world.Spawn(entity);
                    break;
                case 3: // places a woodenplank block
                    entity.tag = "oakplank";
                    entity.albedo = &woodplankTexture;
                    entity.specular = &textureSpecular;
                    entity.model = &cubeModel;
                    entity.shader = &shader;
                    entity.transform.position = vec3(x + 0.0f, y + 0.0f, z + 0.0f);
                    entity.Update = &Rotate;
                    world.Spawn(entity);
                    break;
                case 4: // places a dirt block with different textures on top, sides, and bottom
                    entity.tag = "dirt";
                    entity.albedo = &dirtSideTex;     // side texture (GL_TEXTURE0)
                    entity.specular = &dirtTopTex;    // top texture (GL_TEXTURE1)
                    entity.emission = &dirtBottomTex; // bottom texture (GL_TEXTURE2)
                    entity.model = &cubeModel;
                    entity.shader = &flatShader;      // Use the flat shader for multi-texture blocks
                    entity.transform.position = vec3(x + 0.0f, y + 0.0f, z + 0.0f);
                    world.Spawn(entity);
                    break;
                case 5: // places a brick block
                    entity.tag = "brick";
                    entity.albedo = &brickblock;
                    entity.specular = &textureSpecular;
                    entity.model = &cubeModel;
                    entity.shader = &shader;
                    entity.transform.position = vec3(x + 0.0f, y + 0.0f, z + 0.0f);
                    entity.Update = &Rotate;
                    world.Spawn(entity);
                    break;
                case 6: // places a flower
                    entity.tag = "flower";
                    entity.albedo = &flowerTexture;
                    entity.specular = &textureSpecular;
                    entity.model = &grassModel;
                    entity.shader = &grassShader;
                    entity.transform.position = vec3(x + 0.0f, y + 0.0f, z + 0.0f);
                    entity.Update = &Rotate;
                    world.Spawn(entity);
                    break;
                case 7: // places a fire
                    entity.tag = "fire";
                    entity.albedo = &fireTextures[0];   // Initial texture is the first frame
                    entity.specular = &textureSpecular;
                    entity.model = &fireModel;
                    entity.shader = &fireShader;
                    entity.transform.position = vec3(x + 0.0f, y + 0.0f, z + 0.0f);
                    entity.Update = &AnimateFire;
                    world.Spawn(entity);
                    break;
                case 8: // places a special house block
                    entity.tag = "house";
                    entity.albedo = &houseTexture;
                    entity.specular = &textureSpecular;
                    entity.model = &cubeModel;
                    entity.shader = &shader;
                    entity.transform.position = vec3(x + 0.0f, y + 0.0f, z + 0.0f);
                    entity.Update = &Rotate;
                    world.Spawn(entity);
                default:
                    break;
                }
            }
        }
    }

    // Add some example fire entities in the scene
    Canis::Entity fire1;
    fire1.active = true;
    fire1.tag = "fire";
    fire1.albedo = &fireTextures[0];
    fire1.specular = &textureSpecular;
    fire1.model = &fireModel;
    fire1.shader = &fireShader;
    fire1.transform.position = vec3(5.0f, 1.0f, 5.0f);
    fire1.Update = &AnimateFire;
    world.Spawn(fire1);

    Canis::Entity fire2;
    fire2.active = true;
    fire2.tag = "fire";
    fire2.albedo = &fireTextures[0];
    fire2.specular = &textureSpecular;
    fire2.model = &fireModel;
    fire2.shader = &fireShader;
    fire2.transform.position = vec3(3.0f, 1.0f, 7.0f);
    fire2.Update = &AnimateFire;
    world.Spawn(fire2);

    double deltaTime = 0.0;
    double fps = 0.0;

    // Application loop
    while (inputManager.Update(Canis::GetConfig().width, Canis::GetConfig().heigth))
    {
        deltaTime = frameRateManager.StartFrame();
        Canis::Graphics::ClearBuffer(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);

        // Update fire animation globally
        fireAnimTimer += deltaTime;
        if (fireAnimTimer >= FIRE_ANIM_SPEED) {
            fireAnimTimer -= FIRE_ANIM_SPEED; // Subtract instead of resetting to avoid timing drift
            currentFireFrame = (currentFireFrame + 1) % FIRE_FRAME_COUNT;
            
            // Log for debugging
            Canis::Log("Fire animation frame: " + std::to_string(currentFireFrame));
        }

        world.Update(deltaTime);
        world.Draw(deltaTime);

        editor.Draw();

        window.SwapBuffer();

        // EndFrame will pause the app when running faster than frame limit
        fps = frameRateManager.EndFrame();

        //Canis::Log("FPS: " + std::to_string(fps) + " DeltaTime: " + std::to_string(deltaTime));
    }

    return 0;
}

void Rotate(Canis::World &_world, Canis::Entity &_entity, float _deltaTime)
{
    //_entity.transform.rotation.y += _deltaTime;
}

void AnimateFire(Canis::World &_world, Canis::Entity &_entity, float _deltaTime)
{
    // Update the entity's texture to match the current global frame
    _entity.albedo = &fireTextures[currentFireFrame];
    
    // Optional: Make fire flicker a bit to add realism
    float flicker = 0.9f + 0.1f * sin(_world.GetTime() * 10.0f);
    _entity.transform.scale = vec3(1.0f, flicker, 1.0f);
}

// Function to randomly place flowers and grass in a specific area of the map
void RandomizeGrassAndFlowers(int startY, int endY, int startX, int endX, float grassChance = 0.4f, float flowerChance = 0.3f)
{
    // Verify the map is properly loaded
    if (map.size() < 2 || map[1].size() < endY || map[1][0].size() < endX) {
        Canis::Log("Error: Map is not properly loaded or specified region is out of bounds.");
        return;
    }

    // Make sure to seed the random number generator
    srand(static_cast<unsigned int>(time(nullptr)));

    // For each position in the specified region (second level = index 1)
    for (int y = startY; y < endY; y++) {
        for (int x = startX; x < endX; x++) {
            // Check if the current position is within array bounds
            if (y < map[1].size() && x < map[1][y].size()) {
                // Generate a random value between 0 and 1
                float randomValue = static_cast<float>(rand()) / RAND_MAX;
                
                if (randomValue < grassChance) {
                    // Place grass (2)
                    map[1][y][x] = 2;
                    Canis::Log("Placed grass at [1][" + std::to_string(y) + "][" + std::to_string(x) + "]");
                } else if (randomValue < (grassChance + flowerChance)) {
                    // Place flower (6)
                    map[1][y][x] = 6;
                    Canis::Log("Placed flower at [1][" + std::to_string(y) + "][" + std::to_string(x) + "]");
                } else {
                    // Leave empty (0) - this gives a chance for some spots to remain empty
                    map[1][y][x] = 0;
                    Canis::Log("Left empty at [1][" + std::to_string(y) + "][" + std::to_string(x) + "]");
                }
            }
        }
    }
}

// This function should be called after LoadMap and before entity spawning
void SetupRandomVegetation()
{
    // Randomize grass and flowers in the 5x5 section on level 1 (index 1 in map array)
    // Parameters: startY, endY, startX, endX, grassChance, flowerChance
    RandomizeGrassAndFlowers(5, 10, 15, 20, 0.4f, 0.3f);
}

void LoadMap(std::string _path)
{
    std::ifstream file;
    file.open(_path);

    if (!file.is_open())
    {
        printf("file not found at: %s \n", _path.c_str());
        exit(1);
    }

    int number = 0;
    int layer = 0;

    map.push_back(std::vector<std::vector<unsigned int>>());
    map[layer].push_back(std::vector<unsigned int>());

    while (file >> number)
    {
        if (number == -2) // add new layer
        {
            layer++;
            map.push_back(std::vector<std::vector<unsigned int>>());
            map[map.size() - 1].push_back(std::vector<unsigned int>());
            continue;
        }

        if (number == -1) // add new row
        {
            map[map.size() - 1].push_back(std::vector<unsigned int>());
            continue;
        }

        map[map.size() - 1][map[map.size() - 1].size() - 1].push_back((unsigned int)number);
    }
}

void SpawnLights(Canis::World &_world)
{
    Canis::DirectionalLight directionalLight;
    _world.SpawnDirectionalLight(directionalLight);

    Canis::PointLight pointLight;
    pointLight.position = vec3(0.0f);
    pointLight.ambient = vec3(0.2f);
    pointLight.diffuse = vec3(0.5f);
    pointLight.specular = vec3(1.0f);
    pointLight.constant = 1.0f;
    pointLight.linear = 0.09f;
    pointLight.quadratic = 0.032f;

    _world.SpawnPointLight(pointLight);

    pointLight.position = vec3(0.0f, 0.0f, 1.0f);
    pointLight.ambient = vec3(4.0f, 0.0f, 0.0f);

    _world.SpawnPointLight(pointLight);

    pointLight.position = vec3(-2.0f);
    pointLight.ambient = vec3(0.0f, 4.0f, 0.0f);

    _world.SpawnPointLight(pointLight);

    pointLight.position = vec3(2.0f);
    pointLight.ambient = vec3(0.0f, 0.0f, 4.0f);

    _world.SpawnPointLight(pointLight);
}