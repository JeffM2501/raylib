/*******************************************************************************************
*
*   raylib [core] example - Doing skinning on the gpu using a vertex shader
* 
*   Example originally created with raylib 4.5, last time updated with raylib 4.5
*
*   Example contributed by Daniel Holden (@orangeduck) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2024 Daniel Holden (@orangeduck)
* 
*   Note: Due to limitations in the Apple OpenGL driver, this feature does not work on MacOS
*
********************************************************************************************/

#include "raylib.h"

#include "raymath.h"

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - GPU skinning");

    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 5.0f, 5.0f, 5.0f }; // Camera position
    camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };  // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };      // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                            // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;         // Camera projection type

    // Load gltf model
    Model characterModel = LoadModel("resources/models/gltf/greenman.glb"); // Load character model
    
//     // Load skinning shader
    Shader skinningShader = LoadShader(TextFormat("resources/shaders/glsl%i/skinning.vs", GLSL_VERSION),
                                        TextFormat("resources/shaders/glsl%i/skinning.fs", GLSL_VERSION));
    
    for (int i = 0; i < characterModel.materialCount; i++)
    {
        characterModel.materials[i].shader = skinningShader;
    }
    
    // Load gltf model animations
    int animsCount = 0;
    unsigned int animIndex1 = 0;
    unsigned int animCurrentFrame1 = 0;
	unsigned int animIndex2 = 1;
	unsigned int animCurrentFrame2 = 0;
    ModelAnimation *modelAnimations = LoadModelAnimations("resources/models/gltf/greenman.glb", &animsCount);

    Vector3 position1 = { 0.0f, 0.0f, 0.0f }; // Set model position
	Vector3 position2 = { 0.0f, 0.0f, 2.0f }; // Set model position

    ModelBonePose* pose1 = LoadModelBonePose(characterModel);
    ModelBonePose* pose2 = LoadModelBonePose(characterModel);

    DisableCursor();                    // Limit cursor to relative movement inside the window

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera, CAMERA_THIRD_PERSON);
        
        // Select current animation
        if (IsKeyPressed(KEY_T))
        {
            animIndex1 = (animIndex1 + 1) % animsCount;
            animIndex2 = (animIndex2 + 1) % animsCount;
        }
        else if (IsKeyPressed(KEY_G))
        {
            animIndex1 = (animIndex1 + animsCount - 1) % animsCount;
            animIndex2 = (animIndex2 + animsCount - 1) % animsCount;
        }
 
        // Update model animation
        ModelAnimation anim = modelAnimations[animIndex1];
        animCurrentFrame1 = (animCurrentFrame1 + 1)%anim.frameCount;
        UpdateModelAnimationBonesPose(characterModel, anim, animCurrentFrame1, pose1);

		anim = modelAnimations[animIndex2];
		animCurrentFrame2 = (animCurrentFrame2 + 1) % anim.frameCount;
        float param = (sinf(GetTime()) + 1) / 2.0f;
        InterpolateModelAnimationBonesPose(characterModel, anim, 0, anim.frameCount/2, param, pose2);
        //---------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);
            
            // Draw character models, pose calculation is done in shader (GPU skinning)
			DrawModelPro(characterModel, position1, Vector3Zero(), 0, Vector3One(), WHITE, pose1);
            DrawModelPro(characterModel, position2, Vector3Zero(), 0, Vector3One(), WHITE, pose2);
			DrawGrid(10, 1.0f);
                
            EndMode3D();

            DrawText("Use the T/G to switch animation", 10, 10, 20, GRAY);

            DrawText(TextFormat("param %f", param), 20, 40, 20, RED);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadModelBonePose(pose1);
    UnloadModelBonePose(pose2);
    UnloadModelAnimations(modelAnimations, animsCount); // Unload model animation
    UnloadModel(characterModel);    // Unload model and meshes/material
    UnloadShader(skinningShader);   // Unload GPU skinning shader
    
    CloseWindow();                  // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}