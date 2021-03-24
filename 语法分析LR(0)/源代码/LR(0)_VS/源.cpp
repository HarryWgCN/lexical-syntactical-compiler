#include<iostream>
#include<fstream>
#include<vector>
#include<queue>
#include<map>
#include<set>
#include<stack>
#include <iomanip>
#include<string>

//�����ս���ű���û��0��1�����Ŵ���id��num�ֱ���0��1��ʾ
#define ID '0'
#define NUM '1'
//��
#define EPS "?"
//���������ʾת��״̬
#define GO "S"
//ǰ׺λ�ñ�ʾ
#define NOTE '.'
//��ʼ����
char START = 'E';
//sync��ʶ
#define SYNC -1
//ACC
#define ACC "ACC"

//��Ŀ���� ���ṹ
class node
{
public:
	int id;
	std::vector<std::pair<char, std::string>> basic;//������Ŀ
	std::vector<std::pair<char, std::string>> collection;//�հ���Ŀ
	std::vector<std::pair<char, class node*>> next;//go����Ŀ��ֵ����
	node()
	{
		id = 0;
		basic.clear();
		collection.clear();
		next.clear();
	}
};
typedef class node* node_ptr;

std::map<char, std::vector<std::string>> production;//����ʽ
std::map<int, std::map<char, std::string>> table_record;//������
std::map<char, std::vector<char>> first;//FIRST
std::map<char, std::vector<char>> follow;//FOLLOW
node_ptr head;//��Ŀ����ͷ���
std::queue<char> extra_non;//������ս��
std::vector<char> non_and_ter;//ȫ������
int next_id;//�������
std::string already_cover;//Ϊ�������ػ���ɵĻ��ݣ���¼�ѱ�����㲻�ٱ���

void input_grammar();//�����ķ�
int analyze_pro(std::string);//�ֽ����ʽ
void find_id_num(std::string&);//��id��num��һ���ַ�'0','1'����
void ini_extra_non();//��ʼ����ȫ�ֱ�����������������ս�������ع��ķ�ʹ�ã�
void extend_grammar();//�����ʼ�����ڶ������ʽ�����֣��ع��ķ�
void show_production();//�������ʽ

void construct_closure(node_ptr& now_ptr);//LR(0)��Ŀ���淶��ıհ�����
void show_collection(node_ptr now_ptr);//�����Ŀ����
void construct_table(node_ptr);//���������
void show_ana_table();//���������

char is_about_sum(std::string);//����Ƿ��Ǵ�Լ��Ŀ
bool is_sum(std::string);//�ǹ�Լ��Ŀ
void note_forward(std::string&);//ǰ׺ǰ�ƣ���ǰ�ƣ�
node_ptr check_state_exist(node_ptr, node_ptr);//�Ƿ���������Ŀ��
bool is_non(char);//�ж��Ƿ�Ϊ���ս��
bool to_eps(char ch);//�ж��Ƿ��ƿ�

void compute_first();//����FIRST��
std::vector<char> find_first(char);//�ݹ鹹��FIRST��
void compute_follow();//����FOLLOW��
void show_first();//���FIRST��
void show_follow();//���FOLLOW��

void input_sentence();//�������
void analyze(std::string);//��ʼ����LR(0)����

