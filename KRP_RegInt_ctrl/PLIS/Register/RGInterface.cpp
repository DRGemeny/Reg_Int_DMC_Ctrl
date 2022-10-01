#include <cstring>
#include "RGInterface.h"
#include "../lib_HI_W/HardwareInterface.h"
#include "../../time/time_work.h"

// Определения констант для работы с регистровым интерфейсом ПЛИС
#define WR5(n)		0x61050000|(n<<20)		// Внутренний счетчик, задание времени работы в тактах
#define WR6(n)		0x61060000|(n<<20)		// Задание скважности

#define RR3(n)		0xA1030000|(n<<20)		// Чтение значения внутреннего счетчика
#define RS8(n)		0xA1080000|(n<<20)		// Регистр статуса

#define START(n)	0x21000000|(n<<20)		// Старт
#define STOP(n)		0x22000000|(n<<20)		// Стоп
#define RESET(n)	0x23000000|(n<<20)		// Сброс

#define LOAD(n,m)	0x61000000|(n<<20)|(m<<16)	// Загрузка данных
#define UNLOAD(n,m)	0xA1000000|(n<<20)|(m<<16)	// Выгрузка данных

#define LOAD_FAST(n,m)		0x63000000|(n<<20)|(m<<16)
#define UNLOAD_FAST(n,m,k)	0xA3000000|(n<<20)|(m<<16)|(k-1)

// Класс-исключение возвращает ошибку вызова системной функции
HIError::HIError(string fun_name, int msg) : FunName(fun_name), Msg(msg) {;}
string HIError::GetFun()	{ return FunName; }
int HIError::GetMsg()		{ return Msg; }

// Статические члены и методы класса работы с регистровым интерфейсом
void* WorkRG::Handle = 0;
// Открытие интерфейса ПЛИС
void WorkRG::OPEN()
{
	if(Handle == 0)
	{
		const int ret = OpenInterface("localhost", &Handle);
		if(ret != 0)	throw HIError("OpenInterface", ret);
	}
}
// Закрытие интерфейса ПЛИС
void WorkRG::CLOSE()
{
	if(Handle != 0)
	{
		const int ret = CloseInterface(Handle);
		Handle = 0;
		if(ret != 0)	throw HIError("CloseInterface", ret);
	}
}
// Запуск всех конвееров
void WorkRG::PUSK_ALL(int Bm)
{
	uint32_t PuskCommadn = START(0) & 0x0fffffff;
	int ret = WriteCOCmd(Handle, Bm, PuskCommadn);
	if(ret != 0)
		throw HIError("PUSK_ALL: WriteCOCmd", ret);
}
// Остановка всех конвееров
void WorkRG::STOP_ALL(int Bm)
{
	uint32_t PuskCommadn = STOP(0) & 0x0fffffff;
	int ret = WriteCOCmd(Handle, Bm, PuskCommadn);
	if(ret != 0)
		throw HIError("STOP_ALL: WriteCOCmd", ret);
}
// Сброс всех конвееров
void WorkRG::RESET_ALL(int Bm)
{
	uint32_t PuskCommadn = RESET(0) & 0x0fffffff;
	int ret = WriteCOCmd(Handle, Bm, PuskCommadn);
	if(ret != 0)
		throw HIError("RESET_ALL: WriteCOCmd", ret);
}
//=======================================================================
// Конструктор бъекта класса для работы с регистровым интерфейсом
WorkRG::WorkRG(int bm, int num, string log_name, bool debug):	BM(bm), Num(num), Log(log_name, debug)	{;}
WorkRG::WorkRG(const WorkRG &Rg): BM(Rg.BM), Num(Rg.Num), Log(Rg.Log) {;}
WorkRG WorkRG::operator=(const WorkRG &Rg)
{
	if(&Rg == this)	return *this;
	BM = Rg.BM;
	Num = Rg.Num;
	Log = Rg.Log;

	return *this;
}

WorkRG::~WorkRG()	{ ; }
//=======================================================================
// Устанавливает количество тактов работы ПЛИС
void WorkRG::SetTact(uint64_t tact)
{
	tact -= 3;
	uint32_t *t = (uint32_t*)&tact;

	uint32_t WriteTact = WR5(Num);

	int ret = WriteCOCmd(Handle, BM, WriteTact);
	LogCmdW(WriteTact, ret);

	if(ret != 0)
		throw HIError("WriteCOCmd", ret);

	//===================================================
	ret = WriteBufferCODataReg(Handle, BM, 1, t);
	LogData("WriteBufferCODataReg", 1, t, ret);
	if(ret != 0)
		throw HIError("WriteBufferCODataReg", ret);

	ret = WriteBufferCODataReg(Handle, BM, 1, t+1);
	LogData("WriteBufferCODataReg", 1, t+1, ret);
	if(ret != 0)
		throw HIError("WriteBufferCODataReg", ret);
}

