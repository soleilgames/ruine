#version 100

precision lowp float;

const int MAXLIGHTS = 16;

struct Material
{
  vec3  ambiantColor; // TODO: Should be a vec4
  float shininess;
  vec3  emissiveColor;
  vec3  diffuseColor;
  vec3  specularColor;

  sampler2D diffuseMap;
};

struct PointLight
{
  // TODO: Ambiant color and Specular color
  vec3  position;
  vec3  color;
  float linearAttenuation;
  float quadraticAttenuation;
};

uniform Material material;
uniform vec3 AmbiantLight;
uniform int  numberOfLights;
uniform PointLight pointLight[MAXLIGHTS];

varying vec3 normal;
varying vec2 uv;

varying vec3 lightDirection[MAXLIGHTS];
varying vec3  halfVector[MAXLIGHTS];
varying float attenuation[MAXLIGHTS];

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

  vec3 scatteredLight = AmbiantLight;
  vec3 reflectedLight = vec3(0.0);

  for (int i = 0; i < numberOfLights; ++i) {

    float diffuse  = max(0.0, dot(normal, lightDirection[i]));
    float specular = max(0.0, dot(normal, halfVector[i]));

    if (diffuse == 0.0)
      specular = 0.0;
    else
      specular = pow(specular, material.shininess);

    scatteredLight += pointLight[i].color * diffuse * attenuation[i];
    reflectedLight += pointLight[i].color * specular * attenuation[i];
  }
  // TODO: material.emissiveColor
  vec3 rgb = min(materialColor * scatteredLight + reflectedLight, vec3(1.0));

  gl_FragColor = vec4(rgb, alpha);
}
