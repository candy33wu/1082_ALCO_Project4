#include<iostream>
#include<string>
#include<vector>
#include<sstream>
#include<iomanip>
using namespace std;
string add[3][4];//op,rs1,rs2,rob
string mul[2][4];
string aluAdd[4];
string aluMul[4];
string rat[6];
string **rob;//reg,value,done
vector<string> inputall;
int addCycle = 2, subCycle = 2, mulCycle = 10, divCycle = 40;
double rf[6] = { 0,0,2,4,6,8 };
int emptyA = 0, emptyM = 0;//判斷是否能issue
int aluA = 0, aluM = 0;//0:f , 1:t , 2:last cycle
int stringToInt(string str) 
{
	int num;
	string buf;
	for (int i = 0; i < str.length(); ++i)
		if ((str[i] <= '9' && str[i] >= '0') || str[i] == '-')
			buf += str[i];
	stringstream ss;
	ss << buf;
	ss >> num;
	ss.str("");
	ss.clear();
	return num;
}

double stringToDou(string str) 
{
	double num;
	string buf;
	for (int i = 0; i < str.length(); ++i)
		if ((str[i] <= '9' && str[i] >= '0') || str[i] == '-' || str[i] == '.')
			buf += str[i];
	stringstream ss;
	ss << buf;
	ss >> num;
	ss.str("");
	ss.clear();
	return num;
}

string intToStr(double num)
{
	stringstream ss;
	ss << num;
	string str = ss.str();
	ss.str("");
	ss.clear();
	return str;
}

