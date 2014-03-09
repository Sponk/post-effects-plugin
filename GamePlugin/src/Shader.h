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

#ifndef __SHADER_H__
#define __SHADER_H__

class Shader
{
public:
    Shader(const char* vertexName, const char* fragmentName);

    void Apply();
    void Clear();

    unsigned int ExposeShader() { return m_fx; }

    void SetValue(const char* name, int val) const;
    void SetValue(const char* name, float val) const;
private:
    bool Init();

    unsigned int m_fx;
    unsigned int m_vertShad;
    unsigned int m_pixShad;

    char m_filenames[2][256];
};

#endif /*__SHADER_H__*/