// Устанавливает режим передачи данных со скважностью
void WorkRG::SetSkv(uint16_t len, uint16_t T)
{
	len -= 1;
	T -= 1;

	uint32_t Skv = T;
	Skv = (Skv << 16) | len;

	uint32_t SkvCommand = WR6(Num);

	int ret = WriteCOCmd(Handle, BM, SkvCommand);
	LogCmdW(SkvCommand,ret);

	if(ret != 0)
		throw HIError("WriteCOCmd", ret);

	ret = WriteBufferCODataReg(Handle, BM, 1, &Skv);
	LogData("WriteBufferCODataReg", 1, &Skv, ret);
	if(ret != 0)
		throw HIError("WriteBufferCODataReg", ret);
}

// Запрос на получение количества тактов
uint64_t WorkRG::GetTact()
{
	uint64_t Tact = 0;
	uint32_t *read = (uint32_t*)&Tact;

	uint32_t ReadTact = RR3(Num);

	int ret = WriteCOCmd(Handle, BM, ReadTact);
	LogCmdW(ReadTact, ret);
	if(ret != 0)
		throw HIError("WriteCOCmd", ret);

	//====================================================
	ret = ReadBufferCODataReg(Handle,BM,1, read);
	LogData("ReadBufferCODataReg", 1, read, ret);
	if(ret != 0)
		throw HIError("ReadBufferCODataReg", ret);

	ret = ReadBufferCODataReg(Handle,BM,1, read+1);
	LogData("ReadBufferCODataReg", 1, read+1, ret);
	if(ret != 0)
		throw HIError("ReadBufferCODataReg", ret);

	return Tact;
}

