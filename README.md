# Engine

<p align="center">
  <img src="GithubFile/logo.jpg" alt="Logo"/>
</p>

[Sapphire Engine](http://sapphire.github.io) is a cross-platform 2D and 3D game engine implemented in C++. It's free, open-source, and works on Windows, Linux, Emscripten and Android. The project is currently in an early stage of development. 

[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)

# Table of Contents

- [Features](#features)
- [Cloning the Repository](#cloning)
  - [Repository Structure](#repository_structure)
- [Build and Run Instructions](#build_and_run)
  - [Win32 - Visual Studio 2019](#build_and_run_win32)

<a name="features"></a>
# Features

* Core
  * Modern c++ - support C++17
  * Cross-platform
    * Windows (Win32)
    * Linux (WIP)
    * Android (WIP)
    * Web (Emscripten) (WIP)
  * Minimal other depends
  * High performance
  * Extensive validation and error reporting
  
* Rendering Backend
  * Support GAPI
    * Direct3D 11
	* Direct3D 12
	* Vulkan
	* OpenGL 3.3-4.6

<a name="cloning"></a>
# Cloning the Repository

To get the repository, use the following command:

```
git clone --recursive https://github.com/SapphireEngine/Engine
```

When updating existing repository and update all submodules:

```
git pull
git submodule update --recursive
```

<a name="repository_structure"></a>
## Repository Structure

<a name="build_and_run"></a>
# Build and Run Instructions

Sapphire Engine uses [Premake5](https://premake.github.io/download.html) as a cross-platform build tool. 

<a name="build_and_run_win32"></a>
## Win32 - Visual Studio 2019

In command prompt:
```
cd BuildTools
GenerateVS19.bat
```
And open solution build\SapphireEngine.sln