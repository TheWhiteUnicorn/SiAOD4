#include <iostream>
#include <string>
#include <fstream>

using namespace std;

const int HASH_TABLE_SIZE = 1061;
const int SEARCHING_KEYS = 500;
const double FillLevels[11] = { 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 0.95, 0.999 };

struct TableRec {
	string key;
	bool busy;
};

TableRec hashTable[1000];
int busyRecCounter = 0;

int success = 0, failures = 0;
long long int succTries = 0, failTries = 0;
double avgSucc, avgFail;



void resetTable() {
	for (int i = 0; i < HASH_TABLE_SIZE - 1; ++i) {
		hashTable[i].busy = false;
	}
	busyRecCounter = 0;
}

int HashFunc(string key) {
	char *p_key = new char[key.size()+1];
	strcpy(p_key, key.c_str());

	int cntr = (int)ceil((double)key.size()/2.0) - 1;
	unsigned short int sum;
	_asm {
		mov ESI, p_key
		xor EDI, EDI
		mov AX, [ESI][EDI]
		mov ECX, cntr
		jz fin
	loop1 :
		inc EDI
		inc EDI
		mov DX, [ESI][EDI]
		add AX, DX
		adc AX, 0
		loop loop1
	fin :
		mov sum, AX
	};
	
	const double A = 0.6180339887;
	int h = int(HASH_TABLE_SIZE*fmod((double)sum*A, 1));
	return h;
}

bool GetWord(istream& _in, string& nextWord) {
	if (!_in.eof()) {
		_in >> nextWord;
		return true;
	}
	else {
		return false;
	}
}

int InsertKey(string key) { // возвращает 0 - число уже есть в таблице 1 - число было внесено в таблицу 2 - число не удалось разместить в таблице
	if (busyRecCounter >= HASH_TABLE_SIZE) {
		cout << "Хеш-таблица переполнена";
		return 0;
	}
	int h = HashFunc(key);
	int d = 1; //вспомогательная переменная для квадратичных проб

	while (hashTable[h].busy) {
		if (key == hashTable[h].key) {
			return 0;
		}
		else {
			h += d; //увеличение на i^2
			d += 2;
			if (h > HASH_TABLE_SIZE) {
				h = h - HASH_TABLE_SIZE;
				if (h > HASH_TABLE_SIZE) {
					//Хеш-таблица переполнена
					return 2;
				}
			}
		}
	}
	hashTable[h].busy = true;
	hashTable[h].key = key;
	return 1;
}

bool FindKey(string key) {
	int tries = 1;
	int h = HashFunc(key);
	int d = 1; //вспомогательная переменная для квадратичных проб

	while (hashTable[h].busy) {
		if (key == hashTable[h].key) {
			succTries += tries;
			return true;
		}
		else {
			h += d; //увеличение на i^2
			d += 2;
			if (h > HASH_TABLE_SIZE) {
				h = h - HASH_TABLE_SIZE;
				if (h > HASH_TABLE_SIZE) {
					failTries += tries;
					return false;
				}
			}
			tries++;
		}
	}
	failTries += tries;
	return false;
}

int main() {
	setlocale(0, "Russian");
	char iFileName[] = "words.txt";
	string wordBuf; // Buffer for word, that currently read from file
	ifstream iFile(iFileName);
	if (!iFile.is_open()) {
		cout << "Ошибка открытия файла!";
		system("pause");
		return 0;
	}

	resetTable();

	cout << "  Степень   Количество  |   Среднее число попыток\n";
	cout << "  заполн. | удач неудач |  при удаче | при неудаче\n";
	cout << "-------------------------------------------------------\n";
	
	for (int level = 0; level < 11; level++) { // Проверяем и выводим результаты для разных уровней заполнения
		//Заполнение
		while (busyRecCounter < FillLevels[level] * HASH_TABLE_SIZE) { 
			if(!GetWord(iFile, wordBuf)) iFile.seekg(0);
			int insExitCode = InsertKey(wordBuf);
			if (insExitCode == 1) busyRecCounter++;
			if (insExitCode == 2) break; //Если хоть одно число не поместилось, уже считаем таблицу заполненной
		}
		iFile.seekg(0);
		//Поиск
		success = 0;   failures = 0;
		succTries = 0; failTries = 0;
		for (int i = 0; i < SEARCHING_KEYS; i++) {
			if (!GetWord(iFile, wordBuf)) {
				iFile.seekg(0);
			}
			if (FindKey(wordBuf)) success++;
			else failures++;
		}
		//Подсчет средних и вывод результатов
		avgSucc = success != 0 ? (double)succTries / (double)success : 0;
		avgFail = failures != 0 ? (double)failTries / (double)failures : 0;
		printf("%8.3f %6i %6i %12.3f %12.3f\n", FillLevels[level], success, failures, avgSucc, avgFail);

		
	}
	system("pause");
}