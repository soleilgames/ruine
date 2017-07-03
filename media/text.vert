#version 100

precision lowp float;

attribute vec3 positionAttribute;
attribute vec2 uvAttribute;

uniform mat4 ModelMatrix;

varying vec2 uv;

void
main()
{
  gl_Position = ModelMatrix * vec4(positionAttribute, 1.0);
  uv          = uvAttribute;
}
