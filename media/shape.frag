#version 100

precision mediump float;

struct Material
{
  vec3 emission;
  vec3 ambiantColor;
  vec3 diffuseColor;
  vec3 specularColor;
  float shininess;
};

uniform Material material;

varying vec4 color;

void
main()
{
  gl_FragColor = vec4(material.ambiantColor, 1.0) * color;
}
