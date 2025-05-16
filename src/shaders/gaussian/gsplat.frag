#version 450

//output write
layout (location = 0) out vec4 outFragColor;

layout (location = 4) in vec4 frag_color;  // radiance of splat
layout (location = 5) in vec4 frag_cov2inv;  // inverse of the 2D screen space covariance matrix of the guassian
layout (location = 6) in vec2 frag_p;  // 2D screen space center of the guassian


void main()
{
    vec2 d = (gl_FragCoord.xy - frag_p);

    mat2 cov2Dinv = mat2(frag_cov2inv.xy, frag_cov2inv.zw);
    float g = exp(-0.5 * dot(d, cov2Dinv * d));

    float alpha = frag_color.a * g;

    outFragColor.rgb = alpha * frag_color.rgb;
    outFragColor.a = alpha;

    if (alpha <= (1.0 / 256.0))
    {
        discard;
    }

    //outFragColor = vec4(0.5, 0.5, 0.5, 1.0);
}
