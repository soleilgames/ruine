#version 100

precision lowp float;

const int MAXLIGHTS = 10;

struct Material
{
  vec3  ambiantColor; // TODO: Should be a vec4
  float shininess;
  vec3  emissiveColor;
  vec3  diffuseColor;
  vec3  specularColor;

  sampler2D diffuseMap;
};

uniform Material material;

varying vec3 normal;
varying vec2 uv;

varying vec3 scatteredLight;
varying vec3 reflectedLight;

void
main()
{
  vec3  materialColor = material.ambiantColor;
  float alpha         = 1.0;
  if (uv.x > -42.0) {
    vec4 textureColor = texture2D(material.diffuseMap, uv);

    materialColor = textureColor.rgb;
    alpha         = textureColor.a;
  }
  // TODO: material.emissiveColor
  vec3 rgb = min(materialColor * scatteredLight + reflectedLight, vec3(1.0));

  gl_FragColor = vec4(rgb, alpha);
}
