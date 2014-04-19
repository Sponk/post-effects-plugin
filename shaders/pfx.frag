#version 130

#ifdef GL_ES
precision highp float;
#endif

#define PI 3.14159265

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

// Fog
uniform vec3 fogColor =  vec3(0.0, 0.0, 0.0);
uniform float minDist = 10.0;
uniform int fogEnable = 0;

vec4 bloom_old()
{			
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
	
	return gl_FragColor;
}

vec4 bloom_new()
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
	
	return gl_FragColor;
}

vec4 radial_blur()
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

	return mix( color, sum, t );
}

float linearize_depth(vec2 uv)
{
  float n = 1.0; // camera z near
  float f = 100.0; // camera z far
  float z = texture2D(Texture[1], uv).x;
  return (2.0 * n) / (f + n - z * (f - n));	
}

void fog(vec3 col, vec3 fogCol, float distance)
{
	float depth = 1 - linearize_depth(texcoord);
	vec3 z = vec3(depth, depth, depth);

	//if(length(d) < minDist)
	//	return;

	const float LOG2 = 1.442695;
	float fogFactor = exp2( -distance * 
					   distance * 
					   z * 
					   z * 
					   LOG2 ).x;
	fogFactor = clamp(fogFactor, 0.0, 1.0);
	gl_FragColor.rgb = mix(fogCol, col, 1-fogFactor);
}

void gamma_correction()
{
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

float rand(vec2 co)
{
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float rand2(vec2 co, float rnd_scale)
{
	vec2 v1 = vec2(92.,80.);
	vec2 v2 = vec2(41.,62.);
  return fract(sin(dot(co.xy ,v1)) + cos(dot(co.xy ,v2)) * rnd_scale);
}

vec4 ice2(float rnd_factor, float rnd_scale) 
{ 
  vec2 uv = texcoord;
  
  vec3 tc = vec3(1.0, 0.0, 0.0);
  vec2 rnd = vec2(rand2(uv.xy, rnd_scale),rand2(uv.yx, rnd_scale));  
  tc = texture2D(Texture[0], uv+rnd*rnd_factor).rgb;  
  
  return vec4(tc, 1.0);
}

vec4 dissolve(float rnd_factor, float rnd_scale)
{
  vec2 uv = texcoord;
  
  vec3 tc = vec3(1.0, 0.0, 0.0);
  vec2 rnd = vec2(rand(uv.xy),rand(uv.yx)); 
  tc = texture2D(Texture[0], uv+rnd*rnd_factor).rgb;  
  
  return vec4(tc, 1.0);
}

vec4 fxaa(sampler2D textureSampler, vec2 vertTexcoord, vec2 texcoordOffset) 
{
  // The parameters are hardcoded for now, but could be
  // made into uniforms to control fromt he program.
  float FXAA_SPAN_MAX = 8.0;
  float FXAA_REDUCE_MUL = 1.0/8.0;
  float FXAA_REDUCE_MIN = (1.0/128.0);

  vec3 rgbNW = texture2D(textureSampler, vertTexcoord + (vec2(-1.0, -1.0) * texcoordOffset)).xyz;
  vec3 rgbNE = texture2D(textureSampler, vertTexcoord + (vec2(+1.0, -1.0) * texcoordOffset)).xyz;
  vec3 rgbSW = texture2D(textureSampler, vertTexcoord + (vec2(-1.0, +1.0) * texcoordOffset)).xyz;
  vec3 rgbSE = texture2D(textureSampler, vertTexcoord + (vec2(+1.0, +1.0) * texcoordOffset)).xyz;
  vec3 rgbM  = texture2D(textureSampler, vertTexcoord).xyz;
	
  vec3 luma = vec3(0.299, 0.587, 0.114);
  float lumaNW = dot(rgbNW, luma);
  float lumaNE = dot(rgbNE, luma);
  float lumaSW = dot(rgbSW, luma);
  float lumaSE = dot(rgbSE, luma);
  float lumaM  = dot( rgbM, luma);
	
  float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
  float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
	
  vec2 dir;
  dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
  dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));
	
  float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
	  
  float rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);
	
  dir = min(vec2(FXAA_SPAN_MAX,  FXAA_SPAN_MAX), 
        max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), dir * rcpDirMin)) * texcoordOffset;
		
  vec3 rgbA = (1.0/2.0) * (
              texture2D(textureSampler, vertTexcoord + dir * (1.0/3.0 - 0.5)).xyz +
              texture2D(textureSampler, vertTexcoord + dir * (2.0/3.0 - 0.5)).xyz);
  vec3 rgbB = rgbA * (1.0/2.0) + (1.0/4.0) * (
              texture2D(textureSampler, vertTexcoord + dir * (0.0/3.0 - 0.5)).xyz +
              texture2D(textureSampler, vertTexcoord + dir * (3.0/3.0 - 0.5)).xyz);
  float lumaB = dot(rgbB, luma);

  vec4 outfrag;
  
  if((lumaB < lumaMin) || (lumaB > lumaMax))
  {
    outfrag.xyz = rgbA;
  } 
  else 
  {
    outfrag.xyz = rgbB;
  }
  
  return outfrag;
}

