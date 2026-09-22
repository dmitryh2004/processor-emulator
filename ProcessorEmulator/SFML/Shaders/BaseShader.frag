// GLSL шейдер для наложения текстуры (совместимый с SFML)
uniform sampler2D texture; // Сюда SFML автоматически передаст текущую текстуру

void main()
{
    // Получаем цвет пикселя из текстуры по текущим текстурным координатам
    vec4 pixelColor = texture2D(texture, gl_TexCoord[0].xy);

    // Умножаем на gl_Color, чтобы учитывался базовый цвет геометрии (например, m_shape.setFillColor())
    gl_FragColor = gl_Color * pixelColor;
}
