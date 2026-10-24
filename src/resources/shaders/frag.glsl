#version 330

in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragPosition;
in vec3 fragNormal;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

out vec4 finalColor;

uniform vec4 ambient;
uniform vec3 viewPos;

void main() {
  vec4 texelColor = texture(texture0, fragTexCoord);
  vec3 normal = normalize(fragNormal);
  vec3 viewD = normalize(viewPos - fragPosition);
  vec3 light = normalize(vec3(3.0, 5.0, 4.0));
  vec3 lightColor = vec3(1.0);

  finalColor = vec4(1.0);
  //finalColor = 0.5*colDiffuse*vec4(lightColor*dot(normal, light), 1.0) + 0.5;
  finalColor *= texelColor;
  finalColor *= fragColor;
}
