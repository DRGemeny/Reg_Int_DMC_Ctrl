#pragma once
#include <stdint.h>
#include <string>
#include "../../log/Log.h"

using namespace std;

class WorkRG;				// Класс - объект регистровый интерфейс пользователя ПЛИС
class HIError;				// Класс - объект, который содержит код ошибки вызова функции работы с ПЛИС

class HIError
{
private:
	string FunName;
	int Msg;
public:
	HIError(string fun_name, int msg);
	int GetMsg();
	string GetFun();
};

class WorkRG
{
private:
	static void *Handle;	// Указатель на интерфейс пользователя

	int BM;					// Номер Базового модуля ПЛИС
	int Num;				// Порядковый номер интерфейса
	CLog Log;				// Лог файл объекта "регистровый интерфейс"

public:
	WorkRG(int bm, int num, string log_name, bool debug);
	WorkRG(const WorkRG &Rg);
	WorkRG operator=(const WorkRG &Rg);
	~WorkRG();

	static void OPEN();					// Открытие интерфейса для работы с ПЛИС
	static void CLOSE();				// Закрытие интерфейса для работы с ПЛИС

	static void PUSK_ALL(int Bm);		// Запуск всех конвееров
	static void STOP_ALL(int Bm);		// Остановка всех конвееров
	static void RESET_ALL(int Bm);		// Сброс всех конвееров


	// Задание времени работы в тактах
	void SetTact(uint64_t tact);

	// Задание скважности
	// len - длительность импульса; T - период повторения
	void SetSkv(uint16_t len, uint16_t T);

	// Получение значение статуса 128-бит слово
	void GetStatus(uint32_t *Status, uint32_t read_word=4);
	// Получение числа тактов
	uint64_t GetTact();

	// Чтение регистра комадн
	uint32_t GetCmdReg();

	void Pusk();
	void Stop();
	void Reset();
	void ResetLVDS();

	// Функции загрузки/выгрузки данных
	// Data - массив данных
	// Size - размер массива Data в байтах
	// Rg - номер регистра записи
	void Load(void *Data, int Size, int Rg);			// Функция загрузки данных в медленном режиме

	// Data - массив данных
	// Size - размер массива Data в байтах
	// Rg - номер регистра чтения
	void UnLoad(void *Data, int Size, int Rg);			// Функция выгрузки данных в медленном режиме

	// Data - массив данных
	// Size - размер массива Data в байтах
	// Rg - номер загружаемого регистра
	void LoadFast(void *Data, int Size, int Rg);		// Функция загрузки данных в быстром режиме

	// Data - массив данных
	// Size - размер массива Data в байтах
	// Rg - номер загружаемого регистра
	void UnLoadFast(void *Data, int Size, int Rg);		// Функция выгрузки данных в быстром режиме
	//===============================================
	void PrintLogConsole(bool flag);
private:
	uint32_t HandleHex();

	// Функции вывода сообщений о ходе работы с ПЛИС
	void LogWriteReg(uint32_t reg_code, uint32_t val, int ret);
	void LogReadReg(uint32_t reg_code, uint32_t val, int ret);
	void LogCmdW(uint32_t cmd, int ret);
	void LogCmdR(uint32_t cmd, int ret);
	void LogData(string fun_name, int len, uint32_t *buff, int ret);
	void LogMem(string fun_name, int len, int ret);
};
