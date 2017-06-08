#version 100

precision highp float;

struct Material
{
  vec3  ambiantColor; // TODO: Should be a vec4
  float shininess;

#if 0
  // TODO: More material properties
  vec3  emission;
  vec3  diffuseColor;
  vec3  specularColor;
#endif
};

struct PointLight
{
  vec3  position;
  vec3  color;
  float linearAttenuation;
  float quadraticAttenuation;
};

uniform Material material;
uniform vec3 AmbiantLight;
uniform vec3 EyeDirection;
uniform PointLight pointLight; // TODO: Multiple lights

varying vec4 color;
varying vec3 normal;
varying vec4 position;

void
main()
{
  const float ConstantAttenuation = 1.0; // TODO: If kept, put it in an uniform
  const float Strength            = 1.0; // TODO: If kept, put it in an uniform

  vec3  lightDirection = pointLight.position - vec3(position);
  float lightDistance  = length(lightDirection);
  lightDirection       = lightDirection / lightDistance;

  float attenuation =
    1.0 / (ConstantAttenuation + pointLight.linearAttenuation * lightDistance +
           pointLight.quadraticAttenuation * lightDistance * lightDistance);

  vec3 halfVector = normalize(lightDirection + EyeDirection);

  float diffuse  = max(0.0, dot(normal, lightDirection));
  float specular = max(0.0, dot(normal, halfVector));

  specular =
    (diffuse == 0.0) ? (0.0) : (pow(specular, material.shininess) * Strength);

  vec3 scatteredLight = AmbiantLight + pointLight.color * diffuse * attenuation;
  vec3 reflectedLight = pointLight.color * specular * attenuation;
  vec3 rgb =
    min(material.ambiantColor * scatteredLight + reflectedLight, vec3(1.0));

  gl_FragColor = vec4(rgb, 1.0);

#if 0 // Do not use the vertex color anymore
  gl_FragColor        = min(color * scatteredLight, vec4(1.0));
#endif
}
