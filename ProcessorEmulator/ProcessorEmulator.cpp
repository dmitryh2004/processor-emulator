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

    // background sprite
    Image bgSprite = Image("bgSprite", sf::Vector2f(1440.f, 900.f), backgroundTexture);

    // top panel - start
    std::shared_ptr<Button> infoButton = std::make_shared<Button>("infoButton", sf::Vector2f(24.f, 24.f), sf::Vector2f(1406.f, 6.f), buttonTexture);
    infoButton->setShader(&shader);
    infoButton->SetOnClickSound(&clickSound);

    std::shared_ptr<Button> startButton = std::make_shared<Button>("startButton", sf::Vector2f(24.f, 24.f), sf::Vector2f(674.f, 6.f), buttonTexture);
    startButton->setShader(&shader);
    startButton->SetOnClickSound(&clickSound);

    std::shared_ptr<Button> stepButton = std::make_shared<Button>("stepButton", sf::Vector2f(24.f, 24.f), sf::Vector2f(708.f, 6.f), buttonTexture);
    stepButton->setShader(&shader);
    stepButton->SetOnClickSound(&clickSound);

    std::shared_ptr<Button> stopButton = std::make_shared<Button>("stopButton", sf::Vector2f(24.f, 24.f), sf::Vector2f(742.f, 6.f), buttonTexture);
    stopButton->setShader(&shader);
    stopButton->SetOnClickSound(&clickSound);

    std::shared_ptr<Text> header = std::make_shared<Text>("header", font, L"Эмулятор процессора", 18, sf::Vector2f(10.f, 9.f));

    std::shared_ptr<Panel> panel = std::make_shared<Panel>("headerPanel", sf::Vector2f(1440.f, 36.f));
    panel->addObject(header);
    panel->addObject(infoButton);
    panel->addObject(startButton);
    panel->addObject(stepButton);
    panel->addObject(stopButton);
    // top panel - end

    // code panel - start
    std::shared_ptr<Button> saveCodeButton = std::make_shared<Button>("saveCodeButton", sf::Vector2f(24.f, 24.f), sf::Vector2f(426.f, 10.f), buttonTexture);
    saveCodeButton->setShader(&shader);
    saveCodeButton->SetOnClickSound(&clickSound);

    std::shared_ptr<Button> loadCodeButton = std::make_shared<Button>("loadCodeButton", sf::Vector2f(24.f, 24.f), sf::Vector2f(392.f, 10.f), buttonTexture);
    loadCodeButton->setShader(&shader);
    loadCodeButton->SetOnClickSound(&clickSound);

    std::shared_ptr<Text> codePanelHeader = std::make_shared<Text>("codePanelHeader", font, L"Код", 18, sf::Vector2f(10.f, 10.f));

    std::shared_ptr<Panel> codePanel = std::make_shared<Panel>("codePanel", sf::Vector2f(460.f, 544.f), sf::Vector2f(0.f, 46.f));
    codePanel->addObject(codePanelHeader);
    codePanel->addObject(saveCodeButton); 
    codePanel->addObject(loadCodeButton);
    // code panel - end

    sf::Clock clock;
    while (window.isOpen())
    {
        // get mouse position
        sf::Vector2i mousePosInt = sf::Mouse::getPosition(window);
        sf::Vector2f mousePosFloat = sf::Vector2f(static_cast<float>(mousePosInt.x), static_cast<float>(mousePosInt.y));

        while (const std::optional event = window.pollEvent())
        {
            // Закрытие окна
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            panel->checkForEvents(*event, window, mousePosFloat);
            codePanel->checkForEvents(*event, window, mousePosFloat);
        }
        
        sf::Time deltaTime = clock.restart();
        panel->update(deltaTime);
        codePanel->update(deltaTime);
        
        window.clear();
        window.draw(bgSprite);
        window.draw(*panel);
        window.draw(*codePanel);
        window.display();
    }

    bgMusic.stop();
}
