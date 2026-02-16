#version 460 core

out vec4 FragColor;

in vec3 TexCoords;

uniform vec3 u_SunDirection;
uniform float u_SunIntensity;
uniform float u_SunBloomSize;
uniform float u_SunRadius;
uniform float u_StarSize;
uniform float u_StarDensity;

uniform vec3 u_DayZenithColor;
uniform vec3 u_DaySunColor;
uniform vec3 u_DayHorizonColor;
uniform vec3 u_SunsetZenithColor;
uniform vec3 u_SunsetSunColor;
uniform vec3 u_SunsetHorizonColor;
uniform vec3 u_NightZenithColor;
uniform vec3 u_NightSunColor;
uniform vec3 u_NightHorizonColor;


void main()
{
	vec3 viewDir = normalize(TexCoords);

	vec3 lightDirection = -u_SunDirection;

	// caluclate sun hieght
	float sunHeight = lightDirection.y;

	float gradient = smoothstep(0.0, 0.8, viewDir.y);

	// determine mixing
	float sunsetMix = smoothstep(-0.1, 0.2, sunHeight) * (1.0 - smoothstep(0.2, 0.5, sunHeight));
	float dayMix = smoothstep(0.2, 0.4, sunHeight);
	float nightMix = 1.0 - smoothstep(-0.2, 0.1, sunHeight);

	// calc sky color
	vec3 finalSky = vec3(0.0f);

	// Day State
	vec3 dayColor = mix(u_DayHorizonColor, u_DayZenithColor, gradient);
	vec3 sunsetColor = mix(u_SunsetHorizonColor, u_SunsetZenithColor, gradient);
	vec3 nightColor = mix(u_NightHorizonColor, u_NightZenithColor, gradient);

	finalSky = dayColor * dayMix + sunsetColor * sunsetMix + nightColor * nightMix;

	// Sun disc
	float sunDot = dot(viewDir, lightDirection);
	float sunDisk = smoothstep(u_SunRadius, u_SunRadius + 0.001, sunDot);
	float sunBloom = smoothstep(u_SunBloomSize, 1.0, sunDot) * 0.5;

	vec3 finalSun = u_DaySunColor * dayMix + u_SunsetSunColor * sunsetMix + u_NightSunColor * nightMix;

	finalSky += (sunDisk + sunBloom) * finalSun * (u_SunIntensity * 10.0);

	if (nightMix > 0.0)
	{
		// should maybe later pass in noise map instead
		vec2 skyUV = vec2(atan(viewDir.z, viewDir.x), asin(viewDir.y));
		float starRes = u_StarSize;
		vec2 quantUV = floor(skyUV * starRes);
		float starNoise = fract(sin(dot(quantUV, vec2(12.9898, 78.233))) * 43758.5453);
		float starVisible = step(u_StarDensity, starNoise);
		finalSky += vec3(starVisible * nightMix);
	}

	FragColor = vec4(finalSky, 1.0);
}