# MagmaCraft

<p align="center">
<img src="resources/magma_logo.png" width="400" alt="MagmaCraft Logo">
</p>

## A Custom Voxel Multiplayer Game

**MagmaCraft** is a voxel-based sandbox game built entirely from scratch using C++ and OpenGL. Built upon the custom **Magma Framework**, it serves as a technical showcase of voxel rendering, client-server networking, and entity management without relying on existing game engines.

<img src="resources/ss.png" alt="MagmaCraft Screenshot">

## Key Features

* **Custom Voxel Engine:** Efficient chunk rendering, texture atlasing, and world streaming.
* **Multiplayer Networking:** robust client-server architecture powered by **ENet**.
* Supports Hosting (Listen Server) and Joining (Client).
* Entity interpolation for smooth movement across the network.
* Real-time player synchronization and connection handling.


* **3D Rendering:** Custom rendering pipeline supporting FBX model loading, shaders, and dynamic textures.
* **World Management:** Infinite terrain generation with chunk compression and saving/loading systems.
* **User Interface:** Integrated UI for server browsing, login, and in-game HUDs using ImGui and a custom 2D renderer.

## Tech Stack

* **Language:** C++ (C++20)
* **Graphics:** OpenGL (Modern Core Profile)
* **Networking:** ENet
* **Windowing/Input:** SDL2
* **Math:** GLM
* **Build System:** CMake

## Getting Started

MagmaCraft uses **CMake** for easy cross-platform building.

1. **Clone the Repository** (Ensure you have Visual Studio or a C++ compiler installed).
2. Open Visual Studio and select **File -> Open -> CMake Project**.
3. Navigate to the repository folder and select `CMakeLists.txt`.
4. Allow CMake to configure the project (dependencies are managed via Vcpkg or submodules where applicable).
5. Select `MagmaCraft.exe` as the startup item and run!

**Note:** Ensure the `resources` folder is in your working directory or next to the executable for textures and models to load correctly.

## Controls

* **W, A, S, D**: Move
* **Space**: Jump
* **Mouse**: Look
* **T**: Play Audio Test
* **Esc**: Open Menu / Release Mouse

## License

MagmaCraft is open source and available under the **MIT License**. Feel free to use the underlying Magma Framework for your own learning or game projects.

**Created by Giovanni Perez Colon**