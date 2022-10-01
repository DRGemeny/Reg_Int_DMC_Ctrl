#include "DMC.h"
#include "../lib_HI_W/HardwareInterface.h"
#include <sstream>
#include <cstring>


template<class W, class R>
R Convert(W val)
{
	stringstream ss;
	ss << val;
	R ret;
	ss >> ret;
	return ret;
}

// Класс-исключение возвращает ошибку вызова системной функции
HIError::HIError(string fun_name, int msg) : FunName(fun_name), Msg(msg) {;}
string HIError::GetFun()	{ return FunName; }
int HIError::GetMsg()		{ return Msg; }

// Конструктор ===========================================================================
DMC::DMC(void *Handle, uint32_t Bm, uint32_t Dmc, int32_t LimitShow, bool debug) :
	_Handle(Handle), _Bm(Bm), _Dmc(Dmc),
	scOperSeg(0x0020), scParamSeg(0x0021), scDataSeg(0x0022), scCycleSeg(0x0023),
	_EnableLimitShow(LimitShow > 0), _LimitShow(LimitShow),
	_DmcLog("LogBM[" + Convert<uint32_t, string>(Bm) + "]DMC[" + Convert<uint32_t, string>(Dmc) + "].txt", debug, true)
{;}
DMC::DMC(const DMC &obj): _Handle(obj._Handle), _Bm(obj._Bm), _Dmc(obj._Dmc),
	scOperSeg(obj.scOperSeg), scParamSeg(obj.scParamSeg), scDataSeg(obj.scDataSeg), scCycleSeg(obj.scCycleSeg),
	_EnableLimitShow(obj._EnableLimitShow), _LimitShow(obj._LimitShow),
	_DmcLog(obj._DmcLog)
{;}
DMC &DMC::operator=(DMC &obj)
{
	if(this != &obj)
	{
		_Handle = obj._Handle;
		_Bm = obj._Bm;
		_Dmc = obj._Dmc;

		scOperSeg = obj.scOperSeg;
		scParamSeg = obj.scParamSeg;
		scDataSeg = obj.scDataSeg;
		scCycleSeg = obj.scCycleSeg;

		_EnableLimitShow = obj._EnableLimitShow;
		_LimitShow = obj._LimitShow;
		_DmcLog = obj._DmcLog;
	}
	return *this;
}
//========================================================================================
// Коды сегментов
int16_t DMC::OperSeg()	{ return scOperSeg; }
int16_t DMC::DataSeg()	{ return scDataSeg; }
int16_t DMC::CycleSeg()	{ return scCycleSeg;}
int16_t DMC::ParamSeg()	{ return scParamSeg;}
//========================================================================================
// Работа с регистром КРП
void DMC::WriteReg(uint32_t RegCode, uint64_t Value)
{
	int ret = WriteDMCReg64(_Handle, _Bm, RegCode, _Dmc, Value);
	LogWorkReg("WriteDMCReg64", RegCode, Value, ret);

	if( ret != brSuccess )
		throw HIError("WriteDMCReg64", ret);
}
uint64_t DMC::ReadReg(uint32_t RegCode)
{
	uint64_t Value = 0;
	int ret = ReadDMCReg64(_Handle, _Bm, RegCode, _Dmc, &Value);
	LogWorkReg("ReadDMCReg64", RegCode, Value, ret);

	if( ret != brSuccess )
		throw HIError("ReadDMCReg64", ret);
	return Value;
}
//========================================================================================
// Чтение/запись данных в КРП
void DMC::LoadData64(void *Data, int Size, uint32_t Addr)
{
	const int Delim = sizeof(uint64_t);
	const int Count = Size / Delim;
	const int Lost = (Size % Delim) != 0;
	const int N = Count + Lost;
	//===========================================

	uint64_t *buff;
	int ret = SpecBuffAlloc(_Handle, N, &buff);
	LogMem("SpecBuffAlloc", N, ret);
	if(ret != 0)
		throw HIError("SpecBuffAlloc", ret);

	memset(buff, 0, N*Delim);
	memmove(buff, Data, Size);

	ret = WriteMem64Fast(_Handle, _Bm, scDataSeg, _Dmc, Addr, 1, N, buff);
	LogData("WriteMem64Fast", N, buff, ret);
	if(ret != 0)
		throw HIError("WriteMem64Fast", ret);

	ret = SpecBuffFree(_Handle, N, buff);
	LogMem("SpecBuffFree", N, ret);
	if(ret != 0)
		throw HIError("SpecBuffFree", ret);
}

