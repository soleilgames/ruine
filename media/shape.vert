#version 100

precision mediump float;

attribute vec4 positionAttribute;
attribute vec3 normalAttribute;
attribute vec4 colorAttribute;

uniform mat4 MVPMatrix;
uniform mat4 MVMatrix;
uniform mat3 NormalMatrix;

varying vec4 color;
varying vec3 normal;
varying vec4 position;

void
main()
{
  color       = colorAttribute;
  normal      = normalize(NormalMatrix * normalAttribute);
  position    = MVMatrix * positionAttribute;
  gl_Position = MVPMatrix * positionAttribute;
}
