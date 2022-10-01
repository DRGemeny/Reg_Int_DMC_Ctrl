//#pragma once

#include "Log.h"
#include <sstream>
//===========================================================
// Переменные и методы для работы с цветом текста консоли ===============
#ifdef _WIN32
	HANDLE CLog::hstdout = 0;
#endif
#ifdef linux
	string CLog::StrSetColor("\x1b[");
#endif
	COLOR_LOG CLog::_ColorConsole = C_WHITE;

// Инициализация всех лог объектов
void CLog::INIT()
{
#ifdef _WIN32
	if(hstdout == 0)
	{
		hstdout = GetStdHandle(STD_OUTPUT_HANDLE);						// получение дескриптора настроек консоли

		CONSOLE_SCREEN_BUFFER_INFO con_buff;
		GetConsoleScreenBufferInfo(hstdout, &con_buff);
		_ColorConsole = (COLOR_LOG)(con_buff.wAttributes & 0xf);	// сохранение цвета консоли
	}
#endif

#ifdef linux
	_ColorConsole = C_OFF;
#endif
}
// освобождение ресурсов для лог объектов
void CLog::FREE()
{
#ifdef _WIN32
	if(hstdout != 0)
	{
		CloseHandle(hstdout);
		hstdout = 0;
	}
#endif
}

// Установка цвета текста в консоли для Windows
#ifdef _WIN32
void CLog::SetColor(COLOR_LOG color)
{
	if(hstdout != 0)
		SetConsoleTextAttribute(hstdout, color);
}
#endif

#ifdef linux
#define BOLD_FONT 0x100
#define COLOR_FONT 0xff
void CLog::SetColor(COLOR_LOG color)
{
	const int bold = (BOLD_FONT & color) != 0;
	const int color_use = COLOR_FONT & color;
	stringstream ss;
	ss << StrSetColor	<< bold << CharSeparator
						<< color_use << CharEnd;

	cout << ss.str();
}
#endif
//===========================================================

// Потоковый манипулятор установки цвета текста в консоли
CLog& operator<<(CLog &lf, COLOR_LOG color)			{	lf._ColorTime = color;		return lf;	}
//===========================================================
// Иициализация объекта Логфайл
void CLog::InitLog(string file_name, bool write_console, bool OpenPrm)
{
	_ColorUser = _ColorConsole;
	_ColorTime = _ColorConsole;

	FileName = file_name;
	_LogFile.open(FileName.c_str(), ios::out);

	_Active = _LogFile.is_open();
	if(_Active == 0)
	{
		SetColor(CL_RED);
		cout << "Ошибка: файл " << FileName << " не был создан" << endl;
		SetColor(_ColorConsole);
	}
	wc = write_console;
	F_flags = _LogFile.flags();
	_OpenPrm = OpenPrm;

	if(OpenPrm == false)
		_LogFile.close();
}
//===========================================================
// КОНСТРУКТОРЫ =============================================
CLog::CLog()	{	InitLog("log.txt", true, false);	}

CLog::CLog(string file_name)						{	InitLog(file_name, true, false);	}
CLog::CLog(string file_name, bool write_to_console)	{	InitLog(file_name, write_to_console, false);	}
CLog::CLog(string file_name, bool write_to_console, bool OpenPrm) {	InitLog(file_name, write_to_console, OpenPrm);	}

CLog::CLog(const CLog &obj)
{
	_ColorUser = obj._ColorUser;
	_ColorTime = obj._ColorTime;

	FileName = obj.FileName;
	_Active = obj._Active;
	wc = obj.wc;
	F_flags = obj.F_flags;
	_OpenPrm = obj._OpenPrm;

	if(_OpenPrm == true)
	{
		_LogFile.close();
		_LogFile.open(FileName.c_str(), ios_base::app | ios_base::out);
		_LogFile.flags( F_flags );
		_Active = _LogFile.is_open();
	}
}

CLog& CLog::operator=(const CLog &log)
{
	if(this != &log)
	{
		_ColorUser = log._ColorUser;
		_ColorTime = log._ColorTime;

		FileName = log.FileName;
		_Active = log._Active;
		wc = log.wc;
		F_flags = log.F_flags;
		_OpenPrm = log._OpenPrm;

		if(_OpenPrm == true)
		{
			_LogFile.close();
			_LogFile.open(FileName.c_str(), ios_base::app | ios_base::out);
			_LogFile.flags( F_flags );
			_Active = _LogFile.is_open();
		}
	}

	return *this;
}
//===========================================================
// ДЕСТРУКТОР ================================================
CLog::~CLog()
{
	if(_OpenPrm == true)	_LogFile.close();
}
//===========================================================
// Открытие нового лог файла
void CLog::Open(string file_name)	{	InitLog(file_name, wc, false);	}
void CLog::Open(string file_name, bool write_to_console)	{	InitLog(file_name, write_to_console, false);	}
void CLog::Open(string file_name, bool write_to_console, bool OpenPrm)	{	InitLog(file_name, write_to_console, OpenPrm);	}
//===========================================================
// Установка пользовательского цвета консоли
void CLog::Color(COLOR_LOG color)	{	_ColorUser = color;			_ColorTime = color;	}
void CLog::ResetColor()				{	_ColorUser = _ColorConsole;	_ColorTime = _ColorConsole;}
//===========================================================
// Установка флага вывода в консоль
void CLog::SetWriteConsole(bool flag)	{	wc = flag;	}

// Получение флага вывода в консоль
bool CLog::GetWriteConsole()			{	return wc;	}
//===========================================================
// Проверка записи в текстовый файл
bool CLog::Active()						{	return _Active;	}
