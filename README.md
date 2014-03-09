Post Effects Plugin
===================

The Post Effects Plugin allows you the post processing of your realtime rendered scenes using GLSL shaders. This make effects like 'Motion Blur' or 'Bloom' possible to improve the visual fidelity. 
This code is based on a plugin written by Philipp Geyer (aka Nistur) . 

Features
===================

1) Bloom Shader

2) Radial Blur Shader

3) Completely custamizable via LUA-Script (even uniforms)

Installation
===================

The archive contains an example project showing possible usage in a demo scene. To actually use the plugin in your game you first have to include it: 

First you have to copy the directory 'GamePlugin' into your own project directory. You own code can then be copied to 'GamePlugin/src'. You should not replace any files from this package with your own unless you know what you are doing! 

After you copied all files you might have to register all behaviors/LUA-Functuanility in 'GamePlugin.cpp' like you would do it with the standard Maratis library. 

You have to go to 'GamePlugin/build' with your commandline application and type 'cmake ../src'. You need to have CMake installed for this! 

Compilation is now as easy as typing 'make'. It is alternatively possible to use 'Code::Blocks' or 'VisualStudio' to compile the code.

Usage
===================

The usage is quite easy:

1) enablePostProcessing(vertexShader, fragmentShader) => Enables postprocessing with the specified shaders. Both arguments are paths to the shader files.

2) addIntUniform(name) => Creates a new integer uniform which is sent to the post effects shader.

3) addFloatUniform(name) => Creates a new float uniform which is sent to the post effects shader.

4) setIntUniform(name, value) => Sets the value of the specified (and previously created) uniform variable.

5) setFloatUniform(name, value) => Sets the value of the specified (and previously created) uniform variable.