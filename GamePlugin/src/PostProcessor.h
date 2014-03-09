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

#ifndef __POST_PROCESSOR_H__
#define __POST_PROCESSOR_H__

#include "Shader.h"

#include <MEngine.h>
#include <MTexture.h>
#include <MImage.h>
#include <deque>
#include <vector>

using std::deque;
using std::vector;

class PostProcessor
{
public:
	PostProcessor();

	bool Render();

	void SetShader(Shader* s);
	const Shader* GetShader() { return m_Shader; }

    void UpdateResolution();
	void EraseTextures();

    void AddFloatUniform(const char* name);
    void AddIntUniform(const char* name);

    void SetIntUniformValue(const char* name, int value);
    void SetFloatUniformValue(const char* name, float value);

    void Clear();

private:
	void Set2D(unsigned int w, unsigned int h);
	void DrawQuad(MVector2 scale);

	void SendUniforms();

	unsigned int m_BufferID;
	unsigned int m_ColourTexID;
	unsigned int m_DepthTexID;
	Shader*	m_Shader;
	int m_Resolution;

	typedef struct
	{
        char name[255];
        float value;
	}float_uniform_t;

    typedef struct
	{
        char name[255];
        int value;
	}int_uniform_t;

	vector<float_uniform_t> m_FloatUniformList;
	vector<int_uniform_t> m_IntUniformList;
};

#endif/*__POST_PROCESSOR_H__*/