vec3 normal_from_depth(float depth, vec2 texcoords) 
{
  
  vec2 offset1 = vec2(0.0,0.001);
  vec2 offset2 = vec2(0.001,0.0);
  
  float depth1 = texture2D(Texture[1], texcoords + offset1).r;
  float depth2 = texture2D(Texture[1], texcoords + offset2).r;
  
  vec3 p1 = vec3(offset1, depth1 - depth);
  vec3 p2 = vec3(offset2, depth2 - depth);
  
  vec3 normal = cross(p1, p2);
  normal.z = -normal.z;
  
  return normalize(normal);
}

///////////////////////////////////////////////////////////////////////////////
// SSAO Implementation
///////////////////////////////////////////////////////////////////////////////

int samples = 32; //ao sample count

float radius = 6.0; //ao radius
float aoclamp = 0.25; //depth clamp - reduces haloing at screen edges
bool noise = true; //use noise instead of pattern for sample dithering
float noiseamount = 0.0002; //dithering amount

float diffarea = 0.4; //self-shadowing reduction
float gdisplace = 0.4; //gauss bell center

bool mist = true; //use mist?
float miststart = 0.0; //mist start
float mistend = 16.0; //mist end

uniform int onlyAO = 0; //use only ambient occlusion pass?
float lumInfluence = 0.5; //how much luminance affects occlusion

uniform float Width = 1024;
uniform float Height = 768;

float width = Width;
float height = Height;

vec2 rand3(vec2 coord)
{
	float noiseX = ((fract(1.0-coord.s*(width/2.0))*0.25)+(fract(coord.t*(height/2.0))*0.75))*2.0-1.0;
	float noiseY = ((fract(1.0-coord.s*(width/2.0))*0.75)+(fract(coord.t*(height/2.0))*0.25))*2.0-1.0;
	
	if (noise)
	{
		noiseX = clamp(fract(sin(dot(coord ,vec2(12.9898,78.233))) * 43758.5453),0.0,1.0)*2.0-1.0;
		noiseY = clamp(fract(sin(dot(coord ,vec2(12.9898,78.233)*2.0)) * 43758.5453),0.0,1.0)*2.0-1.0;
	}
	
	return vec2(noiseX,noiseY)*noiseamount;
}

float do_mist()
{
	float depth = linearize_depth(texcoord).x;
	return clamp((depth-miststart)/mistend,0.0,1.0);
}

float compare_depths(float depth1, float depth2, int far)
{   
	float garea = 2.0;
	float diff = (depth1 - depth2)*100.0;

	if (diff<gdisplace)
	{
		garea = diffarea;
	}
	else
	{
		far = 1;
	}
	
	float gauss = pow(2.7182,-2.0*(diff-gdisplace)*(diff-gdisplace)/(garea*garea));
	return gauss;
}   

