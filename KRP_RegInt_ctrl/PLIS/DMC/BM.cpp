#include "BM.h"
#include "../lib_HI_W/HardwareInterface.h"
#include "../../time/time_work.h"
#include <sstream>

#define RIGEL_SUCCESS 0x02200000
#define TAYGETA_SUCCESS 0x02300000
#define TERCIUS_SUCCESS 0x02100000
#define DEFAULT_SUCCESS 0x02000000

template<class W, class R>
R Convert(W val)
{
	stringstream ss;
	ss << val;
	R ret;
	ss >> ret;
	return ret;
}

// Статические члены и методы класса работы с регистровым интерфейсом
void* BM::_Handle = 0;
int BM::_Count = 0;
// Открытие интерфейса ПЛИС
void BM::OPEN()
{
	if(_Handle == 0)
	{
		const int ret = OpenInterface("localhost", &_Handle);
		if(ret != 0)	throw HIError("OpenInterface", ret);
	}
}
// Закрытие интерфейса ПЛИС
void BM::CLOSE()
{
	if(_Handle != 0)
	{
		const int ret = CloseInterface(_Handle);
		_Handle = 0;
		if(ret != 0)	throw HIError("CloseInterface", ret);
	}
}

// Конструктор базового модуля (BM)
BM::BM(PLIS_NAME PlisName, uint32_t Bm, string OutFile, bool Debug) : _Bm(Bm), _DmcMin(-1), _DmcMax(-1), _OutFile(OutFile),
	_BmLog("LogBM[" + Convert<uint32_t, string>(Bm) + "].txt", Debug), _TimeStart(0), _TimeStop(0)
{
	_Count++;
	if(_Count > 1)
		throw BMError("Текущий модуль не позволяет создавать больше одного объекта класса в программе");
	_BmLog << "Handle: " << hex << HandleHex() << dec << "\n";

	switch(PlisName)
	{
	case RIGEL:
		CallStart = &BM::Start;
		CallStop = &BM::Stop;
		CallReset = &BM::ResetByReg;
		CallLoadOutFile = &BM::LoadOutFile32;

		CallLoad = &BM::LoadData32;
		CallUnLoad = &BM::UnLoadData32;

		_Flag = RIGEL_SUCCESS;
		break;

	case TAYGETA:
		CallStart = &BM::StartByReg;
		CallStop = &BM::Stop;
		CallReset = &BM::Reset;
		CallLoadOutFile = &BM::LoadOutFile64;

		CallLoad = &BM::LoadData64;
		CallUnLoad = &BM::UnLoadData64;

		_Flag = TAYGETA_SUCCESS;
		break;

	case TERCIUS:
		CallStart = &BM::StartByReg;
		CallStop = &BM::Stop;
		CallReset = &BM::Reset;
		CallLoadOutFile = &BM::LoadOutFile64;

		CallLoad = &BM::LoadData64;
		CallUnLoad = &BM::UnLoadData64;

		_Flag = TERCIUS_SUCCESS;
		break;

	default:
		CallStart = &BM::Start;
		CallStop = &BM::Stop;
		CallReset = &BM::Reset;
		CallLoadOutFile = &BM::LoadOutFile64;

		CallLoad = &BM::LoadData64;
		CallUnLoad = &BM::UnLoadData64;

		_Flag = DEFAULT_SUCCESS;
	}
}
BM::~BM()	{;}
// Добавление КРП в базовый модуль
void BM::AddDmc(int Dmc_id, int Dmc_LimitShow, bool Dmc_debug)
{
	if(_DMC.size() == 0)
	{
		_DmcMin = Dmc_id;
		_DmcMax = Dmc_id;
	}
	else
	{
		_DmcMin = min(_DmcMin, Dmc_id);
		_DmcMax = max(_DmcMax, Dmc_id);
	}

	_DMC.push_back( DMC(_Handle, _Bm, Dmc_id, Dmc_LimitShow, Dmc_debug) );
}
// Поиск КРП по его id
int BM::Find(int dmc_id)
{
	int ret = -1;

	int i=0;
	const size_t size = _DMC.size();
	while(static_cast<size_t>(i) < size)
	{
		int ind = _DMC[i].Num();
		if(ind == dmc_id)
		{
			ret = i;
			break;
		}
		i++;
	}

	return ret;
}
// Загрузка/выгрузка данных
void BM::LoadData32(int Dmc_id, void *Data, int Size, uint32_t Dmc_addr)
{
	_BmLog << "Загрузка данных в КРП[" << Dmc_id << "] 32 бит\n";

	const int ind = Find(Dmc_id);
	_DMC[ind].LoadData32(Data, Size, Dmc_addr);
}

