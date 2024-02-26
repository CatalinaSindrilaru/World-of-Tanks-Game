#version 330

in vec3 frag_position;
in vec3 frag_color;
in vec3 frag_normal;
in vec3 frag_texture;

// Uniform color
uniform vec4 meshColor;

// New uniform to adjust color
uniform float colorAdjustment;

layout(location = 0) out vec4 out_color;

void main()
{
    if (colorAdjustment == 0) {
        out_color = meshColor * 0.2;
    } else {
        out_color = meshColor * (colorAdjustment / 3.0);

    }
}