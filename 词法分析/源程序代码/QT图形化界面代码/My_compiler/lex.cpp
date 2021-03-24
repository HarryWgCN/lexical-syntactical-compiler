#include"mainwindow.h"

struct Domain
{
    int d_id;//作用域编号
    int layer;//作用域层号（树结构）
    std::vector<std::string> id_list;//此作用域中的标识符列表（不包括子作用域）
    Domain* preceding;//上层节点
    std::vector<Domain*> subsequent;//下层节点
};
typedef Domain* Domain_ptr;

void handle_string(char [], int , std::ofstream&, std::ofstream& );//识别词组
void handle_identifier_num(std::string, std::ofstream&, std::ofstream&);//识别标识符
void ini_head_of_domain();//初始化全局作用域（根节点）
void domain_back();//返回上级域
void domain_derive();//派生出新的域
int table_insert(std::string);//插入标识符到符号表并返回指针（整形编号形式）
int next_d_id = 0;//下一个作用域编号（待分配）
int now_layer = 0;//初始化基于作用域符号表的编号
Domain_ptr root;//相当于全局作用域
Domain_ptr current_domain;//当前作用域
int count_keyword = 0;
int count_identifier = 0;
int count_num = 0;
int count_parenthesis = 0;
int count_operator = 0;
int count_line = 0;
int state = 4;//判断当前正在读的非符号单词是：
//未识别符号（0）、整数（1）、标识符（2）、小数（3）、已识别符号（4）、
//'//'注释未结束状态(5)、'/*'注释未结束状态(6)、

void lex()
{
    int end = 0;
    std::ifstream in_file("a.txt ", std::ios::in);//输入文件
    std::ofstream result_file("result.txt", std::ios::out);//分析输出文件
    std::ofstream adjust_file("adjust.txt", std::ios::out);//更正输出文件
    ini_head_of_domain();
    count_keyword = 0;
    count_identifier = 0;
    count_num = 0;
    count_parenthesis = 0;
    count_operator = 0;
    count_line = 0;
    while (1)
    {
        int len = 0;
        char a[100];
        if (end == 1)//读到文件尾结束读入
            break;
        //以空格和换行符为界限读入一个词组（可能为一个单词或一个类似于‘main（）’的符号组）
        for (int i = 0; ; i++)
        {
            a[i] = in_file.get();//输入文件读取到数组a[]中
            len++;
            if (in_file.eof())//当读到文件尾，置结束信号为1
            {
                end = 1;
                len--;
                break;
            }
            if (a[i] == ' ' || a[i] == '\n')//当读到空格或者回车，停止读入
                break;
        }
        if (state != 5 && state != 6)
            state = 4;
        handle_string(a, len, result_file, adjust_file);//开始识别读到的词组
    }
    std::cout << "编译完毕"; //命令行界面显示,则词法识别成功

    //输出统计信息
    result_file << "Statistical Data:" << std::endl;
    result_file << "Keywords : " << count_keyword << std::endl;
    result_file << "Identifiers : " << count_identifier << std::endl;
    result_file << "Numbers : " << count_num << std::endl;
    result_file << "Parentheses : " << count_parenthesis << std::endl;
    result_file << "Operators : " << count_operator << std::endl;
    result_file << "Lines : " << count_line + 1 << std::endl;
    int total;
    total = count_keyword + count_identifier + count_operator + count_num + count_parenthesis;
    result_file << "Total characters : " <<  total  << std::endl;
    in_file.close();//关闭文件
    result_file.close();//关闭文件
}

