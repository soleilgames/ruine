#version 100

precision lowp float;

attribute vec2 positionAttribute;

uniform mat4 ModelMatrix;

varying vec2 uv;

void
main()
{
  gl_Position = ModelMatrix * vec4(positionAttribute, 0.0, 1.0);
  uv          = positionAttribute;
}
