#include<iostream>
#include<fstream>
#include<vector>
#include<queue>
#include<map>
#include<set>
#include<stack>
#include <iomanip>

//鉴于终结符号表中没有0、1，符号串中id和num分别用0、1表示
#define ID '0'
#define NUM '1'
//空
#define EPS "?"
//开始符号
#define START 'E'
//sync标识
#define SYNC -1

std::map<char, std::vector<std::string>> production;
std::map<char, std::vector<char>> first;
std::map<char, std::map<char, int>> table_record;
std::map<char, std::vector<char>> follow;
std::queue<char> extra_non;

void input_grammar();//读入文法
void ini_extra_non();//初始化各容器，并构建额外非终结符（供消除左递归使用）
void show_production();//输出产生式
void show_first();//输出FIRST集
void show_follow();//输出FOLLOW集
void show_ana_table();//输出分析表
int analyze_pro(std::string);//分解产生式
void find_id_num(std::string&);//将id与num用一个字符'0','1'代替
void deal_with_recur();//处理左递归
void deal_with_common();//处理左公因子
void compute_first();//构造FIRST集
std::vector<char> find_first(char);
void compute_follow();//构造FOLLOW集
int is_non(char);//判断是否为非终结符
int to_eps(char ch);//判断是否推空
void input_sentence();//读入句子
void analyze(std::string);//开始进行LL(1)分析

int main()
{
	ini_extra_non();//初始化辅助额外符号供给
	input_grammar();//读入文法
	deal_with_recur();//消除左递归
	deal_with_common();//消除回溯
	compute_first();//构造FIRST集
	compute_follow();//构造FOLLOW集
	show_production();
	show_first();
	show_follow();//输出信息
	show_ana_table();//输出分析表
	input_sentence();//读入句子
	return 0;
}

void input_grammar()
{
	std::ifstream fin;
	fin.open("grammar.txt", std::ios::in);
	std::string buffer;
	int err;
	while (!fin.eof())
	{
		fin >> buffer;
		err = analyze_pro(buffer);
		if (err == -1)
		{
			printf("Unable to analyze input production file");
			exit(0);
		}
	}
	fin.close();//关闭文件
}

void ini_extra_non()
{
	production.clear();
	first.clear();
	table_record.clear();
	follow.clear();
	extra_non = std::queue<char>();//初始化
	extra_non.push('L');
	extra_non.push('R');
	extra_non.push('H');
	extra_non.push('P');
	extra_non.push('B');
}

void show_production()
{
	std::ofstream fout("production.txt", std::ios::out);
	for (auto i = production.rbegin(); i != production.rend(); i++)//对于每个非终结符
	{
		std::vector<std::string> temp;
		temp = i->second;
		std::cout << i->first << "->";
		fout << i->first << "->";
		for (auto ite = temp.begin(); ite != temp.end();)//对于每个产生式
		{
			std::cout << *ite;
			fout << *ite;
			if (++ite != temp.end())
			{
				std::cout << '|';
				fout << '|';
			}
		}
		std::cout << std::endl;
		fout << std::endl;
	}
}

void show_first()
{
	std::ofstream fout("first.txt", std::ios::out);
	for (auto i = first.rbegin(); i != first.rend(); i++)
	{
		std::vector<char> temp;
		temp = i->second;
		std::map<char, int> temp2;
		temp2 = table_record.find(i->first)->second;
		std::cout << i->first << ": ";
		fout << i->first << ": ";
		for (auto ite = temp.begin(); ite != temp.end();)
		{
			std::cout << *ite;
			fout << *ite;
			if (++ite != temp.end())
			{
				std::cout << '|';
				fout << '|';
			}
		}
		std::cout << std::endl;
		fout << std::endl;
	}
}

void show_follow()
{
	std::ofstream fout("follow.txt", std::ios::out);
	for (auto i = follow.rbegin(); i != follow.rend(); i++)
	{
		std::vector<char> temp;
		temp = i->second;
		std::cout << i->first << "->";
		fout << i->first << "->";
		for (auto ite = temp.begin(); ite != temp.end();)
		{
			std::cout << *ite;
			fout << *ite;
			if (++ite != temp.end())
			{
				std::cout << '|';
				fout << '|';
			}
		}
		std::cout << std::endl;
		fout << std::endl;
	}
}

