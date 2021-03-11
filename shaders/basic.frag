#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;

out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;
//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

//components
vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;

//shadows
in vec4 fragPosLightSpace;
uniform sampler2D shadowMap;

float computeShadow()
{

	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	normalizedCoords = normalizedCoords * 0.5 + 0.5;
	if (normalizedCoords.z > 1.0f)
		return 0.0f;
	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
	float currentDepth = normalizedCoords.z;
	float bias = max(0.05f * (1.0f - dot(normalizedCoords, lightDir)), 0.005f);
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

	return shadow;

}

void computeDirLight()
{
    //compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);

    //normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    //compute view direction (in eye coordinates, the viewer is situated at the origin
    vec3 viewDir = normalize(- fPosEye.xyz);

    //compute ambient light
    ambient = ambientStrength * lightColor;

    //compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    //compute specular light
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    specular = specularStrength * specCoeff * lightColor;
}

uniform vec3 pLightPosition;
    
float constant = 1.0f;
float linear = 0.0045f;
float quadratic = 0.0075f;  


float shininess = 32.0f;

void computePointLight()
{		
	vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
	vec4 lightPosEye = view * model * vec4(pLightPosition, 1.0f);
	
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin

	float dist = length(lightPosEye.xyz - vec3(fPosEye));
	float att = 1.0f / (constant + linear * dist + quadratic * (dist * dist));

	//transform normal
	vec3 normalEye = normalize(fNormal);	
	
	//compute light direction
	vec3 lightDirN = normalize(lightPosEye.xyz - vec3(fPosEye.x,fPosEye.y,fPosEye.z));	

	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
	
	//compute half vector
	vec3 halfVector = normalize(lightDirN + viewDirN);

	//compute ambient light
	ambient += att * ambientStrength * lightColor;
	
	//compute diffuse light
	diffuse += att * max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	
	//compute specular light
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), shininess);
	specular += att * specularStrength * specCoeff * lightColor;	
}

float computeFog()
{
	vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
	float fogDensity = 0.05f;
	float fragmentDistance = length(fPosEye);
	float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

	return clamp(fogFactor, 0.0f, 1.0f);
}

uniform int changeLight; //true - directional; false - point
uniform int fog; 

void main() 
{
    if (changeLight == 1) {
		computeDirLight();
	} else {
		computePointLight();
	}

	float shadow = computeShadow();

	//compute fog
	float fogFactor = computeFog();
    vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);

	ambient *= texture(diffuseTexture, fTexCoords);
	diffuse *= texture(diffuseTexture, fTexCoords);
	specular *= texture(specularTexture, fTexCoords);

    //compute final vertex color
	vec3 color = min((ambient + (1.0f - shadow)*diffuse) + (1.0f - shadow)*specular, 1.0f);

	if (fog == 1) {
		fColor = fogColor * (1 - fogFactor) + vec4(color,1.0f) * fogFactor;
	}
	else {
		fColor = vec4(color, 1.0f);
	}
    
}
