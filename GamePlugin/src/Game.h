/* * Copyright (C) 2014
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#ifndef _GAME_H
#define _GAME_H

#include <MEngine.h>
#include <string>
#include "PostProcessor.h"

class Game : public MGame
{
public:

	Game();
	~Game();

	void onBegin();
	void onEnd();

	void draw();
	void update();

	void enablePostProcessing(bool enabled) { postfxEnabled = enabled; };
	void setPostFXShader(const char* vertShad, const char* fragShad);
	PostProcessor* getPostProcessor() { return &postProcessor; }

	//void onBeginLevel(void){}
	//void onEndLevel(void){}

	//void onBeginScene(void);
	//void onEndScene(void);

private:
	PostProcessor postProcessor;
	bool postfxEnabled;
	std::string vertShad;
	std::string fragShad;

	unsigned int windowWidth;
	unsigned int windowHeight;
};

#endif