void BM::UnLoadData32(int Dmc_id, void *Data, int Size, uint32_t Dmc_addr)
{
	_BmLog << "Выгрузка данных из КРП[" << Dmc_id << "] 32 бит\n";

	const int ind = Find(Dmc_id);
	_DMC[ind].UnLoadData32(Data, Size, Dmc_addr);
}

void BM::LoadData64(int Dmc_id, void *Data, int Size, uint32_t Dmc_addr)
{
	_BmLog << "Загрузка данных в КРП[" << Dmc_id << "] 64 бит\n";

	const int ind = Find(Dmc_id);
	_DMC[ind].LoadData64(Data, Size, Dmc_addr);
}

void BM::UnLoadData64(int Dmc_id, void *Data, int Size, uint32_t Dmc_addr)
{
	_BmLog << "Выгрузка данных из КРП[" << Dmc_id << "] 64 бит\n";

	const int ind = Find(Dmc_id);
	_DMC[ind].UnLoadData64(Data, Size, Dmc_addr);
}

void BM::LoadData(int Dmc_id, void *Data, int Size, uint32_t Dmc_addr)
{
	(this->*CallLoad)(Dmc_id, Data, Size, Dmc_addr);
}

void BM::UnLoadData(int Dmc_id, void *Data, int Size, uint32_t Dmc_addr)
{
	(this->*CallUnLoad)(Dmc_id, Data, Size, Dmc_addr);
}
//====================================================================
// Функции работы с базовым модулем
void BM::LoadOutFile64()
{
	int ret = LoadFile64Fast(_Handle, _Bm, _OutFile.c_str());
	LogLoadOut("LoadFile64Fast", _OutFile, ret);
	if(ret != brSuccess)
		throw HIError("LoadFile64Fast", ret);
}

void BM::LoadOutFile32()
{
	int ret = LoadFileFast(_Handle, _Bm, _OutFile.c_str());
	LogLoadOut("LoadFileFast", _OutFile, ret);
	if(ret != brSuccess)
		throw HIError("LoadFileFast", ret);
}

void BM::LoadOutFile() { (this->*CallLoadOutFile)(); }

void BM::Calibrate()
{
	_BmLog << "BM[" << _Bm << "] Калибровка:\n";
	(this->*CallReset)();

	pause_ms(1);

	for(int i=_DmcMin; i <= _DmcMax; i++)
	{
		const int ind = Find(i);
		if(ind >= 0)
			_DMC[ind].WriteReg(0, 0x40);
	}
	pause_ms(2000);

	for(int i=_DmcMin; i <= _DmcMax; i++)
	{
		const int ind = Find(i);
		if(ind < 0)	continue;

		const uint64_t hw_word = _DMC[ind].ReadReg(0);
		if( hw_word != _Flag )
		{
			_BmLog << "КРП[" << _DMC[ind].Num() << "]: " << CL_RED << "провал\n";
			throw CalibrateError(hw_word);
		}
		_BmLog << "КРП[" << _DMC[ind].Num() << "]: " << CL_GREEN << "успех\n";
	}

	_BmLog << "Загрузка " << _OutFile << " файла\n";
	(this->*CallLoadOutFile)();
}

void BM::Prep()
{
	for(uint32_t i=0; i < _DMC.size(); i++)
		_DMC[i].WriteReg(4, 0);

	int ind = Find(_DmcMin);
	_DMC[ind].WriteReg(0, 4);

	ind = Find(_DmcMax);
	_DMC[ind].WriteReg(0, 9);
}

void BM::Wait(uint32_t Time)
{
	unsigned long time_run = 0;
	int ret = WaitEndStatus(_Handle, _Bm, &time_run, Time);
	_TimeStop = get_time();
	LogWaitBm(time_run, Time, ret);
	if(ret != 0)
	{
		if(time_run > Time)
		{
			_BmLog << CL_RED << "Аварийная остановка\n";
			(this->*CallReset)();
			pause_ms(1);

			(this->*CallStop)();
			(this->*CallReset)();
		}

		throw HIError("WaitEndStatus", ret);
	}
}

void BM::Run(uint32_t TimeWork)
{
	_BmLog << "Предварительная настройка линейки КРП перед запуском\n";
	Prep();

	(this->*CallReset)();
	pause_ms(1);

	_BmLog << "Запуск BM[" << _Bm << "]\n";
	(this->*CallStart)();
	Wait(TimeWork);
	_BmLog << CL_YELLOW << "Время работы: " << _TimeStop - _TimeStart << " сек\n";
}