void DMC::UnLoadData64(void *Data, int Size, uint32_t Addr)
{
	const int Delim = sizeof(uint64_t);
	const int Count = Size / Delim;
	const int Lost = (Size % Delim) != 0;
	const int N = Count + Lost;
	//===========================================

	uint64_t *buff;
	int ret = SpecBuffAlloc(_Handle, N, &buff);
	LogMem("SpecBuffAlloc", N, ret);
	if(ret != 0)
		throw HIError("SpecBuffAlloc", ret);
	memset(buff, 0, N*Delim);

	ret = ReadMem64Fast(_Handle, _Bm, scDataSeg, _Dmc, Addr, 1, N, buff);
	LogData("ReadMem64Fast", N, buff, ret);
	memmove(Data, buff, Size);

	if(ret != 0)
	{
		int retM = SpecBuffFree(_Handle, N, buff);
		LogMem("SpecBuffFree", N, retM);
		throw HIError("ReadMem64Fast", ret);
	}

	ret = SpecBuffFree(_Handle, N, buff);
	LogMem("SpecBuffFree", N, ret);
	if(ret != 0)
		throw HIError("SpecBuffFree", ret);
}

void DMC::LoadData32(void *Data, int Size, uint32_t Addr)
{
	const int Delim = sizeof(uint32_t);
	const int Count = Size / Delim;
	const int LostByte = Size % Delim;
	const int Lost = LostByte != 0;
	const int N = Count + Lost;
	//===========================================

	uint32_t *UserData = (uint32_t*)Data;
	uint64_t *buff;
	int ret = SpecBuffAlloc(_Handle, N, &buff);
	LogMem("SpecBuffAlloc", N, ret);
	if(ret != 0)
		throw HIError("SpecBuffAlloc", ret);

	memset(buff, 0, N*Delim*2);

	for(int i=0; i<Count; i++)
		buff[i] = UserData[i];

	if(Lost != 0)
		memmove(buff+Count, UserData+Count, LostByte);

	ret = WriteMem64Fast(_Handle, _Bm, scDataSeg, _Dmc, Addr, 1, N, buff);
	LogData("WriteMem64Fast", N, buff, ret);
	if(ret != 0)
		throw HIError("WriteMem64Fast", ret);

	ret = SpecBuffFree(_Handle, N, buff);
	LogMem("SpecBuffFree", N, ret);
	if(ret != 0)
		throw HIError("SpecBuffFree", ret);
}

void DMC::UnLoadData32(void *Data, int Size, uint32_t Addr)
{
	const int Delim = sizeof(uint32_t);
	const int Count = Size / Delim;
	const int LostByte = Size % Delim;
	const int Lost = LostByte != 0;
	const int N = Count + Lost;
	//===========================================

	uint32_t *UserData = (uint32_t*)Data;
	uint64_t *buff;
	int ret = SpecBuffAlloc(_Handle, N, &buff);
	LogMem("SpecBuffAlloc", N, ret);
	if(ret != 0)
		throw HIError("SpecBuffAlloc", ret);
	memset(buff, 0, N*Delim*2);

	ret = ReadMem64Fast(_Handle, _Bm, scDataSeg, _Dmc, Addr, 1, N, buff);
	LogData("ReadMem64Fast", N, buff, ret);

	for(int i=0; i<Count; i++)
		UserData[i] = buff[i] & 0xffffffff;

	if(Lost != 0)
		memmove(UserData+Count, buff+Count, LostByte);

	if(ret != 0)
	{
		int retM = SpecBuffFree(_Handle, N, buff);
		LogMem("SpecBuffFree", N, retM);
		throw HIError("ReadMem64Fast", ret);
	}

	ret = SpecBuffFree(_Handle, N, buff);
	LogMem("SpecBuffFree", N, ret);
	if(ret != 0)
		throw HIError("SpecBuffFree", ret);
}
//========================================================================================
void DMC::LogWorkReg(string FunName, uint32_t RegCode, uint64_t Value, int Ret)
{
	_DmcLog << CL_CYAN << FunName << "(" << hex << HandleHex() << ", " << _Bm << ", " << RegCode << ", " << _Dmc << ", " << Value << ") -> " << dec;

	if(Ret != 0)	_DmcLog << CL_RED << Ret << "\n";
	else			_DmcLog << CL_GREEN << Ret << "\n";
}

