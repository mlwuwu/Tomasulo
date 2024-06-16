#include <cstdio>
#include <algorithm>
#include <string>
#include <iostream>
#include <queue>
#include <set>
#include <map>
#include <windows.h>
#include <cstdlib>
int t;
std::queue<int> q;//指令队列（流出队列） 
struct st
{
	int n,step;
};
struct instr  //存放指令的各个细节 
{
	std::string op;
	int fi,fj,fk,time,device,vj,vk,qj,qk,a;  //device为保留站 time是指令执行周期数-load:2个时钟周期；加、减法:2个时钟周期；乘法:10个时钟周期；除法:20个时钟周期
	int issue,execution,write;
	int begin,result1,result2,result3,result4,result5,result6,flag1,flag2; //begin代表正在执行 result存储写入的结果 flag为写结果的标记
	std::string name;
} instruction[100];
std::set<int> step[6];//存放各个阶段的指令队列 
std::set<int>::iterator it,ite;//迭代器，用来枚举 
std::map<std::string,int> map;//用来做映射，将数字与指代的指令的操作码联系 
std::map<int,std::string> mapp;//用来做映射，将数字与指代的保留站联系

int reg[32];//该寄存器将接收对应保留站的结果 
int result[3][32];   //存放寄存器的值，即保留站device产生的结果
int device[8];//存放占用该保留站的指令号
//初始化

inline void init()
{
	map["LD"] = 1;map["SUBD"] = 3;map["ADDD"] = 3;map["DIVD"] = 6;map["MULTD"] = 6;
	mapp[1] = "load1";mapp[2] = "load2";mapp[3] = "add1";mapp[4] = "add2";mapp[5] = "add3"; mapp[6] = "mult1"; mapp[7] = "mult2"; 
}

//读指令 
inline void read()
{
	//读入要执行的指令数 
	scanf("%d",&t);
	//去除多余换行符，防止读入空行 
	char ch; 
	while(1)
	{
		ch = getchar();
		if (ch == '\n') break;	
	}
	for(int i = 1;i <= t;i++)
	{
		std::string s;
		getline(std::cin,s);
		instruction[i].name = s;
		//处理LD指令 
		if (s[0] == 'L') 
		{
			instruction[i].op = "LD";
			instruction[i].time = 2;
			int cnt = 0;
			for(int j = 2;j < s.length();j++)
			{
				if (s[j] != ' ') 
				{
					cnt = j;
					break;
				}
			}
			instruction[i].fi = s[cnt + 1] -'0';
			if (s[cnt + 2] >= '0' && s[cnt + 2] <= '9')
			{
				instruction[i].fi *= 10;
				instruction[i].fi += s[cnt + 2] - '0';
			}

			for(int j = cnt + 1;j < s.length();j++)
			{
				if (s[j] == ',')
				{
					cnt = j;
					break;
				}
			}
			instruction[i].a = s[cnt + 1] - '0';
			if (s[cnt + 2] >= '0' && s[cnt + 2] <= '9')
			{
				instruction[i].a *= 10;
				instruction[i].a += s[cnt + 2] - '0';  //立即数是两位数的情况
			}

			for(int j = cnt + 1;j < s.length();j++)
			{
				if (s[j] == 'R')
				{
					cnt = j;
					break;
				}
			}
			instruction[i].fj = s[cnt + 1] - '0';
			if (s[cnt + 2] >= '0' && s[cnt + 2] <= '9')
			{
				instruction[i].fj *= 10;
				instruction[i].fj += s[cnt + 2] - '0';
			}
		}
		//处理其他指令 
		else
		{
			int cnt = 0;
			if (s[0] == 'M') 
			{
				instruction[i].op = "MULTD";
				instruction[i].time = 10;
			}
			else if (s[0] == 'S') 
			{
				instruction[i].op = "SUBD";
				instruction[i].time = 2;
			}
			else if (s[0] == 'A') 
			{
				instruction[i].op = "ADDD";
				instruction[i].time = 2;
			}
			else if (s[0] == 'D') 
			{
				instruction[i].op = "DIVD";
				instruction[i].time = 20;
			}
			for(int j = 1;j < s.length();j++)
			{
				if (s[j] == 'F') 
				{
					cnt = j;
					break;
				}
			}
			instruction[i].fi = s[cnt + 1] -'0';
			if (s[cnt + 2] >= '0' && s[cnt + 2] <= '9')
			{
				instruction[i].fi *= 10;
				instruction[i].fi += s[cnt + 2] - '0';
			}

			for(int j = cnt + 1;j < s.length();j++)
			{
				if (s[j] == 'F') 
				{
					cnt = j;
					break;
				}
			}
			instruction[i].fj = s[cnt + 1] -'0';
			if (s[cnt + 2] >= '0' && s[cnt + 2] <= '9')
			{
				instruction[i].fj *= 10;
				instruction[i].fj += s[cnt + 2] - '0';
			}
			
			for(int j = cnt + 1;j < s.length();j++)
			{
				if (s[j] == 'F') 
				{
					cnt = j;
					break;
				}
			}
			instruction[i].fk = s[cnt + 1] -'0';
			if (s[cnt + 2] >= '0' && s[cnt + 2] <= '9')
			{
				instruction[i].fk *= 10;
				instruction[i].fk += s[cnt + 2] - '0';
			}
		}
	}
}

