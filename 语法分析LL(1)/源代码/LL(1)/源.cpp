#include<iostream>
#include<fstream>
#include<vector>
#include<queue>
#include<map>
#include<set>
#include<stack>
#include <iomanip>

//�����ս���ű���û��0��1�����Ŵ���id��num�ֱ���0��1��ʾ
#define ID '0'
#define NUM '1'
//��
#define EPS "?"
//��ʼ����
#define START 'E'
//sync��ʶ
#define SYNC -1

std::map<char, std::vector<std::string>> production;
std::map<char, std::vector<char>> first;
std::map<char, std::map<char, int>> table_record;
std::map<char, std::vector<char>> follow;
std::queue<char> extra_non;

void input_grammar();//�����ķ�
void ini_extra_non();//��ʼ����������������������ս������������ݹ�ʹ�ã�
void show_production();//�������ʽ
void show_first();//���FIRST��
void show_follow();//���FOLLOW��
void show_ana_table();//���������
int analyze_pro(std::string);//�ֽ����ʽ
void find_id_num(std::string&);//��id��num��һ���ַ�'0','1'����
void deal_with_recur();//������ݹ�
void deal_with_common();//����������
void compute_first();//����FIRST��
std::vector<char> find_first(char);
void compute_follow();//����FOLLOW��
int is_non(char);//�ж��Ƿ�Ϊ���ս��
int to_eps(char ch);//�ж��Ƿ��ƿ�
void input_sentence();//�������
void analyze(std::string);//��ʼ����LL(1)����

int main()
{
	ini_extra_non();//��ʼ������������Ź���
	input_grammar();//�����ķ�
	deal_with_recur();//������ݹ�
	deal_with_common();//��������
	compute_first();//����FIRST��
	compute_follow();//����FOLLOW��
	show_production();
	show_first();
	show_follow();//�����Ϣ
	show_ana_table();//���������
	input_sentence();//�������
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
	fin.close();//�ر��ļ�
}

void ini_extra_non()
{
	production.clear();
	first.clear();
	table_record.clear();
	follow.clear();
	extra_non = std::queue<char>();//��ʼ��
	extra_non.push('L');
	extra_non.push('R');
	extra_non.push('H');
	extra_non.push('P');
	extra_non.push('B');
}

