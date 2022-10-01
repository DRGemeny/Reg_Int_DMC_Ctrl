#include <iostream>
#include <string>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <iomanip>

using namespace std;

#include "moduls/read_param/ReadFileIni.h"
#include "moduls/log/Log.h"
#include "moduls/PLIS/DMC/BM.h"
#include "moduls/time/time_work.h"

#include "moduls/Random/Random.h"
#include "moduls/Random/FunFloat.h"

// Тип данных для хранения моделирования [делимое / делитель = результат]
typedef pair<pair<double, double>, double> model_data;

// Структура данных для работы непосредственно с ПЛИС
struct WorkDataPLIS
{
public:
	int Size;
	int SizeByte;

	uint32_t* load_data_al;
	uint32_t* load_data_ah;

	uint32_t* load_data_bl;
	uint32_t* load_data_bh;

	uint32_t* unload_data_l;
	uint32_t* unload_data_h;

	WorkDataPLIS(int N)
	{
		Size = N;
		SizeByte = Size * sizeof(uint32_t);

		load_data_al = new uint32_t[Size];
		load_data_ah = new uint32_t[Size];
		load_data_bl = new uint32_t[Size];
		load_data_bh = new uint32_t[Size];
		unload_data_l = new uint32_t[Size];
		unload_data_h = new uint32_t[Size];

		Clear();
	}

	~WorkDataPLIS()
	{
		delete[] load_data_al;
		delete[] load_data_ah;
		delete[] load_data_bl;
		delete[] load_data_bh;
		delete[] unload_data_l;
		delete[] unload_data_h;
	}

	void Clear()
	{
		memset(load_data_al, 0, SizeByte);
		memset(load_data_ah, 0, SizeByte);
		memset(load_data_bl, 0, SizeByte);
		memset(load_data_bh, 0, SizeByte);
		memset(unload_data_l, 0, SizeByte);
		memset(unload_data_h, 0, SizeByte);
	}
};

model_data ModelDiv(double A, double B)
{
	double A2 = A;
	double B2 = B;
	if (TryValue(A) == DENORM) A2 = ConvertBits(ConvertBits(A) & ConvertBits(DoubleRand::ZeroM()));
	if (TryValue(B) == DENORM) B2 = ConvertBits(ConvertBits(B) & ConvertBits(DoubleRand::ZeroM()));
	if (TryValue(A) == NAN_VAL) A2 = DoubleRand::IndP();
	if (TryValue(B) == NAN_VAL) B2 = DoubleRand::IndP();

	double C = A2 / B2;
	if (TryValue(C) == IND) C = DoubleRand::IndP();
	if (TryValue(C) == DENORM) C = ConvertBits(ConvertBits(C) & ConvertBits(DoubleRand::ZeroM()));

	return model_data(pair<double, double>(A, B), C);
}

vector<model_data> GenTest_1()
{
	vector<double> A;
	vector<model_data> Ret;

	DoubleRand DR;
	A.push_back(DR.RandomNorm());
	A.push_back(DR.RandomNorm());
	A.push_back(GetDeNorm(A[1]));
	A.push_back(GetNan(A[1]));
	A.push_back(DR.InfP());
	A.push_back(DR.InfM());
	A.push_back(DR.ZeroP());
	A.push_back(DR.ZeroM());
	A.push_back(DR.NanP());
	A.push_back(DR.NanM());
	A.push_back(DR.MaxNormFloatP());
	A.push_back(DR.MinNormFloatP());

	Ret.push_back( ModelDiv(A[0], A[1]) );
	for(size_t i=1; i<A.size(); i++)
	for(size_t j=1; j<A.size(); j++)
		Ret.push_back( ModelDiv(A[i], A[j]) );

	return Ret;
}

vector<model_data> GenTest_2(int N)
{
	vector<model_data> Ret;
	if (N <= 0) return Ret;

	DoubleRand DR(true);
	Int32Rand IR(-310, 310, true);

	for (int i = 0; i < N; i++)
	{
		double A = pow(DR.RandomNorm(), IR.Random());
		double B = pow(DR.RandomNorm(), IR.Random());
		Ret.push_back( ModelDiv(A, B) );
	}
	return Ret;
}

