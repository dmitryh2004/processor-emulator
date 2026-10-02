#include "ProcessorEmulator.h"
#include "ResourceManager.h"

int main()
{
    // initialization - start
    std::random_device rd;
    std::mt19937 gen(rd()); // Генератор
    std::uniform_int_distribution<> dist(1, 8); // Распределение

    std::setlocale(LC_ALL, ".UTF-8"); // настройка консоли на отображение сообщений в utf-8

    Processor processor(65536);
    RAM ram = processor.GetRAM();

    sf::RenderWindow window(sf::VideoMode({ 1440, 900 }), "SFML window", sf::Style::Titlebar | sf::Style::Close);
    window.setSize(sf::Vector2u(1440, 900));
    window.setFramerateLimit(60);

    sf::Vector2f windowSizeFloat = SFMLUtility::CastVector2uToFloat(window.getSize());

    sf::Color hoverColor = sf::Color::Color(255, 255, 0);
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
    const sf::Texture& ramResetButtonTexture = resources.GetTexture("Assets/Sprites/ramResetButtonSprite.png");

    const sf::Texture& sliderTexture = resources.GetTexture("Assets/Sprites/speed slider.png");

    const sf::Font& textFont = resources.GetFont("Assets/Fonts/Rubik-Medium.ttf");
    const sf::Font& codeFont = resources.GetFont("Assets/Fonts/Courier-New.ttf");
    const sf::Font& registerFont = resources.GetFont("Assets/Fonts/Seven Segment.ttf");

    int i = dist(gen);
    sf::Music& bgMusic = resources.GetMusic("Assets/Sounds/background-music-" + std::to_string(i) + ".mp3");
    bgMusic.setLooping(true);
    bgMusic.play();

    sf::Sound clickSound(resources.GetSoundBuffer("Assets/Sounds/click-sound.mp3"));

    sf::Shader& shader = resources.GetShader("SFML/Shaders/BaseShader.frag", sf::Shader::Type::Fragment);
    sf::Shader& ppShader = resources.GetShader("SFML/Shaders/PostProcessShader.frag", sf::Shader::Type::Fragment);
    ppShader.setUniform("grayTexture", backgroundMaskTexture);

    sf::RenderTexture renderTexture = sf::RenderTexture(window.getSize());

    // resource initialization - end
    // initialization - end

    std::shared_ptr<LogText> logField;

    // background sprite
    Image bgSprite = Image("bgSprite", sf::Vector2f(1440.f, 900.f), backgroundTexture, windowSizeFloat);


    // top panel - start
    std::shared_ptr<Panel> panel = std::make_shared<Panel>("headerPanel", sf::Vector2f(1440.f, 36.f), windowSizeFloat);

    // slider
    std::vector<SliderObject::AnchorPoint> anchors = std::vector<SliderObject::AnchorPoint>();
    anchors.push_back({ 12.f, 1 });
    anchors.push_back({ 29.f, 2 });
    anchors.push_back({ 46.f, 5 });
    anchors.push_back({ 67.f, 10 });
    anchors.push_back({ 90.f, 20 });
    anchors.push_back({ 113.f, 60 });

    std::shared_ptr<SliderObject> slider = std::make_shared<SliderObject>("clockSpeedSlider", 
        sf::Vector2f(130.f, 14.f), 
        windowSizeFloat,
        anchors, 
        1, 
        sf::Vector2f(1238.f, 4.f), 
        BaseObject::Anchor::TopLeft,
        BaseObject::Anchor::TopLeft, 
        0.f,
        sf::Vector2f(1.f, 1.f), 
        nullptr,
        &sliderTexture);

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
    saveCodeButton->SetOnClickAction([&window, &codeField, &logField]() {
        auto dialog = pfd::save_file("Сохранить как...", ".",
            { "Кастомный ассемблерный код (*.asmb)", "*.asmb",
              "Все файлы", "*" });

        if (!dialog.result().empty()) {
            std::string filePathStr = dialog.result();

            // Проверяем, ввёл ли пользователь расширение .asmb
            if (filePathStr.size() < 5 || filePathStr.substr(filePathStr.size() - 5) != ".asmb") {
                filePathStr += ".asmb";
            }

            // Преобразуем UTF-8 строку от pfd в кроссплатформенный std::filesystem::path
            std::filesystem::path filePath = std::filesystem::u8path(filePathStr);

            // Открываем файл в бинарном режиме, чтобы избежать системно-зависимых искажений перевода строк
            std::ofstream file(filePath, std::ios::binary);

            if (!file.is_open()) {
                logField->appendLog("[saveCodeButton] Не удалось сохранить код в следующий файл: " + filePathStr);
                return;
            }

            // 1. Получаем UTF-8 представление строки (в SFML 3 это sf::U8String / std::u8string)
            // Если у вас в коде getTextString() возвращает std::string, лучше вызовите метод напрямую у m_string внутри InputField, 
            // либо временно воспользуйтесь кодом ниже:
            sf::String sfStrText = codeField->getTextString(); // Предполагаем, что getTextString() теперь возвращает sf::String или std::string

            // Надежнее всего получить чистый UTF-8 из sf::String:
            // (Если getTextString() все еще возвращает std::string, измените его возвращаемый тип на sf::String или добавьте новый метод getSfString())
            sf::U8String utf8Str = sfStrText.toUtf8();

            // 2. Записываем байты UTF-8 строки в файл
            file.write(reinterpret_cast<const char*>(utf8Str.data()), utf8Str.size());
            file.close();

            logField->appendLog("[saveCodeButton] Код успешно сохранен в следующий файл: " + filePathStr);
        }
    }
    );

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
    loadCodeButton->SetOnClickAction([&window, &codeField, &logField]() {
        auto dialog = pfd::open_file("Выберите файл для загрузки", ".",
            { "Кастомный ассемблерный код (*.asmb)", "*.asmb",
              "Все файлы", "*" });

        if (!dialog.result().empty()) {
            std::string filePathStr = dialog.result()[0];

            // Преобразуем UTF-8 строку от pfd в кроссплатформенный std::filesystem::path
            std::filesystem::path filePath = std::filesystem::u8path(filePathStr);

            // Открываем файл в бинарном режиме для точного побайтового чтения UTF-8
            std::ifstream file(filePath, std::ios::binary | std::ios::ate);

            if (!file.is_open()) {
                logField->appendLog("[loadCodeButton] Не удалось прочитать файл " + filePathStr);
                return;
            }

            // Определяем размер файла и выделяем буфер
            std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);

            std::string utf8Content;
            if (size > 0) {
                utf8Content.resize(static_cast<size_t>(size));
                if (!file.read(&utf8Content[0], size)) {
                    logField->appendLog("[loadCodeButton] Ошибка при чтении файла " + filePathStr);
                    return;
                }
            }
            file.close();

            // Конвертируем UTF-8 (std::string) в sf::String. 
            // SFML 3 автоматически и корректно переведет многобайтовую кириллицу в UTF-32.
            sf::String sfStrContent = sf::String::fromUtf8(utf8Content.begin(), utf8Content.end());

            // Загружаем текст в текстовое поле
            // (Метод setTextString теперь должен принимать sf::String или std::u32string)
            codeField->setTextString(sfStrContent);

            logField->appendLog("[loadCodeButton] Успешно загружен код из файла: " + filePathStr);
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
    std::shared_ptr<Panel> ramPanel = std::make_shared<Panel>(
        "registerPanel",
        sf::Vector2f(698.f, 279.f),
        windowSizeFloat,
        sf::Vector2f(10.f, 605.f),
        BaseObject::Anchor::TopLeft,
        BaseObject::Anchor::TopLeft
    );

    std::shared_ptr<Text> currentPageText = std::make_shared<Text>(
        "ramCurrentPage", registerFont, ramPanel->getSize(), "1", 18, registerTextColor, false, sf::Vector2f(617.f, 7.f)
    );

    std::shared_ptr<RAMViewer> ramViewer = std::make_shared<RAMViewer>(
        "ramViewer", sf::Vector2f(688.f, 230.f), ramPanel->getSize(), ram, 10, 5, sf::Vector2f(69.f, 46.f), registerFont, textFont,
        12, registerTextColor, hoverColor, 0.f, sf::Vector2f(6.f, 44.f)
    );

    std::shared_ptr<Button> nextPageButton = std::make_shared<Button>("ramNextPageBtn",
        sf::Vector2f(24.f, 24.f),
        ramPanel->getSize(),
        sf::Vector2f(665.f, 6.f),
        openButtonTexture,
        openButtonTexture,
        BaseObject::Anchor::TopLeft,
        BaseObject::Anchor::TopLeft
    );
    nextPageButton->setShader(&shader);
    nextPageButton->SetOnClickSound(&clickSound);
    nextPageButton->SetOnClickAction([&ramViewer, &currentPageText] {
        ramViewer->nextPage();
        currentPageText->setString(std::to_string(ramViewer->getCurrentPage()));
    });

    std::shared_ptr<Button> prevPageButton = std::make_shared<Button>("ramPrevPageBtn",
        sf::Vector2f(24.f, 24.f),
        ramPanel->getSize(),
        sf::Vector2f(584.f, 6.f),
        openButtonTexture,
        openButtonTexture,
        BaseObject::Anchor::TopLeft,
        BaseObject::Anchor::TopLeft
    );
    prevPageButton->setShader(&shader);
    prevPageButton->SetOnClickSound(&clickSound);
    prevPageButton->SetOnClickAction([&ramViewer, &currentPageText] {
        ramViewer->prevPage();
        currentPageText->setString(std::to_string(ramViewer->getCurrentPage()));
    });

    std::shared_ptr<Button> ramResetButton = std::make_shared<Button>("ramResetBtn",
        sf::Vector2f(24.f, 24.f),
        ramPanel->getSize(),
        sf::Vector2f(534.f, 6.f),
        ramResetButtonTexture,
        ramResetButtonTexture,
        BaseObject::Anchor::TopLeft,
        BaseObject::Anchor::TopLeft
    );
    ramResetButton->setShader(&shader);
    ramResetButton->SetOnClickSound(&clickSound);
    ramResetButton->SetOnClickAction([&ramViewer, &logField] {
        ramViewer->reset();
        logField->appendLog("Память успешно сброшена");
    });

    std::shared_ptr<Button> ramSaveButton = std::make_shared<Button>("ramSaveBtn",
        sf::Vector2f(24.f, 24.f),
        ramPanel->getSize(),
        sf::Vector2f(494.f, 6.f),
        saveButtonTexture,
        saveButtonTexture,
        BaseObject::Anchor::TopLeft,
        BaseObject::Anchor::TopLeft
    );
    ramSaveButton->setShader(&shader);
    ramSaveButton->SetOnClickSound(&clickSound);
    ramSaveButton->SetOnClickAction([&ramViewer, &logField] {
        try {
            ramViewer->saveDump();
            logField->appendLog("Дамп памяти сохранен в memdump.bin; проверьте директорию рядом с .exe файлом");
        }
        catch (std::runtime_error e) {
            std::string logString = "Не удалось сохранить дамп памяти: "; 
            logString += e.what();
            logField->appendLog(logString);
        }
    });

    std::shared_ptr<Button> ramLoadButton = std::make_shared<Button>("ramLoadBtn",
        sf::Vector2f(24.f, 24.f),
        ramPanel->getSize(),
        sf::Vector2f(454.f, 6.f),
        openButtonTexture,
        openButtonTexture,
        BaseObject::Anchor::TopLeft,
        BaseObject::Anchor::TopLeft
    );
    ramLoadButton->setShader(&shader);
    ramLoadButton->SetOnClickSound(&clickSound);
    ramLoadButton->SetOnClickAction([&ramViewer, &logField] {
        try {
            ramViewer->loadDump();
            logField->appendLog("Память загружена из дампа memdump.bin");
        }
        catch (std::runtime_error e) {
            std::string logString = "Не удалось загрузить дамп памяти: ";
            logString += e.what();
            logField->appendLog(logString);
        }
    });

    ramPanel->addObject(ramViewer);
    ramPanel->addObject(currentPageText);
    ramPanel->addObject(prevPageButton);
    ramPanel->addObject(nextPageButton);
    ramPanel->addObject(ramResetButton);
    ramPanel->addObject(ramSaveButton);
    ramPanel->addObject(ramLoadButton);
    // ram viewer - end

    // log panel - start
    std::shared_ptr<Panel> logPanel = std::make_shared<Panel>(
        "registerPanel",
        sf::Vector2f(698.f, 279.f),
        windowSizeFloat,
        sf::Vector2f(730.f, 605.f),
        BaseObject::Anchor::TopLeft,
        BaseObject::Anchor::TopLeft
    );

    logField = std::make_shared<LogText>(
        "logField",
        sf::Vector2f(693.f, 230.f),
        logPanel->getSize(),
        codeFont,
        18,
        sf::Vector2f(6.f, 44.f)
    );
    logField->loadSyntaxScheme("SFML/ColorConfigs/log.conf");

    std::shared_ptr<Button> clearLogButton = std::make_shared<Button>("clearLogButton",
        sf::Vector2f(24.f, 24.f),
        ramPanel->getSize(),
        sf::Vector2f(665.f, 6.f),
        ramResetButtonTexture,
        ramResetButtonTexture,
        BaseObject::Anchor::TopLeft,
        BaseObject::Anchor::TopLeft
    );
    clearLogButton->setShader(&shader);
    clearLogButton->SetOnClickSound(&clickSound);
    clearLogButton->SetOnClickAction([&logField] {
        logField->clearLog();
    });

    logPanel->addObject(logField);
    logPanel->addObject(clearLogButton);
    // log panel - end

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

            slider->checkForEvents(*event, window, mousePosFloat);
            panel->checkForEvents(*event, window, mousePosFloat);
            codePanel->checkForEvents(*event, window, mousePosFloat);
            registerPanel->checkForEvents(*event, window, mousePosFloat);
            ccc->checkForEvents(*event, window, mousePosFloat);
            ramPanel->checkForEvents(*event, window, mousePosFloat);
            logPanel->checkForEvents(*event, window, mousePosFloat);
        }
        
        sf::Time deltaTime = clock.restart();
        slider->update(deltaTime, window, mousePosFloat);
        panel->update(deltaTime, window, mousePosFloat);
        codePanel->update(deltaTime, window, mousePosFloat);
        registerPanel->update(deltaTime, window, mousePosFloat);
        ccc->update(deltaTime, window, mousePosFloat);
        ramPanel->update(deltaTime, window, mousePosFloat);
        logPanel->update(deltaTime, window, mousePosFloat);
        
        renderTexture.clear();
        renderTexture.draw(*slider);
        renderTexture.draw(bgSprite);
        renderTexture.draw(*panel);
        renderTexture.draw(*codePanel);
        renderTexture.draw(*registerPanel);
        renderTexture.draw(*ccc);
        renderTexture.draw(*ramPanel);
        renderTexture.draw(*logPanel);
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
