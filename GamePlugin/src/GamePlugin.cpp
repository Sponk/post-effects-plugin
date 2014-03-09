//
// Copyright (C) 2014
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#include "GamePlugin.h"
#include "Game.h"

Game* game = NULL;

/*************************************************
 * LUA functionality
 * TODO: Own source file!
 *************************************************/

/**
 * Enables post processing and initializes it with with
 * the given shaders.
 * Lua usage:
 * @arg vertexShader The vertex shader to use
 * @arg fragmentShader The fragment shader to use
 */
int enablePostProcessing()
{
	MEngine* engine = MEngine::getInstance();
	Game* game = (Game*) engine->getGame();
	MScriptContext* script = engine->getScriptContext();
    const char* vertShad;
    const char* fragShad;

	if(script->getArgsNumber() != 2)
		return 0;

    if((vertShad = script->getString(0)) == NULL || (fragShad = script->getString(1)) == NULL)
    {
        fprintf(stderr, "Error: Could not enable post processing!");
        return 0;
    }

	//bool enabled = (script->getInteger(0) == 1) ? true : false;
	game->enablePostProcessing(true);
    game->setPostFXShader(vertShad, fragShad);

	return 1;
}

int addFloatUniform()
{
    MEngine* engine = MEngine::getInstance();
	Game* game = (Game*) engine->getGame();
	MScriptContext* script = engine->getScriptContext();

	if(script->getArgsNumber() != 1)
        return 0;

    game->getPostProcessor()->AddFloatUniform(script->getString(0));

    return 1;
}

int addIntUniform()
{
    MEngine* engine = MEngine::getInstance();
	Game* game = (Game*) engine->getGame();
	MScriptContext* script = engine->getScriptContext();

	if(script->getArgsNumber() != 1)
        return 0;

    game->getPostProcessor()->AddIntUniform(script->getString(0));

    return 1;
}

int setFloatUniform()
{
    MEngine* engine = MEngine::getInstance();
	Game* game = (Game*) engine->getGame();
	MScriptContext* script = engine->getScriptContext();

	if(script->getArgsNumber() != 2)
        return 0;

    game->getPostProcessor()->SetFloatUniformValue(script->getString(0), script->getFloat(1));

    return 1;
}

int setIntUniform()
{
    MEngine* engine = MEngine::getInstance();
	Game* game = (Game*) engine->getGame();
	MScriptContext* script = engine->getScriptContext();

	if(script->getArgsNumber() != 2)
        return 0;

    game->getPostProcessor()->SetIntUniformValue(script->getString(0), script->getInteger(1));

    return 1;
}

extern "C" void StartPlugin(void)
{
	// get engine
	MEngine * engine = MEngine::getInstance();
    MScriptContext* script = engine->getScriptContext();

    script->addFunction("addFloatUniform", addFloatUniform);
    script->addFunction("addIntUniform", addIntUniform);

    script->addFunction("setFloatUniform", setFloatUniform);
    script->addFunction("setIntUniform", setIntUniform);

    script->addFunction("enablePostProcessing", enablePostProcessing);

	game = new Game();
	engine->setGame(game);
}

extern "C" void EndPlugin(void)
{
	SAFE_DELETE(game);
}
