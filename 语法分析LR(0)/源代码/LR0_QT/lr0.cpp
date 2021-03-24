#include"mainwindow.h"
//鉴于终结符号表中没有0、1，符号串中id和num分别用0、1表示
#define ID '0'
#define NUM '1'
//空
#define EPS "?"
//分析表里表示转移状态
#define GO "S"
//前缀位置表示
#define NOTE '.'
//开始符号
char START = 'E';
//sync标识
#define SYNC -1
//ACC
#define ACC "ACC"

//项目集族 树结构
class node
{
public:
    int id;
    std::vector<std::pair<char, std::string>> basic;//基础项目
    std::vector<std::pair<char, std::string>> collection;//闭包项目
    std::vector<std::pair<char, class node*>> next;//go函数目标值集合
    node()
    {
        id = 0;
        basic.clear();
        collection.clear();
        next.clear();
    }
};
typedef class node* node_ptr;

std::map<char, std::vector<std::string>> production;//产生式
std::map<int, std::map<char, std::string>> table_record;//分析表
std::map<char, std::vector<char>> first;//FIRST
std::map<char, std::vector<char>> follow;//FOLLOW
node_ptr head;//项目集族头结点
std::queue<char> extra_non;//额外非终结符
std::vector<char> non_and_ter;//全部符号
int next_id;//树结点编号
std::string already_cover;//为避免树回环造成的回溯，记录已遍历结点不再遍历

void input_grammar();//读入文法
int analyze_pro(std::string);//分解产生式
void find_id_num(std::string&);//将id与num用一个字符'0','1'代替
void ini_extra_non();//初始化各全局变量，并构建额外非终结符（供拓广文法使用）
void extend_grammar();//如果开始符号在多个产生式左侧出现，拓广文法
void show_production();//输出产生式

void construct_closure(node_ptr& now_ptr);//LR(0)项目集规范组的闭包构建
void show_collection(node_ptr now_ptr);//输出项目集族
void construct_table(node_ptr);//构造分析表
void show_ana_table();//输出分析表

char is_about_sum(std::string);//检查是否是待约项目
bool is_sum(std::string);//是规约项目
void note_forward(std::string&);//前缀前移（点前移）
node_ptr check_state_exist(node_ptr, node_ptr);//是否是已有项目集
bool is_non(char);//判断是否为非终结符
bool to_eps(char ch);//判断是否推空

void compute_first();//构造FIRST集
std::vector<char> find_first(char);//递归构造FIRST集
void compute_follow();//构造FOLLOW集
void show_first();//输出FIRST集
void show_follow();//输出FOLLOW集

void input_sentence();//读入句子
void analyze(std::string);//开始进行LR(0)分析