// Запрос на получение статуса состояния ПЛИС
void WorkRG::GetStatus(uint32_t *Status, uint32_t read_word)
{
	read_word = ((read_word <= 4) && (read_word > 0)) ? (read_word) : (4);
	uint32_t ReadStatus = RS8(Num);

	int ret = WriteCOCmd(Handle, BM, ReadStatus);
	LogCmdW(ReadStatus, ret);
	if(ret != 0)
		throw HIError("WriteCOCmd", ret);

	//===================================================
	for(int i=read_word; i>0; i--)
	{
		const int ind = read_word - i;
		ret = ReadBufferCODataReg(Handle,BM,1, Status+ind);
		LogData("ReadBufferCODataReg", 1, Status+ind, ret);
		if(ret != 0)
			throw HIError("ReadBufferCODataReg", ret);
	}
}
//=======================================================================
// Основная функция загрузки данных в ПЛИС в медленном режиме
void WorkRG::Load(void *Data, int Size, int Rg)
{
	const int Delim = sizeof(uint32_t);
	const int Count = Size / Delim;
	const int Lost = (Size % Delim) != 0;
	const int N = Count + Lost;

	uint32_t *Buff = new uint32_t[N];
	uint32_t *LoadData = (uint32_t*)Data;

	if(Lost != 0)
	{
		memset(Buff, 0, N * Delim);
		memmove(Buff, Data, Size * Delim);
		LoadData = Buff;
	}
	//===========================================
	uint32_t LoadCommadn = LOAD(Num, Rg);

	int ret = WriteCOCmd(Handle, BM, LoadCommadn);
	LogCmdW(LoadCommadn,ret);
	if(ret != 0)
	{
		delete[] Buff;
		throw HIError("WriteCOCmd", ret);
	}

	ret = WriteBufferCODataReg(Handle, BM, N, LoadData);
	LogData("WriteBufferCODataReg", N, LoadData, ret);

	delete[] Buff;
	if(ret != 0)
		throw HIError("WriteBufferCODataReg", ret);
}
// Основная функция выгрузки данных из ПЛИС в медленном режиме
void WorkRG::UnLoad(void *Data, int Size, int Rg)
{
	const int Delim = sizeof(uint32_t);
	const int Count = Size / Delim;
	const int Lost = (Size % Delim) != 0;
	const int N = Count + Lost;

	uint32_t *Buff = new uint32_t[N];
	uint32_t *UnLoadData = (uint32_t*)Data;

	if(Lost != 0)
	{
		memset(Buff, 0, N * Delim);
		UnLoadData = Buff;
	}
	//===========================================
	uint32_t UnLoadCommadn = UNLOAD(Num, Rg);

	int ret = WriteCOCmd(Handle, BM, UnLoadCommadn);
	LogCmdW(UnLoadCommadn, ret);
	if(ret != 0)
	{
		delete[] Buff;
		throw HIError("WriteCOCmd", ret);
	}

	ret = ReadBufferCODataReg(Handle, BM, N, UnLoadData);
	LogData("ReadBufferCODataReg", N, UnLoadData, ret);
	if(Lost != 0)
		memmove(Data, Buff, Size * Delim);

	delete[] Buff;
	if(ret != 0)
		throw HIError("ReadBufferCODataReg", ret);
}
// Основная функция загрузки данных в ПЛИС в быстром режиме
void WorkRG::LoadFast(void *Data, int Size, int Rg)
{
	const int Delim = sizeof(uint64_t);
	const int Count = Size / Delim;
	const int Lost = (Size % Delim) != 0;
	const int N = Count + Lost;
	//===========================================
	uint32_t LoadCommadn = LOAD_FAST(Num, Rg);
	int ret = WriteCOCmd(Handle, BM, LoadCommadn);
	LogCmdW(LoadCommadn, ret);
	if(ret != 0)
		throw HIError("WriteCOCmd", ret);

	uint64_t *buff;
	ret = SpecBuffAlloc(Handle, N, &buff);
	LogMem("SpecBuffAlloc", N, ret);
	if(ret != 0)
		throw HIError("SpecBuffAlloc", ret);

	memset(buff, 0, N*Delim);
	memmove(buff, Data, Size*Delim);

	uint32_t *buff32 = (uint32_t*)buff;
	const int size32 = N * 2;
	ret = WriteCOMem32(Handle, BM, size32, buff32);
	LogData("WriteCOMem32", size32, buff32, ret);
	if(ret != 0)
		throw HIError("WriteCOMem32", ret);

	ret = SpecBuffFree(Handle, N, buff);
	LogMem("SpecBuffFree", N, ret);
	if(ret != 0)
		throw HIError("SpecBuffFree", ret);
}
// Основная функция выгрузки данных из ПЛИС в быстром режиме
void WorkRG::UnLoadFast(void *Data, int Size, int Rg)
{
	const int Delim = sizeof(uint64_t);
	const int Count = Size / Delim;
	const int Lost = (Size % Delim) != 0;
	const int N = Count + Lost;
	//===========================================
	const int size32 = N * 2;
	uint32_t UnLoadCommadn = UNLOAD_FAST(Num, Rg, size32);
	int ret = WriteCOCmd(Handle, BM, UnLoadCommadn);
	LogCmdW(UnLoadCommadn, ret);
	if(ret != 0)
		throw HIError("WriteCOCmd", ret);

	uint64_t *buff;
	ret = SpecBuffAlloc(Handle, N, &buff);
	LogMem("SpecBuffAlloc", N, ret);
	if(ret != 0)
		throw HIError("SpecBuffAlloc", ret);
	memset(buff, 0, N*Delim);

	uint32_t *buff32 = (uint32_t*)buff;
	ret = ReadCOMem32(Handle, BM, size32, buff32);
	LogData("ReadCOMem32", size32, buff32, ret);
	memmove(Data, buff, Size*Delim);

	if(ret != 0)
	{
		int retM = SpecBuffFree(Handle, N, buff);
		LogMem("SpecBuffFree", N, retM);
		throw HIError("ReadCOMem32", ret);
	}

	ret = SpecBuffFree(Handle, N, buff);
	LogMem("SpecBuffFree", N, ret);
	if(ret != 0)
		throw HIError("SpecBuffFree", ret);
}
//=======================================================================
// Чтение регистра команд ПЛИС
uint32_t WorkRG::GetCmdReg()
{
	uint32_t val(0);

	int ret = ReadCOCmd(Handle, BM, &val);
	LogCmdR(val, ret);
	if(ret != 0)
		throw HIError("ReadCOCmd", ret);

	return val;
}
// Запуск ПЛИС
void WorkRG::Pusk()
{
	uint32_t PuskCommadn = START(Num);
	int ret = WriteCOCmd(Handle, BM, PuskCommadn);
	LogCmdW(PuskCommadn, ret);
	if(ret != 0)
		throw HIError("WriteCOCmd", ret);
}
// Остановка ПЛИС
void WorkRG::Stop()
{
	uint32_t StopCommadn = STOP(Num);
	int ret = WriteCOCmd(Handle, BM, StopCommadn);
	LogCmdW(StopCommadn, ret);
	if(ret != 0)
		throw HIError("WriteCOCmd", ret);
}
// Сброс ПЛИС
void WorkRG::Reset()
{
	uint32_t ResetCommadn = RESET(Num);
	int ret = WriteCOCmd(Handle, BM, ResetCommadn);
	LogCmdW(ResetCommadn, ret);
	if(ret != 0)
		throw HIError("WriteCOCmd", ret);
}
// Вообще сброс ПЛИС (применять, только если очень надо)
void WorkRG::ResetLVDS()
{
    uint32_t reg4=0;
    int ret = ReadCOReg(Handle, BM, 4, &reg4);
	LogReadReg(4, reg4, ret);
	if(ret != 0)
		throw HIError("ReadCOReg", ret);

	ret = WriteCOReg(Handle, BM, 4, 0xb0|reg4);
	LogWriteReg(4, 0xb0|reg4, ret);
	if(ret != 0)
		throw HIError("WriteCOReg", ret);

	pause_ms(10);
    ret = WriteCOReg(Handle, BM, 4, reg4);
	LogWriteReg(4, reg4, ret);
    if(ret != 0)
		throw HIError("WriteCOReg", ret);
}
//=======================================================================
// Функции вывода сообщений работы системных функций в консоль и лог. файл
void WorkRG::LogWriteReg(uint32_t reg_code, uint32_t val, int ret)
{
	Log << CL_CYAN << "WriteCOReg" << "(" << hex << HandleHex() << ", " << BM << ", " << reg_code << ", " << val << ") -> " << dec;

	if(ret != 0)	Log << CL_RED << ret << "\n";
	else			Log << CL_GREEN << ret << "\n";
}