float cal_ao(float depth,float dw, float dh)
{   
	float dd = (1.0-depth)*radius;
	
	float temp = 0.0;
	float temp2 = 0.0;
	float coordw = texcoord.x + dw*dd;
	float coordh = texcoord.y + dh*dd;
	float coordw2 = texcoord.x - dw*dd;
	float coordh2 = texcoord.y - dh*dd;
	
	vec2 coord = vec2(coordw , coordh);
	vec2 coord2 = vec2(coordw2, coordh2);
	
	// TODO: Get far via uniform (Also for linearize_depth)!
	int far = 100;
	temp = compare_depths(depth, linearize_depth(coord),far);

	if (far > 0)
	{
		temp2 = compare_depths(linearize_depth(coord2),depth,far);
		temp += (1.0-temp)*temp2;
	}
	
	return temp;
} 

vec4 ssao(vec4 outColor)
{
	vec2 noise = rand3(texcoord); 
	float depth = linearize_depth(texcoord);
	
	float w = (1.0 / width)/clamp(depth,aoclamp,1.0)+(noise.x*(1.0-noise.x));
	float h = (1.0 / height)/clamp(depth,aoclamp,1.0)+(noise.y*(1.0-noise.y));
	
	float pw;
	float ph;
	
	float ao;
	
	float dl = PI*(3.0-sqrt(5.0));
	float dz = 1.0/float(samples);
	float l = 0.0;
	float z = 1.0 - dz/2.0;
	
	for (int i = 0; i <= samples; i ++)
	{     
		float r = sqrt(1.0-z);
		
		pw = cos(l)*r;
		ph = sin(l)*r;
		ao += cal_ao(depth,pw*w,ph*h);        
		z = z - dz;
		l = l + dl;
	}
	
	ao /= float(samples);
	ao = 1.0-ao;	
	
	if (mist)
	{
		ao = mix(ao, 1.0,do_mist());
	}
	
	vec3 color = outColor.rgb;
	
	vec3 lumcoeff = vec3(0.299,0.587,0.114);
	float lum = dot(color.rgb, lumcoeff);
	vec3 luminance = vec3(lum, lum, lum);
	
	vec3 final = vec3(color*mix(vec3(ao),vec3(1.0),luminance*lumInfluence));//mix(color*ao, white, luminance)
	
	if (onlyAO == 1)
	{
		final = vec3(mix(vec3(ao),vec3(1.0),luminance*lumInfluence)); //ambient occlusion only
	}
	
	
	outColor = vec4(final,1.0); 
	return outColor;
	
}

void main()
{
	if(bloomEnable == 1 && radialBlurEnable == 0)
		bloom_old();
	
	else if(radialBlurEnable == 1 && bloomEnable == 0)
		gl_FragColor = radial_blur();
	else if(radialBlurEnable == 1 && bloomEnable == 1)
		gl_FragColor = mix(radial_blur(), bloom_new(), 0.5);
	
	if(fogEnable == 1)
		fog(gl_FragColor.rgb, fogColor, minDist);

	//gl_FragColor = mix(gl_FragColor, fxaa(Texture[0], texcoord, vec4(0.001, 0.01, 0.001, 0.0)), 0.3);
		
	//gl_FragColor = mix(gl_FragColor, ice2(0.01, 0.01), 0.5);

	//gl_FragColor = mix(gl_FragColor, dissolve(0.01, 0.01), 0.5);

	//bloom_old();

	//gl_FragColor = texture2D(Texture[0], texcoord);
	
	//gl_FragColor = tone_map(gl_FragColor, 2.0);
	
	// gl_FragColor *= 1.4;
	
	gl_FragColor = ssao(gl_FragColor);
	
	//gl_FragColor = tone_map(gl_FragColor, 1.9);
	gamma_correction();
	gl_FragColor.a = 1.0;
}
