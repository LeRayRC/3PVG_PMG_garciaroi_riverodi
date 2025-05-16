#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference : require

#include "../input_structures.glsl"

struct vertexData
{
    vec4 posWithAlpha; // center of the gaussian in object coordinates, with alpha in w
    vec4 r_sh0; // sh coeff for red channel (up to third-order)
    vec4 g_sh0; // sh coeff for green channel
    vec4 b_sh0;  // sh coeff for blue channel
    vec3 cov3_col0; // 3x3 covariance matrix of the splat in object coordinates.
    int padding1; //Vulkan PADDING
    vec3 cov3_col1;
    int padding2; //Vulkan PADDING
    vec3 cov3_col2;
    int padding3; //Vulkan PADDING
};

layout(buffer_reference, std430) readonly buffer VertexBuffer{ 
	vertexData data[];
};

//push constants block
layout( push_constant ) uniform constants
{	
	mat4 render_matrix;
    vec4 gsData;
	VertexBuffer vertexBuffer;
} PushConstants;

//UNIFORMS
//uniform mat4 viewMat;
//uniform mat4 projMat;
//uniform vec4 projParams;  // x = HEIGHT / tan(FOVY / 2) (X IS NOT USE), y = Z_NEAR, z = Z_FAR
//uniform vec4 viewport;  // x, y, WIDTH, HEIGHT
//uniform vec3 eye;

//OUT
//layout(location = 0) out vec4 color;
layout (location = 0) out vec4 geom_color;  // radiance of splat
layout (location = 1) out vec4 geom_cov2;  // 2D screen space covariance matrix of the gaussian
layout (location = 2) out vec2 geom_p;  // the 2D screen space center of the gaussian, (z is alpha)
layout (location = 3) out vec2 viewport;  

vec3 ComputeRadianceFromSH(const vec3 v, vertexData d)
{
    float b[4];

    float vx2 = v.x * v.x;
    float vy2 = v.y * v.y;
    float vz2 = v.z * v.z;

    // zeroth order
    b[0] = 0.28209479177387814;

    // first order
    float k1 = 0.4886025119029199;
    b[1] = -k1 * v.y;
    b[2] = k1 * v.z;
    b[3] = -k1 * v.x;

    float re = (b[0] * d.r_sh0.x + b[1] * d.r_sh0.y + b[2] * d.r_sh0.z + b[3] * d.r_sh0.w);
    float gr = (b[0] * d.g_sh0.x + b[1] * d.g_sh0.y + b[2] * d.g_sh0.z + b[3] * d.g_sh0.w);
    float bl = (b[0] * d.b_sh0.x + b[1] * d.b_sh0.y + b[2] * d.b_sh0.z + b[3] * d.b_sh0.w);
    return vec3(0.5, 0.5, 0.5) + vec3(re, gr, bl);
}

void main() {
    vertexData d = PushConstants.vertexBuffer.data[gl_VertexIndex];
    float alpha = d.posWithAlpha.w;
    vec4 t = globalData.view * vec4(d.posWithAlpha.xyz, 1.0);

    float X0 = 0.0f * (0.00001 * PushConstants.gsData.x);  // one weird hack to prevent projParams from being compiled away
    float Y0 = 0.0f;
    float WIDTH = PushConstants.gsData.z;
    float HEIGHT = PushConstants.gsData.w;
    float Z_NEAR = PushConstants.gsData.x;
    float Z_FAR = PushConstants.gsData.y;

    viewport.x = PushConstants.gsData.z;
    viewport.y = PushConstants.gsData.w;

    // J is the jacobian of the projection and viewport transformations.
    // this is an affine approximation of the real projection.
    // because gaussians are closed under affine transforms.
    float SX = globalData.proj[0][0];
    float SY = globalData.proj[1][1];
    float WZ =  globalData.proj[3][2];
    float tzSq = t.z * t.z;
    float jsx = -(SX * WIDTH) / (2.0 * t.z);
    float jsy = -(SY * HEIGHT) / (2.0 * t.z);
    float jtx = (SX * t.x * WIDTH) / (2.0 * tzSq);
    float jty = (SY * t.y * HEIGHT) / (2.0 * tzSq);
    float jtz = ((Z_FAR - Z_NEAR) * WZ) / (2.0 * tzSq);
    mat3 J = mat3(vec3(jsx, 0.0, 0.0),
                  vec3(0.0, jsy, 0.0),
                  vec3(jtx, jty, jtz));

    // combine the affine transforms of W (view) and J (approx of viewportMat * globalData.proj)
    // using the fact that the new transformed covariance matrix V_Prime = JW * V * (JW)^T
    mat3 W = mat3(globalData.view);
    mat3 V = mat3(d.cov3_col0, d.cov3_col1, d.cov3_col2);
    mat3 JW = J * W;
    mat3 V_prime = JW * V * transpose(JW);

    // now we can 'project' the 3D covariance matrix onto the xy plane by just dropping the last column and row.
    mat2 cov2D = mat2(V_prime);

    // use the fact that the convolution of a gaussian with another gaussian is the sum
    // of their covariance matrices to apply a low-pass filter to anti-alias the splats
    cov2D[0][0] += 0.3;
    cov2D[1][1] += 0.3;
    cov2D[0][1] = -cov2D[0][1]; // Flip xy component
    //cov2D[1][0] = -cov2D[1][0]; // Flip yx component
    geom_cov2 = vec4(cov2D[0], cov2D[1]); // cram it into a vec4

    // geom_p is the gaussian center transformed into screen space
    vec4 p4 = globalData.proj * t;
    geom_p = vec2(p4.x / p4.w, p4.y / p4.w);
    geom_p.x = 0.5 * (WIDTH + (geom_p.x * WIDTH) + (2.0 * X0));
    geom_p.y = 0.5 * (HEIGHT + (geom_p.y * HEIGHT) + (2.0 * Y0));

    // compute radiance from sh
    vec3 v = normalize(d.posWithAlpha.xyz - globalData.cameraPos);
    geom_color = vec4(ComputeRadianceFromSH(v, d), alpha);

	gl_Position = p4;
    gl_PointSize = 2.0f;
}