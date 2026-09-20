#include "ProcessorEmulator.h"
#include "ResourceManager.h"

int main()
{
    sf::RenderWindow window(sf::VideoMode({ 800, 600 }), "SFML window"); 
    window.setFramerateLimit(60);

    ResourceManager resources;

    const sf::Font& font = resources.GetFont("Assets/Fonts/Rubik-Medium.ttf");
    sf::Sprite sprite = sf::Sprite(resources.GetTexture("Assets/Sprites/ad 3.png"));

    sf::Text text(font, "Hello SFML", 50);

    sf::Music& bgMusic = resources.GetMusic("Assets/Sounds/background-music.mp3");
    bgMusic.setLooping(true);
    bgMusic.play();

    sf::Sound clickSound(resources.GetSoundBuffer("Assets/Sounds/click-sound.mp3"));

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            // Закрытие окна
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            // Проверка клика мыши
            if (const auto* mouseClick = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseClick->button == sf::Mouse::Button::Left) {
                    clickSound.play();
                }
            }
        }


        window.clear();
        window.draw(text);
        window.draw(sprite);
        window.display();
    }

    bgMusic.stop();
}
