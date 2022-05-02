#version 330

// Constants
const float PI = 3.14159265358979323846264;

// HEADERS: deferred_shader_inputs.fs
struct ShaderInput {
    bool foreground;
    vec3 diffuseReflectance;
    vec3 normal;
    float eta;
    float alpha;
    vec3 fragPosNDC;
};
ShaderInput getShaderInputs(vec2 texCoord);

// Uniforms
uniform mat4 mP;
uniform vec2 viewportSize;
uniform sampler2D depthTex;
uniform int numSamples;

uniform vec3 ambientRadiance;
uniform float ambientOcclusionRange;

// Inputs
in vec2 geom_texCoord;

// Outputs
out vec4 fragColor;

struct Frame {
    vec3 origin;

    // (s, t, normal) form a right-handed coordinate system
    vec3 s, t, normal;
};

Frame makeFrame(vec3 o, vec3 n) {
    Frame f;
    f.origin = o;

    n = normalize(n);
    f.normal = n;

    vec3 nonParallel;
    float minComponent = min(abs(n.x), min(abs(n.y), abs(n.z)));
    if (abs(n.x) == minComponent) {
        nonParallel = vec3(1, 0, 0);
    } else if (abs(n.y) == minComponent) {
        nonParallel = vec3(0, 1, 0);
    } else {
        nonParallel = vec3(0, 0, 1);
    }

    f.s = normalize(cross(n, nonParallel));
    f.t = normalize(cross(n, f.s));
    return f;
}

bool isOccluded(Frame f, vec3 pFrame) {
    // Transform p from frame space to screen space
    vec3 pEye = f.origin + pFrame.x * f.s + pFrame.y * f.t + pFrame.z * f.normal;
    float distCamToP = length(pEye - vec3(0, 0, 0));

    vec4 pEye4 = vec4(pEye, 1);
    vec4 pNDC4 = mP * pEye4;
    pNDC4 /= pNDC4.w;

    vec3 pScreen = (pNDC4.xyz + 1) / 2;

    float qDepthScreen = texture(depthTex, pScreen.xy).x;

    // Transform q from screen space to eye space
    vec4 qNDC4 = vec4(pNDC4.xy, 2 * qDepthScreen - 1, 1);
    vec4 qEye4 = inverse(mP) * qNDC4;
    qEye4 /= qEye4.w;
    vec3 qEye = qEye4.xyz;
    float distCamToQ = length(qEye - vec3(0, 0, 0));

    return distCamToQ + 1e-5 < distCamToP && distCamToP < distCamToQ + 4 * ambientOcclusionRange;
}

float randConst(vec2 co) {
    return fract(sin(dot(co.xy,vec2(12.9898,78.233))) * 43758.5453);
}

float random(inout vec2 co) {
    co.x = randConst(co + vec2(1, 0));
    co.y = randConst(co + vec2(0, 1));
    return randConst(co);
}

vec3 randomOrientation(vec2 sample) {
    float r = sqrt(sample.x);
    float phi = 2 * PI * sample.y;
    return vec3(r * cos(phi), r * sin(phi), sqrt(1 - sample.x));
}

float ssaoOcclusionFactor(int N, Frame f, vec2 randomSeed) {
    float numOccluded = 0;

    for (int i = 0; i < N; i++) {
        vec3 orientation = randomOrientation(vec2(random(randomSeed), random(randomSeed)));
        float radius = ambientOcclusionRange * pow(random(randomSeed), 1.0 / 3.0);
        vec3 point = radius * orientation;
        numOccluded += isOccluded(f, point) ? 1 : 0;
    }

    return 1 - numOccluded / N;
}

float averageValue(int N, inout vec2 co) {
    float acc = 0;
    for (int i = 0; i < N; i++) {
        acc += random(co);
    }
    return acc / N;
}

void main() {
    ShaderInput inputs = getShaderInputs(geom_texCoord);
    if (!inputs.foreground) {
        fragColor = vec4(0, 0, 0, 0);
        return;
    }

    vec4 fragPosNDC4 = vec4(inputs.fragPosNDC, 1);
    vec4 fragPosEye4 = inverse(mP) * fragPosNDC4;
    fragPosEye4 /= fragPosEye4.w;
    vec3 fragPosEye = fragPosEye4.xyz;

    Frame f = makeFrame(fragPosEye, inputs.normal);
    vec2 randomSeed = geom_texCoord;
    float occlusionFactor = ssaoOcclusionFactor(numSamples, f, randomSeed);
    vec3 fragColor3 = occlusionFactor * ambientRadiance * inputs.diffuseReflectance;
    fragColor = vec4(fragColor3, 1);
}
