#version 330 core

struct Material
{
    vec3 ambientColor;
    float ambientStrength;
    vec3 diffuseColor;
    vec3 specularColor;
    float shininess;
};

struct LightSource
{
    vec3 position;
    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
    float focalStrength;
    float specularIntensity;
};

#define TOTAL_LIGHTS 4

in vec3 fragmentPosition;
in vec3 fragmentVertexNormal;
in vec2 fragmentTextureCoordinate;

out vec4 outFragmentColor;

uniform bool bUseTexture = false;
uniform bool bUseLighting = false;
uniform vec4 objectColor = vec4(1.0f);
uniform sampler2D objectTexture;
uniform vec3 viewPosition;
uniform vec2 UVscale = vec2(1.0f, 1.0f);
uniform LightSource lightSources[TOTAL_LIGHTS];
uniform Material material;

vec3 CalcLightSource(LightSource light, vec3 lightNormal, vec3 vertexPosition, vec3 viewDirection);

void main()
{
   vec4 baseColor = bUseTexture
      ? texture(objectTexture, fragmentTextureCoordinate * UVscale)
      : objectColor;

   if (bUseLighting)
   {
      vec3 lightNormal = normalize(fragmentVertexNormal);
      vec3 viewDirection = normalize(viewPosition - fragmentPosition);
      vec3 phongResult = vec3(0.0f);

      for (int i = 0; i < TOTAL_LIGHTS; i++)
      {
         phongResult += CalcLightSource(lightSources[i], lightNormal, fragmentPosition, viewDirection);
      }

      outFragmentColor = vec4(phongResult * baseColor.rgb, baseColor.a);
   }
   else
   {
      outFragmentColor = baseColor;
   }
}

vec3 CalcLightSource(LightSource light, vec3 lightNormal, vec3 vertexPosition, vec3 viewDirection)
{
   vec3 ambient = light.ambientColor * material.ambientColor * material.ambientStrength;
   vec3 lightDirection = normalize(light.position - vertexPosition);
   float impact = max(dot(lightNormal, lightDirection), 0.0);
   vec3 diffuse = impact * light.diffuseColor * material.diffuseColor;
   vec3 reflectDir = reflect(-lightDirection, lightNormal);
   float specularComponent = pow(max(dot(viewDirection, reflectDir), 0.0), light.focalStrength);
   vec3 specular = light.specularColor * light.specularIntensity * material.shininess * specularComponent * material.specularColor;
   return ambient + diffuse + specular;
}
