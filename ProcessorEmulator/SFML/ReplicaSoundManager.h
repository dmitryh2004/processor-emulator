#pragma once
#include "RandomSoundPlayer.h"

#ifdef MEMES
	std::map<int, std::string> startParsingPhrases = {
		{0, "Опять ассемблер? Какое у нас там стоп-слово..."},
		{1, "Опять работать? Тебе меня не жалко?"},
		{2, "Может быть, сам на листочке скомпилируешь? Ну ладно, ладно, так уж и быть..."},
		{3, "sudo rm -rf / \n... Странно, раньше всегда работало. Ну ладно, сейчас посмотрю твой код"}
	};
	std::map<int, std::string> emptyProgramPhrases = {
		{0, "Чтобы запустить программу на процессоре, ее надо сначала написать, наверное? Сам-то как считаешь?"},
		{1, "Нет ТЗ - результат хз\n(с) Конфуций, 485 г. до н.э."},
		{2, "По-моему, ты забыл написать ассемблерный код. Или я его телепатией должен получить?"},
		{3, "Код, видимо, это алмаз из \"Большого куша\"..."},
		{4, "Ну, бездействие - это тоже в каком-то роде действие..."}
	};
	std::map<int, std::string> parsingFailedPhrases = {
		{0, "Если бы мне платили по одному рублю за каждую ошибку в ассемблере, я бы уже разбогател:"},
		{1, "Похоже, кто-то не читал методичку по ассемблеру этого процессора:"},
		{2, "Прежде чем записывать сюда код, убедись, что он написан правильно:"},
		{3, "Ты пишешь код на моем ассемблере, но делаешь это без знания синтаксиса:"}
	};
	std::map<int, std::string> parsingSuccessPhrases = {
		{0, "Ну, похоже на правду. А вот работает ли это, это уже другой вопрос"},
		{1, "Ладно, методичку по коду ты читал. На работу программы сейчас посмотрим..."},
		{2, "Ну, вроде ок. Сейчас запущу"},
		{3, "Так, на процессоре оно запустится. Надеюсь, это не трояны"},
		{4, "Технически оно работает. Время для рубрики ЭЭЭЭЭЭЭЭЭЭЭкспериментов"}
	};
	std::map<int, std::string> anotherProgramIsRunningPhrases = {
		{0, "Не так быстро - я еще не закончил с текущей программой"},
		{1, "Сначала разберись, что ты хочешь запустить, а потом уже запускай"},
		{2, "Если ты не знал, я однопоточный процессор. Другую программу я сейчас чисто физически не смогу запустить"},
		{3, "Мне кажется, многозадачность здесь - не лучшее решение. Лучше сделать что-то одно, но качественно"}
	};
#else
	std::map<int, std::string> startParsingPhrases = { {0, "Начинаем парсинг программы..."} };
	std::map<int, std::string> emptyProgramPhrases = { {0, "Парсинг невозможен: программа пуста"} };
	std::map<int, std::string> parsingFailedPhrases = { {0, "При парсинге найдены следующие ошибки: "} };
	std::map<int, std::string> parsingSuccessPhrases = { {0, "Парсинг программы выполнен успешно!"} };
	std::map<int, std::string> anotherProgramIsRunningPhrases = { {0, "Пока выполняется одна программа, другую загрузить нельзя"}};
#endif

class ReplicaSoundManager {
public:
	ReplicaSoundManager(RandomSoundPlayer& sp, 
		int pnfsa, 
		int ppfsa, 
		int apirsa):
		soundPlayer(sp),
		programNotFoundSoundsAmount(pnfsa),
		programParseFailedSoundsAmount(ppfsa),
		anotherProgramIsRunningSoundsAmount(apirsa)
	{

	}

	bool playProgramNotFoundSound(int index) {
		if (index < 0 || index >= programNotFoundSoundsAmount) {
			return false;
		}

		return soundPlayer.Play(index);
	}

	bool playProgramParseFailedSound(int index) {
		if (index < 0 || index >= programParseFailedSoundsAmount) {
			return false;
		}

		return soundPlayer.Play(index + programNotFoundSoundsAmount);
	}

	bool playAnotherProgramIsRunningSound(int index) {
		if (index < 0 || index >= anotherProgramIsRunningSoundsAmount) {
			return false;
		}

		return soundPlayer.Play(index + programNotFoundSoundsAmount + programParseFailedSoundsAmount);
	}

	int programNotFoundSoundsCount() { return programNotFoundSoundsAmount; }
	int programParseFailedSoundsCount() { return programParseFailedSoundsAmount; }
	int anotherProgramIsRunningSoundsCount() { return anotherProgramIsRunningSoundsAmount; }
private:
	RandomSoundPlayer& soundPlayer;
	int programNotFoundSoundsAmount;
	int programParseFailedSoundsAmount;
	int anotherProgramIsRunningSoundsAmount;
};