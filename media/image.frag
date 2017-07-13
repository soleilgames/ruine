#version 100

precision lowp float;

uniform sampler2D image;
uniform vec4 color;

varying vec2 uv;

void
main()
{
  gl_FragColor = texture2D(image, uv) * color;
}