void BM::Start()
{
	_TimeStart = get_time();
	int ret = RunDMC(_Handle, _Bm);
	LogWorkBm("RunDMC", ret);
	if(ret != 0)
		throw HIError("RunDMC", ret);
}

void BM::Stop()
{
	int ret = StopDMC(_Handle, _Bm);
	_TimeStop = get_time();
	LogWorkBm("StopDMC", ret);
	if(ret != 0)
		throw HIError("StopDMC", ret);
}

void BM::Reset()
{
	int ret = GlobalReset(_Handle, _Bm);
	LogWorkBm("GlobalReset", ret);
	if(ret != 0)
		throw HIError("GlobalReset", ret);
}

void BM::StartByReg()
{
	_TimeStart = get_time();
	int ret = WriteCOReg(_Handle, _Bm, 2, 2);
	LogCOReg("Старт:", "WriteCOReg", 2, 2, ret);
	if(ret != 0)
		throw HIError("WriteCOReg", ret);
}

void BM::ResetByReg()
{
	int ret = WriteCOReg(_Handle, _Bm, 2, 3);
	LogCOReg("Сброс:", "WriteCOReg", 2, 3, ret);
	if(ret != 0)
		throw HIError("WriteCOReg", ret);
}

void BM::StopByReg()
{
	int ret = WriteCOReg(_Handle, _Bm, 2, 0);
	_TimeStop = get_time();
	LogCOReg("Стоп:", "WriteCOReg", 2, 0, ret);
	if(ret != 0)
		throw HIError("WriteCOReg", ret);
}
//====================================================================
// Cкважность
//====================================================================
void BM::SetSkv(int Dmc_id, uint16_t LenSkv, uint16_t PauseSkv)
{
	const int ind = Find(Dmc_id);
	_DMC[ind].WriteReg(5, BildSkv(LenSkv, PauseSkv));
}

uint32_t BM::BildSkv(uint16_t Len, uint16_t Pause)
{
	const uint32_t flag = 1<<31;
	const uint32_t len  = (Len - 1) & (0x1ff);				//(0:9)   биты
	uint32_t period = (Pause - 1) & (0x1ff);				//(16:25) биты
	period = period << 16;
	return (flag | len | period);
}
//====================================================================
void BM::LogWorkBm(string FunName, int Ret)
{
	_BmLog << CL_CYAN << FunName << "(" << hex << HandleHex() << ", " << _Bm << ") -> " << dec;

	if(Ret != 0)	_BmLog << CL_RED << Ret << "\n";
	else			_BmLog << CL_GREEN << Ret << "\n";
}

void BM::LogWaitBm(uint32_t Time, uint32_t TimeOut, int Ret)
{
	_BmLog << CL_CYAN << "WaitEndStatus" << "(" << hex << HandleHex() << dec << ", " << _Bm << ", " << Time << ", " << TimeOut << ") -> ";

	if(Ret != 0)	_BmLog << CL_RED << Ret << "\n";
	else			_BmLog << CL_GREEN << Ret << "\n";
}

void BM::LogLoadOut(string FunName, string FileName, int Ret)
{
	_BmLog << CL_CYAN << FunName << "(" << hex << HandleHex() << ", " << _Bm << ", " << FileName << ") -> " << dec;

	if(Ret != 0)	_BmLog << CL_RED << Ret << "\n";
	else			_BmLog << CL_GREEN << Ret << "\n";
}

void BM::LogCOCmd(string FunName, uint32_t Val, int Ret)
{
	_BmLog << CL_CYAN << FunName << "(" << hex << HandleHex() << dec << ", " << _Bm << ", " << Val << ") -> ";

	if(Ret != 0)	_BmLog << CL_RED << Ret << "\n";
	else			_BmLog << CL_GREEN << Ret << "\n";
}

void BM::LogCOReg(string FlagWork, string FunName, uint32_t RegCode, uint32_t Val, int Ret)
{
	_BmLog << FlagWork << " " << CL_CYAN << FunName << "(" << hex << HandleHex() << dec << ", " << _Bm << ", " << RegCode << ", " << Val << ") -> ";

	if(Ret != 0)	_BmLog << CL_RED << Ret << "\n";
	else			_BmLog << CL_GREEN << Ret << "\n";
}
//====================================================================
uint32_t BM::HandleHex()		{	return (_Handle == 0) ? (0) : *((uint32_t*)_Handle);	}