void handle_string(char a[], int len, std::ofstream &result_file, std::ofstream& adjust_file)
{
    std::string b;//存放识别出的长度大于一的标识符，非标识符符号直接处理
    b.clear();//初始化
    for (int i = 0; i < len; i++)//开始进行数组a[]内字符识别
    {
        if (state == 5)
        {
            if (a[i] == '\n')//结束'\\'注释
                state = 4;
            else//注释未结束
            {
                adjust_file << a[i];
                continue;
            }
        }
        if (state == 6)
        {
            if (a[i] == '*' && a[i + 1] == '/')//结束/*注释
                state = 4;
            else//注释未结束
            {
                adjust_file << a[i];
                continue;
            }
        }
        //识别符号
        count_operator++;//预先加上计数
        switch (a[i])
        {
            case '{'://识别符号'{'
            {
                domain_derive();//分化作用域
                count_operator--;
                count_parenthesis++;//括号计数加一，减去预先加上计数
                if (!b.empty())//若已经存放了标识符，先处理标识符
                //若规范书写（标识符和运算符间有空格），可以不用这一条，但可以考虑多种情况）
                {
                    handle_identifier_num(b, result_file, adjust_file);
                    b.clear();
                }
                b.push_back(a[i]);
                result_file << "<parenthesis," << b << ">" << std::endl;
                adjust_file << b;
                b.clear();
            }; break;

            case '}'://识别符号 '}'
            {
                domain_back();//返回上级作用域
                count_operator--;
                count_parenthesis++;//括号计数加一，减去预先加上计数
                if (!b.empty())//若已经存放了标识符，先处理标识符
                {
                    handle_identifier_num(b, result_file, adjust_file);
                    b.clear();
                }
                b.push_back(a[i]);
                result_file << "<parenthesis," << b << ">" << std::endl;
                adjust_file << b;
                b.clear();
            }; break;

            case '('://识别符号'('
            {
                count_operator--;
                count_parenthesis++;//括号计数加一，减去预先加上计数
                if (!b.empty())//若已经存放了标识符，先处理标识符
                {
                    handle_identifier_num(b, result_file, adjust_file);
                    b.clear();
                }
                b.push_back(a[i]);
                result_file << "<parenthesis," << b << ">" << std::endl;
                adjust_file << b;
                b.clear();
            }; break;

            case ')'://识别符号')'
            {
                count_operator--;
                count_parenthesis++;//括号计数加一，减去预先加上计数
                if (!b.empty())//若已经存放了标识符，先处理标识符
                {
                    handle_identifier_num(b, result_file, adjust_file);
                    b.clear();
                }
                b.push_back(a[i]);
                result_file << "<parenthesis," << b << ">" << std::endl;
                adjust_file << b;
                b.clear();
            }; break;

            case '+'://识别以符号'+'开头的的符号
            {
                if (!b.empty())//若已经存放了标识符，先处理标识符
                {
                    handle_identifier_num(b, result_file, adjust_file);
                    b.clear();
                }
                if (a[i + 1] == '=')//识别符号'+='
                {
                    b.push_back(a[i]);
                    b.push_back(a[i + 1]);
                    i++;
                    result_file << "<arith_op," << b << ">" << std::endl;
                    adjust_file << b;
                    b.clear();
                }
                else if (a[i + 1] == '+')//识别符号'++'
                {
                    b.push_back(a[i]);
                    b.push_back(a[i + 1]);
                    i++;
                    result_file << "<arith_op," << b << ">" << std::endl;
                    adjust_file << b;
                    b.clear();
                }
                else//识别符号'+'
                {
                    b.push_back(a[i]);
                    result_file << "<arith_op," << b << ">" << std::endl;
                    adjust_file << b;
                    b.clear();
                }
            }; break;

            case '-'://识别以符号'-'开头的的符号
            {
                if(!b.empty())//若已经存放了标识符，先处理标识符
                {
                    handle_identifier_num(b, result_file, adjust_file);
                    b.clear();
                }
                if (a[i + 1] == '>')//识别符号'->'
                {
                    b.push_back(a[i]);
                    b.push_back(a[i + 1]);
                    i++;
                    result_file << "<struct_op," << b << ">" << std::endl;
                    adjust_file << b;
                    b.clear();
                }
                else if (a[i + 1] == '=')//识别符号'-='
                {
                    b.push_back(a[i]);
                    b.push_back(a[i + 1]);
                    i++;
                    result_file << "<arith_op," << b << ">" << std::endl;
                    adjust_file << b;
                    b.clear();
                }
                else if (a[i + 1] == '-')//识别符号'--'
                {
                    b.push_back(a[i]);
                    b.push_back(a[i + 1]);
                    i++;
                    result_file << "<arith_op," << b << ">" << std::endl;
                    adjust_file << b;
                    b.clear();
                }
                else//识别符号'-'
                {
                    b.push_back(a[i]);
                    result_file << "<arith_op," << b << ">" << std::endl;
                    adjust_file << b;
                    b.clear();
                }
            }; break;
            case '*'://识别以符号'*'开头的的符号
            {
                if (!b.empty())//若已经存放了标识符，先处理标识符
                {
                    handle_identifier_num(b, result_file, adjust_file);
                    b.clear();
                }
                if (a[i + 1] == '=')//识别符号'*='
                {
                    b.push_back(a[i]);
                    b.push_back(a[i + 1]);
                    i++;
                    result_file << "<arith_op," << b << ">" << std::endl;
                    adjust_file << b;
                    b.clear();
                }
                else if (a[i + 1] == '/')//识别符号'*/'
                {
                    b.push_back(a[i]);
                    b.push_back(a[i + 1]);
                    i++;
                    result_file << "<comment," << b << ">" << std::endl;
                    adjust_file << b;
                    b.clear();
                }
                //*后为字母，则认为是指针变量
                else if ((a[i + 1] >= 'a' && a[i + 1] <= 'z') || (a[i + 1] >= 'A' && a[i + 1] <= 'Z') || a[i + 1] == '.')//识别符号'*/'
                {
                    b.push_back(a[i]);
                    result_file << "<pointer_op," << b << ">" << std::endl;
                    adjust_file << b;
                    b.clear();
                }
                else//识别符号'*'
                {
                    b.push_back(a[i]);
                    result_file << "<arith_op," << b << ">" << std::endl;
                    adjust_file << b;
                    b.clear();
                }
            }; break;
            case '|'://识别以符号'|'开头的的符号
            {
                if (!b.empty())//若已经存放了标识符，先处理标识符
                {
                    handle_identifier_num(b, result_file, adjust_file);
                    b.clear();
                }
                if (a[i + 1] == '=')//识别符号'|='
                {
                    b.push_back(a[i]);
                    b.push_back(a[i + 1]);
                    i++;
                    result_file << "<arith_op," << b << ">" << std::endl;
                    adjust_file << b;
                    b.clear();
                }
                else if (a[i + 1] == '|')//识别符号'||'
                {
                    b.push_back(a[i]);
                    b.push_back(a[i + 1]);
                    i++;
                    result_file << "<log_op," << b << ">" << std::endl;
                    adjust_file << b;
                    b.clear();
                }
                else//识别符号'|'
                {
                    b.push_back(a[i]);
                    result_file << "<arith_op," << b << ">" << std::endl;
                    adjust_file << b;
                    b.clear();
                }
            }; break;
            case '&'://识别以符号'&'开头的的符号
            {
                if (!b.empty())//若已经存放了标识符，先处理标识符
                {
                    handle_identifier_num(b, result_file, adjust_file);
                    b.clear();
                }
                if (a[i + 1] == '=')//识别符号'&='
                {
                    b.push_back(a[i]);
                    b.push_back(a[i + 1]);
                    i++;
                    result_file << "<arith_op," << b << ">" << std::endl;
                    adjust_file << b;
                    b.clear();
                }
                else if (a[i + 1] == '&')//识别符号'&&'
                {
                    b.push_back(a[i]);
                    b.push_back(a[i + 1]);
                    i++;
                    result_file << "<log_op," << b << ">" << std::endl;
                    adjust_file << b;
                    b.clear();
                }
                //后跟字母，判断为取地址符
                else if ((a[i + 1] >= 'a' && a[i + 1] <= 'z') || (a[i + 1] >= 'A' && a[i + 1] <= 'Z') || a[i + 1] == '.')//识别符号'*/'
                {
                    b.push_back(a[i]);
                    result_file << "<loc_op," << b << ">" << std::endl;
                    adjust_file << b;
                    b.clear();
                }
                else//识别符号'&'
                {
                    b.push_back(a[i]);
                    result_file << "<arith_op," << b << ">" << std::endl;
                    adjust_file << b;
                    b.clear();
                }
            }; break;
            case '^'://识别以符号'^'开头的的符号
            {
                if (!b.empty())//若已经存放了标识符，先处理标识符
                {
                    handle_identifier_num(b, result_file, adjust_file);
                    b.clear();
                }
                if (a[i + 1] == '=')//识别符号'^='
                {
                    b.push_back(a[i]);
                    b.push_back(a[i + 1]);
                    i++;
                    result_file << "<arith_op," << b << ">" << std::endl;
                    adjust_file << b;
                    b.clear();
                }
                else//识别符号'^'
                {
                    b.push_back(a[i]);
                    result_file << "<arith_op," << b << ">" << std::endl;
                    adjust_file << b;
                    b.clear();
                }
            }; break;
            case '/'://识别以符号'/'开头的的符号
            {
                if (!b.empty())//若已经存放了标识符，先处理标识符
                {
                    handle_identifier_num(b, result_file, adjust_file);
                    b.clear();
                }
                if (a[i + 1] == '=')//识别符号'/='
                {
                    b.push_back(a[i]);
                    b.push_back(a[i + 1]);
                    i++;
                    result_file << "<arith_op," << b << ">" << std::endl;
                    adjust_file << b;
                    b.clear();
                }
                else if (a[i + 1] == '/') // 识别'//'
                {
                    //忽略注释
                    b.push_back(a[i]);
                    b.push_back(a[i + 1]);
                    i++;
                    state = 5;//置为//注释未结束状态
                    result_file << "<coment," << b << ">";
                    result_file << " Comments here." << std::endl;
                    adjust_file << b;
                    b.clear();
                }
                else if (a[i + 1] == '*') // 识别'/*'
                {
                    //忽略注释
                    b.push_back(a[i]);
                    b.push_back(a[i + 1]);
                    i++;
                    state = 6;//置为/*注释未结束状态
                    result_file << "<comment," << b << ">";
                    result_file << " Comments here." << std::endl;
                    adjust_file << b;
                    b.clear();
                }
                else//识别'/'
                {
                    b.push_back(a[i]);
                    result_file << "<arith_op," << b << ">" << std::endl;
                    adjust_file << b;
                    b.clear();
                }
            }; break;

            case ';'://识别符号';'
            {
                if (!b.empty())//若已经存放了标识符，先处理标识符
                {
                    handle_identifier_num(b, result_file, adjust_file);
                    b.clear();
                }
                b.push_back(a[i]);
                result_file << "<delimiter," << b << ">" << std::endl;
                adjust_file << b;
                b.clear();
            }; break;

            case '<'://识别以符号'<'开头的的符号
            {
                if (!b.empty())//若已经存放了标识符，先处理标识符
                {
                    handle_identifier_num(b, result_file, adjust_file);
                    b.clear();
                }
                if (a[i + 1] == '=')//识别符号'<='
                {
                    b.push_back(a[i]);
                    b.push_back(a[i + 1]);
                    i++;
                    result_file << "<comp_op," << b << ">" << std::endl;
                    adjust_file << b;
                    b.clear();
                }
                else if (a[i + 1] == '=')//识别符号'<<'
                {
                    b.push_back(a[i]);
                    b.push_back(a[i + 1]);
                    i++;
                    result_file << "<shift_op," << b << ">" << std::endl;
                    adjust_file << b;
                    b.clear();
                }
                else//识别符号'<'
                {
                    b.push_back(a[i]);
                    result_file << "<comp_op," << b << ">" << std::endl;
                    adjust_file << b;
                    b.clear();
                }
            }; break;

            case '>'://识别以符号'>'开头的的符号
            {
                if (!b.empty())//若已经存放了标识符，先处理标识符
                {
                    handle_identifier_num(b, result_file, adjust_file);
                    b.clear();
                }
                if (a[i + 1] == '=')//识别符号'>='
                {
                    b.push_back(a[i]);
                    b.push_back(a[i + 1]);
                    i++;
                    result_file << "<comp_op," << b << ">" << std::endl;
                    adjust_file << b;
                    b.clear();
                }
                else if (a[i + 1] == '>')//识别符号'>>'
                {
                    b.push_back(a[i]);
                    b.push_back(a[i + 1]);
                    i++;
                    result_file << "<shift_op," << b << ">" << std::endl;
                    adjust_file << b;
                    b.clear();
                }
                else//识别符号'>'
                {
                    b.push_back(a[i]);
                    result_file << "<comp_op," << b << ">" << std::endl;
                    adjust_file << b;
                    b.clear();
                }
            }; break;

            case '='://识别以符号'='开头的的符号
            {
                if (!b.empty())//若已经存放了标识符，先处理标识符
                {
                    handle_identifier_num(b, result_file, adjust_file);
                    b.clear();
                }
                if (a[i + 1] == '=')//识别符号'=='
                {
                    b.push_back(a[i]);
                    b.push_back(a[i + 1]);
                    i++;
                    result_file << "<comp_op," << b << ">" << std::endl;
                    adjust_file << b;
                    b.clear();
                }
                else//识别符号'='
                {
                    b.push_back(a[i]);
                    result_file << "<assign_op," << b << ">" << std::endl;
                    adjust_file << b;
                    b.clear();
                }
                break;
            }
            case ':'://识别以':'开头的符号
            {
                if (!b.empty())//若已经存放了标识符，先处理标识符
                {
                    handle_identifier_num(b, result_file, adjust_file);
                    b.clear();
                }
                if (a[i + 1] == '=')//识别符号':='
                {
                    b.push_back(a[i]);
                    b.push_back(a[i + 1]);
                    result_file << "<assign_op," << b << ">" << std::endl;
                    adjust_file << b;
                    b.clear();
                    i++;
                }
                else//识别符号':'
                {
                    b.push_back(a[i]);
                    result_file << "<" << b << ",>" << std::endl;
                    adjust_file << b;
                    b.clear();
                }
                break;
            }
            case ','://识别以','开头的符号
            {
                if (!b.empty())//若已经存放了标识符，先处理标识符
                {
                    handle_identifier_num(b, result_file, adjust_file);
                    b.clear();
                }
                b.push_back(a[i]);
                result_file << "<" << b << ",>" << std::endl;
                adjust_file << b;
                b.clear();
                break;
            }
            case '.'://识别以'.'开头或在中间出现的符号
            {
                if (b.empty() && a[i] == '.')//未识别到字符且为'.'开头，忽略','
                {
                    result_file << "Warning!!!! Invalid identifier(starting with '.')" << b;
                    result_file << " at line " << count_line << std::endl;
                    b.clear();
                }
                //若已经存放了标识符，且不是正在处理数字，先处理标识符
                if (!b.empty() && state != 1 && state != 3)
                {
                    handle_identifier_num(b, result_file, adjust_file);
                    b.clear();
                }
                if (state == 2)//正在识别标识符
                {
                    b.push_back(a[i]);
                    result_file << "<struct_op," << b << ">" << std::endl;
                    adjust_file << b;
                    count_operator--;
                    b.clear();
                }
                else if(state == 1)//正在识别整数
                {
                    b.push_back(a[i]);
                    state = 3;//开始识别小数
                    count_operator--;//减去预先加上的次数
                }
                else if(state == 3)//正在识别小数，出现小数点
                {
                    result_file << "Warning!!!! Too many dots";
                    result_file << " at line " << count_line + 1 << std::endl;
                    count_operator--;
                }
                break;
            }
            case '!'://识别以'!'开头的符号
            {
                if (!b.empty())//若已经存放了标识符，先处理标识符
                {
                    handle_identifier_num(b, result_file, adjust_file);
                    b.clear();
                }
                if (a[i + 1] == '=')//识别符号'!='
                {
                    b.push_back(a[i]);
                    b.push_back(a[i + 1]);
                    result_file << "<comp_op," << b << ">" << std::endl;
                    adjust_file << b;
                    b.clear();
                    i++;
                }
                else//不是'!='
                {
                    b.push_back(a[i]);
                    result_file << "Warning!!!! Invalid usage of '!' " << b;
                    result_file << " at line " << count_line + 1 << std::endl;
                    b.clear();
                }
                break;
            }
            case ' ':
            {
                if (!b.empty())//处理标识符
                {
                    handle_identifier_num(b, result_file, adjust_file);
                    b.clear();
                }
                adjust_file << a[i];
                break;
            }
            case '\n':
            {
                count_line++;
                if (!b.empty())//处理标识符
                {
                    handle_identifier_num(b, result_file, adjust_file);
                    b.clear();
                }
                adjust_file << a[i];
                break;
            }
            case '\t':
            {
                adjust_file << a[i];
                //忽略
                break;
            }
            default:
            {
                count_operator--;//未成功识别，减去预先加上的计数
                if(state == 4)
                    state = 0;//置为未识别符号状态
                break;
            }
        }

        //识别数字
        if ((a[i] >= '0' && a[i] <= '9'))//识别小数及整数
        {
            if (state == 0)//未识别，先识别到的是数字，则进入数字识别状态
                state = 1;//置为数字识别状态
            b.push_back(a[i]);
        }
        //识别字母
        if (a[i] == '_' || (a[i] >= 'a' && a[i] <= 'z') || (a[i] >= 'A' && a[i] <= 'Z') )
        {
            if (state == 0)
                state = 2;//置为标识符
            if (state == 1)//以数字开头却有字母出现，将数字删除，开始识别字母开头的标识符
            {
                result_file << "Warning!!!! Invalid identifier(starting with a number) " << b;
                result_file << " at line " << count_line + 1 << std::endl;
                b.clear();
            }
            b.push_back(a[i]);
        }

        //无法识别字符,剩余单词继续识别，从单词中剔除无法识别的符号
        if (state == 0)
        {
            result_file << "Warning!!!! Unknown character " << a[i];
            result_file << " at line " << count_line + 1 << std::endl;
        }
    }
}

