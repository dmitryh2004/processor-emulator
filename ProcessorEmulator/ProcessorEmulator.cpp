#include "ProcessorEmulator.h"
#include "ResourceManager.h"

int main()
{
    // initialization - start
    std::setlocale(LC_ALL, ".UTF-8"); // настройка консоли на отображение сообщений в utf-8

    RAM ram(65536);

    sf::RenderWindow window(sf::VideoMode({ 1440, 900 }), "SFML window", sf::Style::Titlebar | sf::Style::Close);
    window.setSize(sf::Vector2u(1440, 900));
    window.setFramerateLimit(60);

    sf::Vector2f windowSizeFloat = SFMLUtility::CastVector2uToFloat(window.getSize());

    sf::Color registerTextColor = sf::Color::Color(0, 192, 0);
    sf::Color registerTextColorModified = sf::Color::Color(0, 255, 0);

    sf::Color ccTextColor = sf::Color::Color(0, 128, 0); 
    sf::Color ccTextColorNotActive = sf::Color::Color(128, 128, 128);

    // resource initialization - start
    ResourceManager resources;

    const sf::Texture& backgroundTexture = resources.GetTexture("Assets/Sprites/background alpha.png");
    const sf::Texture& backgroundMaskTexture = resources.GetTexture("Assets/Sprites/background mask.png");

    const sf::Texture& buttonTexture = resources.GetTexture("Assets/Sprites/ButtonTexture.png");
    const sf::Texture& infoButtonTexture = resources.GetTexture("Assets/Sprites/infoButtonSprite.png");
    const sf::Texture& openButtonTexture = resources.GetTexture("Assets/Sprites/openButtonSprite.png");
    const sf::Texture& saveButtonTexture = resources.GetTexture("Assets/Sprites/saveButtonSprite.png");
    const sf::Texture& startButtonTexture = resources.GetTexture("Assets/Sprites/startButtonSprite.png");
    const sf::Texture& stepButtonTexture = resources.GetTexture("Assets/Sprites/stepButtonSprite.png");
    const sf::Texture& stopButtonTexture = resources.GetTexture("Assets/Sprites/stopButtonSprite.png");

    const sf::Font& textFont = resources.GetFont("Assets/Fonts/Rubik-Medium.ttf");
    const sf::Font& codeFont = resources.GetFont("Assets/Fonts/Courier-New.ttf");
    const sf::Font& registerFont = resources.GetFont("Assets/Fonts/Seven Segment.ttf");

    sf::Music& bgMusic = resources.GetMusic("Assets/Sounds/background-music.mp3");
    bgMusic.setLooping(true);
    bgMusic.play();

    sf::Sound clickSound(resources.GetSoundBuffer("Assets/Sounds/click-sound.mp3"));

    sf::Shader& shader = resources.GetShader("SFML/Shaders/BaseShader.frag", sf::Shader::Type::Fragment);
    sf::Shader& ppShader = resources.GetShader("SFML/Shaders/PostProcessShader.frag", sf::Shader::Type::Fragment);
    ppShader.setUniform("grayTexture", backgroundMaskTexture);

    sf::RenderTexture renderTexture = sf::RenderTexture(window.getSize());

    // resource initialization - end
    // initialization - end

    // background sprite
    Image bgSprite = Image("bgSprite", sf::Vector2f(1440.f, 900.f), backgroundTexture, windowSizeFloat);

    // top panel - start
    std::shared_ptr<Panel> panel = std::make_shared<Panel>("headerPanel", sf::Vector2f(1440.f, 36.f), windowSizeFloat);

    std::shared_ptr<Button> infoButton = std::make_shared<Button>("infoButton", 
        sf::Vector2f(24.f, 24.f), 
        panel->getSize(), 
        sf::Vector2f(-10.f, 0.f), 
        infoButtonTexture, 
        infoButtonTexture,
        BaseObject::Anchor::CenterRight, 
        BaseObject::Anchor::CenterRight
    );
    infoButton->setShader(&shader);
    infoButton->SetOnClickSound(&clickSound);

    std::shared_ptr<Button> startButton = std::make_shared<Button>("startButton",
        sf::Vector2f(24.f, 24.f),
        panel->getSize(),
        sf::Vector2f(-40.f, 0.f),
        startButtonTexture,
        startButtonTexture,
        BaseObject::Anchor::Center,
        BaseObject::Anchor::Center
    ); 
    startButton->setShader(&shader);
    startButton->SetOnClickSound(&clickSound);

    std::shared_ptr<Button> stepButton = std::make_shared<Button>("stepButton",
        sf::Vector2f(24.f, 24.f),
        panel->getSize(),
        sf::Vector2f(0.f, 0.f),
        stepButtonTexture,
        stepButtonTexture,
        BaseObject::Anchor::Center,
        BaseObject::Anchor::Center
    );
    stepButton->setShader(&shader);
    stepButton->SetOnClickSound(&clickSound);

    std::shared_ptr<Button> stopButton = std::make_shared<Button>("stopButton",
        sf::Vector2f(24.f, 24.f),
        panel->getSize(),
        sf::Vector2f(40.f, 0.f),
        stopButtonTexture,
        stopButtonTexture,
        BaseObject::Anchor::Center,
        BaseObject::Anchor::Center
    );
    stopButton->setShader(&shader);
    stopButton->SetOnClickSound(&clickSound);

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

    std::shared_ptr<InputField> codeField = std::make_shared<InputField>(
        "codeField",
        sf::Vector2f(447.f, 493.f),
        codePanel->getSize(),
        codeFont,
        18,
        sf::Vector2f(8.f, 44.f)
    );

    std::shared_ptr<Button> saveCodeButton = std::make_shared<Button>("saveCodeButton",
        sf::Vector2f(24.f, 24.f),
        codePanel->getSize(),
        sf::Vector2f(-7.f, 6.f),
        saveButtonTexture,
        saveButtonTexture,
        BaseObject::Anchor::TopRight,
        BaseObject::Anchor::TopRight
    );
    saveCodeButton->setShader(&shader);
    saveCodeButton->SetOnClickSound(&clickSound);
    saveCodeButton->SetOnClickAction([&window, &codeField]() {
        auto dialog = pfd::save_file("Сохранить как...", ".",
            { "Кастомный ассемблерный код (*.asmb)", "*.asmb",
              "Все файлы", "*" });

        if (!dialog.result().empty()) {
            std::string filePathStr = dialog.result();

            // Проверяем, ввёл ли пользователь расширение .asmb
            if (filePathStr.size() < 5 || filePathStr.substr(filePathStr.size() - 5) != ".asmb") {
                filePathStr += ".asmb";
            }

            std::cout << "[saveCodeButton] Save file location: " << filePathStr << std::endl;

            // Преобразуем UTF-8 строку от pfd в кроссплатформенный std::filesystem::path
            std::filesystem::path filePath = std::filesystem::u8path(filePathStr);

            // Передаем объект path напрямую в поток
            std::ofstream file(filePath);

            if (!file.is_open()) {
                std::cerr << "[saveCodeButton] " << std::strerror(errno) << " for path: " << filePathStr << std::endl;
                return;
            }

            file << codeField->getTextString();
            file.close();
        }
    });

    std::shared_ptr<Button> loadCodeButton = std::make_shared<Button>("loadCodeButton",
        sf::Vector2f(24.f, 24.f),
        codePanel->getSize(),
        sf::Vector2f(-47.f, 6.f),
        openButtonTexture,
        openButtonTexture,
        BaseObject::Anchor::TopRight,
        BaseObject::Anchor::TopRight
    );
    loadCodeButton->setShader(&shader);
    loadCodeButton->SetOnClickSound(&clickSound);
    loadCodeButton->SetOnClickAction([&window, &codeField]() {
        auto dialog = pfd::open_file("Выберите файл для загрузки", ".",
            { "Кастомный ассемблерный код (*.asmb)", "*.asmb",
              "Все файлы", "*" });

        if (!dialog.result().empty()) {
            std::string filePathStr = dialog.result()[0];
            std::cout << "[loadCodeButton] Opening file at location: " << filePathStr << std::endl;

            // Преобразуем UTF-8 строку от pfd в кроссплатформенный std::filesystem::path
            std::filesystem::path filePath = std::filesystem::u8path(filePathStr);

            // Передаем объект path напрямую в поток
            std::ifstream file(filePath);

            if (!file.is_open()) {
                std::cerr << "[loadCodeButton] Unable to read file " << filePathStr << std::endl;
                return;
            }

            std::string fileContent = std::string(std::istreambuf_iterator<char>(file),
                std::istreambuf_iterator<char>());
            codeField->setTextString(fileContent);
        }
    });

    codePanel->addObject(saveCodeButton); 
    codePanel->addObject(loadCodeButton);
    codePanel->addObject(codeField);
    // code panel - end

    // registers panel - start
    std::shared_ptr<Panel> registerPanel = std::make_shared<Panel>(
        "registerPanel",
        sf::Vector2f(460.f, 544.f),
        windowSizeFloat,
        sf::Vector2f(0.f, 46.f),
        BaseObject::Anchor::TopCenter,
        BaseObject::Anchor::TopCenter
    );

    std::shared_ptr<RegisterContainer> outRegister = std::make_shared<RegisterContainer>(
        "registerContainerOut",
        "OUT",
        registerFont,
        18,
        registerPanel->getSize(),
        registerTextColor,
        sf::Vector2f(-24.f, 46.f),
        BaseObject::Anchor::TopCenter,
        BaseObject::Anchor::TopCenter
    );

    std::shared_ptr<RegisterContainer> irRegister = std::make_shared<RegisterContainer>(
        "registerContainerIR",
        "IR",
        registerFont,
        18,
        registerPanel->getSize(),
        registerTextColor,
        sf::Vector2f(-24.f, 77.f),
        BaseObject::Anchor::TopCenter,
        BaseObject::Anchor::TopCenter
    );

    std::shared_ptr<RegisterContainer> marRegister = std::make_shared<RegisterContainer>(
        "registerContainerMar",
        "MAR",
        registerFont,
        18,
        registerPanel->getSize(),
        registerTextColor,
        sf::Vector2f(-24.f, 108.f),
        BaseObject::Anchor::TopCenter,
        BaseObject::Anchor::TopCenter
    );

    std::shared_ptr<RegisterContainer> mdrRegister = std::make_shared<RegisterContainer>(
        "registerContainerMdr",
        "MDR",
        registerFont,
        18,
        registerPanel->getSize(),
        registerTextColor,
        sf::Vector2f(-24.f, 139.f),
        BaseObject::Anchor::TopCenter,
        BaseObject::Anchor::TopCenter
    );

    std::shared_ptr<RegisterContainer> acRegister = std::make_shared<RegisterContainer>(
        "registerContainerAc",
        "AC",
        registerFont,
        18,
        registerPanel->getSize(),
        registerTextColor,
        sf::Vector2f(-24.f, 170.f),
        BaseObject::Anchor::TopCenter,
        BaseObject::Anchor::TopCenter
    );

    std::shared_ptr<RegisterContainer> pcRegister = std::make_shared<RegisterContainer>(
        "registerContainerPc",
        "PC",
        registerFont,
        18,
        registerPanel->getSize(),
        registerTextColor,
        sf::Vector2f(-24.f, 201.f),
        BaseObject::Anchor::TopCenter,
        BaseObject::Anchor::TopCenter
    );

    registerPanel->addObject(outRegister);
    registerPanel->addObject(irRegister);
    registerPanel->addObject(marRegister);
    registerPanel->addObject(mdrRegister);
    registerPanel->addObject(acRegister);
    registerPanel->addObject(pcRegister);
    // registers panel - end

    // current command - start
    std::shared_ptr<CurrentCommandContainer> ccc = std::make_shared<CurrentCommandContainer>("currentCommand", textFont,
        18,
        sf::Vector2f(448.f, 494.f),
        windowSizeFloat,
        ccTextColor,
        ccTextColorNotActive,
        sf::Vector2f(-18.f, 91.f),
        BaseObject::Anchor::TopRight,
        BaseObject::Anchor::TopRight);

    ccc->SetActive(true);
    ccc->SetCurrentValues(3828350977, 7);
    // current command - end

    // ram viewer - start
    RAMViewer ramViewer(
        "ramViewer", sf::Vector2f(688.f, 230.f), windowSizeFloat, ram, 10, 5, sf::Vector2f(68.f, 45.f), registerFont, 
        14, registerTextColor, 0.f, sf::Vector2f(16.f, 649.f)
    );
    // ram viewer - end

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
            registerPanel->checkForEvents(*event, window, mousePosFloat);
            ccc->checkForEvents(*event, window, mousePosFloat);
        }
        
        sf::Time deltaTime = clock.restart();
        panel->update(deltaTime);
        codePanel->update(deltaTime);
        registerPanel->update(deltaTime);
        ccc->update(deltaTime);
        
        renderTexture.clear();
        renderTexture.draw(bgSprite);
        renderTexture.draw(*panel);
        renderTexture.draw(*codePanel);
        renderTexture.draw(*registerPanel);
        renderTexture.draw(*ccc);
        renderTexture.display();

        sf::Sprite result = sf::Sprite(renderTexture.getTexture());

        window.clear();

        sf::RenderStates resultStates;
        resultStates.shader = &ppShader;

        window.draw(result, resultStates);
        window.display();
    }

    bgMusic.stop();
}
