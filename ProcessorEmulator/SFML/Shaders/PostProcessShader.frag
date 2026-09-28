#version 110

uniform sampler2D texture;     // Автоматически передаваемая renderTexture (основная сцена)
uniform sampler2D grayTexture; // Наша кастомная маска в оттенках серого

void main() {
    // В старом, но стандартизированном GLSL 110 gl_TexCoord должен быть явно указан как [0]
    // Также мы используем стандартную функцию texture2D
    vec4 sceneColor = texture2D(texture, gl_TexCoord[0].xy);

    vec2 textureCoords = gl_TexCoord[0].xy;
    textureCoords.y = 1.0 - textureCoords.y;
    vec4 maskColor = texture2D(grayTexture, textureCoords);
    
    // Перемножаем RGB каналы
    vec3 finalRgb = sceneColor.rgb * maskColor.r;
    
    // Выводим итоговый цвет
    gl_FragColor = vec4(finalRgb, sceneColor.a);
}