void show_ana_table()
{
	std::ofstream fout("ana_table.txt", std::ios::out);
	std::vector<char> terminals{'(', ')', '*', '+','-', '/', '$', '1'};
	std::cout << std::setw(10) << " ";
	fout << std::setw(10) << " ";
	for (auto i = terminals.begin(); i != terminals.end(); i++)
	{
		std::cout << std::setiosflags(std::ios::left) << std::setw(10) <<*i;
		fout << std::setiosflags(std::ios::left) << std::setw(10) << *i;
	}
	std::cout << std::endl;
	fout << std::endl;
	//输出表
	for (auto i = table_record.begin(); i != table_record.end(); i++)//对于每个非终结符的FIRST集
	{
		std::cout << std::setiosflags(std::ios::left) << std::setw(10) << i->first;
		fout << std::setiosflags(std::ios::left) << std::setw(10) << i->first;
		std::map<char, int> temp = i->second;
		int now_ter = 0;//当前要输出的终结符
		std::map<int, int> out;//记录在第几个输出第几个
		for (auto j = temp.begin(); j != temp.end(); j++)//对于每个FIRST_RECORD集元素
		{
			for (auto k = terminals.begin(); k != terminals.end(); k++)//对于每个终结符
			{
				if (j->first == *k)
				{
					out.insert(std::pair<int, int>(static_cast<int>(k - terminals.begin()), j->second));
					break;
				}
			}
		}
		for (int k = 0; k < terminals.size(); k++)
		{
			if (out.find(k) != out.end())
			{
				std::string s;
				s.push_back(i->first);
				if (out.find(k)->second == -1)
					s = "sync";
				else
					s += "->" + production.find(i->first)->second[out.find(k)->second];
				std::cout << std::setiosflags(std::ios::left) << std::setw(10) << s;
				fout << std::setiosflags(std::ios::left) << std::setw(10) << s;
			}
			else
			{
				std::cout << std::setiosflags(std::ios::left) << std::setw(10) << " ";
				fout << std::setiosflags(std::ios::left) << std::setw(10) << " ";
			}
		}
		std::cout << std::endl;
		fout << std::endl;
	}
}

int analyze_pro(std::string buffer)
{
	if (buffer.size() <= 2)
		return -1;
	char non;
	non = buffer[0];
	char cur;
	cur = buffer[1];
	if (cur != '-')
		return -1;
	cur = buffer[2];
	if (cur != '>')
		return -1;
	std::string str = "";
	std::vector<std::string> nu;
	nu.clear();
	//建立一个非终结符的索引
	production.insert(std::pair<char, std::vector<std::string>>(non, nu));
	for (int i = 3; i < buffer.size(); i++)
	{
		if (buffer[i] == '|')//一个候选式
		{
			find_id_num(str);
			production.find(non)->second.push_back(str);		
			str.clear();
		}
		else
			str.push_back(buffer[i]);
	}
	//最好有一个候选式后不跟“|”，单独处理
	find_id_num(str);
	production.find(non)->second.push_back(str);
	str.clear();
	return 1;
}

void find_id_num(std::string& str)
{
	if (str.find("id") != str.npos)
	{
		auto loc = str.find("id");
		str.erase(loc, 2);
		str.insert(loc, 1, ID);
	}
	if (str.find("num") != str.npos)
	{
		auto loc = str.find("num");
		str.erase(loc, 3);
		str.insert(loc, 1, NUM);
	}
}

