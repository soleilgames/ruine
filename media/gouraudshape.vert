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

const int MAXLIGHTS = 10; // TODO: In a header

attribute vec4 positionAttribute;
attribute vec3 normalAttribute;
attribute vec4 colorAttribute;
attribute vec2 uvAttribute;

uniform mat4 MVPMatrix;
uniform mat4 MVMatrix;
uniform mat3 NormalMatrix;
uniform vec3 EyeDirection;

uniform int numberOfLights;
uniform PointLight pointLight[MAXLIGHTS];

varying vec4 color;
varying vec3 normal;
varying vec2 uv;

varying vec3 lightDirection[MAXLIGHTS];
varying vec3  halfVector[MAXLIGHTS];
varying float attenuation[MAXLIGHTS];

void
main()
{
  const float ConstantAttenuation = .50; // TODO: If kept, put it in an uniform

  for (int i = 0; i < numberOfLights; ++i) {
    vec3 localLightDir;
    
    localLightDir   = pointLight[i].position - vec3(MVMatrix * positionAttribute);
    float lightDistance = length(localLightDir);
    localLightDir   = localLightDir / lightDistance;
    attenuation[i] =
      1.0 /
      (ConstantAttenuation + pointLight[i].linearAttenuation * lightDistance +
       pointLight[i].quadraticAttenuation * lightDistance * lightDistance);
    halfVector[i] = normalize(localLightDir + EyeDirection);
    lightDirection[i] = localLightDir;
  }

  normal      = normalize(NormalMatrix * normalAttribute);
  uv          = uvAttribute;
  gl_Position = MVPMatrix * positionAttribute;
}
