#pragma once
#include <stdint.h>
#include <string>
#include "../../log/Log.h"

using namespace std;

class DMC;
class HIError;

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

class DMC
{
private:
	void *_Handle;				// дескриптор интерфейса
	uint32_t _Bm;				// номер базового модуля
	uint32_t _Dmc;				// номер КРП в БМ

	int16_t scOperSeg;			// 0x0020 сегмент операторов
	int16_t scParamSeg;			// 0x0021 сегмент параметров
	int16_t scDataSeg;			// 0x0022 сегмент данных
	int16_t scCycleSeg;			// 0x0023 сегмент циклов

	bool _EnableLimitShow;		// Включение вывода части массива
	int _LimitShow;		// Задание части массива для вывода
	CLog _DmcLog;

public:
	DMC(void *Handle, uint32_t Bm, uint32_t Dmc, int32_t LimitShow = -1, bool debug=true);
	DMC(const DMC &obj);
	DMC &operator=(DMC &obj);

	uint32_t Num();				// Возвращает номер КРП
	// Возвращает код сегмента операторов
	int16_t OperSeg();
	int16_t ParamSeg();
	int16_t DataSeg();
	int16_t CycleSeg();

	// Работа с регистрами КРП
	void WriteReg(uint32_t RegCode, uint64_t Value);
	uint64_t ReadReg(uint32_t RegCode);
	//========================
	// Загрузка/выгрузка данных

	// Data - массив загружаемых данных
	// Size - размер массива Data в байтах
	// Addr - адрес записи
	void LoadData32(void *Data, int Size, uint32_t Addr);	// Загрузка данных в КРП 32

	// Data - массив загружаемых данных
	// Size - размер массива Data в байтах
	// Addr - адрес записи
	void UnLoadData32(void *Data, int Size, uint32_t Addr);	// Загрузка данных из КРП 32

	// Data - массив выгружаемых данных
	// Size - размер массива Data в байтах
	// Addr - адрес чтения
	void LoadData64(void *Data, int Size, uint32_t Addr);	// Загрузка данных в КРП 64

	// Data - массив выгружаемых данных
	// Size - размер массива Data в байтах
	// Addr - адрес чтения
	void UnLoadData64(void *Data, int Size, uint32_t Addr);	// Загрузка данных из КРП 64
	//========================

private:
	void LogWorkReg(string FunName, uint32_t RegCode, uint64_t Value, int Ret);
	void LogData(string FunName, int N, uint32_t *Data, int Ret);
	void LogData(string FunName, int N, uint64_t *Data, int Ret);
	void LogMem(string fun_name, int len, int ret);

	uint32_t HandleHex();
};