void deal_with_recur()
{
	std::map<char, std::vector<std::string>> to_be_deal = production;//待处理的非终结符
	for (auto i = to_be_deal.rbegin(); i != to_be_deal.rend(); i++)//对于每个非终结符
	{
		std::vector<std::string> temp;
		temp = i->second;
		std::vector<std::string> with_recur;
		std::vector<std::string> without_recur;
		for (auto ite = temp.begin(); ite != temp.end(); ite++)//对于产生的每个候选式
		{
			if ((*ite)[0] == i->first)//第一个字符与非终结符相同
			{
				ite->erase(0, 1);//去掉非终结符
				with_recur.push_back(*ite);//左递归候选式后串
				continue;//进行操作后直接会进行else语句，不合理，故continue
			}
			else
				without_recur.push_back(*ite);//非左递归候选式
		}
		if (with_recur.empty())//无左递归
			continue;
		production.find(i->first)->second.clear();
		char extra = extra_non.front();
		extra_non.pop();//取出一个额外非终结符，供消除左递归
		//初始化额外非终结符对应的候选式集合
		std::vector<std::string> nu;
		nu.clear();
		production.insert(std::pair<char, std::vector<std::string>>(extra, nu));
		
		//开始加入重新构造的候选式
		std::vector<std::string> add;
		for (auto ite = without_recur.begin(); ite != without_recur.end(); ite++)
		{
			add.push_back((*ite) + extra);//加入非递归候选式+extra非终结符
		}
		production.find(i->first)->second = add;
		for (auto ite = with_recur.begin(); ite != with_recur.end(); ite++)
		{
			production.find(extra)->second.push_back((*ite) + extra);
		}
		production.find(extra)->second.push_back(EPS);
	}
}

void deal_with_common()
{
	std::map<char, std::vector<std::string>> to_be_deal = production;//待处理的非终结符
	for (auto i = to_be_deal.rbegin(); i != to_be_deal.rend(); i++)//对于每个非终结符
	{
		std::vector<std::string> temp;
		temp = i->second;
		std::vector<std::string> with_recur;
		std::vector<std::string> without_recur;
		std::string matcher;
		std::string matchee;//两个有公共左因子的候选式
		std::vector<std::string>::iterator er;
		std::vector<std::string>::iterator ee;//两个候选式的iterator
		std::string match;//公共左因子
		for (auto ite = temp.begin(); ite != temp.end(); ite++)//对于产生的每个候选式
		{
			er = ite;
			matcher = *ite;
			match.push_back((*ite)[0]);
			for (auto ite2 = temp.begin(); ite2 != temp.end(); ite2++)//对于产生的每个候选式
			{
				if ((*ite2)[0] == (*ite)[0])//有公共左因子
				{
					ee = ite2;
					matchee = *ite2;
					for (int j = 1; j < ite->size() && j < ite2->size(); j++)//尝试匹配更长的字符串
					{
						if ((*ite)[j] == (*ite2)[j])//若后续字符仍然相等加长公共左因子
							match.push_back((*ite)[j]);
					}
				}
			}
			matchee.clear();
		}
		if (matchee.empty())
			continue;
		char extra = extra_non.front();
		extra_non.pop();//取出一个额外非终结符，供消除公共左因子
		match.clear();
		matcher.erase(0, match.size());
		matchee.erase(0, match.size());//去除公共左因子
		i->second.push_back(match + extra);

		std::vector<std::string> nu;
		nu.clear();
		production.insert(std::pair<char, std::vector<std::string>>(extra, nu));
		production.find(extra)->second.clear();
		production.find(extra)->second.push_back(matcher);
		production.find(extra)->second.push_back(matchee);
	}
}