void handle_identifier_num(std::string b, std::ofstream& result_file, std::ofstream& adjust_file)//对标识符（包括关键字）或数字进行识别
{
    if (b[0] >= '0' && b[0] <= '9')//以数字开始判断为数字
    {
        count_num++;
        if (b.find('.') != b.npos)//有小数点存在
            result_file << "<decimal," << b << ">" << std::endl;
        else
            result_file << "<integer," << b << ">" << std::endl;//识别整数
    }
    else//以字母开头，判断为标识符
    {
        count_keyword++;
        if (b == "int")
            result_file << "<keyword," << b << ">" << std::endl;
        else if (b == "float")
            result_file << "<keyword," << b << ">" << std::endl;
        else if (b == "char")
            result_file << "<keyword," << b << ">" << std::endl;
        else if (b == "double")
            result_file << "<keyword," << b << ">" << std::endl;
        else if (b == "short")
            result_file << "<keyword," << b << ">" << std::endl;
        else if (b == "long")
            result_file << "<keyword," << b << ">" << std::endl;
        else if (b == "return")
            result_file << "<keyword," << b << ">" << std::endl;
        else if (b == "main")
            result_file << "<keyword," << b << ">" << std::endl;
        else if (b == "do")
            result_file << "<keyword," << b << ">" << std::endl;
        else if (b == "void")
            result_file << "<keyword," << b << ">" << std::endl;
        else if (b == "while")
            result_file << "<keyword," << b << ">" << std::endl;
        else if (b == "for")
            result_file << "<keyword," << b << ">" << std::endl;
        else if (b == "if")
            result_file << "<keyword," << b << ">" << std::endl;
        else if (b == "else")
            result_file << "<keyword," << b << ">" << std::endl;
        else if (b == "switch")
            result_file << "<keyword," << b << ">" << std::endl;
        else if (b == "case")
            result_file << "<keyword," << b << ">" << std::endl;
        else if (b == "const")
            result_file << "<keyword," << b << ">" << std::endl;
        else if (b == "break")
            result_file << "<keyword," << b << ">" << std::endl;
        else
        {
            count_keyword--;
            if (b.length() > 32)//标识符过长截断标识符
            {
                result_file << "Warning!!!! Too long an identifier " << b;
                result_file << " at line " << count_line + 1 << std::endl;
                //从32位（第33个字符）开始截断
                std::string sub_string = b.substr(0, 32);
                count_identifier++;
                result_file << "<" << sub_string << ",";
                result_file << current_domain->d_id << "(" << table_insert(sub_string) << ")>" << std::endl;//识别自定义变量
                adjust_file << sub_string;//直接输出修改文件
                return;//返回
            }
            count_identifier++;
            result_file << "<" << b << ",";
            result_file << current_domain->d_id << "("<< table_insert(b) << ")>" << std::endl;//识别自定义变量
        }
    }
    adjust_file << b;//直接输出
}