int main()
{
	rf[0] = 0;
	string input;
	int p, pc;
	getline(cin, input);
	bool push = true;
	while (input[0] != cin.eof()) {
		push = true;
		p = input.find("//", 0); //解決註解
		if (p != input.npos) {
			input = input.substr(0, p);
			if (p == 0) {
				push = false;
			}
		}
		if (push)
			inputall.push_back(input);
		getline(cin, input);
	}

	rob = new string * [inputall.size()];
	for (int i = 0; i < inputall.size(); ++i)
		rob[i] = new string[3];
	int current = 0;
	int currentA = 0, currentM = 0, currentRob = 0;
	int cycle = 0;
	int countAdd = 0, countMul = 0;
	string bufferA[5] , bufferM[5];//value,ROB,op,rs1,rs2存計算結果值
	bool issue,change ;
	do{
		issue = false;
		change = false;

		if (countAdd == 0 && aluA == 1) {
			rob[stringToInt(bufferA[1])][1] = bufferA[0];
			rob[stringToInt(bufferA[1])][2] = "1";
			aluA = 2;
			if (rat[stringToInt(rob[stringToInt(bufferA[1])][0])] == bufferA[1]) {
				rat[stringToInt(rob[stringToInt(bufferA[1])][0])] = "";
				for (int i = 0; i < 3; ++i) {
					if (add[i][1] == bufferA[1])
						add[i][1] = bufferA[0];
					if (add[i][2] == bufferA[1])
						add[i][2] = bufferA[0];
					if (i < 2) {
						if (mul[i][1] == bufferA[1])
							mul[i][1] = bufferA[0];
						if (mul[i][2] == bufferA[1])
							mul[i][2] = bufferA[0];
					}
				}
			}
			change = true;
		}//write back
		if (countMul == 0 && aluM == 1) {
			rob[stringToInt(bufferM[1])][1] = bufferM[0];
			rob[stringToInt(bufferM[1])][2] = "1";
			aluM = 2;
			if (rat[stringToInt(rob[stringToInt(bufferM[1])][0])] == bufferM[1]) {
				rat[stringToInt(rob[stringToInt(bufferM[1])][0])] = "";
				for (int i = 0; i < 3; ++i) {
					if (add[i][1] == bufferM[1])
						add[i][1] = bufferM[0];
					if (add[i][2] == bufferM[1])
						add[i][2] = bufferM[0];
					if (i < 2) {
						if (mul[i][1] == bufferM[1])
							mul[i][1] = bufferM[0];
						if (mul[i][2] == bufferM[1])
							mul[i][2] = bufferM[0];
					}
				}
			}
			change = true;
		}//write back


		if (countAdd == 0 && emptyA != 0 && aluA == 0) {
			for (int i = 0; i < 3; ++i) {
				if (add[i][0].length() != 0) {
					if (add[i][1].find("ROB", 0) == add[i][1].npos && add[i][2].find("ROB", 0) == add[i][2].npos) {
						if (add[i][0].find("+", 0) == add[i][0].npos)//-
						{
							countAdd = subCycle + 1;
							bufferA[0] = intToStr(stringToDou(add[i][1]) - stringToDou(add[i][2]));//結果
							bufferA[1] = add[i][3];//rob
							bufferA[2] = "-";
							bufferA[3] = add[i][1];//rs1
							bufferA[4] = add[i][2];//re2
							aluA = 1;
							change = true;
							break;
						}
						else {
							countAdd = addCycle + 1;
							bufferA[0] = intToStr(stringToDou(add[i][1]) + stringToDou(add[i][2]));
							bufferA[1] = add[i][3];
							bufferA[2] = "+";
							bufferA[3] = add[i][1];
							bufferA[4] = add[i][2];
							aluA = 1;
							change = true;
							break;
						}
					}
				}
			}
		}//dispatch
		if (countMul == 0 && emptyM != 0 && aluM == 0) {
			for (int j = 0 , i = 0; i < 2; ++i) {
				if (mul[i][0].length() != 0) {
					if (mul[i][1].find("ROB", 0) == mul[i][1].npos && mul[i][2].find("ROB", 0) == mul[i][2].npos) {
						if (mul[i][j].find("/", 0) == mul[i][j].npos)//*
						{
							countMul = mulCycle + 1;
							bufferM[0] = intToStr(stringToDou(mul[i][1]) * stringToDou(mul[i][2]));
							bufferM[1] = mul[i][3];
							bufferM[2] = "*";
							bufferM[3] = mul[i][1];
							bufferM[4] = mul[i][2];
							aluM = 1;
							change = true;
							break;
						}
						else {
							countMul = divCycle + 1;
							bufferM[0] = intToStr(stringToDou(mul[i][1]) / stringToDou(mul[i][2]));
							bufferM[1] = mul[i][3];
							bufferM[2] = "/";
							bufferM[3] = mul[i][1];
							bufferM[4] = mul[i][2];
							aluM = 1;
							change = true;
							break;
						}

					}
				}
			}
			
		}//dispatch
		if (current < inputall.size()) {
			if (inputall[current].find("ADD", 0) != inputall[current].npos) {
				if (emptyA < 3) {
					p = inputall[current].find(" ", 0);
					add[currentA][0] = "+";
					inputall[current] = inputall[current].substr(p + 1, inputall[current].length());
					p = inputall[current].find(",", 0);
					rob[current][0] = inputall[current].substr(0, p);
					inputall[current] = inputall[current].substr(p + 1, inputall[current].length());
					for (int i = 1 ; i < 3; ++i) {
						p = inputall[current].find(",", 0);
						add[currentA][i] = inputall[current].substr(0, p);
						inputall[current] = inputall[current].substr(p + 1, inputall[current].length());
					}
					add[currentA][3] = "ROB" + intToStr(current);
					rob[current][1] = "0";
					++emptyA;

					for (int i = 1; i < 3; ++i) {
						if (add[currentA][i].find("F", 0) != add[currentA][i].npos) {
							if (!rat[stringToInt(add[currentA][i])].empty())
								add[currentA][i] = rat[stringToInt(add[currentA][i])];
							else
								add[currentA][i] = intToStr(rf[stringToInt(add[currentA][i])]);
						}
						else
							add[currentA][i] = add[currentA][i];
					}
					rat[stringToInt(rob[current][0])] = "ROB" + intToStr(current);
					issue = true;
					change = true;
					currentA++;
				}
			}

			else if (inputall[current].find("SUB", 0) != inputall[current].npos) {
				if (emptyA < 3) {
					p = inputall[current].find(" ", 0);
					add[currentA][0] = "-";
					inputall[current] = inputall[current].substr(p + 1, inputall[current].length());
					p = inputall[current].find(",", 0);
					rob[current][0] = inputall[current].substr(0, p);
					inputall[current] = inputall[current].substr(p + 1, inputall[current].length());
					for (int i = 1; i < 3; ++i) {
						p = inputall[current].find(",", 0);
						add[currentA][i] = inputall[current].substr(0, p);
						inputall[current] = inputall[current].substr(p + 1, inputall[current].length());
					}
					add[currentA][3] = "ROB" + intToStr(current);
					rob[current][1] = "0";
					for (int i = 1; i < 3; ++i) {
						if (add[currentA][i].find("F", 0) != add[currentA][i].npos) {
							if (!rat[stringToInt(add[currentA][i])].empty())
								add[currentA][i] = rat[stringToInt(add[currentA][i])];
							else
								add[currentA][i] = intToStr(rf[stringToInt(add[currentA][i])]);
						}
						else
							add[currentA][i] = add[currentA][i];
					}
					rat[stringToInt(rob[current][0])] = "ROB" + intToStr(current);
					++emptyA;
					issue = true;
					change = true;
					currentA++;
				}
			}
			else if (inputall[current].find("MUL", 0) != inputall[current].npos) {
				if (emptyM < 2) {
					p = inputall[current].find(" ", 0);
					mul[currentM][0] = "*";
					inputall[current] = inputall[current].substr(p + 1, inputall[current].length());
					p = inputall[current].find(",", 0);
					rob[current][0] = inputall[current].substr(0, p);
					inputall[current] = inputall[current].substr(p + 1, inputall[current].length());
					for (int i = 1; i < 3; ++i) {
						p = inputall[current].find(",", 0);
						mul[currentM][i] = inputall[current].substr(0, p);
						inputall[current] = inputall[current].substr(p + 1, inputall[current].length());
					}
					mul[currentM][3] = "ROB" + intToStr(current);
					rob[current][1] = "0";
					for (int i = 1; i < 3; ++i) {
						if (mul[currentM][i].find("F", 0) != mul[currentM][i].npos) {
							if (!rat[stringToInt(mul[currentM][i])].empty())
								mul[currentM][i] = rat[stringToInt(mul[currentM][i])];
							else
								mul[currentM][i] = intToStr(rf[stringToInt(mul[currentM][i])]);
						}
						else
							mul[currentM][i] = mul[currentM][i];
					}
					rat[stringToInt(rob[current][0])] = "ROB" + intToStr(current);
					++emptyM;
					issue = true;
					change = true;
					currentM++;
				}
			}
			else if (inputall[current].find("DIV", 0) != inputall[current].npos) {
				if (emptyM < 2) {
					p = inputall[current].find(" ", 0);
					mul[currentM][0] = "/";
					inputall[current] = inputall[current].substr(p + 1, inputall[current].length());
					p = inputall[current].find(",", 0);
					rob[current][0] = inputall[current].substr(0, p);
					inputall[current] = inputall[current].substr(p + 1, inputall[current].length());
					for (int i = 1; i < 3; ++i) {
						p = inputall[current].find(",", 0);
						mul[currentM][i] = inputall[current].substr(0, p);
						inputall[current] = inputall[current].substr(p + 1, inputall[current].length());
					}
					mul[currentM][3] = "ROB" + intToStr(current);
					rob[current][1] = "0";
					for (int i = 1; i < 3; ++i) {
						if (mul[currentA][i].find("F", 0) != mul[currentA][i].npos) {
							if (!rat[stringToInt(mul[currentA][i])].empty())
								mul[currentA][i] = rat[stringToInt(mul[currentA][i])];
							else
								mul[currentA][i] = intToStr(rf[stringToInt(mul[currentA][i])]);
						}
						else
							mul[currentA][i] = mul[currentA][i];
					}
					rat[stringToInt(rob[current][0])] = "ROB" + intToStr(current);
					++emptyM;
					issue = true;
					change = true;
					currentM++;
				}
			}
		}//issue
		if ((rob[currentRob][2] == "1") || !(current < inputall.size() || emptyA || emptyM || countAdd || countMul)) {
			rf[stringToInt(rob[currentRob][0])] = stringToDou(rob[currentRob][1]);
			for (int i = 0; i < 3; ++i) {
				if (stringToInt(add[i][1]) == currentRob)
					add[i][1] = rob[currentRob][1];
				if (stringToInt(add[i][2]) == currentRob)
					add[i][2] = rob[currentRob][1];
				if (i < 2) {
					if (stringToInt(mul[i][1]) == currentRob)
						mul[i][1] = rob[currentRob][1];
					if (stringToInt(mul[i][2]) == currentRob)
						mul[i][2] = rob[currentRob][1];
				}
			}
			currentRob++;
			change = true;
		}//renew RF
		if (countAdd > 0)
			countAdd--;
		if (countMul > 0)
			countMul--;
		if (aluA == 2) {
			for (int i = 0; i < 3; ++i) {
				if (bufferA[1] == add[i][3]) {
					for (int j = 0; j < 4; ++j) {
						add[i][j] = "";
					}
					currentA = i;
					emptyA--;
					break;
				}
			}
			aluA = 0;
			change = true;
		}//clear RS
		else if (aluM == 2) {
			for (int i = 0; i < 2; ++i) {
				if (bufferM[1] == mul[i][3]) {
					for (int j = 0; j < 4; ++j) {
						mul[i][j] = "";
					}
					currentM = i;
					emptyM--;
					break;
				}
			}
			aluM = 0;
			change = true;
		}//clear RS
		++cycle;
		if (issue)
			current++;
		if (change) {
			cout << "Cycle：" << cycle << endl;
			cout << " _RF_____________________" << endl;
			for (int i = 1; i < 6; ++i)
				cout << " |   " << "F" << i << " |   " << setw(10) << rf[i] << "  |" << endl;
			cout << "--------------------------" << endl << " _RAT____________________" << endl;
			for (int i = 1; i < 6; ++i)
				cout << " |   " << "F" << i << " |   " << setw(10) << rat[i] << "  |" << endl;
			cout << "-------------------------" << endl << " _RS____________________________________________________" << endl;
			for (int i = 0; i < 3; ++i)
				cout << " |   " << "RS" << i << " |   " << setw(10) << add[i][0] << "  |  " << setw(10) << add[i][1] << "  |  " << setw(10) << add[i][2] << "  |  " << endl;
			cout << "--------------------------------------------------------" << endl;
			cout << "BUFFER:";
			if (countAdd != 0)
				cout << "(" << bufferA[1] << ")" << bufferA[3] << bufferA[2] << bufferA[4] << endl << endl;
			else
				cout << "empty" << endl << endl;
			cout << " _RS____________________________________________________" << endl;
			for (int i = 0; i < 2; ++i)
				cout << " |   " << "RS" << i << " |   " << setw(10) << mul[i][0] << "  |  " << setw(10) << mul[i][1] << "  |  " << setw(10) << mul[i][2] << "  |  " << endl;
			cout << "--------------------------------------------------------" << endl;
			cout << "BUFFER:";

			if (countMul != 0)
				cout << "(" << bufferM[1] << ")" << bufferM[3] << bufferM[2] << bufferM[4] << endl << endl;
			else
				cout << "empty" << endl << endl;
			cout << " _ROB_________________________________________________________" << endl;
			for (int i = 0; i < inputall.size(); ++i) {
				cout << " |   " << "ROB" << i << " |   " << setw(10) << rob[i][0] << setw(10) << "  |  " << setw(10) << rob[i][1] << "  |  " << setw(10) << rob[i][2] << "  |  " << endl;
			}
			cout << "--------------------------------------------------------------" << endl;
			cout << endl << endl;
		}
	} while (current < inputall.size() || emptyA || emptyM || countAdd || countMul || currentRob < inputall.size());
}
