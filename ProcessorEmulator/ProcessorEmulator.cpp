#include "ProcessorEmulator.h"
#include "ResourceManager.h"

int main()
{
    sf::RenderWindow window(sf::VideoMode({ 1440, 900 }), "SFML window", sf::Style::Titlebar | sf::Style::Close);
    window.setSize(sf::Vector2u(1440, 900));
    window.setFramerateLimit(60);

    sf::Vector2f windowSizeFloat = SFMLUtility::CastVector2uToFloat(window.getSize());

    ResourceManager resources;

    const sf::Texture& backgroundTexture = resources.GetTexture("Assets/Sprites/background.png");

    const sf::Texture& buttonTexture = resources.GetTexture("Assets/Sprites/ButtonTexture.png");
    const sf::Texture& infoButtonTexture = resources.GetTexture("Assets/Sprites/infoButtonSprite.png");

    const sf::Font& textFont = resources.GetFont("Assets/Fonts/Rubik-Medium.ttf");
    const sf::Font& codeFont = resources.GetFont("Assets/Fonts/Courier-New.ttf");
    const sf::Font& registerFont = resources.GetFont("Assets/Fonts/Seven Segment.ttf");

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
        textFont,
        panel->getSize(),
        "Эмулятор процессора"_sf,
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

    std::shared_ptr<InputField> codeField = std::make_shared<InputField>(
        "codeField",
        sf::Vector2f(460.f, 500.f),
        codePanel->getSize(),
        codeFont,
        18,
        sf::Vector2f(0.f, 44.f)
    );

    std::shared_ptr<Button> saveCodeButton = std::make_shared<Button>("saveCodeButton",
        sf::Vector2f(24.f, 24.f),
        codePanel->getSize(),
        sf::Vector2f(-10.f, 10.f),
        buttonTexture,
        buttonTexture,
        BaseObject::Anchor::TopRight,
        BaseObject::Anchor::TopRight
    );
    saveCodeButton->setShader(&shader);
    saveCodeButton->SetOnClickSound(&clickSound);
    saveCodeButton->SetOnClickAction([&window, &codeField]() {
        // Используем pfd::save_file вместо pfd::open_file
        auto dialog = pfd::save_file("Сохранить как...", ".",
            { "Кастомный ассемблерный код (*.asmb)", "*.asmb",
              "Все файлы", "*" });

        if (!dialog.result().empty()) {
            std::string filePath = dialog.result(); // pfd::save_file возвращает std::string, а не std::vector

            // Проверяем, ввёл ли пользователь расширение .asmb, и добавляем его при необходимости
            if (filePath.size() < 5 || filePath.substr(filePath.size() - 5) != ".asmb") {
                filePath += ".asmb";
            }

            std::cout << "[saveCodeButton] Выбран файл для сохранения: " << filePath << std::endl;

            std::ofstream file(filePath);

            if (!file.is_open()) {
                std::cerr << "[saveCodeButton] " << std::strerror(errno) << " for path: " << filePath << std::endl;
                return;
            }

            file << codeField->getTextString();
            file.close();
        }
    });


    std::shared_ptr<Button> loadCodeButton = std::make_shared<Button>("loadCodeButton",
        sf::Vector2f(24.f, 24.f),
        codePanel->getSize(),
        sf::Vector2f(-44.f, 10.f),
        buttonTexture,
        buttonTexture,
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
            std::string filePath = dialog.result()[0];
            std::cout << "[loadCodeButton] Выбран файл для загрузки: " << filePath << std::endl;
            
            std::ifstream file(filePath);

            if (!file.is_open()) {
                std::cerr << "[loadCodeButton] Unable to read file " << filePath << std::endl;
                return;
            }

            std::string fileContent = std::string(std::istreambuf_iterator<char>(file),
                std::istreambuf_iterator<char>());
            codeField->setTextString(fileContent);
        }
    });

    std::shared_ptr<Text> codePanelHeader = std::make_shared<Text>(
        "codeHeader",
        textFont,
        codePanel->getSize(),
        "Код"_sf,
        18,
        sf::Vector2f(10.f, 18.f),
        BaseObject::Anchor::TopLeft,
        BaseObject::Anchor::CenterLeft
    );

    codePanel->addObject(codePanelHeader);
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
        sf::Vector2f(-24.f, 43.f),
        BaseObject::Anchor::TopCenter,
        BaseObject::Anchor::TopCenter
    );

    std::shared_ptr<RegisterContainer> irRegister = std::make_shared<RegisterContainer>(
        "registerContainerIR",
        "IR",
        registerFont,
        18,
        registerPanel->getSize(),
        sf::Vector2f(-24.f, 74.f),
        BaseObject::Anchor::TopCenter,
        BaseObject::Anchor::TopCenter
    );

    std::shared_ptr<RegisterContainer> marRegister = std::make_shared<RegisterContainer>(
        "registerContainerMar",
        "MAR",
        registerFont,
        18,
        registerPanel->getSize(),
        sf::Vector2f(-24.f, 105.f),
        BaseObject::Anchor::TopCenter,
        BaseObject::Anchor::TopCenter
    );

    std::shared_ptr<RegisterContainer> mdrRegister = std::make_shared<RegisterContainer>(
        "registerContainerMdr",
        "MDR",
        registerFont,
        18,
        registerPanel->getSize(),
        sf::Vector2f(-24.f, 136.f),
        BaseObject::Anchor::TopCenter,
        BaseObject::Anchor::TopCenter
    );

    std::shared_ptr<RegisterContainer> acRegister = std::make_shared<RegisterContainer>(
        "registerContainerAc",
        "AC",
        registerFont,
        18,
        registerPanel->getSize(),
        sf::Vector2f(-24.f, 167.f),
        BaseObject::Anchor::TopCenter,
        BaseObject::Anchor::TopCenter
    );

    std::shared_ptr<RegisterContainer> pcRegister = std::make_shared<RegisterContainer>(
        "registerContainerPc",
        "PC",
        registerFont,
        18,
        registerPanel->getSize(),
        sf::Vector2f(-24.f, 201.f),
        BaseObject::Anchor::TopCenter,
        BaseObject::Anchor::TopCenter
    );

    std::shared_ptr<Text> registerPanelHeader = std::make_shared<Text>(
        "registerHeader",
        textFont,
        registerPanel->getSize(),
        "Текущее состояние процессора"_sf,
        18,
        sf::Vector2f(10.f, 18.f),
        BaseObject::Anchor::TopLeft,
        BaseObject::Anchor::CenterLeft
    );

    registerPanel->addObject(outRegister);
    registerPanel->addObject(irRegister);
    registerPanel->addObject(marRegister);
    registerPanel->addObject(mdrRegister);
    registerPanel->addObject(acRegister);
    registerPanel->addObject(pcRegister);
    registerPanel->addObject(registerPanelHeader);
    // registers panel - end

    // current command - start
    std::shared_ptr<Panel> currentCommandPanel = std::make_shared<Panel>(
        "currentCommandPanel",
        sf::Vector2f(460.f, 544.f),
        windowSizeFloat,
        sf::Vector2f(-10.f, 46.f),
        BaseObject::Anchor::TopRight,
        BaseObject::Anchor::TopRight
    );

    std::shared_ptr<Text> currentCommandHeader = std::make_shared<Text>(
        "currentCommandHeader",
        textFont,
        currentCommandPanel->getSize(),
        "Текущая команда"_sf,
        18,
        sf::Vector2f(10.f, 18.f),
        BaseObject::Anchor::TopLeft,
        BaseObject::Anchor::CenterLeft
    );

    std::shared_ptr<Text> ccMachineCodeKey = std::make_shared<Text>(
        "ccMachineCodeKey",
        textFont,
        currentCommandPanel->getSize(),
        "Машинный код"_sf,
        14,
        sf::Vector2f(10.f, 46.f),
        BaseObject::Anchor::TopLeft,
        BaseObject::Anchor::TopLeft
    );

    std::shared_ptr<Text> ccAssemblerCommandKey = std::make_shared<Text>(
        "ccAssemblerCommandKey",
        textFont,
        currentCommandPanel->getSize(),
        "Команда ассемблера"_sf,
        14,
        sf::Vector2f(10.f, 66.f),
        BaseObject::Anchor::TopLeft,
        BaseObject::Anchor::TopLeft
    );

    std::shared_ptr<Text> ccAssemblerDecodedHeader = std::make_shared<Text>(
        "ccAssemblerDecodedHeader",
        textFont,
        currentCommandPanel->getSize(),
        "Расшифровка команды:"_sf,
        14,
        sf::Vector2f(10.f, 106.f),
        BaseObject::Anchor::TopLeft,
        BaseObject::Anchor::TopLeft
    );

    std::shared_ptr<Text> ccAssemblerOperationKey = std::make_shared<Text>(
        "ccAssemblerOperationKey",
        textFont,
        currentCommandPanel->getSize(),
        "Операция ассемблера"_sf,
        14,
        sf::Vector2f(10.f, 131.f),
        BaseObject::Anchor::TopLeft,
        BaseObject::Anchor::TopLeft
    );

    std::shared_ptr<Text> ccDestinationKey = std::make_shared<Text>(
        "ccDestinationKey",
        textFont,
        currentCommandPanel->getSize(),
        "Запись"_sf,
        14,
        sf::Vector2f(10.f, 151.f),
        BaseObject::Anchor::TopLeft,
        BaseObject::Anchor::TopLeft
    );

    std::shared_ptr<Text> ccOpAKey = std::make_shared<Text>(
        "ccOpAKey",
        textFont,
        currentCommandPanel->getSize(),
        "Операнд А"_sf,
        14,
        sf::Vector2f(10.f, 171.f),
        BaseObject::Anchor::TopLeft,
        BaseObject::Anchor::TopLeft
    );

    std::shared_ptr<Text> ccOpBKey = std::make_shared<Text>(
        "ccOpBKey",
        textFont,
        currentCommandPanel->getSize(),
        "Операнд B"_sf,
        14,
        sf::Vector2f(10.f, 191.f),
        BaseObject::Anchor::TopLeft,
        BaseObject::Anchor::TopLeft
    );

    std::shared_ptr<Text> ccOpAddrKey = std::make_shared<Text>(
        "ccOpAddrKey",
        textFont,
        currentCommandPanel->getSize(),
        "Адрес"_sf,
        14,
        sf::Vector2f(10.f, 211.f),
        BaseObject::Anchor::TopLeft,
        BaseObject::Anchor::TopLeft
    );

    std::shared_ptr<Text> ccDescriptionKey = std::make_shared<Text>(
        "ccDescriptionKey",
        textFont,
        currentCommandPanel->getSize(),
        "Описание:"_sf,
        14,
        sf::Vector2f(10.f, 251.f),
        BaseObject::Anchor::TopLeft,
        BaseObject::Anchor::TopLeft
    );

    // values

    std::shared_ptr<Text> ccMachineCodeValue = std::make_shared<Text>(
        "ccMachineCodeValue",
        textFont,
        currentCommandPanel->getSize(),
        "0000 0000 0000 0000"_sf,
        14,
        sf::Vector2f(10.f, 46.f),
        BaseObject::Anchor::TopCenter,
        BaseObject::Anchor::TopLeft
    );

    std::shared_ptr<Text> ccAssemblerCommandValue = std::make_shared<Text>(
        "ccAssemblerCommandValue",
        textFont,
        currentCommandPanel->getSize(),
        "STOR"_sf,
        14,
        sf::Vector2f(10.f, 66.f),
        BaseObject::Anchor::TopCenter,
        BaseObject::Anchor::TopLeft
    );

    std::shared_ptr<Text> ccAssemblerOperationValue = std::make_shared<Text>(
        "ccAssemblerOperationValue",
        textFont,
        currentCommandPanel->getSize(),
        "MOV"_sf,
        14,
        sf::Vector2f(10.f, 131.f),
        BaseObject::Anchor::TopCenter,
        BaseObject::Anchor::TopLeft
    );

    std::shared_ptr<Text> ccDestinationValue = std::make_shared<Text>(
        "ccDestinationValue",
        textFont,
        currentCommandPanel->getSize(),
        "Запись"_sf,
        14,
        sf::Vector2f(10.f, 151.f),
        BaseObject::Anchor::TopCenter,
        BaseObject::Anchor::TopLeft
    );

    std::shared_ptr<Text> ccOpAValue = std::make_shared<Text>(
        "ccOpAValue",
        textFont,
        currentCommandPanel->getSize(),
        "Операнд А"_sf,
        14,
        sf::Vector2f(10.f, 171.f),
        BaseObject::Anchor::TopCenter,
        BaseObject::Anchor::TopLeft
    );

    std::shared_ptr<Text> ccOpBValue = std::make_shared<Text>(
        "ccOpBValue",
        textFont,
        currentCommandPanel->getSize(),
        "Операнд B"_sf,
        14,
        sf::Vector2f(10.f, 191.f),
        BaseObject::Anchor::TopCenter,
        BaseObject::Anchor::TopLeft
    );

    std::shared_ptr<Text> ccOpAddrValue = std::make_shared<Text>(
        "ccOpAddrValue",
        textFont,
        currentCommandPanel->getSize(),
        "Адрес"_sf,
        14,
        sf::Vector2f(10.f, 211.f),
        BaseObject::Anchor::TopCenter,
        BaseObject::Anchor::TopLeft
    );

    std::shared_ptr<Text> ccDescriptionValue = std::make_shared<Text>(
        "ccDescriptionValue",
        textFont,
        currentCommandPanel->getSize(),
        "Команда JMP записывает значение A в регистр PC,\n\
позволяя таким образом реализовать условные\n\
переходы между блоками программы."_sf,
        14,
        sf::Vector2f(10.f, 271.f),
        BaseObject::Anchor::TopLeft,
        BaseObject::Anchor::TopLeft
    );

    currentCommandPanel->addObject(currentCommandHeader);
    currentCommandPanel->addObject(ccMachineCodeKey);
    currentCommandPanel->addObject(ccAssemblerCommandKey);
    currentCommandPanel->addObject(ccAssemblerDecodedHeader);
    currentCommandPanel->addObject(ccAssemblerOperationKey);
    currentCommandPanel->addObject(ccDestinationKey);
    currentCommandPanel->addObject(ccOpAKey);
    currentCommandPanel->addObject(ccOpBKey);
    currentCommandPanel->addObject(ccOpAddrKey);
    currentCommandPanel->addObject(ccDescriptionKey);


    currentCommandPanel->addObject(ccMachineCodeValue);
    currentCommandPanel->addObject(ccAssemblerCommandValue);
    currentCommandPanel->addObject(ccAssemblerOperationValue);
    currentCommandPanel->addObject(ccDestinationValue);
    currentCommandPanel->addObject(ccOpAValue);
    currentCommandPanel->addObject(ccOpBValue);
    currentCommandPanel->addObject(ccOpAddrValue);
    currentCommandPanel->addObject(ccDescriptionValue);
    // current command - end

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
            currentCommandPanel->checkForEvents(*event, window, mousePosFloat);
        }
        
        sf::Time deltaTime = clock.restart();
        panel->update(deltaTime);
        codePanel->update(deltaTime);
        registerPanel->update(deltaTime);
        currentCommandPanel->update(deltaTime);
        
        window.clear();
        window.draw(bgSprite);
        window.draw(*panel);
        window.draw(*codePanel);
        window.draw(*registerPanel);
        window.draw(*currentCommandPanel);
        window.display();
    }

    bgMusic.stop();
}
