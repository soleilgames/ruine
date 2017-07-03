#version 100

precision lowp float;

uniform sampler2D FontAtlas;
uniform vec4 Color;

varying vec2 uv;

void
main()
{
  vec3 textColor = vec3(Color);
  gl_FragColor   = vec4(textColor, texture2D(FontAtlas, uv).a * Color.a);
}
