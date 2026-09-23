#include "ProcessorEmulator.h"
#include "ResourceManager.h"

int main()
{
    sf::RenderWindow window(sf::VideoMode({ 1440, 900 }), "SFML window", sf::Style::Titlebar | sf::Style::Close);
    window.setSize(sf::Vector2u(1440, 900));
    window.setFramerateLimit(60);

    sf::Vector2f windowSizeFloat = SFMLUtility::CastVector2uToFloat(window.getSize());

    ResourceManager resources;

    const sf::Texture& backgroundTexture = resources.GetTexture("Assets/Sprites/ad 3.png");

    const sf::Texture& buttonTexture = resources.GetTexture("Assets/Sprites/ButtonTexture.png");
    const sf::Texture& infoButtonTexture = resources.GetTexture("Assets/Sprites/infoButtonSprite.png");

    const sf::Font& font = resources.GetFont("Assets/Fonts/Rubik-Medium.ttf");

    sf::Music& bgMusic = resources.GetMusic("Assets/Sounds/background-music.mp3");
    bgMusic.setLooping(true);
    bgMusic.play();

    sf::Sound clickSound(resources.GetSoundBuffer("Assets/Sounds/click-sound.mp3"));

    sf::Shader& shader = resources.GetShader("SFML/Shaders/BaseShader.frag", sf::Shader::Type::Fragment);

    // background sprite
    Image bgSprite = Image("bgSprite", sf::Vector2f(1440.f, 900.f), backgroundTexture, windowSizeFloat);

    // top panel - start
    std::shared_ptr<Panel> panel = std::make_shared<Panel>("headerPanel", sf::Vector2f(1440.f, 36.f), windowSizeFloat);

    std::shared_ptr<Button> infoButton = std::make_shared<Button>("infoButton", 
        sf::Vector2f(24.f, 24.f), 
        panel->getSize(), 
        sf::Vector2f(-10.f, 0.f), 
        buttonTexture, 
        infoButtonTexture,
        BaseObject::Anchor::CenterRight, 
        BaseObject::Anchor::CenterRight
    );
    infoButton->setShader(&shader);
    infoButton->SetOnClickSound(&clickSound);

    std::shared_ptr<Button> startButton = std::make_shared<Button>("startButton",
        sf::Vector2f(24.f, 24.f),
        panel->getSize(),
        sf::Vector2f(-34.f, 0.f),
        buttonTexture,
        buttonTexture,
        BaseObject::Anchor::Center,
        BaseObject::Anchor::Center
    ); 
    startButton->setShader(&shader);
    startButton->SetOnClickSound(&clickSound);

    std::shared_ptr<Button> stepButton = std::make_shared<Button>("stepButton",
        sf::Vector2f(24.f, 24.f),
        panel->getSize(),
        sf::Vector2f(0.f, 0.f),
        buttonTexture,
        buttonTexture,
        BaseObject::Anchor::Center,
        BaseObject::Anchor::Center
    );
    stepButton->setShader(&shader);
    stepButton->SetOnClickSound(&clickSound);

    std::shared_ptr<Button> stopButton = std::make_shared<Button>("stopButton",
        sf::Vector2f(24.f, 24.f),
        panel->getSize(),
        sf::Vector2f(34.f, 0.f),
        buttonTexture,
        buttonTexture,
        BaseObject::Anchor::Center,
        BaseObject::Anchor::Center
    );
    stopButton->setShader(&shader);
    stopButton->SetOnClickSound(&clickSound);

    std::shared_ptr<Text> header = std::make_shared<Text>(
        "header",
        font,
        panel->getSize(),
        L"Эмулятор процессора",
        18,
        sf::Vector2f(10.f, 0.f),
        BaseObject::Anchor::CenterLeft,
        BaseObject::Anchor::CenterLeft
    );

    panel->addObject(header);
    panel->addObject(infoButton);
    panel->addObject(startButton);
    panel->addObject(stepButton);
    panel->addObject(stopButton);
    // top panel - end

    // code panel - start
    std::shared_ptr<Panel> codePanel = std::make_shared<Panel>(
        "codePanel",
        sf::Vector2f(460.f, 544.f),
        windowSizeFloat,
        sf::Vector2f(10.f, 46.f)
    );

    std::shared_ptr<Button> saveCodeButton = std::make_shared<Button>("saveCodeButton",
        sf::Vector2f(24.f, 24.f),
        codePanel->getSize(),
        sf::Vector2f(-10.f, 0.f),
        buttonTexture,
        buttonTexture,
        BaseObject::Anchor::TopRight,
        BaseObject::Anchor::TopRight
    );
    saveCodeButton->setShader(&shader);
    saveCodeButton->SetOnClickSound(&clickSound);

    std::shared_ptr<Button> loadCodeButton = std::make_shared<Button>("loadCodeButton",
        sf::Vector2f(24.f, 24.f),
        codePanel->getSize(),
        sf::Vector2f(-44.f, 0.f),
        buttonTexture,
        buttonTexture,
        BaseObject::Anchor::TopRight,
        BaseObject::Anchor::TopRight
    );
    loadCodeButton->setShader(&shader);
    loadCodeButton->SetOnClickSound(&clickSound);

    std::shared_ptr<Text> codePanelHeader = std::make_shared<Text>(
        "header",
        font,
        codePanel->getSize(),
        L"Код",
        18,
        sf::Vector2f(10.f, 0.f)
    );

    codePanel->addObject(codePanelHeader);
    codePanel->addObject(saveCodeButton); 
    codePanel->addObject(loadCodeButton);
    // code panel - end

    sf::Clock clock;
    while (window.isOpen())
    {
        // get mouse position
        sf::Vector2i mousePosInt = sf::Mouse::getPosition(window);
        sf::Vector2f mousePosFloat = SFMLUtility::CastVector2iToFloat(mousePosInt);

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