void compute_first()
{
	for (auto i = production.rbegin(); i != production.rend(); i++)//对于每个非终结符
	{
		//初始化非终结符对应的FIRST集、FOLLOW集（顺便）
		std::vector<char> nu;
		nu.clear();
		first.insert(std::pair<char, std::vector<char>>(i->first, nu));
		follow.insert(std::pair<char, std::vector<char>>(i->first, nu));
		//初始化table_record集，用于记录FIRST及中的元素是哪个候选式推导出来的，相当于分析表
		std::map<char, int> mu;
		mu.clear();
		table_record.insert(std::pair<char, std::map<char, int>>(i->first, mu));

		std::vector<std::string> temp = i->second;
		for (auto ite = temp.begin(); ite != temp.end(); ite++)//对于产生的每个候选式
		{
			std::vector<char> result = find_first((*ite)[0]);
			
			//对于当前结果的每一个元素，添加记录
			for (auto ite2 = result.begin(); ite2 != result.end(); ite2++)
			{
				//记录此FIRST集元素是从哪个候选式得出的
				table_record.find(i->first)->second.insert(std::pair<char, int>(*ite2, static_cast<int>(ite - temp.begin())));
			}
			//添加结果
			first.find(i->first)->second.insert(first.find(i->first)->second.end(), result.begin(), result.end());
			if (to_eps((*ite)[0]) && ite + 1 != temp.end())
			{
				std::vector<char> result = find_first((*ite)[1]);
				//添加结果
				first.find(i->first)->second.insert(first.find(i->first)->second.end(), result.begin(), result.end());
				//添加FIRST候选式推出记录
				for (auto ite2 = result.begin(); ite2 != result.end(); ite2++)
				{
					table_record.find(i->first)->second.insert(std::pair<char, int>(*ite2, static_cast<int>(ite - temp.begin())));
				}
			}
		}
	}
}

std::vector<char> find_first(char ch)
{
	std::vector<char> result;
	if (!is_non(ch))
	{
		result.push_back(ch);
		return result;
	}
	std::vector<std::string> temp = production.find(ch)->second;
	std::vector<char> sub_result;
	for (auto ite = temp.begin(); ite != temp.end(); ite++)//对于产生的每个候选式
	{
		sub_result = find_first((*ite)[0]);
		result.insert(result.end(), sub_result.begin(), sub_result.end());
		
		if (to_eps((*ite)[0]))//第一个非终结符可以推出空，则再加上后续的FIRST集
		{
			sub_result = find_first((*ite)[1]);
			result.insert(result.end(), sub_result.begin(), sub_result.end());
		}
	}
	return result;
}

void compute_follow()
{
	std::vector<std::vector<char>> to_add;//将后一个非终结符的FOLLOW集加入到前一个非终结符FOLLOW集中
	for (auto i = production.rbegin(); i != production.rend(); i++)//对于每个非终结符
	{
		std::vector<std::string> temp;
		temp = i->second;
		for (auto ite = temp.begin(); ite != temp.end(); ite++)//对于每个产生式
		{
			for (int j = 0; j < ite->size(); j++)//遍历产生式的字符
			{
				if (!is_non((*ite)[j]))
					continue;
				//对于非终结符
				//1、后方无，将产生式左侧非终结符和当前非终结符加入到待加入集合
				if (j + 1 == ite->size())
				{
					std::vector<char> temp;
					temp.push_back((*ite)[j]);
					temp.push_back(i->first);
					to_add.push_back(temp);
					continue;
				}
				//2、后方为终结符，直接加到FOLLOW集
				if (!is_non((*ite)[j + 1]))
				{
					follow.find((*ite)[j])->second.push_back((*ite)[j + 1]);
					continue;
				}
				//3、后方有终结符则
				if (is_non((*ite)[j + 1]))
				{
					//若不退空，则加入他的FIRST集
					std::vector<char> add;
					add = first.find((*ite)[j + 1])->second;
					follow.find((*ite)[j])->second.insert(follow.find((*ite)[j])->second.begin(), add.begin(), add.end());
					//若其推空，将其和当前非终结符加入到待加入集合
					if (to_eps((*ite)[j + 1]))
					{
						std::vector<char> temp;
						temp.push_back((*ite)[j]);
						temp.push_back((*ite)[j + 1]);
						to_add.push_back(temp);
						continue;
					}
				}
			}
		}
	}

	//开始符号加入'$'
	follow.find(START)->second.push_back('$');

	//去重、去空EPS
	for (auto i = follow.begin(); i != follow.end(); i++)//对于每个非终结符
	{
		std::set<char>s(i->second.begin(), i->second.end());
		i->second.assign(s.begin(), s.end());
		std::vector<char>::iterator loc;
		//去EPS
		for (auto j = i->second.begin(); j != i->second.end(); j++)
		{
			if ((*j) == EPS[0])
			{
				loc = j;
				i->second.erase(loc);
				break;
			}
		}
	}
	
	//先去to_add重
	{
		std::set<std::vector<char>>s(to_add.begin(), to_add.end());
		to_add.assign(s.begin(), s.end());
	}
	//处理to_add
	while (1)
	{
		int changed = 0;
		for (auto i = to_add.begin(); i != to_add.end(); i++)
		{
			std::vector<char> temp = follow.find((*i)[0])->second;
			//FOLLOW集加入到目标FOLLOW集中
			follow.find((*i)[0])->second.insert(follow.find((*i)[0])->second.begin(), follow.find((*i)[1])->second.begin(), follow.find((*i)[1])->second.end());
			//去重
			for (auto j = follow.begin(); j != follow.end(); j++)
			{
				std::set<char>s(j->second.begin(), j->second.end());
				j->second.assign(s.begin(), s.end());
			}
			//类似于冒泡排序的中止思想，没有变化则停止
			if (temp != follow.find((*i)[0])->second)
				changed++;
		}
		if (!changed)
			break;
	}

	//添加基于FOLLOW集的table_record记录
	for (auto i = table_record.begin(); i != table_record.end(); i++)//对于每一个非终结符
	{
		if (to_eps(i->first))//如果推出空EPS
		{
			std::vector<char> temp = follow.find(i->first)->second;
			for (auto j = temp.begin(); j != temp.end(); j++)//对于每个FOLLOW集元素
			{
				//构造一条含有空产生式分析表记录
				//将记录加入到非终结符的条目下
				table_record.find(i->first)->second.insert(std::pair<char, int>(*j, production.find(i->first)->second.size() - 1));
			}
		}
		else//设置sync标识
		{
			std::vector<char> temp = follow.find(i->first)->second;
			for (auto j = temp.begin(); j != temp.end(); j++)//对于每个FOLLOW集元素
			{
				//将sync记录加入到非终结符的对应终结符的!空!条目下
				if(table_record.find(i->first)->second.find(*j) == table_record.find(i->first)->second.end())
					table_record.find(i->first)->second.insert(std::pair<char, int>(*j, SYNC));
			}
		}
	}
}

