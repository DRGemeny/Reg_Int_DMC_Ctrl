#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <stdint.h>

using namespace std;

#ifdef _WIN32
	#include <Windows.h>
#endif

// Цвета текста
#ifdef _WIN32
	enum COLOR_LOG
	{
		C_BLACK=0,		// Черный
		C_BLUE,			// Синий
		C_GREEN,		// Зеленый
		C_CYAN,			// Голубой
		C_RED,			// Красный
		C_VIOLET,		// Лиловый
		C_YELLOW,		// Желтый
		C_WHITE,		// Белый
		C_GREY,			// Серый
		CL_BLUE,		// Светло-синий
		CL_GREEN,		// Светло-зеленый
		CL_CYAN,		// Светло-голубой
		CL_RED,			// Светло-красный
		CL_VIOLET,		// Светло-лиловый
		CL_YELLOW,		// Светло-желтый
		CL_WHITE		// Ярко-белый

	};
#endif

#ifdef linux
	enum COLOR_LOG
	{
		C_OFF = 0,			// Сброс настроек консоли

		C_BLACK = 30,		// Черный
		C_RED,				// Красный
		C_GREEN,			// Зеленый
		C_YELLOW,			// Желтый
		C_BLUE,				// Синий
		C_VIOLET,			// Лиловый
		C_CYAN,				// Голубой
		C_WHITE,			// Белый

		C_GREY = 90,		// Серый
		CL_RED,				// Светло-красный
		CL_GREEN,			// Светло-зеленый
		CL_YELLOW,			// Светло-желтый
		CL_BLUE,			// Светло-синий
		CL_VIOLET,			// Светло-лиловый
		CL_CYAN,			// Светло-голубой
		CL_WHITE,			// Ярко-белый

		CB_BLACK = 256+30,	// Жирный черный
		CB_RED,				// Жирный красный
		CB_GREEN,			// Жирный зеленый
		CB_YELLOW,			// Жирный желтый
		CB_BLUE,			// Жирный синий
		CB_VIOLET,			// Жирный лиловый
		CB_CYAN,			// Жирный голубой
		CB_WHITE,			// Жирный белый

		CB_GREY = 256+90,	// Жирный серый
		CLB_RED,			// Жирный Светло-красный
		CLB_GREEN,			// Жирный Светло-зеленый
		CLB_YELLOW,			// Жирный Светло-желтый
		CLB_BLUE,			// Жирный Светло-синий
		CLB_VIOLET,			// Жирный Светло-лиловый
		CLB_CYAN,			// Жирный Светло-голубой
		CLB_WHITE			// Жирный Ярко-белый

	};
#endif

class CLog
{
private:
	string FileName;				// Имя файла в который пишутся данные
	bool _Active;					// доступ к файлу
	bool wc;						// флаг вывода данных в консоль
	bool _OpenPrm;					// Флаг постоянного открытия файла
	fstream _LogFile;				// Лог файл

	COLOR_LOG _ColorUser;			// Цвет текста при выводе данных на консоль текущего объекта
	COLOR_LOG _ColorTime;			// Цвет текста при выводе данных на консоль текущего объекта в текущий момент

	ios_base::fmtflags F_flags;
	// Статические члены класса - члены класса которые являются общими для всех созданных объектов
	static COLOR_LOG _ColorConsole;		// Цвет текста в консоли (считывается непосредственно из консоли ОДИН РАЗ)
	#ifdef _WIN32
		static HANDLE hstdout;		// Указатель на консоль (только WINDOWS)
	#endif
	#ifdef linux
		static string StrSetColor;
		const static char CharSeparator = ';';
		const static char CharEnd = 'm';
	#endif
public:
	//===================================================
	// Конструктор
	CLog();
	CLog(string file_name);
	CLog(string file_name, bool write_to_console);
	CLog(string file_name, bool write_to_console, bool OpenPrm);
	CLog(const CLog &obj);
	CLog& operator=(const CLog &log);

	// Деструктор
	~CLog();

	// Инициализация и освобождение ресурсов для объектов
	static void INIT();
	static void FREE();
	//===================================================
	// Задание имени лог файла
	void Open(string file_name);
	void Open(string file_name, bool write_to_console);
	void Open(string file_name, bool write_to_console, bool OpenPrm);

	void Color(COLOR_LOG color);								// Установка пользовательского цвета текста в консоли для текущего объекта
	void ResetColor();

	void SetWriteConsole(bool flag);							// Установка флага вывода сообщений в консоль
	bool GetWriteConsole();										// Получение флага вывода сообщений в консоль

	bool Active();												// Провекра записи в текстовый файл

	// Вывод сообщений в лог файл
	template<class T>
	CLog& operator<<(T word)
	{
		if(_Active == false)
			return *this;

		PrintToFile(word);

		if(wc == false)	return *this;

		SetColor(_ColorTime);
		PrintToConsole(word);
		_ColorTime = _ColorUser;
		SetColor(_ColorConsole);

		return *this;
	}

private:
	void InitLog(string file_name, bool write_console, bool OpenPrm);			// Инициализация Лог файла

	// Установка цвета следующего выведенного сообщения
	friend CLog& operator<<(CLog& lf, COLOR_LOG color);			// Задание цвета вывода сообщения
	static void SetColor(COLOR_LOG color);						// Настройка консоли (Установка цвета текста вывода)

	// Вывод в лог файл
	template<class T>
	void PrintToFile(T word)
	{
		if(_OpenPrm == false)
		{
			_LogFile.open(FileName.c_str(), ios_base::app | ios_base::out);
			_LogFile.flags(F_flags);
			_LogFile << word;
			F_flags = _LogFile.flags();
			_LogFile.close();
		}
		else
		{
			_LogFile << word;
			_LogFile.flush();
			F_flags = _LogFile.flags();
		}
	}

	// Вывод в консоль
	template<class T>
	void PrintToConsole(T word)
	{
		ios_base::fmtflags flags_back = cout.flags();
		cout.flags(F_flags);
		cout << word << flush;
		cout.flags(flags_back);
	}
};
//=======================================================
