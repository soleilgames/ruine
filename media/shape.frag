#version 100

precision highp float;

struct Material
{
  vec3  ambiantColor; // TODO: Should be a vec4
  float shininess;
  vec3  emissiveColor;
  vec3  diffuseColor;
  vec3  specularColor;
};

struct PointLight
{
  // TODO: Ambiant color and Specular color
  vec3  position;
  vec3  color;
  float linearAttenuation;
  float quadraticAttenuation;
};

const int MAXLIGHTS = 10;

uniform Material material;
uniform vec3 AmbiantLight;
uniform vec3 EyeDirection;
uniform PointLight pointLight[MAXLIGHTS]; // TODO: Multiple lights
uniform int        numberOfLights;

varying vec4 color;
varying vec3 normal;
varying vec4 position;

void
main()
{
  const float ConstantAttenuation = 1.0; // TODO: If kept, put it in an uniform
  const float Strength            = 1.0; // TODO: If kept, put it in an uniform

  vec3 scatteredLight = AmbiantLight;
  vec3 reflectedLight = vec3(0.0);

  for (int i = 0; i < numberOfLights; ++i) {
    vec3  lightDirection = pointLight[i].position - vec3(position);
    float lightDistance  = length(lightDirection);
    lightDirection       = lightDirection / lightDistance;

    float attenuation =
      1.0 /
      (ConstantAttenuation + pointLight[i].linearAttenuation * lightDistance +
       pointLight[i].quadraticAttenuation * lightDistance * lightDistance);

    vec3 halfVector = normalize(lightDirection + EyeDirection);

    float diffuse  = max(0.0, dot(normal, lightDirection));
    float specular = max(0.0, dot(normal, halfVector));

    specular =
      (diffuse == 0.0) ? (0.0) : (pow(specular, material.shininess) * Strength);

    scatteredLight +=
      material.ambiantColor * attenuation +
      pointLight[i].color * material.diffuseColor * diffuse * attenuation;
    reflectedLight +=
      pointLight[i].color * material.specularColor * specular * attenuation;
  }
  vec3 rgb =
    min(material.emissiveColor + scatteredLight + reflectedLight, vec3(1.0));

  gl_FragColor = vec4(rgb, 1.0);

#if 0 // Do not use the vertex color anymore
  gl_FragColor        = min(color * scatteredLight, vec4(1.0));
#endif
}