int is_non(char ch)
{
	if (ch >= 65 && ch <= 90)
		return 1;
	return 0;
}

int to_eps(char ch)
{
	if (!is_non(ch))
		return 0;
	std::vector<std::string> temp = production.find(ch)->second;
	for (auto i = temp.begin(); i != temp.end(); i++)
	{
		if ((*i) == EPS)
			return 1;
	}
	return 0;
}

void input_sentence()
{
	std::ifstream fin;
	fin.open("sentence.txt", std::ios::in);
	std::string s;
	fin >> s;
	//将数字换为num（"1")
	for (auto i = s.begin(); i != s.end(); i++)
	{
		if (*i >= 48 && *i <= 57)
		{
			auto loc = i;
			while (i != s.end())
			{
				if (!(*i >= 48 && *i <= 57))
					break;
				i++;
			}
			s.erase(loc, i);
			s.insert(loc, '1');
		}
		if (i == s.end())
			break;
	}
	fin.close();//关闭文件
	//进入分析程序
	analyze(s);
}

void analyze(std::string s)
{
	std::ofstream fout("analyze.txt", std::ios::out);
	//列名
	std::cout << std::setw(4) << "步骤";
	std::cout << std::setw(10) << "栈";
	std::cout << std::setw(20) << "输入";
	std::cout << std::setw(10) << "输出";
	std::cout << std::setw(10) << "左句型";
	fout << std::setw(4) << "步骤";
	fout << std::setw(10) << "栈";
	fout << std::setw(20) << "输入";
	fout << std::setw(10) << "输出";
	fout << std::setw(30) << "左句型";
	//初始化
	std::stack<char> stack;
	stack.push('$');
	stack.push(START);//栈初始化
	std::string stack_str;
	stack_str.push_back('$');
	stack_str += START;//字符串表示栈初始化（栈不方便遍历）
	std::string output;//输出初始化
	s.push_back('$');//输入初始化
	std::string left_sen;
	int loc;//左句型辅助弹压下标
	left_sen.push_back(START);//左句型初始化

	//输出一行信息
	std::cout << std::endl;
	std::cout << std::setw(4) << "0";
	std::cout << std::setw(10) << stack_str;
	std::cout << std::setw(20) << s;
	std::cout << std::setw(10) << output;
	std::cout << std::setw(30) << left_sen;
	fout << std::endl;
	fout << std::setw(4) << "0";
	fout << std::setw(10) << stack_str;
	fout << std::setw(20) << s;
	fout << std::setw(10) << output;
	fout << std::setw(30) << left_sen;

	//开始分析
	for (int i = 1; stack.top() != '$'; i++)
	{
		//先弹出栈顶符号
		char top = stack.top();
		stack.pop();
		stack_str.pop_back();
		if (is_non(top))//弹出非终结符
		{
			loc = left_sen.find(top);
			left_sen.erase(loc, 1);
		}
		//栈顶为非终结符则根据输入头进行栈符号添加
		if (is_non(top))
		{
			std::string push;
			//如果遇到错误
			if (table_record.find(top)->second.find(s[0]) == table_record.find(top)->second.end())
			{
				stack.push(top);
				stack_str.push_back(top);
				left_sen.push_back(top);//不弹出符号
				s.erase(0, 1);//句子指针向前
				output = "错误句子!!!!!!!!!!";
				std::cout << std::endl;
				std::cout << std::setw(4) << i;
				std::cout << std::setw(10) << stack_str;
				std::cout << std::setw(20) << s;
				std::cout << std::setw(10) << output;
				std::cout << std::setw(30) << left_sen;
				fout << std::endl;
				fout << std::setw(4) << i;
				fout << std::setw(10) << stack_str;
				fout << std::setw(20) << s;
				fout << std::setw(10) << output;
				fout << std::setw(30) << left_sen;
				output.clear();//清除输出
				continue;
			}
			int num = table_record.find(top)->second.find(s[0])->second;//候选式编号
			//判断sync标识
			if (num == SYNC)//如果是sync符号弹出栈顶元素后直接下一轮
			{
				output = "ERROR!!!SYNC";
				std::cout << std::endl;
				std::cout << std::setw(4) << i;
				std::cout << std::setw(10) << stack_str;
				std::cout << std::setw(20) << s;
				std::cout << std::setw(10) << output;
				std::cout << std::setw(30) << left_sen;
				fout << std::endl;
				fout << std::setw(4) << i;
				fout << std::setw(10) << stack_str;
				fout << std::setw(20) << s;
				fout << std::setw(10) << output;
				fout << std::setw(30) << left_sen;
				output.clear();//清除输出
				continue;
			}

			push = production.find(top)->second[num];//找到候选式字符
			output.push_back(top);
			output += "->" + push;//输出
			if (push != "?")
			{
				for (auto i = push.rbegin(); i != push.rend(); i++)
					stack.push(*i);//逆序！！！候选式入栈
				stack_str.append(push.rbegin(), push.rend());//逆序！！！插入候选式
				left_sen.insert(left_sen.begin() + loc, push.begin(), push.end());//正序！！！
			}
		}
		//栈顶为终结符则匹配弹出
		else//若为EPS，则直接忽略
		{
			if (s[0] == top)
			{
				s.erase(0, 1);//输入弹出
				output = "匹配";
			}
		}
		
		//输出一行信息
		std::cout << std::endl;
		std::cout << std::setw(4) << i;
		std::cout << std::setw(10) << stack_str;
		std::cout << std::setw(20) << s;
		std::cout << std::setw(10) << output;
		std::cout << std::setw(30) << left_sen;
		fout << std::endl;
		fout << std::setw(4) << i;
		fout << std::setw(10) << stack_str;
		fout << std::setw(20) << s;
		fout << std::setw(10) << output;
		fout << std::setw(30) << left_sen;
		output.clear();//清除输出
	}
	if (s.size() != 1)
	{
		std::cout << std::endl;
		std::cout << std::setw(10) << "错误句子!!!!!!!!!!";
		fout << std::endl;
		fout << std::setw(10) << "错误句子!!!!!!!!!!";
	}
}
