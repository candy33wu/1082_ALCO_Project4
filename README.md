# Project 4: Instruction Scheduling  
## 主要目標:  
此程式編寫目的為實現Tomasulo's Algorithm ，該功能為排程instructions執行順序，可避免data dependece的錯誤，以確保執行指定任務時存取值正確。  

## 程式運行方式:  

**該程式執行軟體為Visual 2019。**    
請將程式碼複製至上述軟體中，再編譯執行。    

## 簡要使用說明:   
- register 陣列中的值初始設定為 {0,0,2,4,6,8}。  
- 如要修改cycle time，請直接修改程式碼設定值 (預設為 add: 2 , sub: 2, mul: 10, div:40 cycles)。    
- Input: 使用鍵盤輸入一段組合語言，輸入完畢後請按下2次enter鍵，即可執行；其中，每段指令皆須做換行分隔 (可執行之基本指令包含: add, addi, sub, mul, muli, div) 。  

> 輸入範例    
![avatar](https://upload.cc/i1/2020/06/14/YxqZDm.jpg)  

- Output: 螢幕顯示有變化之 cycle中組合語言指令的實際執行狀況(內容包含 RAT,REGS,ROB,IQ, 以及分為 add, sub/ mul, div兩組的 RS ,  ALU (BUFFER) )  
> 輸出範例  
![avatar](https://upload.cc/i1/2020/06/14/ZOlYHx.jpg)  
![avatar](https://upload.cc/i1/2020/06/14/w2pRGt.jpg) 
![avatar](https://upload.cc/i1/2020/06/14/JNxHB6.jpg) 
![avatar](https://upload.cc/i1/2020/06/14/4jQk6g.jpg) 
![avatar](https://upload.cc/i1/2020/06/14/U9sfeA.jpg) 
![avatar](https://upload.cc/i1/2020/06/14/pcaKR5.jpg) 
![avatar](https://upload.cc/i1/2020/06/14/aNX6Vi.jpg) 
![avatar](https://upload.cc/i1/2020/06/14/Dn03LA.jpg)  

## 程式結構說明:    

### 基本原理:    
 - 當前執行指令對應之RS有空位時 issue 該行指令進入RS。  
 - 目標 rd 存入ROB，按程式碼之先後順序放入，以供確保 WAW - dependence發生時值不會發生存取錯誤。  
 - RAT 存放各 register最後於ROB出現的位置。  
 - 當某 RS中的 instruction 所有需要的運算元素都到齊時，且對應的 ALU為空的，則該 instrustion可進入對應 ALU 執行。  
 - ALU 結束後將值 write result 填入 ROB中的 value那一格，並將 done 改為 1。  
 ( ROB有1個指標，用以告知欲存放位置)將值傳給所需資源者，RAT若與該ROB相同時，將 RAT擦除，RF 內值更新。  
 - 當完成 write result ，將對應 RS 清空。  
 - 重複上方動作直至任務完成。  
 

![avatar](https://upload.cc/i1/2020/06/14/hp58Ka.jpg)  
### 引用函式庫說明:   
```cpp  
#include<iostream> //負責輸出/輸入  
```  
```cpp  
#include<string>  //負責字串處理  
```  
```cpp  
#include<vector> //container  
```  
```cpp  
#include<sstream> // string 轉 int/double & int/double 轉 string
```  
```cpp  
#include<iomanip> // 排版整潔
```  
### Global變數說明:   
```cpp  
vector<string> inputall;  
//inputall: 用以存放每行輸入之指令資訊，待後續處理  
```  
```cpp  
string add[3][4]; string mul[2][4];
//模擬實際RS存放
```  
```cpp  
string aluAdd[4]; string aluMul[4];
//ALU 模擬
```  
```cpp  
string rat[6];//模擬RAT 
```  
```cpp  
int rf[6]; //存放registor file內的值  
```  
```cpp  
string **rob;//模擬ROB實際存放狀況
```  
```cpp  
int addCycle = 2, subCycle = 2, mulCycle = 10, divCycle = 40;
// 各種指令所需cycle time設定
```   
```cpp  
int emptyA = 0, emptyM = 0;//判斷是否能issue
```  
```cpp  
int aluA = 0, aluM = 0;//記ALU執行狀況
```  
### 詳細程式碼說明   

```cpp  
int stringToInt(string str) {
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
```  
>將 String轉為 Int  
```cpp  
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
```  
>將 String轉為 Double  
```cpp
string intToStr(double num)
{
	stringstream ss;
	ss << num;
	string str = ss.str();
	ss.str("");
	ss.clear();
	return str;
}
```
>實際為將 Double轉為 String  
```cpp
int main()
{
 rf[0] = 0;
 string input;
 int p ;
 bool push = true;
 getline(cin, input);
 int current = 0;
 int currentA = 0, currentM = 0, currentRob = 0;
 int cycle = 0;
 int countAdd = 0, countMul = 0;
 string bufferA[5] , bufferM[5];//value,ROB,op,rs1,rs2存計算結果值
 bool issue,change ;
```
>reg[0]為 F0不可更改  
input取各行 cin的指令  
p為一暫存值，為找尋 string中某字元的位置  
push決定是否保留該行程式碼  
current紀錄當前待 issue位置  
currentA, currentM, currentRob 紀錄欲存入位置  
cycle 記錄當前 cycle數   
countAdd , countMul 紀錄 ALU執行所需之剩餘 cycle time  
 bufferA[5] , bufferM[5] 暫存計算結果資訊  
 issue, change 紀錄是否做該動作  
 
 ```cpp
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
 ```
> 處理每行輸入之字串，首先偋除註解，留下指令。   
初始 ROB  

```cpp  
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
       mul[i][2] = bufferA[0]; }} 
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
```
> ROB值更新為所得結果，並將 done改為 1，以表示完成計算  
> 找到 RS中等待其計算結果的 instructions 並將值寫入，以供後續使用。  
write result & catch.       
```cpp  
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
```
> 如果 ALU為空，且所需之 data 皆齊全，做 dispatch。  
暫存計算結果進對應 buffer中，直至跑完對應 cycle 數始作後續動作。  

```cpp
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
```  
>  當對應 RS有空位，則將 current指向的 instruction放入該空位中。  
檢查該 instruction所需之 data 是否被記錄在 RAT中;如果有以紀錄之  ROB替代，反之，則尋找相對 RF之值。   
```cpp
if ((rob[currentRob][2] == "1")||!(current < inputall.size() || emptyA 
    || emptyM || countAdd || countMul)) {
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
```  
> 在對應 cycle時，將計算完成的值更新到 RF中。   

```cpp
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
	if(issue)
		current++;
```
> 如 ALU中有正在執行的 instruction，舊更新其對應的 cycle time。  
當完成計算後，將對應的 RS清空。  
```cpp
    if (change) {
	   cout << "Cycle：" << cycle << endl;
    	cout << " _RF_____________________" << endl;
    	for (int i = 1; i < 6; ++i)
        	cout << " |   " << "F" << i << " |   " << setw(10) <<rf[i] << "  |" << endl;
    	    cout << "--------------------------" << endl << " _RAT____________________" << endl;
		    for (int i = 1; i < 6; ++i)
			    cout << " |   "  <<"F" << i << " |   " << setw(10) << rat[i] << "  |" << endl;
		    cout << "-------------------------" << endl << " _RS____________________________________________________" << endl;
		    for (int i = 0; i < 3; ++i)
			    cout << " |   " << "RS" << i << " |   " << setw(10) << add[i][0] << "  |  " << setw(10) << add[i][1] << "  |  " << setw(10) << add[i][2] << "  |  " << endl;
		    	cout << "--------------------------------------------------------" << endl;
		    cout << "BUFFER:";
	    	if (countAdd != 0)
		    	cout << "(" << bufferA[1] << ")" << bufferA[3] << bufferA[2] <<     bufferA[4] << endl << endl;
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
	    	for (int i = 0; i < inputall.size(); ++i){
			    cout<< " |   "  <<  "ROB" << i << " |   " << setw(10) << rob[i][0] << setw(10) << "  |  " << setw(10) << rob[i][1] << "  |  " <<     setw(10) << rob[i][2] << "  |  " << endl;
		    }
		    cout << "------------------------------------------------------------- -" << endl;
		    cout << endl << endl;
    	}	
    } while (current < inputall.size() || emptyA || emptyM || countAdd || countMul || currentRob < inputall.size());
}
```
> 歷盡千辛萬苦地排版，成就完美的輸出。  

## 常見問題    
- 輸入多餘的空格    
- register必須由"F"+ 數字 構成    
 
