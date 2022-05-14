#version 330

// Uniforms
uniform sampler2D imageTex;
uniform sampler2D depthTex;
uniform sampler2D normalsTex;
uniform sampler2D materialsTex;
uniform vec2 viewportSize;
uniform bool fxaaEnabled;
uniform bool shadeOcean = false;

// Depth line Properties
uniform int depthLineWidth;
uniform float depthLineThreshold;
uniform float AAThreshold = 0.5;
uniform float AAIntensity = 2;

// Normal line Properties
uniform int normalLineWidth;
uniform float normalLineThreshold;

// Inputs
in vec2 geom_texCoord;

// Outputs
out vec4 fragColor;

float DepthCalc(vec2 ScreenSpaceUV) {
	vec2 FrameBufferWH = viewportSize;
	float MyDepth = texture(depthTex, ScreenSpaceUV).r;
	float TempDepth = texture(depthTex, ScreenSpaceUV + depthLineWidth * vec2(1, 0) / FrameBufferWH).r;
	float DepthCount = abs(MyDepth - TempDepth);

	TempDepth = texture(depthTex, ScreenSpaceUV + depthLineWidth * vec2(-1, 0) / FrameBufferWH).r;
	DepthCount += abs(MyDepth - TempDepth);

	TempDepth = texture(depthTex, ScreenSpaceUV + depthLineWidth * vec2(0, 1) / FrameBufferWH).r;
	DepthCount += abs(MyDepth - TempDepth);

	TempDepth = texture(depthTex, ScreenSpaceUV + depthLineWidth * vec2(0, -1) / FrameBufferWH).r;
	DepthCount += abs(MyDepth - TempDepth);

	return DepthCount;
}

float NormalCalc(vec2 ScreenSpaceUV) {
	vec2 FrameBufferWH = viewportSize;
	vec3 MyNormal = normalize(2 * texture(normalsTex, ScreenSpaceUV).xyz - 1);
	vec3 TempNormal = normalize(2 * texture(normalsTex, ScreenSpaceUV + normalLineWidth * vec2(1, 0) / FrameBufferWH).xyz - 1);
	vec3 NormalDiff = abs(MyNormal - TempNormal);
    float NormalCount = NormalDiff.x + NormalDiff.y + NormalDiff.z;

	TempNormal = normalize(2 * texture(normalsTex, ScreenSpaceUV + normalLineWidth * vec2(-1, 0) / FrameBufferWH).xyz - 1);
	NormalDiff = abs(MyNormal - TempNormal);
    NormalCount += NormalDiff.x + NormalDiff.y + NormalDiff.z;

	TempNormal = normalize(2 * texture(normalsTex, ScreenSpaceUV + normalLineWidth * vec2(0, 1) / FrameBufferWH).xyz - 1);
	NormalDiff = abs(MyNormal - TempNormal);
    NormalCount += NormalDiff.x + NormalDiff.y + NormalDiff.z;

	TempNormal = normalize(2 * texture(normalsTex, ScreenSpaceUV + normalLineWidth * vec2(0, -1) / FrameBufferWH).xyz - 1);
	NormalDiff = abs(MyNormal - TempNormal);
    NormalCount += NormalDiff.x + NormalDiff.y + NormalDiff.z;

	return NormalCount;
}

vec3 AfterColor(vec2 ScreenSpaceUV) {
	if (DepthCalc(ScreenSpaceUV) > depthLineThreshold || NormalCalc(ScreenSpaceUV) > normalLineThreshold) {
        return vec3(0);
    } else {
		return texture(imageTex, ScreenSpaceUV).xyz;
	}
}

vec3 lerp(vec3 oriColor, vec3 newColor, float alpha) {
	return (1 - alpha) * oriColor + alpha * newColor;
}

