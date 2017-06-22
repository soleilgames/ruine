#version 100

precision lowp float;

struct PointLight
{
  // TODO: Ambiant color and Specular color
  vec3  position;
  vec3  color;
  float linearAttenuation;
  float quadraticAttenuation;
};

struct Material
{
  vec3  ambiantColor; // TODO: Should be a vec4
  float shininess;
  vec3  emissiveColor;
  vec3  diffuseColor;
  vec3  specularColor;

  sampler2D diffuseMap;
};

const int MAXLIGHTS = 10; // TODO: In a header

attribute vec4 positionAttribute;
attribute vec3 normalAttribute;
attribute vec4 colorAttribute;
attribute vec2 uvAttribute;

uniform mat4 MVPMatrix;
uniform mat4 MVMatrix;
uniform mat3 NormalMatrix;
uniform vec3 EyeDirection;

uniform Material material;
uniform vec3 AmbiantLight;
uniform int  numberOfLights;
uniform PointLight pointLight[MAXLIGHTS];

varying vec4 color;
varying vec2 uv;

varying vec3 scatteredLight;
varying vec3 reflectedLight;

void
main()
{
  const float ConstantAttenuation = .50; // TODO: If kept, put it in an uniform

  scatteredLight = AmbiantLight;
  reflectedLight = vec3(0.0);

  vec3 normal = normalize(NormalMatrix * normalAttribute);
  for (int i = 0; i < numberOfLights; ++i) {
    vec3 lightDirection;

    lightDirection =
      pointLight[i].position - vec3(MVMatrix * positionAttribute);
    float lightDistance = length(lightDirection);
    lightDirection      = lightDirection / lightDistance;
    float attenuation =
      1.0 /
      (ConstantAttenuation + pointLight[i].linearAttenuation * lightDistance +
       pointLight[i].quadraticAttenuation * lightDistance * lightDistance);
    vec3 halfVector = normalize(lightDirection + EyeDirection);

    float diffuse  = max(0.0, dot(normal, lightDirection));
    float specular = max(0.0, dot(normal, halfVector));

    if (diffuse == 0.0)
      specular = 0.0;
    else
      specular = pow(specular, material.shininess);

    scatteredLight += pointLight[i].color * diffuse * attenuation;
    reflectedLight += pointLight[i].color * specular * attenuation;
  }

  uv          = uvAttribute;
  gl_Position = MVPMatrix * positionAttribute;
}