int lr()
{
    ini_extra_non();//初始化各全局变量、初始化辅助额外符号供给
    input_grammar();//读入文法
    extend_grammar();//视情况拓广文法
    show_production();//输出产生式

    head->collection.push_back(std::pair<char, std::string>(START, NOTE + production.find(START)->second[0]));//初始化第一个项目集
    head->basic = head->collection;//项目集族头结点初始化
    construct_closure(head);//构造LR(1)项目及规范族

    already_cover.clear();//防回溯初始化
    show_collection(head);//输出项目集规范族

    compute_first();
    show_first();
    compute_follow();//构造FOLLOW集
    show_follow();//输出FOLLOW集

    already_cover.clear();//防回溯初始化
    construct_table(head);//构造分析表
    show_ana_table();//输出分析表
    input_sentence();//读入句子并分析
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

void extend_grammar()
{
    //开始符号候选式多于一个
    if (production.find(START)->second.size() != 1)
    {
        std::vector<std::string> temp;
        std::string temp2;
        temp2.push_back(START);
        temp.push_back(temp2);
        START = extra_non.front();
        extra_non.pop();//取额外非终结符作为新的开始符号
        non_and_ter.push_back(START);
        //插入新的开始符号产生式
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
    extra_non = std::queue<char>();//初始化
    non_and_ter = { 'E', 'F', 'T', '+', '-', '*', '/', '1', '(', ')' };//全部符号初始化
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

void construct_closure(node_ptr& now_ptr)
{
    //遍历当前项目集，有待约项目则加入NOTE后的非终结符
    std::string already_add;
    for (auto i = now_ptr->collection.begin(); i != now_ptr->collection.end(); i++)
    {
        if (is_about_sum(i->second) != EPS[0])//是待约项目
        {
            char non = is_about_sum(i->second);
            if (already_add.find(non) != already_add.npos)
                continue;//如果这个非终结符的候选式已经被加入过了，直接下一循环
            already_add.push_back(non);//标识这个非终结符的候选式已经加入过了
            for (auto j = production.find(non)->second.begin(); j != production.find(non)->second.end(); j++)
            {
                //添加项目
                std::pair<char, std::string> temp(non, NOTE + (*j));
                now_ptr->collection.push_back(std::pair<char, std::string>(non, NOTE + (*j)));
            }
            i = now_ptr->collection.begin();//重新定位
        }
    }

    //std::cout << "!!!!!!!!!!!!!!!!!!!" << std::endl;
    //遍历所有项目，对于每个非终结符和终结符，寻找go函数，创建新的项目集，递归调用本函数
    for (auto j = non_and_ter.begin(); j != non_and_ter.end(); j++)
    {
        std::vector<std::pair<char, std::string>> temp;
        //对于每个项目
        for (auto i = now_ptr->collection.begin(); i != now_ptr->collection.end(); i++)
        {
            if (is_sum(i->second))//是规约项目,下一循环
                continue;
            if (i->second[i->second.find(NOTE) + 1] == *j)//匹配NOTE后的符号
            {
                std::string temp_str = i->second;
                note_forward(temp_str);//NOTE前移
                //前移后的项目加入到temp
                temp.push_back(std::pair<char, std::string>(i->first, temp_str));
            }
        }
        if (!temp.empty())//有go函数存在
        {
            node_ptr new_ptr;
            new_ptr = new node;
            new_ptr->collection = temp;
            new_ptr->basic = new_ptr->collection;
            //检查此项目集是否已经存在
            if (check_state_exist(head, new_ptr) != NULL)
            {
                now_ptr->next.push_back(std::pair<char, node*>(*j, check_state_exist(head, new_ptr)));
                continue;
            }
            //增加到当前项目的next中(go函数）
            now_ptr->next.push_back(std::pair<char, node*>(*j, new_ptr));
            new_ptr->id = next_id++;
            construct_closure(new_ptr);//树形伸展进行闭包操作和延伸
        }
    }
}

char is_about_sum(std::string s)
{
    auto i = s.find(NOTE);
    if (i != s.size() - 1)
    {
        if (is_non(s[i + 1]))//待约项目
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
    //从头结点广度遍历寻找相同的basic
    if (check->basic == target->basic)
    {
        return check;
    }
    //遍历子节点递归寻找
    already_cover.push_back(static_cast<char>(check->id + 48));//记录已经遍历的结点
    for (auto i = check->next.begin(); i != check->next.end(); i++)
    {
        //如果已经遍历过该节点，进入下一循环
        if (already_cover.find(static_cast<char>(i->second->id + 48)) != already_cover.npos)
            continue;
        //递归遍历子节点
        node_ptr temp = check_state_exist(i->second, target);
        if (temp != NULL)//当前找到了结果就不用遍历子节点，直接返回结果
            return temp;
    }
    return NULL;
}

void show_collection(node_ptr now_ptr)
{
    if (already_cover.find(static_cast<char>(now_ptr->id + 48)) != already_cover.npos)
        return;
    //输出结点id（项目id）（状态id）
    std::cout << now_ptr->id << std::endl;
    //遍历该项目集的项目
    for (auto i = now_ptr->collection.begin(); i != now_ptr->collection.end(); i++)
    {
        std::cout << i->first << "->" << i->second << std::endl;
    }
    std::cout << "--------------------------" << std::endl;
    already_cover.push_back(static_cast<char>(now_ptr->id + 48));
    //遍历go函数目标状态
    for (auto i = now_ptr->next.begin(); i != now_ptr->next.end(); i++)
    {
        show_collection(i->second);
    }
}

void construct_table(node_ptr now_ptr)
{
    std::vector<char> columns{ '(', ')', '*', '+','-', '/', '1','$',  'E', 'F', 'T', START };
    //已经遍历此点，不再遍历
    if (already_cover.find(static_cast<char>(now_ptr->id + 48)) != already_cover.npos)
        return;
    //初始化状态对应分析表记录
    std::map<char, std::string> temp;
    table_record.insert(std::pair<int, std::map<char, std::string>>(now_ptr->id, temp));
    //对于每一个项目集，检查规约项目
    for (auto i = now_ptr->collection.begin(); i != now_ptr->collection.end(); i++)
    {
        if (is_sum(i->second))//是规约项目
        {
            i->second.pop_back();//去除NOTE
            //为每FOLLOW集中的终结符在这个状态下添加规约动作
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
                //为开始符号的规约符号添加ACC
                if (i->first == START)
                {
                    table_record.find(now_ptr->id)->second.insert(std::pair<char, std::string>(*j, ACC));
                }
                table_record.find(now_ptr->id)->second.insert(std::pair<char, std::string>(*j, s));
            }
            i->second.push_back(NOTE);//补回NOTE
        }
    }
    already_cover.push_back(static_cast<char>(now_ptr->id + 48));
    //遍历go函数目标状态
    for (auto i = now_ptr->next.begin(); i != now_ptr->next.end(); i++)
    {
        //标识状态转移
        std::string s;
        s = "shift" + std::to_string(i->second->id);
        table_record.find(now_ptr->id)->second.insert(std::pair<char, std::string>(i->first, s));
        construct_table(i->second);
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

        std::vector<std::string> temp = i->second;
        for (auto ite = temp.begin(); ite != temp.end(); ite++)//对于产生的每个候选式
        {
            if ((*ite)[0] == i->first)//左递归 不考虑
                continue;
            std::vector<char> result = find_first((*ite)[0]);
            //添加结果
            first.find(i->first)->second.insert(first.find(i->first)->second.end(), result.begin(), result.end());
            if (to_eps((*ite)[0]) && ite + 1 != temp.end())
            {
                std::vector<char> result = find_first((*ite)[1]);
                //添加结果
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
    for (auto ite = temp.begin(); ite != temp.end(); ite++)//对于产生的每个候选式
    {
        if ((*ite)[0] == ch)//不考虑左递归
            continue;
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
        std::vector<char> nu;
        nu.clear();
        follow.insert(std::pair<char, std::vector<char>>(i->first, nu));
    }
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
}

void show_first()
{
    std::ofstream fout("first.txt", std::ios::out);
    std::cout << std::endl << "FIRST————————————" << std::endl;
    fout << std::endl << "FIRST————————————" << std::endl;
    for (auto i = first.rbegin(); i != first.rend(); i++)
    {
        std::vector<char> temp;
        temp = i->second;
        //if (!(i->first == 'E' || i->first == 'L'))
            //continue;
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
    std::cout << std::endl << "FOLLOW————————————" << std::endl;
    fout << std::endl << "FOLLOW————————————" << std::endl;
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
    //输出表
    for (auto i = table_record.begin(); i != table_record.end(); i++)//对于每个非终结符的FIRST集
    {
        std::cout << std::setiosflags(std::ios::left) << std::setw(10) << i->first;
        fout << std::setiosflags(std::ios::left) << std::setw(10) << i->first;
        std::map<char, std::string> temp = i->second;
        int now_ter = 0;//当前要输出的终结符
        std::map<int, std::string> out;//记录在第几个输出第几个
        for (auto j = temp.begin(); j != temp.end(); j++)//对于每个FIRST_RECORD集元素
        {
            for (auto k = columns.begin(); k != columns.end(); k++)//对于每个终结符
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
    std::cout << std::setw(6) << "STEP";
    std::cout << std::setw(40) << "STACK";
    std::cout << std::setw(15) << "INPUT";
    std::cout << std::setw(10) << "ACTION";
    fout << std::setw(6) << "STEP";
    fout << std::setw(40) << "STACK";
    fout << std::setw(15) << "INPUT";
    fout << std::setw(10) << "ACTION";
    //初始化
    std::stack<char> stack;
    stack.push('0');//栈初始化
    std::string stack_str;
    stack_str.push_back('0');//字符串表示栈初始化（栈不方便遍历）
    std::string output;//输出初始化
    s.push_back('$');//输入初始化

    //开始分析
    for (int i = 0;; i++)
    {
        //依据栈顶状态以及当前输入指针所指符号进行相应的操作
        //取栈顶状态
        char state = stack.top();
        //取动作
        if (table_record.find(state - 48)->second.find(s[0]) == table_record.find(state - 48)->second.end())//错误情况
        {
            std::cout << std::endl;
            std::cout << std::setw(10) << "ERROR!!!!!!!!!!";
            fout << std::endl;
            fout << std::setw(10) << "ERROR!!!!!!!!!!";
            return;
        }
        output = table_record.find(state - 48)->second.find(s[0])->second;

        //输出一行信息
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

        if (output == ACC)//ACC结束句子分析
        {
            std::cout << "Analyzed successfully" << std::endl;
            fout << "Analyzed successfully" << std::endl;
            return;
        }

        //实行ACTION
        if (output.find("shift") != output.npos)//移进动作
        {
            int next_state = std::atoi(output.substr(5).c_str());
            stack.push(s[0]);
            stack_str += s[0];
            s.erase(0, 1);
            stack.push(static_cast<char>(next_state + 48));
            stack_str += "'" + std::to_string(next_state) + "'";
        }
        else//规约动作
        {
            //后序规约字符串
            //从栈中弹出
            char new_symbol = output[0];
            output = output.substr(3);//将产生式的左右分离
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
                }//消除stack_str最后的状态
                if (stack.top() == *j)//规约
                {
                    stack.pop();
                    stack_str.pop_back();
                }
                else//规约出现错误
                {
                    std::cout << "Wrong!!!!!!!!!summary";
                    fout << "Wrong!!!!!!!!!summary";
                    return;
                }
            }
            //根据栈顶状态和规约产生式左侧符号进行状态转移
            if (table_record.find(stack.top() - 48)->second.find(new_symbol) == table_record.find(stack.top() - 48)->second.end())//错误情况
            {
                std::cout << std::endl;
                std::cout << std::setw(10) << "ERROR!!!!!!!!!!";
                fout << std::endl;
                fout << std::setw(10) << "ERROR!!!!!!!!!!";
                return;
            }
            //规约后对于栈顶符号和新加入的符号进行状态转移
            output = table_record.find(stack.top() - 48)->second.find(new_symbol)->second;
            int next_state = std::atoi(output.substr(5).c_str());
            stack.push(new_symbol);
            stack_str += new_symbol;
            stack.push(static_cast<char>(next_state + 48));
            stack_str += "'" + std::to_string(next_state) + "'";
        }
        output.clear();//清除输出
    }
}
