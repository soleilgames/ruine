#version 100

precision lowp float;

attribute vec3 positionAttribute;

uniform vec3 min;
uniform vec3 max;
uniform mat4 VPMatrix;

void
main()
{
  vec4 point;

  point.x = (positionAttribute.x < 0.0) ? (min.x) : (max.x);
  point.y = (positionAttribute.y < 0.0) ? (min.y) : (max.y);
  point.z = (positionAttribute.z < 0.0) ? (min.z) : (max.z);
  point.w = 1.0;

  gl_Position = VPMatrix * point;
}