void ini_head_of_domain()
{
    root = new Domain;
    next_d_id = 0;
    root->d_id = next_d_id++;//作用域编号递增
    root->layer = now_layer;//层数加深
    root->preceding = NULL;
    root->subsequent.clear();//设定成员属性
    current_domain = root;
}

int table_insert(std::string id)
{
    //在当前域中查找id是否已经存在
    std::vector<std::string>::iterator itr = std::find(current_domain->id_list.begin(), current_domain->id_list.end(), id);
    if (itr == current_domain->id_list.end())
    {
        current_domain->id_list.push_back(id);
        return current_domain->id_list.size() - 1;
    }
    else
    {
        return itr - current_domain->id_list.begin();
    }
    return 0;
}

void domain_back()
{
    if (current_domain->preceding != NULL)
    {
        current_domain = current_domain->preceding;//回到前驱作用域
        now_layer--;
    }
    else
        std::cout << "Warning!!!! Cannot find upper domain." << std::endl;
}

void domain_derive()
{
    Domain_ptr new_ptr = new Domain;
    new_ptr->d_id = next_d_id ++;//作用域编号递增
    new_ptr->layer = ++ now_layer;//层数加深
    new_ptr->preceding = current_domain;//设置前驱作用域
    new_ptr->subsequent.clear();//设定成员属性
    current_domain->subsequent.push_back(new_ptr);//设置后继作用域
    current_domain = new_ptr;//设置当前作用域
}
