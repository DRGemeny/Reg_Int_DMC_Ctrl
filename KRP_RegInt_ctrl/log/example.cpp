#include <clocale>
#include <iostream>
#include "Log.h"

using namespace std;

int main()
{
	setlocale(LC_ALL, "rus");
	cout << "Привет мир!" << endl;

	CLog::INIT();
	int a=1;
	float b=(float)1.1;
	double c=1.001;
	uint32_t d = 0 - 1;
	uint64_t e = 0 - 1;
	CLog log("test_log.txt");
	CLog log2("test_log2.txt");
	log.Color(CL_WHITE);
	log2.Color(C_GREY);

	log << "Привет мир" << "\n"; 
	log2 << "Привет мир" << "\n";
	log << CL_YELLOW << a << "\n";
	log2 << C_YELLOW << a << "\n";
	log << CL_BLUE << b << "\n";
	log2 << C_BLUE << b << "\n";
	log << CL_RED << c << "\n";
	log2 << C_RED << c << "\n";
	log << log_hex << CL_GREEN << d << "\n";
	log2 << C_GREEN << d << "\n";
	log << CL_CYAN << e << "\n";
	log2 << C_CYAN << e << "\n";


	//int z = cout.flags();
	cout << d << "\n";
	
	cout << hex << e << endl;
	//int zz = cout.flags();

	CLog::FREE();
	return 0;
}