void main() {
    fragColor = texture(imageTex, geom_texCoord);

    bool isOceanPixel = texture(materialsTex, geom_texCoord).z == 1;
    if (!shadeOcean && isOceanPixel) {
        return;
    }

	bool fxaaDepth = false;
	bool fxaaNormal = false;

    float myDepth = texture(depthTex, geom_texCoord).x;
    float leftDepth = texture(depthTex, geom_texCoord + vec2(-depthLineWidth, 0) / viewportSize).x;
    float depthCountL = abs(myDepth - leftDepth);
    float rightDepth = texture(depthTex, geom_texCoord + vec2(depthLineWidth, 0) / viewportSize).x;
    float depthCountR = abs(myDepth - rightDepth);
    float downDepth = texture(depthTex, geom_texCoord + vec2(0, -depthLineWidth) / viewportSize).x;
    float depthCountD = abs(myDepth - downDepth);
    float upDepth = texture(depthTex, geom_texCoord + vec2(0, depthLineWidth) / viewportSize).x;
    float depthCountU = abs(myDepth - upDepth);

    float depthTest = depthCountL + depthCountR + depthCountD + depthCountU;
    if (depthTest > depthLineThreshold) {
        fragColor.xyz = vec3(0);
        fragColor.w = 1;
		fxaaDepth = true;
    }

    vec3 myNormal = normalize(2 * texture(normalsTex, geom_texCoord).xyz - 1);
    vec3 leftNormal = normalize(2 * texture(normalsTex, geom_texCoord + vec2(-normalLineWidth, 0) / viewportSize).xyz - 1);
    vec3 normalDiff = abs(myNormal - leftNormal);
    float normalCountL = normalDiff.x + normalDiff.y + normalDiff.z;
    vec3 rightNormal = normalize(2 * texture(normalsTex, geom_texCoord + vec2(normalLineWidth, 0) / viewportSize).xyz - 1);
    normalDiff = abs(myNormal - rightNormal);
    float normalCountR = normalDiff.x + normalDiff.y + normalDiff.z;
    vec3 downNormal = normalize(2 * texture(normalsTex, geom_texCoord + vec2(0, -normalLineWidth) / viewportSize).xyz - 1);
    normalDiff = abs(myNormal - downNormal);
    float normalCountD = normalDiff.x + normalDiff.y + normalDiff.z;
    vec3 upNormal = normalize(2 * texture(normalsTex, geom_texCoord + vec2(0, normalLineWidth) / viewportSize).xyz - 1);
    normalDiff = abs(myNormal - upNormal);
    float normalCountU = normalDiff.x + normalDiff.y + normalDiff.z;

	float normalTest = normalCountL + normalCountR + normalCountD + normalCountU;
    if (normalTest > normalLineThreshold) {
        fragColor.xyz = vec3(0);
        fragColor.w = 1;
		fxaaNormal = true;
    }

	// FXAA
	if (fxaaEnabled && fxaaDepth) {
        vec3 MyColor = fragColor.xyz;

		bool isHorizontal = ((depthCountU + depthCountD) >= (depthCountL + depthCountR));
		float stepLength = isHorizontal ? 1 / viewportSize.y : 1 / viewportSize.x;

		vec2 currentUv = geom_texCoord;

		vec2 offset = isHorizontal ? vec2(1 / viewportSize.x, 0.0) : vec2(0.0, 1 / viewportSize.y);
		vec2 uv1 = currentUv - offset;
		vec2 uv2 = currentUv + offset;

		float DepthEnd1 = DepthCalc(uv1);
		float DepthEnd2 = DepthCalc(uv2);

		bool reached1 = DepthEnd1 <= depthLineThreshold;
		bool reached2 = DepthEnd2 <= depthLineThreshold;
		bool reachedBoth = reached1 && reached2;

		if (!reached1) {
			uv1 -= offset;
		}
		if (!reached2) {
			uv2 += offset;
		}

		if (!reachedBoth) {
			for (int i = 0; i < 20; i++) {
				if (!reached1) {
					DepthEnd1 = DepthCalc(uv1);
				}
				if (!reached2) {
					DepthEnd2 = DepthCalc(uv2);
				}

				reached1 = DepthEnd1 <= depthLineThreshold;
				reached2 = DepthEnd2 <= depthLineThreshold;
				reachedBoth = reached1 && reached2;

				if (!reached1) {
					uv1 -= offset;
				}
				if (!reached2) {
					uv2 += offset;
				}

				if (reachedBoth) { break; }
			}
		}

		float distance1 = isHorizontal ? (geom_texCoord.x - uv1.x) : (geom_texCoord.y - uv1.y);
		float distance2 = isHorizontal ? (uv2.x - geom_texCoord.x) : (uv2.y - geom_texCoord.y);

		float extra = isHorizontal ? 1 / viewportSize.x : 1 / viewportSize.y;
		distance1 -= extra;
		distance2 -= extra;
		float distanceFinal = min(distance1, distance2);

		float edgeThickness = (distance1 + distance2);
		float pixelOffset = clamp((AAThreshold - distanceFinal / edgeThickness) * AAIntensity, 0, 1);
		if (edgeThickness == 0) {
			pixelOffset = clamp(AAThreshold * AAIntensity, 0, 1);
		}

		if (isHorizontal) {
			if (depthCountU < depthCountD) {
				vec2 lerpUV = geom_texCoord + depthLineWidth * vec2(0, 1) / viewportSize;
				if (DepthCalc(lerpUV) > depthLineThreshold) {
					MyColor = lerp(MyColor, AfterColor(geom_texCoord + vec2(0, -1) / viewportSize), pixelOffset);
				}
				MyColor = lerp(MyColor, AfterColor(geom_texCoord + vec2(0, 1) / viewportSize), pixelOffset);
			}
			else {
				vec2 lerpUV = geom_texCoord + depthLineWidth * vec2(0, -1) / viewportSize;
				if (DepthCalc(lerpUV) > depthLineThreshold) {
					MyColor = lerp(MyColor, AfterColor(geom_texCoord + vec2(0, 1) / viewportSize), pixelOffset);
				}
				MyColor = lerp(MyColor, AfterColor(geom_texCoord + vec2(0, -1) / viewportSize), pixelOffset);
			}
		}
		else {
			if (depthCountR < depthCountL) {
				vec2 lerpUV = geom_texCoord + depthLineWidth * vec2(1, 0) / viewportSize;
				if (DepthCalc(lerpUV) > depthLineThreshold) {
					MyColor = lerp(MyColor, AfterColor(geom_texCoord + vec2(-1, 0) / viewportSize), pixelOffset);
				}
				MyColor = lerp(MyColor, AfterColor(geom_texCoord + vec2(1, 0) / viewportSize), pixelOffset);
			}
			else {
				vec2 lerpUV = geom_texCoord + depthLineWidth * vec2(-1, 0) / viewportSize;
				if (DepthCalc(lerpUV) > depthLineThreshold) {
					MyColor = lerp(MyColor, AfterColor(geom_texCoord + vec2(1, 0) / viewportSize), pixelOffset);
				}
				MyColor = lerp(MyColor, AfterColor(geom_texCoord + vec2(-1, 0) / viewportSize), pixelOffset);
			}
		}

		fragColor.xyz = MyColor.xyz;
	} else if (fxaaEnabled && fxaaNormal) {
		vec3 MyColor = fragColor.xyz;

		bool isHorizontal = ((normalCountU + normalCountD) >= (normalCountL + normalCountR));
		float stepLength = isHorizontal ? 1 / viewportSize.y : 1 / viewportSize.x;

		vec2 currentUv = geom_texCoord;

		vec2 offset = isHorizontal ? vec2(1 / viewportSize.x, 0.0) : vec2(0.0, 1 / viewportSize.y);
		vec2 uv1 = currentUv - offset;
		vec2 uv2 = currentUv + offset;

		float NormalEnd1 = NormalCalc(uv1);
		float NormalEnd2 = NormalCalc(uv2);

		bool reached1 = NormalEnd1 <= normalLineThreshold;
		bool reached2 = NormalEnd2 <= normalLineThreshold;
		bool reachedBoth = reached1 && reached2;

		if (!reached1) {
			uv1 -= offset;
		}
		if (!reached2) {
			uv2 += offset;
		}

		if (!reachedBoth) {
			for (int i = 0; i < 20; i++) {
				if (!reached1) {
					NormalEnd1 = NormalCalc(uv1);
				}
				if (!reached2) {
					NormalEnd2 = NormalCalc(uv2);
				}

				reached1 = NormalEnd1 <= normalLineThreshold;
				reached2 = NormalEnd2 <= normalLineThreshold;
				reachedBoth = reached1 && reached2;

				if (!reached1) {
					uv1 -= offset;
				}
				if (!reached2) {
					uv2 += offset;
				}

				if (reachedBoth) { break; }
			}
		}

		float distance1 = isHorizontal ? (geom_texCoord.x - uv1.x) : (geom_texCoord.y - uv1.y);
		float distance2 = isHorizontal ? (uv2.x - geom_texCoord.x) : (uv2.y - geom_texCoord.y);

		float extra = isHorizontal ? 1 / viewportSize.x : 1 / viewportSize.y;
		distance1 -= extra;
		distance2 -= extra;
		float distanceFinal = min(distance1, distance2);

		float edgeThickness = (distance1 + distance2);
		float pixelOffset = clamp((AAThreshold - distanceFinal / edgeThickness) * AAIntensity, 0, 1);
		if (edgeThickness == 0) {
			pixelOffset = clamp(AAThreshold * AAIntensity, 0, 1);
		}

		if (isHorizontal) {
			if (normalCountU < normalCountD) {
				vec2 lerpUV = geom_texCoord + normalLineWidth * vec2(0, 1) / viewportSize;
				if (NormalCalc(lerpUV) > normalLineThreshold) {
					MyColor = lerp(MyColor, AfterColor(geom_texCoord + vec2(0, -1) / viewportSize), pixelOffset);
				}
				MyColor = lerp(MyColor, AfterColor(geom_texCoord + vec2(0, 1) / viewportSize), pixelOffset);
			}
			else {
				vec2 lerpUV = geom_texCoord + normalLineWidth * vec2(0, -1) / viewportSize;
				if (NormalCalc(lerpUV) > normalLineThreshold) {
					MyColor = lerp(MyColor, AfterColor(geom_texCoord + vec2(0, 1) / viewportSize), pixelOffset);
				}
				MyColor = lerp(MyColor, AfterColor(geom_texCoord + vec2(0, -1) / viewportSize), pixelOffset);
			}
		}
		else {
			if (normalCountR < normalCountL) {
				vec2 lerpUV = geom_texCoord + normalLineWidth * vec2(1, 0) / viewportSize;
				if (NormalCalc(lerpUV) > normalLineThreshold) {
					MyColor = lerp(MyColor, AfterColor(geom_texCoord + vec2(-1, 0) / viewportSize), pixelOffset);
				}
				MyColor = lerp(MyColor, AfterColor(geom_texCoord + vec2(1, 0) / viewportSize), pixelOffset);
			}
			else {
				vec2 lerpUV = geom_texCoord + normalLineWidth * vec2(-1, 0) / viewportSize;
				if (NormalCalc(lerpUV) > normalLineThreshold) {
					MyColor = lerp(MyColor, AfterColor(geom_texCoord + vec2(1, 0) / viewportSize), pixelOffset);
				}
				MyColor = lerp(MyColor, AfterColor(geom_texCoord + vec2(-1, 0) / viewportSize), pixelOffset);
			}
		}

		fragColor.xyz = MyColor.xyz;
	}
}
