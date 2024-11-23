#version 330 core
// in vec4 color;

out vec4 FragColor;
uniform vec3 shapeColor; // Single color for the entire shap
void main()
{
    FragColor = vec4(shapeColor,1.0f);
}