vector<model_data> GenDataTest(int N)
{
	vector<model_data> Test1 = GenTest_1();
	vector<model_data> Test2 = GenTest_2(N - static_cast<int>(Test1.size()));
	vector<model_data> Ret(N);
	if(N <= Test1.size())
		copy(Test1.begin(), Test1.begin() + N, Ret.begin());
	else
	{
		copy(Test1.begin(), Test1.end(), Ret.begin());
		copy(Test2.begin(), Test2.end(), Ret.begin() + Test1.size());
	}

	return Ret;
}

void WriteDataForPLIS(vector<model_data>& Data, WorkDataPLIS& DataPlis, int Pos, int Count)
{
	DataPlis.Clear();
	for (int i = 0; i < Count; i++)
	{
		int Index = i + Pos;
		pair<uint32_t, uint32_t> A = Split(ConvertBits(Data[Index].first.first));
		pair<uint32_t, uint32_t> B = Split(ConvertBits(Data[Index].first.second));

		DataPlis.load_data_al[i] = A.first;
		DataPlis.load_data_ah[i] = A.second;
		DataPlis.load_data_bl[i] = B.first;
		DataPlis.load_data_bh[i] = B.second;
	}
}

void ReadDataFromPLIS(vector<double> &Data, WorkDataPLIS& DataPlis, int Pos, int Count)
{
	for (int i = 0; i < Count; i++)
	{
		int Index = i + Pos;
		Data[Index] = ConvertBits(Combine(DataPlis.unload_data_l[i], DataPlis.unload_data_h[i]));
	}
}

int Compare_PC_PLIS_Full(vector<model_data>& Data, vector<double>& ReadPlis, CLog& Log)
{
	Log << "Сравнение результатов:\n";
	if (Data.size() != ReadPlis.size())
	{
		Log << "Размерности данных не совпадают\n";
		return -1;
	}

	Log << setfill('0');
	int c_error = 0;
	for (size_t i = 0; i < Data.size(); i++)
	{
		uint64_t data_pc = ConvertBits(Data[i].second);
		uint64_t data_plis = ConvertBits(ReadPlis[i]);

		Log << "[" << i << "]\n";
		Log << "\t" << Data[i].first.first << " / " << Data[i].first.second << " = " << Data[i].second << "\n";
		Log << hex << "PC\t"
			<< setw(16) << ConvertBits(Data[i].first.first) << " / "
			<< setw(16) << ConvertBits(Data[i].first.second) << " = "
			<< setw(16) << data_pc << "\n";
		Log << "PLIS\t"
			<< setw(16) << ConvertBits(Data[i].first.first) << " / "
			<< setw(16) << ConvertBits(Data[i].first.second) << " = "
			<< setw(16) << data_plis << dec;

		if (data_pc != data_plis)
		{
			Log << "\tX";
			c_error += 1;
		}
		Log << "\n-------------------------------------------------------------\n";
	}
	return c_error;
}

int Compare_PC_PLIS(vector<model_data>& Data, vector<double>& ReadPlis, CLog& Log)
{
	Log << "Сравнение результатов:\n";
	if (Data.size() != ReadPlis.size())
	{
		Log << "Размерности данных не совпадают\n";
		return -1;
	}

	Log << setfill('0');
	int c_error = 0;
	for (size_t i = 0; i < Data.size(); i++)
	{
		uint64_t data_pc = ConvertBits(Data[i].second);
		uint64_t data_plis = ConvertBits(ReadPlis[i]);

		if (data_pc == data_plis) continue;

		Log << "[" << i << "]\n";
		Log << "\t" << Data[i].first.first << " / " << Data[i].first.second << " = " << Data[i].second << "\n";
		Log << hex << "PC\t"
			<< setw(16) << ConvertBits(Data[i].first.first) << " / "
			<< setw(16) << ConvertBits(Data[i].first.second) << " = "
			<< setw(16) << data_pc << "\n";
		Log << "PLIS\t"
			<< setw(16) << ConvertBits(Data[i].first.first) << " / "
			<< setw(16) << ConvertBits(Data[i].first.second) << " = "
			<< setw(16) << data_plis << dec << "\tX";

		Log << "\n-------------------------------------------------------------\n";
		c_error += 1;
	}
	return c_error;
}

