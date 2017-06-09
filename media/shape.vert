#version 100

precision mediump float;

attribute vec4 positionAttribute;
attribute vec3 normalAttribute;
attribute vec4 colorAttribute;
attribute vec2 uvAttribute;

uniform mat4 MVPMatrix;
uniform mat4 MVMatrix;
uniform mat3 NormalMatrix;

varying vec4 color;
varying vec3 normal;
varying vec4 position;
varying vec2 uv;

void
main()
{
  color       = colorAttribute;
  normal      = normalize(NormalMatrix * normalAttribute);
  position    = MVMatrix * positionAttribute;
  uv          = uvAttribute;
  gl_Position = MVPMatrix * positionAttribute;
}