int main()
{
	ini_extra_non();//��ʼ����ȫ�ֱ�������ʼ������������Ź���
	input_grammar();//�����ķ�
	extend_grammar();//������ع��ķ�
	show_production();//�������ʽ

	head->collection.push_back(std::pair<char, std::string>(START, NOTE + production.find(START)->second[0]));//��ʼ����һ����Ŀ��
	head->basic = head->collection;//��Ŀ����ͷ����ʼ��
	construct_closure(head);//����LR(1)��Ŀ���淶��

	already_cover.clear();//�����ݳ�ʼ��
	show_collection(head);//�����Ŀ���淶��

	compute_first();
	show_first();
	compute_follow();//����FOLLOW��
	show_follow();//���FOLLOW��

	already_cover.clear();//�����ݳ�ʼ��
	construct_table(head);//���������
	show_ana_table();//���������
	input_sentence();//������Ӳ�����
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

void extend_grammar()
{
	//��ʼ���ź�ѡʽ����һ��
	if (production.find(START)->second.size() != 1)
	{
		std::vector<std::string> temp;
		std::string temp2;
		temp2.push_back(START);
		temp.push_back(temp2);
		START = extra_non.front();
		extra_non.pop();//ȡ������ս����Ϊ�µĿ�ʼ����
		non_and_ter.push_back(START);
		//�����µĿ�ʼ���Ų���ʽ
		production.insert(std::pair<char, std::vector<std::string>>(START, temp));
	}
}

void ini_extra_non()
{
	production.clear();
	table_record.clear();
	START = 'E';
	next_id = 0;
	head = new class node;
	head->id = next_id++;
	extra_non = std::queue<char>();//��ʼ��
	non_and_ter = { 'E', 'F', 'T', '+', '-', '*', '/', '1', '(', ')' };//ȫ�����ų�ʼ��
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

void construct_closure(node_ptr& now_ptr)
{
	//������ǰ��Ŀ�����д�Լ��Ŀ�����NOTE��ķ��ս��
	std::string already_add;
	for (auto i = now_ptr->collection.begin(); i != now_ptr->collection.end(); i++)
	{
		if (is_about_sum(i->second) != EPS[0])//�Ǵ�Լ��Ŀ
		{
			char non = is_about_sum(i->second);
			if (already_add.find(non) != already_add.npos)
				continue;//���������ս���ĺ�ѡʽ�Ѿ���������ˣ�ֱ����һѭ��
			already_add.push_back(non);//��ʶ������ս���ĺ�ѡʽ�Ѿ��������
			for (auto j = production.find(non)->second.begin(); j != production.find(non)->second.end(); j++)
			{
				//�����Ŀ
				std::pair<char, std::string> temp(non, NOTE + (*j));
				now_ptr->collection.push_back(std::pair<char, std::string>(non, NOTE + (*j)));
			}
			i = now_ptr->collection.begin();//���¶�λ
		}
	}

	//std::cout << "!!!!!!!!!!!!!!!!!!!" << std::endl;
	//����������Ŀ������ÿ�����ս�����ս����Ѱ��go�����������µ���Ŀ�����ݹ���ñ�����
	for (auto j = non_and_ter.begin(); j != non_and_ter.end(); j++)
	{
		std::vector<std::pair<char, std::string>> temp;
		//����ÿ����Ŀ
		for (auto i = now_ptr->collection.begin(); i != now_ptr->collection.end(); i++)
		{
			if (is_sum(i->second))//�ǹ�Լ��Ŀ,��һѭ��
				continue;
			if (i->second[i->second.find(NOTE) + 1] == *j)//ƥ��NOTE��ķ���
			{
				std::string temp_str = i->second;
				note_forward(temp_str);//NOTEǰ��
				//ǰ�ƺ����Ŀ���뵽temp
				temp.push_back(std::pair<char, std::string>(i->first, temp_str));
			}
		}
		if (!temp.empty())//��go��������
		{
			node_ptr new_ptr;
			new_ptr = new node;
			new_ptr->collection = temp;
			new_ptr->basic = new_ptr->collection;
			//������Ŀ���Ƿ��Ѿ�����
			if (check_state_exist(head, new_ptr) != NULL)
			{
				now_ptr->next.push_back(std::pair<char, node*>(*j, check_state_exist(head, new_ptr)));
				continue;
			}
			//���ӵ���ǰ��Ŀ��next��(go������
			now_ptr->next.push_back(std::pair<char, node*>(*j, new_ptr));
			new_ptr->id = next_id++;
			construct_closure(new_ptr);//������չ���бհ�����������
		}
	}
}

char is_about_sum(std::string s)
{
	auto i = s.find(NOTE);
	if (i != s.size() - 1)
	{
		if (is_non(s[i + 1]))//��Լ��Ŀ
			return s[i + 1];
	}
	return EPS[0];
}

bool is_sum(std::string s)
{
	if (s.find(NOTE) == s.size() - 1)
		return true;
	return false;
}

void note_forward(std::string& s)
{
	int i = s.find(NOTE);
	s.erase(i, 1);
	s.insert(i + 1, 1, NOTE);
}

node_ptr check_state_exist(node_ptr check, node_ptr target)
{
	if (check == head)
		already_cover.clear();
	//��ͷ����ȱ���Ѱ����ͬ��basic
	if (check->basic == target->basic)
	{
		return check;
	}
	//�����ӽڵ�ݹ�Ѱ��
	already_cover.push_back(static_cast<char>(check->id + 48));//��¼�Ѿ������Ľ��
	for (auto i = check->next.begin(); i != check->next.end(); i++)
	{
		//����Ѿ��������ýڵ㣬������һѭ��
		if (already_cover.find(static_cast<char>(i->second->id + 48)) != already_cover.npos)
			continue;
		//�ݹ�����ӽڵ�
		node_ptr temp = check_state_exist(i->second, target);
		if (temp != NULL)//��ǰ�ҵ��˽���Ͳ��ñ����ӽڵ㣬ֱ�ӷ��ؽ��
			return temp;
	}
	return NULL;
}

void show_collection(node_ptr now_ptr)
{
	if (already_cover.find(static_cast<char>(now_ptr->id + 48)) != already_cover.npos)
		return;
	//������id����Ŀid����״̬id��
	std::cout << now_ptr->id << std::endl;
	//��������Ŀ������Ŀ
	for (auto i = now_ptr->collection.begin(); i != now_ptr->collection.end(); i++)
	{
		std::cout << i->first << "->" << i->second << std::endl;
	}
	std::cout << "--------------------------" << std::endl;
	already_cover.push_back(static_cast<char>(now_ptr->id + 48));
	//����go����Ŀ��״̬
	for (auto i = now_ptr->next.begin(); i != now_ptr->next.end(); i++)
	{
		show_collection(i->second);
	}
}

void construct_table(node_ptr now_ptr)
{
	std::vector<char> columns{ '(', ')', '*', '+','-', '/', '1','$',  'E', 'F', 'T', START };
	//�Ѿ������˵㣬���ٱ���
	if (already_cover.find(static_cast<char>(now_ptr->id + 48)) != already_cover.npos)
		return;
	//��ʼ��״̬��Ӧ�������¼
	std::map<char, std::string> temp;
	table_record.insert(std::pair<int, std::map<char, std::string>>(now_ptr->id, temp));
	//����ÿһ����Ŀ��������Լ��Ŀ
	for (auto i = now_ptr->collection.begin(); i != now_ptr->collection.end(); i++)
	{
		if (is_sum(i->second))//�ǹ�Լ��Ŀ
		{
			i->second.pop_back();//ȥ��NOTE
			//ΪÿFOLLOW���е��ս�������״̬����ӹ�Լ����
			for (auto j = columns.begin(); j != columns.end(); j++)
			{
				if (is_non(*j))
					continue;
				int find = 0;
				for (auto k = follow.find(i->first)->second.begin(); k != follow.find(i->first)->second.end(); k++)
				{
					if (*k == *j)
						find = 1;
				}
				if (find == 0)
					continue;
				std::string s;
				s.push_back(i->first);
				s += "->" + i->second;
				//Ϊ��ʼ���ŵĹ�Լ�������ACC
				if (i->first == START)
				{
					table_record.find(now_ptr->id)->second.insert(std::pair<char, std::string>(*j, ACC));
				}
				table_record.find(now_ptr->id)->second.insert(std::pair<char, std::string>(*j, s));
			}
			i->second.push_back(NOTE);//����NOTE
		}
	}
	already_cover.push_back(static_cast<char>(now_ptr->id + 48));
	//����go����Ŀ��״̬
	for (auto i = now_ptr->next.begin(); i != now_ptr->next.end(); i++)
	{
		//��ʶ״̬ת��
		std::string s;
		s = "shift" + std::to_string(i->second->id);
		table_record.find(now_ptr->id)->second.insert(std::pair<char, std::string>(i->first, s));
		construct_table(i->second);
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

		std::vector<std::string> temp = i->second;
		for (auto ite = temp.begin(); ite != temp.end(); ite++)//���ڲ�����ÿ����ѡʽ
		{
			if ((*ite)[0] == i->first)//��ݹ� ������
				continue;
			std::vector<char> result = find_first((*ite)[0]);
			//��ӽ��
			first.find(i->first)->second.insert(first.find(i->first)->second.end(), result.begin(), result.end());
			if (to_eps((*ite)[0]) && ite + 1 != temp.end())
			{
				std::vector<char> result = find_first((*ite)[1]);
				//��ӽ��
				first.find(i->first)->second.insert(first.find(i->first)->second.end(), result.begin(), result.end());
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
		if ((*ite)[0] == ch)//��������ݹ�
			continue;
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
		std::vector<char> nu;
		nu.clear();
		follow.insert(std::pair<char, std::vector<char>>(i->first, nu));
	}
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
}

void show_first()
{
	std::ofstream fout("first.txt", std::ios::out);
	std::cout << std::endl << "FIRST������������������������" << std::endl;
	fout << std::endl << "FIRST������������������������" << std::endl;
	for (auto i = first.rbegin(); i != first.rend(); i++)
	{
		std::vector<char> temp;
		temp = i->second;
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
	std::cout << std::endl << "FOLLOW������������������������" << std::endl;
	fout << std::endl << "FOLLOW������������������������" << std::endl;
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
	std::vector<char> columns{ '(', ')', '*', '+','-', '/', '1','$',  'E', 'F', 'T'};
	std::cout << std::setw(10) << " ";
	fout << std::setw(10) << " ";
	for (auto i = columns.begin(); i != columns.end(); i++)
	{
		std::cout << std::setiosflags(std::ios::left) << std::setw(10) << *i;
		fout << std::setiosflags(std::ios::left) << std::setw(10) << *i;
	}
	std::cout << std::endl;
	fout << std::endl;
	//�����
	for (auto i = table_record.begin(); i != table_record.end(); i++)//����ÿ�����ս����FIRST��
	{
		std::cout << std::setiosflags(std::ios::left) << std::setw(10) << i->first;
		fout << std::setiosflags(std::ios::left) << std::setw(10) << i->first;
		std::map<char, std::string> temp = i->second;
		int now_ter = 0;//��ǰҪ������ս��
		std::map<int, std::string> out;//��¼�ڵڼ�������ڼ���
		for (auto j = temp.begin(); j != temp.end(); j++)//����ÿ��FIRST_RECORD��Ԫ��
		{
			for (auto k = columns.begin(); k != columns.end(); k++)//����ÿ���ս��
			{
				if (j->first == *k)
				{
					out.insert(std::pair<int, std::string>(static_cast<int>(k - columns.begin()), j->second));
					break;
				}
			}
		}
		for (int k = 0; k < columns.size(); k++)
		{
			if (out.find(k) != out.end())
			{
				std::cout << std::setiosflags(std::ios::left) << std::setw(10) << out.find(k)->second;
				fout << std::setiosflags(std::ios::left) << std::setw(10) << out.find(k)->second;
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

bool to_eps(char ch)
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

bool is_non(char ch)
{
	if (ch >= 65 && ch <= 90)
		return 1;
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
	std::cout << std::setw(6) << "STEP";
	std::cout << std::setw(40) << "STACK";
	std::cout << std::setw(15) << "INPUT";
	std::cout << std::setw(10) << "ACTION";
	fout << std::setw(6) << "STEP";
	fout << std::setw(40) << "STACK";
	fout << std::setw(15) << "INPUT";
	fout << std::setw(10) << "ACTION";
	//��ʼ��
	std::stack<char> stack;
	stack.push('0');//ջ��ʼ��
	std::string stack_str;
	stack_str.push_back('0');//�ַ�����ʾջ��ʼ����ջ�����������
	std::string output;//�����ʼ��
	s.push_back('$');//�����ʼ��

	//��ʼ����
	for (int i = 0;; i++)
	{
		//����ջ��״̬�Լ���ǰ����ָ����ָ���Ž�����Ӧ�Ĳ���
		//ȡջ��״̬
		char state = stack.top();
		//ȡ����
		if (table_record.find(state - 48)->second.find(s[0]) == table_record.find(state - 48)->second.end())//�������
		{
			std::cout << std::endl;
			std::cout << std::setw(10) << "ERROR!!!!!!!!!!";
			fout << std::endl;
			fout << std::setw(10) << "ERROR!!!!!!!!!!";
			return;
		}
		output = table_record.find(state - 48)->second.find(s[0])->second;

		//���һ����Ϣ
		std::cout << std::endl;
		std::cout << std::setw(6) << i;
		std::cout << std::setw(40) << stack_str;
		std::cout << std::setw(15) << s;
		std::cout << std::setw(10) << output;

		fout << std::endl;
		fout << std::setw(6) << i;
		fout << std::setw(40) << stack_str;
		fout << std::setw(15) << s;
		fout << std::setw(10) << output;

		if (output == ACC)//ACC�������ӷ���
		{
			std::cout << "Analyzed successfully" << std::endl;
			fout << "Analyzed successfully" << std::endl;
			return;
		}

		//ʵ��ACTION
		if (output.find("shift") != output.npos)//�ƽ�����
		{
			int next_state = std::atoi(output.substr(5).c_str());
			stack.push(s[0]);
			stack_str += s[0];
			s.erase(0, 1);
			stack.push(static_cast<char>(next_state + 48));
			stack_str += "'" + std::to_string(next_state) + "'";
		}
		else//��Լ����
		{
			//�����Լ�ַ���
			//��ջ�е���
			char new_symbol = output[0];
			output = output.substr(3);//������ʽ�����ҷ���
			for (auto j = output.rbegin(); j != output.rend(); j++)
			{
				stack.pop();
				while (1)
				{
					stack_str.pop_back();
					if (stack_str.back() == '\'')
					{
						stack_str.pop_back();
						break;
					}
				}//����stack_str����״̬
				if (stack.top() == *j)//��Լ
				{
					stack.pop();
					stack_str.pop_back();
				}
				else//��Լ���ִ���
				{
					std::cout << "Wrong!!!!!!!!!summary";
					fout << "Wrong!!!!!!!!!summary";
					return;
				}
			}
			//����ջ��״̬�͹�Լ����ʽ�����Ž���״̬ת��
			if (table_record.find(stack.top() - 48)->second.find(new_symbol) == table_record.find(stack.top() - 48)->second.end())//�������
			{
				std::cout << std::endl;
				std::cout << std::setw(10) << "ERROR!!!!!!!!!!";
				fout << std::endl;
				fout << std::setw(10) << "ERROR!!!!!!!!!!";
				return;
			}
			//��Լ�����ջ�����ź��¼���ķ��Ž���״̬ת��
			output = table_record.find(stack.top() - 48)->second.find(new_symbol)->second;
			int next_state = std::atoi(output.substr(5).c_str());
			stack.push(new_symbol);
			stack_str += new_symbol;
			stack.push(static_cast<char>(next_state + 48));
			stack_str += "'" + std::to_string(next_state) + "'";
		}
		output.clear();//������
	}
}