void WorkRG::LogReadReg(uint32_t reg_code, uint32_t val, int ret)
{
	Log << CL_CYAN << "ReadCOReg" << "(" << hex << HandleHex() << ", " << BM << ", " << reg_code << ", " << val << ") -> " << dec;

	if(ret != 0)	Log << CL_RED << ret << "\n";
	else			Log << CL_GREEN << ret << "\n";
}

void WorkRG::LogCmdW(uint32_t cmd, int ret)
{
	Log << CL_CYAN << "WriteCOCmd" << "(" << hex << HandleHex() << ", " << BM << ", " << cmd << ") -> " << dec;

	if(ret != 0)	Log << CL_RED << ret << "\n";
	else			Log << CL_GREEN << ret << "\n";
}

void WorkRG::LogCmdR(uint32_t cmd, int ret)
{
	Log << CL_CYAN << "ReadCOCmd" << "(" << hex << HandleHex() << ", " << BM << ", " << cmd << ") -> " << dec;

	if(ret != 0)	Log << CL_RED << ret << "\n";
	else			Log << CL_GREEN << ret << "\n";
}

void WorkRG::LogData(string fun_name, int len, uint32_t *buff, int ret)
{
	Log << CL_CYAN << fun_name << "(" << hex << HandleHex() << ", " << BM << ", " << len << ", buff) -> " << dec;

	if(ret != 0)	Log << CL_RED << ret << "\n";
	else			Log << CL_GREEN << ret << "\n";

	bool fw = Log.GetWriteConsole();
	Log.SetWriteConsole(false);

	for(int i=0; i<len; i++)
		Log << "buff[" << i << "] = " << hex << buff[i] << dec << "\n";
	Log << "==================================================\n";

	Log.SetWriteConsole(fw);
}

void WorkRG::LogMem(string fun_name, int len, int ret)
{
	Log << CL_CYAN << fun_name << "(" << hex << HandleHex() << ", " << len << ", buff) -> " << dec;

	if(ret != 0)	Log << CL_RED << ret << "\n";
	else			Log << CL_GREEN << ret << "\n";
}

//=======================================================================
uint32_t WorkRG::HandleHex()			{	return (Handle == 0) ? (0) : *((uint32_t*)Handle);	}
void WorkRG::PrintLogConsole(bool flag)	{	Log.SetWriteConsole(flag);		}
//=======================================================================