//输出结果 
inline void print(int x)
{
	printf("                                                周期%d:\n",x); 
	//输出指令状态表 
	printf("指令状态:\n");
	printf("指令名        |Issue|Execution completed|Write Result|\n");
	for(int i = 1;i <= t;i++)
	{
		std::cout << instruction[i].name;
		for(int j = 1;j <= 14 - instruction[i].name.length();j++) printf(" ");
		printf("|");
		if (instruction[i].issue != 0) printf("%5d|",instruction[i].issue);
		else 
		{
			for(int j = 1;j <= 5;j++) printf(" ");
			printf("|");
		}
		
		if (instruction[i].execution != 0) printf("%19d|",instruction[i].execution); //执行完毕
		else if(instruction[i].begin>0){
			for(int j = 1;j <= 11;j++) printf(" ");
			printf("正在执行");
			printf("|");
		}
		else 
		{
			for(int j = 1;j <= 19;j++) printf(" "); //还未开始执行
			printf("|");
		}
		
		if (instruction[i].write != 0) printf("%12d|",instruction[i].write);
		else 
		{
			for(int j = 1;j <= 12;j++) printf(" ");
			printf("|");
		}
		
		printf("\n");
		
		
		
	}
	printf("\n");
	//输出保留站内容 
	printf("保留站内容:\n");
	printf("保留站名称|Busy |Op   |Vj           |Vk           |Qj   |Qk   |A         |\n");
	for(int i = 1;i <= 7;i++)
	{
		std::cout << mapp[i];
		for(int j = 1;j <= 10 - mapp[i].length();j++) printf(" ");
		printf("|");
		if(device[i] != 0 && instruction[device[i]].write == x)
		{
			printf("no   |     |             |             |     |     |          |\n"); 
			continue;
		}
		else if (device[i] != 0 && instruction[device[i]].write != x) printf("yes  |");
		else 
		{
			printf("no   |     |             |             |     |     |          |\n");
			continue;
		}
		std::cout << instruction[device[i]].op;
		for(int j = 1;j <= 5 - instruction[device[i]].op.length();j++) printf(" ");
		printf("|");
		if (instruction[device[i]].op[0] == 'L')      
		{                                                    
			//检测第一操作数是否就绪   
			if(reg[instruction[device[i]].fj] == 0)
			printf("R[R%d]        |             |0    |     ",instruction[device[i]].fj);
			else{
			printf("             |             |");  
			std::cout << mapp[reg[instruction[device[i]].fj]];   //要写这个fj寄存器的device保留站编号  用mapp从编号映射出保留站名称字符串            
			for(int j = 1;j <= 5 - mapp[reg[instruction[device[i]].fj]].length();j++) printf(" ");
			printf("|     ");
			}

			//printf("%d,%d",instruction[device[i]].begin,instruction[device[i]].execution); 
			if (instruction[i].begin > 0 || instruction[i].execution != 0) printf("|R[R%d]+%-4d|",instruction[device[i]].fj,instruction[device[i]].a); //正在执行 计算有效地址放入A字段 
			else printf("|%-10d|",instruction[device[i]].a);     //最后放入 a 保证字段对齐
		}
		
		else 
		{   //分别判断第一第二操作数 
			//printf("%d,%d,%d",instruction[device[i]].result1,instruction[device[i]].result2,instruction[device[i]].result3);
			if(instruction[device[i]].flag1 == 1 && instruction[device[i]].qj == 0){     //vj字段          //接收保留站产生的结果
				if(instruction[device[i]].result3==1) printf("Mem[R[R%d]+%d]|",instruction[device[i]].result1,instruction[device[i]].result2);  
				else if(instruction[device[i]].result3==2) printf("R[F%d]+R[F%d]  |",instruction[device[i]].result1,instruction[device[i]].result2); 
				else if(instruction[device[i]].result3==3) printf("R[F%d]-R[F%d]  |",instruction[device[i]].result1,instruction[device[i]].result2); 
				else if(instruction[device[i]].result3==4) printf("R[F%d]*R[F%d]  |",instruction[device[i]].result1,instruction[device[i]].result2); 
				else if(instruction[device[i]].result3==5) printf("R[F%d]/R[F%d]  |",instruction[device[i]].result1,instruction[device[i]].result2); 
			}    

			else if(instruction[device[i]].flag1 != 1 && instruction[device[i]].qj == 0){                  
				if(result[2][instruction[device[i]].fj]!=0){                                                       //从寄存器中取刚被写入的数据
				if(result[2][instruction[device[i]].fj]==1) printf("Mem[R[R%d]+%d]|",result[0][instruction[device[i]].fj],result[1][instruction[device[i]].fj]);  
				else if(result[2][instruction[device[i]].fj]==2) printf("R[F%d]+R[F%d]  |",result[0][instruction[device[i]].fj],result[1][instruction[device[i]].fj]); 
				else if(result[2][instruction[device[i]].fj]==3) printf("R[F%d]-R[F%d]  |",result[0][instruction[device[i]].fj],result[1][instruction[device[i]].fj]); 
				else if(result[2][instruction[device[i]].fj]==4) printf("R[F%d]*R[F%d]  |",result[0][instruction[device[i]].fj],result[1][instruction[device[i]].fj]); 
				else if(result[2][instruction[device[i]].fj]==5) printf("R[F%d]/R[F%d]  |",result[0][instruction[device[i]].fj],result[1][instruction[device[i]].fj]);
				}
				else printf("R[F%d]        |",instruction[device[i]].fj);                                        //从寄存器中取已就绪的数据
			}
			else 
			printf("             |");

			//printf("%d,%d,%d,%d,%d,%d",instruction[device[i]].flag2,device[i],instruction[device[i]].qk,instruction[device[i]].device,instruction[device[i]].fk,result[2][instruction[device[i]].fk]);

			if(instruction[device[i]].flag2 == 1 && instruction[device[i]].qk == 0){      //vk字段
				if(instruction[device[i]].result6==1) printf("Mem[R[R%d]+%d]|",instruction[device[i]].result4,instruction[device[i]].result5);  
				else if(instruction[device[i]].result6==2) printf("R[F%d]+R[F%d]  |",instruction[device[i]].result4,instruction[device[i]].result5); 
				else if(instruction[device[i]].result6==3) printf("R[F%d]-R[F%d]  |",instruction[device[i]].result4,instruction[device[i]].result5); 
				else if(instruction[device[i]].result6==4) printf("R[F%d]*R[F%d]  |",instruction[device[i]].result4,instruction[device[i]].result5); 
				else if(instruction[device[i]].result6==5) printf("R[F%d]/R[F%d]  |",instruction[device[i]].result4,instruction[device[i]].result5); 
			}    

			else if(instruction[device[i]].flag2 != 1 && instruction[device[i]].qk == 0){  
				if(result[2][instruction[device[i]].fk]!=0){
				if(result[2][instruction[device[i]].fk]==1) printf("Mem[R[R%d]+%d]|",result[0][instruction[device[i]].fk],result[1][instruction[device[i]].fk]);  
				else if(result[2][instruction[device[i]].fk]==2) printf("R[F%d]+R[F%d]  |",result[0][instruction[device[i]].fk],result[1][instruction[device[i]].fk]); 
				else if(result[2][instruction[device[i]].fk]==3) printf("R[F%d]-R[F%d]  |",result[0][instruction[device[i]].fk],result[1][instruction[device[i]].fk]); 
				else if(result[2][instruction[device[i]].fk]==4) printf("R[F%d]*R[F%d]  |",result[0][instruction[device[i]].fk],result[1][instruction[device[i]].fk]); 
				else if(result[2][instruction[device[i]].fk]==5) printf("R[F%d]/R[F%d]  |",result[0][instruction[device[i]].fk],result[1][instruction[device[i]].fk]);
				}
				else printf("R[F%d]        |",instruction[device[i]].fk);
			}

			else 
			printf("             |");
			
			if(instruction[device[i]].qj == 0)  //qj字段 
			printf("0    |");
			else{                                                
			//printf("%d",instruction[device[i]].qj);  
			std::cout << mapp[instruction[device[i]].qj];                            
			for(int j = 1;j <= 5 - mapp[instruction[device[i]].qj].length();j++) printf(" ");
			printf("|");
			} 
			
			if(instruction[device[i]].qk == 0)  //qk字段
			printf("0    |");
			else{                                               
			//printf("%d",instruction[device[i]].qk);  
			std::cout << mapp[instruction[device[i]].qk];                           
			for(int j = 1;j <= 5 - mapp[instruction[device[i]].qk].length();j++) printf(" ");
			printf("|");
			} 						
			printf("          |");     //最后保证a字段对齐
		}
		printf("\n");
	}
	printf("\n");
	//输出寄存器状态表 
	printf("寄存器状态表:\n");
	printf("寄存器号|");
	for(int i = 0;i <= 12;i+=2)
		{ 
			printf("F%-12d|",i);	
		}
	printf("...|F30  |\n");

	printf("Qi      |");
	for(int i = 0;i <= 12;i+=2)
		{ 
			if (reg[i] != 0)
				    {  
				    std::cout << mapp[reg[i]];
					for(int j = 1;j <= 13 - mapp[reg[i]].length();j++) printf(" "); 
					}
			else printf("             "); 
			printf("|");
		}
	printf("...|F30  |\n");

	printf("值      |");	
	for(int i = 0;i <= 12;i+=2)
		{      			
			if(reg[i] == 0 && result[2][i]==1) printf("Mem[R[R%d]+%d]",result[0][i],result[1][i]);  
			else if(reg[i] == 0 && result[2][i]==2) printf("R[F%d]+R[F%d]  ",result[0][i],result[1][i]); 
			else if(reg[i] == 0 && result[2][i]==3) printf("R[F%d]-R[F%d]  ",result[0][i],result[1][i]); 
			else if(reg[i] == 0 && result[2][i]==4) printf("R[F%d]*R[F%d]  ",result[0][i],result[1][i]); 
			else if(reg[i] == 0 && result[2][i]==5) printf("R[F%d]/R[F%d]  ",result[0][i],result[1][i]); 

//if(reg[i] ！= 0 && result[2][i]==1) printf("Mem[R[R%d]+%d]",result[0][i],result[1][i]); 


			else printf("             ");
			printf("|");
		}
	printf("...|F30  |\n");
}
//算法具体流程 
inline void procedure() 
{
	for(int i = 1;i <= t;i++) q.push(i); //所有指令按顺序进入队列
	int time = 0;
	while(1)
	{
		system("Pause");
		printf("------------------------------------------------------------------------------------------------------------------------\n");
		std::queue<st> now;//储存当前周期的队列变换 st是结构体 储存n和step
		time++; 
		//流出阶段 
		if (!q.empty())
		{
			int x = q.front(); //从指令队列头部取一条指令
			//寻找是否存在空闲保留站 
			if (map[instruction[x].op] == 1)
			{
				if (device[1] == 0) instruction[x].device = 1;
				else if (device[2] == 0) instruction[x].device = 2;
			}
			else if (map[instruction[x].op] == 3)
			{
				if (device[3] == 0) instruction[x].device = 3;
				else if (device[4] == 0) instruction[x].device = 4;
				else if (device[5] == 0) instruction[x].device = 5;
			}
			else if (map[instruction[x].op] == 6)
			{
				if (device[6] == 0) instruction[x].device = 6;
				else if (device[7] == 0) instruction[x].device = 7;
			}
			//分配保留站 把代表该指令的编号放到指令x的结构体的device中

			if (instruction[x].device != 0)  
			{
				 
					instruction[x].issue = time; //可以流出
					reg[instruction[x].fi] = instruction[x].device; //即将要写目的寄存器的指令是x reg里面存的是保留站号
					if (reg[instruction[x].fj] == 0) instruction[x].qj = 0; //第一操作数就绪  
					else 
					{
						instruction[x].qj = reg[instruction[x].fj];   
					}

					if (reg[instruction[x].fk] == 0) instruction[x].qk = 0; //第二操作数就绪  如果没有指令要写fk寄存器 qk为0
					else 
					{
						instruction[x].qk = reg[instruction[x].fk]; //要写fk寄存器的指令的保留站名称放到qk    
					}
					device[instruction[x].device] = x; 
					//将流出阶段完成的x放入执行队列 包括待执行以及正在执行的指令
					now.push((st){x,2});
					q.pop();	
			}
		}
		//执行阶段 
		for(it = step[2].begin();it != step[2].end();++it)
		{
			int x = *it; 
			//判断两个源操作数是否就绪 
			if (instruction[x].op[0] == 'L' ||(instruction[x].qj==0 && instruction[x].qk==0))
			{
				instruction[x].begin++;
				instruction[x].time--;
				
				if (instruction[x].time == 0)
			{
				instruction[x].execution = time; //达到所需的执行周期，执行完毕之后，将执行完的时钟周期数放到指令的结构体的execution里保存
				instruction[x].begin=0;
				//将执行阶段完成的指令x放入写结果队列，并从执行队列删除 
				now.push((st){x,3});
			}
			}
		}
		 //写结果阶段
		for(it = step[3].begin();it != step[3].end();++it)
		{
			int x = *it;  //printf("%d",x); //x为指令号，代表正在写结果的指令
				instruction[x].write = time;
				for(int i = 0;i <= 12;i+=2){
					if(reg[i] == instruction[x].device){
					result[0][i]=instruction[x].fj;
					if(instruction[x].op=="LD") result[1][i]=instruction[x].a;
					else result[1][i]=instruction[x].fk;
					if (instruction[x].op == "LD") result[2][i]=1;
					else if (instruction[x].op == "ADDD") result[2][i]=2;
					else if (instruction[x].op == "SUBD") result[2][i]=3;
					else if (instruction[x].op == "MULTD") result[2][i]=4;
					else if (instruction[x].op == "DIVD") result[2][i]=5;
				}
				}
				for(int i = 0;i <= 12;i+=2){
					if(reg[i] == instruction[x].device) reg[i] = 0;   
				}
				device[instruction[x].device] = 0;

				for (int i = 1; i <= 7; i++) {    
        			if(device[i]!=0){                                            
					if (instruction[device[i]].qj == instruction[x].device) {
            				instruction[device[i]].result1 = instruction[x].fj;
            				if(instruction[x].op=="LD") instruction[device[i]].result2=instruction[x].a;
					else instruction[device[i]].result2 = instruction[x].fk; 
				
					if (instruction[x].op == "LD") {instruction[device[i]].result3=1; }
					else if (instruction[x].op == "ADDD") {instruction[device[i]].result3=2;}
					else if (instruction[x].op == "SUBD") {instruction[device[i]].result3=3;}
					else if (instruction[x].op == "MULTD") {instruction[device[i]].result3=4;}
					else if (instruction[x].op == "DIVD") {instruction[device[i]].result3=5;}
					instruction[device[i]].flag1 = 1; 
					instruction[device[i]].qj = 0;                                     
        				}
        				if (instruction[device[i]].qk == instruction[x].device) {
            				instruction[device[i]].result4 = instruction[x].fj;
            				if(instruction[x].op=="LD") instruction[device[i]].result5=instruction[x].a;
					else instruction[device[i]].result5 = instruction[x].fk; 
				
					if (instruction[x].op == "LD") {instruction[device[i]].result6=1;}
					else if (instruction[x].op == "ADDD") {instruction[device[i]].result6=2;}
					else if (instruction[x].op == "SUBD") {instruction[device[i]].result6=3;}
					else if (instruction[x].op == "MULTD") {instruction[device[i]].result6=4;}
					else if (instruction[x].op == "DIVD") {instruction[device[i]].result6=5;}
					 
					instruction[device[i]].qk = 0;    
					instruction[device[i]].flag2 = 1; 
        				}
				}
    			}
				//将写结果阶段完成的x放入结束队列，并从写结果队列中删除 
				now.push((st){x,4});	
				break;  //控制一个周期最多只有一条指令写结果
		}
				//执行该指令周期的队列更新 
		while(!now.empty())
		{
			st x = now.front();
			now.pop();
			step[x.step - 1].erase(x.n);
			step[x.step].insert(x.n);
		}
		//输出结果 
		print(time);
		//判断是否所有指令均已完成
		if (step[4].size() == t) break;
	}
}
int main()
{
	SetConsoleOutputCP(65001); 
	init();
	read();
	procedure();
}

/*测试样例 
2
ADDD F2,F6,F4
LD F2,45(R3)
//写后写冲突

3
MULTD F0,F2,F4
MULTD F6,F8,F0
ADDD F0,F8,F2
//读后写冲突

3
LD F6,34(R2)
LD F6,45(R3)
ADDD F0,F2,F6
//写后读冲突 
*/
