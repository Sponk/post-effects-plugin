#ifdef GL_ES
precision highp float;
#endif

uniform sampler2D Texture[4];
varying vec2 texcoord;

uniform float gamma;

// TODO: Light exposure berechnen um bloomStrength zu bestimmen bzw. tone mapping zu machen
uniform float bloomStrength;
uniform int bloomEnable;

// Radial blur
uniform int radialBlurEnable;
uniform float sampleDist; // = 2.0;
uniform float sampleStrength; // = 1.6; 

// TODO: Fertig machen!
vec3 fogColor =  vec3(0.0, 0.0, 0.0);
float minDist = 1.5;

void bloom_old()
{
	if(texcoord.x < 0.5)
		return;
		
	vec4 sum = vec4(0);
	vec2 texCoord = texcoord;
	int j;
	int i;

	for( i= -4 ;i < 4; i++)
	{
			for (j = -3; j < 3; j++)
			{
				sum += texture2D(Texture[0], texCoord + vec2(j, i)*0.004) * 0.2;
			}
	}
	   if (texture2D(Texture[0], texCoord).r < 0.3)
	{
	   gl_FragColor = sum*sum*0.012 + texture2D(Texture[0], texCoord);
	}
	else
	{
		if (texture2D(Texture[0], texCoord).r < 0.5)
		{
			gl_FragColor = sum*sum*0.009 + texture2D(Texture[0], texCoord);
		}
		else
		{
			gl_FragColor = sum*sum*0.0075 + texture2D(Texture[0], texCoord);
		}
	}
}

void bloom_new()
{
	vec4 pxl = texture2D(Texture[0], texcoord);
	float avg = ((pxl.r + pxl.g + pxl.b) / 3.0);
 
	// bloom
	 {
		vec4 sum = vec4(0.0);
		for (int i = -5; i <= 5; i++)
		{
			for (int j = -5; j <= 5; j++)
			{
			 sum += texture2D(Texture[0], (texcoord + vec2(i, j)
				   * 0.0018)) * bloomStrength;//0.015;
			}
		}
 
		if (avg < 0.025)
		{
			gl_FragColor = pxl + sum * 0.335;
		}
		else if (avg < 0.10)
		{
			gl_FragColor = pxl + (sum * sum) * 0.5;
		}
		else if (avg < 0.88)
		{
			gl_FragColor = pxl + ((sum * sum) * 0.333);
		}
		else if (avg >= 0.88)
		{
			gl_FragColor = pxl + sum;
		}
		else
		{
			gl_FragColor = pxl;
		}
	}
}

void radial_blur()
{
	float samples[10] =
	float[](-0.08,-0.05,-0.03,-0.02,-0.01,0.01,0.02,0.03,0.05,0.08);
 
	vec2 dir = 0.5 - texcoord; 
 
	float dist = sqrt(dir.x*dir.x + dir.y*dir.y); 
 
	dir = dir/dist; 
	vec4 color = texture2D(Texture[0],texcoord); 
 
	vec4 sum = color;

	for (int i = 0; i < 10; i++)
	{
		sum += texture2D(Texture[0], texcoord + dir * samples[i] * sampleDist );
	}
 
	sum *= 1.0/11.0;
 
	float t = dist * sampleStrength;
	t = clamp( t ,0.0,1.0); //0 &lt;= t &lt;= 1

	gl_FragColor = mix( color, sum, t );
}

void fog(vec3 col)
{
	vec3 d = texture2D(Texture[1], texcoord).xyz;
	
	//if(length(d) < minDist)
	//	return;
	
	vec3 fogCol = vec3(1.0, 1.0, 1.0);
	
	d.rgb = ((d.rgb - 0.5f) * max(3.0, 0)) + 0.5f;
	col *= fogCol * d;
	gl_FragColor = vec4(col.r, col.g, col.b, 1.0);
}

void gamma_correction()
{
	vec2 uv = gl_TexCoord[0].xy;
	vec3 color = gl_FragColor.xyz;

	gl_FragColor.rgb = pow(color, vec3(1.0 / gamma));
}

vec4 tone_map(vec4 hdrColor, float exposure)
{
    vec4 ldrColor = 1.f - exp2 (-hdrColor * exposure);
    ldrColor.a = 1.f;

    return ldrColor;
}

void anti_aliasing(void)
{
    
}

void main()
{
	if(bloomEnable == 1 && radialBlurEnable == 0)
		bloom_new();
		
	if(radialBlurEnable)
		radial_blur();
	
	//bloom_old();

	//gl_FragColor = texture2D(Texture[0], texcoord);
	
	//gl_FragColor = tone_map(gl_FragColor, 2.0);
	
	// gl_FragColor *= 1.4;
	
	//gl_FragColor = tone_map(gl_FragColor, 1.9);
	gamma_correction();
}
