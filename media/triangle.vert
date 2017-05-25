#version 100

precision highp float;

uniform float time;
attribute vec4 positionAttribute;
attribute vec4 colorAttribute;

varying vec4 color;

void
main()
{
  vec4 modifier = vec4(time, time, time, 1.0);
  gl_Position   = positionAttribute * modifier;
  color         = colorAttribute * modifier;
}
