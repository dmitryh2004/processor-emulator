// GLSL шейдер для наложения текстуры поверх фона (совместимый с SFML)
uniform sampler2D bgTexture; // Задний фон кнопки
uniform sampler2D fgTexture; // Текст / иконка кнопки

void main()
{
    // Получаем цвета пикселей из обеих текстур
    vec4 bgPixelColor = texture2D(bgTexture, gl_TexCoord[0].xy);
    vec4 fgPixelColor = texture2D(fgTexture, gl_TexCoord[0].xy);

    // Стандартное альфа-смешивание (Alpha Blending)
    // fgPixelColor.a — это прозрачность переднего плана (от 0.0 до 1.0)
    vec3 mixedColor = mix(bgPixelColor.rgb, fgPixelColor.rgb, fgPixelColor.a);
    
    // Вычисляем итоговую альфа-прозрачность (обычно это просто прозрачность фона,
    // либо комбинация, если сам фон тоже может быть прозрачным)
    float mixedAlpha = max(bgPixelColor.a, fgPixelColor.a);

    // Возвращаем итоговый пиксель
    gl_FragColor = vec4(mixedColor, mixedAlpha);
}