void show_production()
{
	std::ofstream fout("production.txt", std::ios::out);
	for (auto i = production.rbegin(); i != production.rend(); i++)//����ÿ�����ս��
	{
		std::vector<std::string> temp;
		temp = i->second;
		std::cout << i->first << "->";
		fout << i->first << "->";
		for (auto ite = temp.begin(); ite != temp.end();)//����ÿ������ʽ
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
	//�����
	for (auto i = table_record.begin(); i != table_record.end(); i++)//����ÿ�����ս����FIRST��
	{
		std::cout << std::setiosflags(std::ios::left) << std::setw(10) << i->first;
		fout << std::setiosflags(std::ios::left) << std::setw(10) << i->first;
		std::map<char, int> temp = i->second;
		int now_ter = 0;//��ǰҪ������ս��
		std::map<int, int> out;//��¼�ڵڼ�������ڼ���
		for (auto j = temp.begin(); j != temp.end(); j++)//����ÿ��FIRST_RECORD��Ԫ��
		{
			for (auto k = terminals.begin(); k != terminals.end(); k++)//����ÿ���ս��
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
	//����һ�����ս��������
	production.insert(std::pair<char, std::vector<std::string>>(non, nu));
	for (int i = 3; i < buffer.size(); i++)
	{
		if (buffer[i] == '|')//һ����ѡʽ
		{
			find_id_num(str);
			production.find(non)->second.push_back(str);		
			str.clear();
		}
		else
			str.push_back(buffer[i]);
	}
	//�����һ����ѡʽ�󲻸���|������������
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
	std::map<char, std::vector<std::string>> to_be_deal = production;//������ķ��ս��
	for (auto i = to_be_deal.rbegin(); i != to_be_deal.rend(); i++)//����ÿ�����ս��
	{
		std::vector<std::string> temp;
		temp = i->second;
		std::vector<std::string> with_recur;
		std::vector<std::string> without_recur;
		for (auto ite = temp.begin(); ite != temp.end(); ite++)//���ڲ�����ÿ����ѡʽ
		{
			if ((*ite)[0] == i->first)//��һ���ַ�����ս����ͬ
			{
				ite->erase(0, 1);//ȥ�����ս��
				with_recur.push_back(*ite);//��ݹ��ѡʽ��
				continue;//���в�����ֱ�ӻ����else��䣬��������continue
			}
			else
				without_recur.push_back(*ite);//����ݹ��ѡʽ
		}
		if (with_recur.empty())//����ݹ�
			continue;
		production.find(i->first)->second.clear();
		char extra = extra_non.front();
		extra_non.pop();//ȡ��һ��������ս������������ݹ�
		//��ʼ��������ս����Ӧ�ĺ�ѡʽ����
		std::vector<std::string> nu;
		nu.clear();
		production.insert(std::pair<char, std::vector<std::string>>(extra, nu));
		
		//��ʼ�������¹���ĺ�ѡʽ
		std::vector<std::string> add;
		for (auto ite = without_recur.begin(); ite != without_recur.end(); ite++)
		{
			add.push_back((*ite) + extra);//����ǵݹ��ѡʽ+extra���ս��
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
	std::map<char, std::vector<std::string>> to_be_deal = production;//������ķ��ս��
	for (auto i = to_be_deal.rbegin(); i != to_be_deal.rend(); i++)//����ÿ�����ս��
	{
		std::vector<std::string> temp;
		temp = i->second;
		std::vector<std::string> with_recur;
		std::vector<std::string> without_recur;
		std::string matcher;
		std::string matchee;//�����й��������ӵĺ�ѡʽ
		std::vector<std::string>::iterator er;
		std::vector<std::string>::iterator ee;//������ѡʽ��iterator
		std::string match;//����������
		for (auto ite = temp.begin(); ite != temp.end(); ite++)//���ڲ�����ÿ����ѡʽ
		{
			er = ite;
			matcher = *ite;
			match.push_back((*ite)[0]);
			for (auto ite2 = temp.begin(); ite2 != temp.end(); ite2++)//���ڲ�����ÿ����ѡʽ
			{
				if ((*ite2)[0] == (*ite)[0])//�й���������
				{
					ee = ite2;
					matchee = *ite2;
					for (int j = 1; j < ite->size() && j < ite2->size(); j++)//����ƥ��������ַ���
					{
						if ((*ite)[j] == (*ite2)[j])//�������ַ���Ȼ��ȼӳ�����������
							match.push_back((*ite)[j]);
					}
				}
			}
			matchee.clear();
		}
		if (matchee.empty())
			continue;
		char extra = extra_non.front();
		extra_non.pop();//ȡ��һ��������ս��������������������
		match.clear();
		matcher.erase(0, match.size());
		matchee.erase(0, match.size());//ȥ������������
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
	for (auto i = production.rbegin(); i != production.rend(); i++)//����ÿ�����ս��
	{
		//��ʼ�����ս����Ӧ��FIRST����FOLLOW����˳�㣩
		std::vector<char> nu;
		nu.clear();
		first.insert(std::pair<char, std::vector<char>>(i->first, nu));
		follow.insert(std::pair<char, std::vector<char>>(i->first, nu));
		//��ʼ��table_record�������ڼ�¼FIRST���е�Ԫ�����ĸ���ѡʽ�Ƶ������ģ��൱�ڷ�����
		std::map<char, int> mu;
		mu.clear();
		table_record.insert(std::pair<char, std::map<char, int>>(i->first, mu));

		std::vector<std::string> temp = i->second;
		for (auto ite = temp.begin(); ite != temp.end(); ite++)//���ڲ�����ÿ����ѡʽ
		{
			std::vector<char> result = find_first((*ite)[0]);
			
			//���ڵ�ǰ�����ÿһ��Ԫ�أ���Ӽ�¼
			for (auto ite2 = result.begin(); ite2 != result.end(); ite2++)
			{
				//��¼��FIRST��Ԫ���Ǵ��ĸ���ѡʽ�ó���
				table_record.find(i->first)->second.insert(std::pair<char, int>(*ite2, static_cast<int>(ite - temp.begin())));
			}
			//��ӽ��
			first.find(i->first)->second.insert(first.find(i->first)->second.end(), result.begin(), result.end());
			if (to_eps((*ite)[0]) && ite + 1 != temp.end())
			{
				std::vector<char> result = find_first((*ite)[1]);
				//��ӽ��
				first.find(i->first)->second.insert(first.find(i->first)->second.end(), result.begin(), result.end());
				//���FIRST��ѡʽ�Ƴ���¼
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
	for (auto ite = temp.begin(); ite != temp.end(); ite++)//���ڲ�����ÿ����ѡʽ
	{
		sub_result = find_first((*ite)[0]);
		result.insert(result.end(), sub_result.begin(), sub_result.end());
		
		if (to_eps((*ite)[0]))//��һ�����ս�������Ƴ��գ����ټ��Ϻ�����FIRST��
		{
			sub_result = find_first((*ite)[1]);
			result.insert(result.end(), sub_result.begin(), sub_result.end());
		}
	}
	return result;
}

void compute_follow()
{
	std::vector<std::vector<char>> to_add;//����һ�����ս����FOLLOW�����뵽ǰһ�����ս��FOLLOW����
	for (auto i = production.rbegin(); i != production.rend(); i++)//����ÿ�����ս��
	{
		std::vector<std::string> temp;
		temp = i->second;
		for (auto ite = temp.begin(); ite != temp.end(); ite++)//����ÿ������ʽ
		{
			for (int j = 0; j < ite->size(); j++)//��������ʽ���ַ�
			{
				if (!is_non((*ite)[j]))
					continue;
				//���ڷ��ս��
				//1�����ޣ�������ʽ�����ս���͵�ǰ���ս�����뵽�����뼯��
				if (j + 1 == ite->size())
				{
					std::vector<char> temp;
					temp.push_back((*ite)[j]);
					temp.push_back(i->first);
					to_add.push_back(temp);
					continue;
				}
				//2����Ϊ�ս����ֱ�Ӽӵ�FOLLOW��
				if (!is_non((*ite)[j + 1]))
				{
					follow.find((*ite)[j])->second.push_back((*ite)[j + 1]);
					continue;
				}
				//3�������ս����
				if (is_non((*ite)[j + 1]))
				{
					//�����˿գ����������FIRST��
					std::vector<char> add;
					add = first.find((*ite)[j + 1])->second;
					follow.find((*ite)[j])->second.insert(follow.find((*ite)[j])->second.begin(), add.begin(), add.end());
					//�����ƿգ�����͵�ǰ���ս�����뵽�����뼯��
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

	//��ʼ���ż���'$'
	follow.find(START)->second.push_back('$');

	//ȥ�ء�ȥ��EPS
	for (auto i = follow.begin(); i != follow.end(); i++)//����ÿ�����ս��
	{
		std::set<char>s(i->second.begin(), i->second.end());
		i->second.assign(s.begin(), s.end());
		std::vector<char>::iterator loc;
		//ȥEPS
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
	
	//��ȥto_add��
	{
		std::set<std::vector<char>>s(to_add.begin(), to_add.end());
		to_add.assign(s.begin(), s.end());
	}
	//����to_add
	while (1)
	{
		int changed = 0;
		for (auto i = to_add.begin(); i != to_add.end(); i++)
		{
			std::vector<char> temp = follow.find((*i)[0])->second;
			//FOLLOW�����뵽Ŀ��FOLLOW����
			follow.find((*i)[0])->second.insert(follow.find((*i)[0])->second.begin(), follow.find((*i)[1])->second.begin(), follow.find((*i)[1])->second.end());
			//ȥ��
			for (auto j = follow.begin(); j != follow.end(); j++)
			{
				std::set<char>s(j->second.begin(), j->second.end());
				j->second.assign(s.begin(), s.end());
			}
			//������ð���������ֹ˼�룬û�б仯��ֹͣ
			if (temp != follow.find((*i)[0])->second)
				changed++;
		}
		if (!changed)
			break;
	}

	//��ӻ���FOLLOW����table_record��¼
	for (auto i = table_record.begin(); i != table_record.end(); i++)//����ÿһ�����ս��
	{
		if (to_eps(i->first))//����Ƴ���EPS
		{
			std::vector<char> temp = follow.find(i->first)->second;
			for (auto j = temp.begin(); j != temp.end(); j++)//����ÿ��FOLLOW��Ԫ��
			{
				//����һ�����пղ���ʽ�������¼
				//����¼���뵽���ս������Ŀ��
				table_record.find(i->first)->second.insert(std::pair<char, int>(*j, production.find(i->first)->second.size() - 1));
			}
		}
		else//����sync��ʶ
		{
			std::vector<char> temp = follow.find(i->first)->second;
			for (auto j = temp.begin(); j != temp.end(); j++)//����ÿ��FOLLOW��Ԫ��
			{
				//��sync��¼���뵽���ս���Ķ�Ӧ�ս����!��!��Ŀ��
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
	//�����ֻ�Ϊnum��"1")
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
	fin.close();//�ر��ļ�
	//�����������
	analyze(s);
}

void analyze(std::string s)
{
	std::ofstream fout("analyze.txt", std::ios::out);
	//����
	std::cout << std::setw(4) << "����";
	std::cout << std::setw(10) << "ջ";
	std::cout << std::setw(20) << "����";
	std::cout << std::setw(10) << "���";
	std::cout << std::setw(10) << "�����";
	fout << std::setw(4) << "����";
	fout << std::setw(10) << "ջ";
	fout << std::setw(20) << "����";
	fout << std::setw(10) << "���";
	fout << std::setw(30) << "�����";
	//��ʼ��
	std::stack<char> stack;
	stack.push('$');
	stack.push(START);//ջ��ʼ��
	std::string stack_str;
	stack_str.push_back('$');
	stack_str += START;//�ַ�����ʾջ��ʼ����ջ�����������
	std::string output;//�����ʼ��
	s.push_back('$');//�����ʼ��
	std::string left_sen;
	int loc;//����͸�����ѹ�±�
	left_sen.push_back(START);//����ͳ�ʼ��

	//���һ����Ϣ
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

	//��ʼ����
	for (int i = 1; stack.top() != '$'; i++)
	{
		//�ȵ���ջ������
		char top = stack.top();
		stack.pop();
		stack_str.pop_back();
		if (is_non(top))//�������ս��
		{
			loc = left_sen.find(top);
			left_sen.erase(loc, 1);
		}
		//ջ��Ϊ���ս�����������ͷ����ջ�������
		if (is_non(top))
		{
			std::string push;
			//�����������
			if (table_record.find(top)->second.find(s[0]) == table_record.find(top)->second.end())
			{
				stack.push(top);
				stack_str.push_back(top);
				left_sen.push_back(top);//����������
				s.erase(0, 1);//����ָ����ǰ
				output = "�������!!!!!!!!!!";
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
				output.clear();//������
				continue;
			}
			int num = table_record.find(top)->second.find(s[0])->second;//��ѡʽ���
			//�ж�sync��ʶ
			if (num == SYNC)//�����sync���ŵ���ջ��Ԫ�غ�ֱ����һ��
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
				output.clear();//������
				continue;
			}

			push = production.find(top)->second[num];//�ҵ���ѡʽ�ַ�
			output.push_back(top);
			output += "->" + push;//���
			if (push != "?")
			{
				for (auto i = push.rbegin(); i != push.rend(); i++)
					stack.push(*i);//���򣡣�����ѡʽ��ջ
				stack_str.append(push.rbegin(), push.rend());//���򣡣��������ѡʽ
				left_sen.insert(left_sen.begin() + loc, push.begin(), push.end());//���򣡣���
			}
		}
		//ջ��Ϊ�ս����ƥ�䵯��
		else//��ΪEPS����ֱ�Ӻ���
		{
			if (s[0] == top)
			{
				s.erase(0, 1);//���뵯��
				output = "ƥ��";
			}
		}
		
		//���һ����Ϣ
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
		output.clear();//������
	}
	if (s.size() != 1)
	{
		std::cout << std::endl;
		std::cout << std::setw(10) << "�������!!!!!!!!!!";
		fout << std::endl;
		fout << std::setw(10) << "�������!!!!!!!!!!";
	}
}