void DMC::LogData(string FunName, int N, uint32_t *Data, int Ret)
{
	_DmcLog << CL_CYAN << FunName << "(" << hex << HandleHex() << ", " << _Bm << ", " << scDataSeg << dec << ", " << _Dmc
		<< ", 0, 1, " << N << ", buff) -> ";

	if(Ret != 0)	_DmcLog << CL_RED << Ret << "\n";
	else			_DmcLog << CL_GREEN << Ret << "\n";

	bool fw = _DmcLog.GetWriteConsole();
	_DmcLog.SetWriteConsole(false);

	bool ELS = _EnableLimitShow & ((N - 2*_LimitShow) > 0);
	if (ELS == false)
	{
		for (int i = 0; i < N; i++)
			_DmcLog << "buff[" << i << "] = " << hex << Data[i] << dec << "\n";
	}
	else
	{
		for (int i = 0; i < _LimitShow; i++)
			_DmcLog << "buff[" << i << "] = " << hex << Data[i] << dec << "\n";
		_DmcLog << "...\n";
		_DmcLog << "...\n";
		_DmcLog << "...\n";
		for (int i = N - _LimitShow; i < N; i++)
			_DmcLog << "buff[" << i << "] = " << hex << Data[i] << dec << "\n";
	}
	_DmcLog << "==================================================\n";

	_DmcLog.SetWriteConsole(fw);
}

void DMC::LogData(string FunName, int N, uint64_t *Data, int Ret)
{
	_DmcLog << CL_CYAN << FunName << "(" << hex << HandleHex() << ", " << _Bm << ", " << scDataSeg << dec << ", " << _Dmc
		<< ", 0, 1, " << N << ", buff) -> ";

	if(Ret != 0)	_DmcLog << CL_RED << Ret << "\n";
	else			_DmcLog << CL_GREEN << Ret << "\n";

	bool fw = _DmcLog.GetWriteConsole();
	_DmcLog.SetWriteConsole(false);

	bool ELS = _EnableLimitShow & ((N - 2 * _LimitShow) > 0);
	if (ELS == false)
	{
		for (int i = 0; i < N; i++)
			_DmcLog << "buff[" << i << "] = " << hex << Data[i] << dec << "\n";
	}
	else
	{
		for (int i = 0; i < _LimitShow; i++)
			_DmcLog << "buff[" << i << "] = " << hex << Data[i] << dec << "\n";
		_DmcLog << "...\n";
		_DmcLog << "...\n";
		_DmcLog << "...\n";
		for (int i = N - _LimitShow; i < N; i++)
			_DmcLog << "buff[" << i << "] = " << hex << Data[i] << dec << "\n";
	}
	_DmcLog << "==================================================\n";

	_DmcLog.SetWriteConsole(fw);
}
void DMC::LogMem(string fun_name, int len, int ret)
{
	_DmcLog << CL_CYAN << fun_name << "(" << hex << HandleHex() << dec << ", " << _Bm << ", " << len << ", buff) -> ";

	if(ret != 0)	_DmcLog << CL_RED << ret << "\n";
	else			_DmcLog << CL_GREEN << ret << "\n";
}
//========================================================================================
uint32_t DMC::HandleHex()	{	return (_Handle == 0) ? (0) : *((uint32_t*)_Handle);	}
uint32_t DMC::Num()			{	return _Dmc;	}
