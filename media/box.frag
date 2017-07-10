#version 100

precision lowp float;

uniform vec4 color;

void
main()
{
  gl_FragColor = color;
}
