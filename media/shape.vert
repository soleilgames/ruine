#version 100

precision mediump float;

attribute vec4 positionAttribute;
attribute vec4 colorAttribute;

uniform mat4 Model; // Currently include ViewProjection

varying vec4 color;

void
main()
{
  gl_Position = Model * positionAttribute;
  color       = colorAttribute;
}
