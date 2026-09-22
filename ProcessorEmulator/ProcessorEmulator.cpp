#include "ProcessorEmulator.h"
#include "ResourceManager.h"

int main()
{
    sf::RenderWindow window(sf::VideoMode({ 1440, 900 }), "SFML window", sf::Style::Titlebar | sf::Style::Close);
    window.setSize(sf::Vector2u(1440, 900));
    window.setFramerateLimit(60);

    ResourceManager resources;

    const sf::Texture& buttonTexture = resources.GetTexture("Assets/Sprites/ButtonTexture.png");
    const sf::Texture& backgroundTexture = resources.GetTexture("Assets/Sprites/ad 3.png");

    const sf::Font& font = resources.GetFont("Assets/Fonts/Rubik-Medium.ttf");

    sf::Music& bgMusic = resources.GetMusic("Assets/Sounds/background-music.mp3");
    bgMusic.setLooping(true);
    bgMusic.play();

    sf::Sound clickSound(resources.GetSoundBuffer("Assets/Sounds/click-sound.mp3"));

    const sf::Shader& shader = resources.GetShader("SFML/Shaders/BaseShader.frag", sf::Shader::Type::Fragment);

    Image bgSprite = Image(sf::Vector2f(1440.f, 900.f), backgroundTexture);

    std::shared_ptr<Button> button = std::make_shared<Button>(sf::Vector2f(50.f, 50.f), sf::Vector2f(100.f, 0.f), buttonTexture);
    button->setShader(&shader);
    button->SetOnClickSound(&clickSound);

    std::shared_ptr<Text> header = std::make_shared<Text>(font, L"Эмулятор процессора", 18, sf::Vector2f(10.f, 9.f));

    std::shared_ptr<Panel> panel = std::make_shared<Panel>(sf::Vector2f(1440.f, 36.f));
    panel->addObject(header, 1);
    panel->addObject(button, 0);

    sf::Clock clock;
    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            // Закрытие окна
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            // Проверка клика мыши
            /*
            if (const auto* mouseClick = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseClick->button == sf::Mouse::Button::Left) {
                    clickSound.play();
                }
            }
            */

            button->checkForEvents(*event, window);
        }
        
        sf::Time deltaTime = clock.restart();
        button->update(deltaTime);
        
        window.clear();
        window.draw(bgSprite);
        window.draw(*header);
        window.draw(*button);
        window.display();
    }

    bgMusic.stop();
}
