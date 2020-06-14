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