int main()
{
	const string LogName("log.txt");		// Лог файл основной программы
	const string ParamName("param.ini");	// Файл параметров настройки основной программы

	setlocale(LC_ALL, "rus");				// настройка локали (вывод кириллицы в консоль)

	CLog::INIT();							// Инициализация модуля использования лог файлов
	CLog Log(LogName, true);				// Инициализация объекта лог файл для управляющей программы

	ReadFileIni Param(ParamName);			// Чтение файла параметров управляющей программы

	if(Param.GetActive() == false)			// Если файл не был открыт
	{
		Log << CL_RED << "Обшибка: " << "файл " << ParamName << " не был открыт\n";
		CLog::FREE();
		return 0;
	}

	// Инициализация параметров управляющей программы
	const int BmNum = Param.GetInt("bm");						// Номер базового модуля
	const string OutFile = Param.GetString("out_file");			// Названеи *.out файла
	const int DmcIn_al = Param.GetInt("dmc_in_al");				// Номер КРП в который будут загружаться данные
	const int DmcIn_ah = Param.GetInt("dmc_in_ah");
	const int DmcIn_bl = Param.GetInt("dmc_in_bl");
	const int DmcIn_bh = Param.GetInt("dmc_in_bh");

	const int DmcOut_l= Param.GetInt("dmc_out_l");				// Номер КРП из которого будут выгружаться данных
	const int DmcOut_h= Param.GetInt("dmc_out_h");

	const int debugLog = Param.GetInt("debug_log", 0);			// Вывод дополнительной информации в консоль
	const int debugBm  = Param.GetInt("debug_bm", 0);
	const int debugDmc = Param.GetInt("debug_dmc", 0);
	const int debugCompare = Param.GetInt("debug_compare", 0);

	const uint32_t WorkPlis = Param.GetUint32("time_work",10000);	// Время работы ПЛИС

	const int N = Param.GetInt("N", 0);							// Количество загружаемых/выгружаемых данных
	const int Size = Param.GetInt("Size", N);					// Количество необходимых данных для тестирования

	Log.SetWriteConsole(debugLog != 0);

	Log << "Параметры запуска:\n";
	Log << CL_WHITE << "bm" << " = " << BmNum << "\n";
	Log << CL_WHITE << "out_file" << " = " << OutFile << "\n";
	Log << CL_WHITE << "dmc_in_al" << " = " << DmcIn_al << "\n";
	Log << CL_WHITE << "dmc_in_ah" << " = " << DmcIn_ah << "\n";
	Log << CL_WHITE << "dmc_in_bl" << " = " << DmcIn_bl << "\n";
	Log << CL_WHITE << "dmc_in_bh" << " = " << DmcIn_bh << "\n";
	Log << CL_WHITE << "dmc_out_l" << " = " << DmcOut_l << "\n";
	Log << CL_WHITE << "dmc_out_h" << " = " << DmcOut_h << "\n";

	Log << CL_WHITE << "time_work" << " = " << WorkPlis << "\n";
	Log << CL_WHITE << "N" << " = " << N << "\n";
	Log << CL_WHITE << "Size" << " = " << Size << "\n";

	if(N <= 0)
	{
		Log.SetWriteConsole(true);
		Log << CL_RED << "Не задан объем передаваемых данных\n";
		CLog::FREE();
		return 0;
	}

	vector<model_data> ModelData = GenDataTest(Size);
	vector<double> ReadData(Size);

	try
	{
		WorkDataPLIS WDP(N);					// Данные для работы с ПЛИС
		BM::OPEN();								// Открываем интерфейс для работы с ПЛИС

		BM Bm(TAYGETA, BmNum, OutFile, debugBm!=0);			// Инициализируем базовый модуль
		Bm.AddDmc(DmcIn_al, debugDmc!=0);						// Инициализация КРП базового модуля
		Bm.AddDmc(DmcIn_ah, debugDmc!=0);
		Bm.AddDmc(DmcIn_bl, debugDmc!=0);
		Bm.AddDmc(DmcIn_bh, debugDmc!=0);

		Bm.AddDmc(DmcOut_l, debugDmc!=0);
		Bm.AddDmc(DmcOut_h, debugDmc!=0);

		Bm.Kalibrovka();				// Калибровка базового модуля

		int pos = 0;
		while (pos != Size)
		{
			int Count = min(Size - pos, N);
			Log << "Обработка данных [" << pos << " ; " << pos + Count << "]\n";
			WriteDataForPLIS(ModelData, WDP, pos, Count);				// Формирование данных для загрузки в ПЛИС

			// Загрузка данных в ПЛИС
			Bm.LoadData(DmcIn_al, WDP.load_data_al, WDP.SizeByte, 0);
			Bm.LoadData(DmcIn_ah, WDP.load_data_ah, WDP.SizeByte, 0);
			Bm.LoadData(DmcIn_bl, WDP.load_data_bl, WDP.SizeByte, 0);
			Bm.LoadData(DmcIn_bh, WDP.load_data_bh, WDP.SizeByte, 0);

			Bm.Run(WorkPlis);						// Запуск базового модуля

			// Выгрузка данных из ПЛИС
			Bm.UnLoadData(DmcOut_l, WDP.unload_data_l, WDP.SizeByte, 0);
			Bm.UnLoadData(DmcOut_h, WDP.unload_data_h, WDP.SizeByte, 0);

			ReadDataFromPLIS(ReadData, WDP, pos, Count);				// Сохранение выгруженных данных в отдельный массив

			pos += Count;
		}
		BM::CLOSE();							// Закрываем интерфейс для работы с ПЛИС
	}
	catch(HIError msg)
	{
		Log.SetWriteConsole(true);
		Log << CL_RED << "Ошибка: " << "вызов системной функции\n";
		Log << CL_CYAN << msg.GetFun() << " -> " << CL_RED << msg.GetMsg() << "\n";
		BM::CLOSE();					// Завершения работы с ПЛИС (закрытие интерфейса)
	}
	catch(KalibrovkaError msg)
	{
		Log.SetWriteConsole(true);
		Log << CL_RED << "Ошибка: " << "калибровка КРП не прошла -> " << CL_RED << hex << msg.Get() << dec << "\n";
		BM::CLOSE();					// Завершения работы с ПЛИС (закрытие интерфейса)
	}
	catch(BMError msg)
	{
		Log.SetWriteConsole(true);
		Log << CL_RED << "Ошибка: " << msg.Get() << "\n";
		BM::CLOSE();					// Завершения работы с ПЛИС (закрытие интерфейса)
	}
	catch (exception msg)
	{
		Log.SetWriteConsole(true);
		Log << CL_RED << "Ошибка: " << msg.what() << "\n";
		BM::CLOSE();					// Завершения работы с ПЛИС (закрытие интерфейса)
	}

	CLog CompareLog("Compare.txt", false, false);
	int c_error;
	if (debugCompare == 0)	c_error = Compare_PC_PLIS(ModelData, ReadData, CompareLog);
	else					c_error = Compare_PC_PLIS_Full(ModelData, ReadData, CompareLog);

	Log.SetWriteConsole(true);
	if (c_error != 0)	Log << CL_RED << "Провал:";
	else				Log << CL_GREEN << "Успех:";

	Log << " ошибок " << c_error << " из " << ReadData.size() << "\n";

	CLog::FREE();							// Освобождение ресурсов занимаемые модулем лог файлов
	return 0;
}
