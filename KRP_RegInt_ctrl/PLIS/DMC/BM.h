#pragma once

#include <vector>
#include <string>
#include <stdint.h>

#include "DMC.h"

using namespace std;

enum PLIS_NAME
{
	RIGEL = 1,
	TAYGETA,
	TERCIUS
};

class BMError
{
private:
	string _Msg;
public:
	BMError(string msg) : _Msg(msg) {;}
	string Get() {return _Msg;}
};

class CalibrateError
{
private:
	uint64_t val;

public:
	CalibrateError() : val(0) {;}
	CalibrateError(uint64_t num) : val(num) {;}
	uint64_t Get() {return val;}
};

class BM
{
private:
	static void *_Handle;		// дескриптор интерфейса
	static int _Count;			// Количество объектов класса BM
	int _Bm;					// номер базового модуля

	vector<DMC> _DMC;			// массив КРП в базовом модуле
	int _DmcMin;
	int _DmcMax;

	string _OutFile;			// Out файл для базового модуля
	CLog _BmLog;				// Лог файл, для документирование всех действий с данным BM

	uint64_t _Flag;				// Флаг проверки калибровки КРП

	double _TimeStart;			// Время начала работы BM
	double _TimeStop;			// Время окончания работы BM

	typedef void(BM::*Call)();
	Call CallReset;
	Call CallStart;
	Call CallStop;
	Call CallLoadOutFile;

	typedef void(BM::*WriteRead)(int, void*, int, uint32_t);
	WriteRead CallLoad;
	WriteRead CallUnLoad;

public:
	BM(PLIS_NAME PlisName, uint32_t Bm, string OutFile, bool Debug=false);		// Инициализация Базового модуля.
	~BM();

	static void OPEN();										// Открытие интерфейса
	static void CLOSE();									// Закрытие интерфейса

	void AddDmc(int Dmc_id, int Dmc_Limit_Show=-1, bool Dmc_debug=false);

	void LoadData(int Dmc_id, void *Data, int Size, uint32_t Dmc_addr);
	void UnLoadData(int Dmc_id, void *Data, int Size, uint32_t Dmc_addr);

	// Функции для работы с КРП
	void LoadOutFile();						// Принудительная загрузка *.out файла
	void Calibrate();						// Калибровка КРП
	void Run(uint32_t TimeWork);			// запуск
	//===========================
	void SetSkv(int Dmc_id, uint16_t LenSkv, uint16_t PauseSkv);	// установка скважности КРП

private:
	int Find(int Dmc_id);

	uint32_t BildSkv(uint16_t Len, uint16_t Pause);

	void LogWorkBm(string FunName, int Ret);
	void LogWaitBm(uint32_t Time, uint32_t TimeOut, int Ret);
	void LogLoadOut(string FunName, string FileName, int Ret);
	void LogCOCmd(string FunName, uint32_t Val, int Ret);
	void LogCOReg(string FlagWork, string FunName, uint32_t RegCode, uint32_t Val, int Ret);

	// Функции работы с КРП
	void LoadOutFile32();		// Загрузка Out файла
	void LoadOutFile64();

	// Загрузка/выгрузка данных
	void LoadData32(int Dmc_id, void *Data, int Size, uint32_t Dmc_addr);
	void UnLoadData32(int Dmc_id, void *Data, int Size, uint32_t Dmc_addr);

	void LoadData64(int Dmc_id, void *Data, int Size, uint32_t Dmc_addr);
	void UnLoadData64(int Dmc_id, void *Data, int Size, uint32_t Dmc_addr);
	//==========================

	void Prep();				// Подготовка к запуску
	void Start();				// Запуск БМ
	void Wait(uint32_t Time);	// Ожидание остановки КРП
	void Stop();				// стоп
	void Reset();				// сброс

	void StartByReg();			// старт через WriteCOReg
	void ResetByReg();			// сброс через WriteCOReg
	void StopByReg();			// стоп через WriteCOReg
	//===========================

	uint32_t HandleHex();
};
