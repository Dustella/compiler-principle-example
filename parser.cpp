#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <stack>
#include <string>


using namespace std;
const int maxnlen = 1e4;

class Grammar {
private:
  set<char> Vn;
  set<char> Vt;
  char S;

  //    这是文法的存储结构
  map<char, set<string>> P;

  //    这是文法的FIRST集和FOLLOW集
  map<char, set<char>> FIRST;
  map<char, set<char>> FOLLOW;

  //    这是文法的预测分析表
  map<string, string> Table;

public:
  Grammar(string filename) {
    Vn.clear();
    Vt.clear();
    P.clear();
    FIRST.clear();
    FOLLOW.clear();
    ifstream in(filename);
    if (!in.is_open()) {
      cout << "文法  文件打开失败" << endl;
      exit(1);
    }
    char *buffer = new char[maxnlen];
    in.getline(buffer, maxnlen, '#');
    string temps = "";
    bool is_sethead = 0;
    for (int i = 0; i < strlen(buffer); i++) {
      if (buffer[i] == '\n' || buffer[i] == ' ')
        continue;
      if (buffer[i] == ';') {
        if (!is_sethead) {
          this->setHead(temps[0]);
          is_sethead = 1;
        }
        this->add(temps);
        temps = "";
      } else
        temps += buffer[i];
    }
    delete buffer;
    /*
        输出Vn，Vt，set

    */
    print();
  }

  void setHead(char c) { S = c; }

  void add(string s) {
    char s1 = s[0];
    string s2 = "";
    int num = 0;
    for (int i = 0; i < s.length(); i++) {
      if (s[i] == '>')
        num = i;
      if (num == 0)
        continue;
      if (i > num)
        s2 += s[i];
    }
    s2 += ';';
    Vn.insert(s1);
    string temp = "";
    // set<char>::iterator iter1 = s2.begin();
    for (int i = 0; i < s2.length(); i++) { // char s : s2
      char s = s2[i];
      if (!isupper(s) && s != '|' && s != ';' && s != '@')
        Vt.insert(s);
      if (s == '|' || s == ';') {
        P[s1].insert(temp);
        temp = "";
      } else {
        temp += s;
      }
    }
  }

  void print() {
    cout << "当前分析文法为：" << endl << endl;
    for (set<char>::iterator it = Vn.begin(); it != Vn.end(); it++) {
      char cur_s = *it;
      for (set<string>::iterator it1 = P[cur_s].begin(); it1 != P[cur_s].end();
           it1++) {
        string cur_string = *it1;
        cout << cur_s << "->" << cur_string << endl;
      }
    }
  }

  void getFirst() {
    FIRST.clear();
    // 判断迭代次数
    //        并不是很清楚这个迭代次数的意义，可能是为了防止死循环
    int iter = 4;
    while (iter--) {
      for (set<char>::iterator it = Vn.begin(); it != Vn.end(); it++) {
        char cur_s = *it;
        /*请编程实现以下功能
        ***************************************************************************************
        cur_s->cur_string[0]
            a加到A的FIRST集
        cur_s->cur_string[0]
            B的FITRST集加到A的FIRST集
        */
        for (set<string>::iterator it1 = P[cur_s].begin();
             it1 != P[cur_s].end(); it1++) {
          string cur_string = *it1;
          for (int i = 0; i < cur_string.length(); i++) {
            /*第一步：
                B->Ac
                将c加到A的first集
            */
            if (!isupper(cur_string[i])) {
              FIRST[cur_s].insert(cur_string[i]);
              break;
            }
            /*第二步：
                B->AC
                将C的first集加到A的first集
            */
            else {
              char cur_s1 = cur_string[i];
              for (set<char>::iterator it2 = FIRST[cur_s1].begin();
                   it2 != FIRST[cur_s1].end(); it2++) {
                FIRST[cur_s].insert(*it2);
              }
            }
            // 第三步：遍历C的first去除@，加到A的first集
            if (FIRST[cur_string[i]].count('@') == 0)
              break;
            else
              continue;
          }
        }

        /* 第四步：
         * AC/ACK为最后两个或者三个
         * B->AC
         * B->ACK(K的first集含有@)
         * 将B的first集加入到C的first集
         * */
        for (set<string>::iterator it1 = P[cur_s].begin();
             it1 != P[cur_s].end(); it1++) {
          string cur_string = *it1;
          if (cur_string.length() < 2)
            continue;
          if (isupper(cur_string[cur_string.length() - 1]) &&
              isupper(cur_string[cur_string.length() - 2])) {
            char cur_s1 = cur_string[cur_string.length() - 1];
            char cur_s2 = cur_string[cur_string.length() - 2];
            if (FIRST[cur_s1].count('@') > 0) {
              for (set<char>::iterator it2 = FIRST[cur_s2].begin();
                   it2 != FIRST[cur_s2].end(); it2++) {
                FIRST[cur_s1].insert(*it2);
              }
            }
          }
          //                    if (cur_string.length() < 3)continue;
        }
      }
    }
    // 输出FIRST集
    cout << "FIRST集为：" << endl << endl;
    for (set<char>::iterator it = Vn.begin(); it != Vn.end(); it++) {
      char cur_s = *it;
      cout << "FIRST()   " << cur_s;
      for (set<char>::iterator it1 = FIRST[cur_s].begin();
           it1 != FIRST[cur_s].end(); it1++) {
        cout << "       " << *it1;
      }
      cout << endl;
    }
  }

  void getFollow() {
    FOLLOW.clear();
    FOLLOW[S].insert('#');
    // 判断迭代次数
    int iter = 4;
    while (iter--) {
      for (set<char>::iterator it = Vn.begin(); it != Vn.end(); it++) {
        char cur_s = *it;
        /*请编程实现以下功能
        ***************************************************************************************
        cur_s->cur_string[0]
        a加到A的FIRST集
        cur_s->cur_string[0]
        B的FITRST集加到A的FIRST集
        */
        for (set<string>::iterator it1 = P[cur_s].begin();
             it1 != P[cur_s].end(); it1++) {
          string cur_string = *it1;
          for (int i = 0; i < cur_string.length() - 1; i++) {
            /*第一步：
                B->Ac
                将c加到A的follow集
            */
            if (!isupper(cur_string[i])) {
              if (isupper(cur_string[i + 1])) {
                FOLLOW[cur_string[i]].insert(cur_string[i + 1]);
              }
            }

            /*第二步：
                B->AC
                将C的first集加到A的follow集
            */
            else {
              char cur_s1 = cur_string[i];
              char cur_s2 = cur_string[i + 1];
              for (set<char>::iterator it2 = FIRST[cur_s2].begin();
                   it2 != FIRST[cur_s2].end(); it2++) {
                FOLLOW[cur_s1].insert(*it2);
              }
            }

            // 第三步：遍历C的first去除@，加到A的follow集
            if (FIRST[cur_string[i + 1]].count('@') == 0)
              break;
            else
              continue;

            /* 第四步：
            AC/ACK为最后两个或者三个
            B->AC
            B->ACK(K的first集含有@)
            将B的follow集加入到C的follow集
            */
            if (cur_string.length() < 2)
              continue;
            if (isupper(cur_string[cur_string.length() - 1]) &&
                isupper(cur_string[cur_string.length() - 2])) {
              char cur_s1 = cur_string[cur_string.length() - 1];
              char cur_s2 = cur_string[cur_string.length() - 2];
              if (FIRST[cur_s1].count('@') > 0) {
                for (set<char>::iterator it2 = FOLLOW[cur_s].begin();
                     it2 != FOLLOW[cur_s].end(); it2++) {
                  FOLLOW[cur_s1].insert(*it2);
                }
              }
            }
          }
        }
      }
    }
    // 输出FOLLOW集
    cout << "FOLLOW集为：" << endl << endl;
    for (set<char>::iterator it = Vn.begin(); it != Vn.end(); it++) {
      char cur_s = *it;
      cout << "FOLLOW()  " << cur_s;
      for (set<char>::iterator it1 = FOLLOW[cur_s].begin();
           it1 != FOLLOW[cur_s].end(); it1++) {
        cout << "       " << *it1;
      }
      cout << endl;
    }
  }

  void getTable() {
    set<char> Vt_temp;
    // int i = 0; i < s2.length() ; i++
    // set<char>::iterator iter1;
    for (set<char>::iterator iter1 = Vt.begin(); iter1 != Vt.end(); iter1++) {
      // char c=Vt[iter1];
      Vt_temp.insert(*iter1);
    }
    Vt_temp.insert('#');
    for (auto it = Vn.begin(); it != Vn.end(); it++) {
      char cur_s = *it;
      for (auto it1 = P[cur_s].begin(); it1 != P[cur_s].end(); it1++) {
        /*
        产生式为
            cur_s->cur_string
        */
        string cur_string = *it1;
        if (isupper(cur_string[0])) {
          char first_s = cur_string[0];
          for (auto it2 = FIRST[first_s].begin(); it2 != FIRST[first_s].end();
               it2++) {
            string TableLeft = "";
            TableLeft = TableLeft + cur_s + *it2;
            Table[TableLeft] = cur_string;
          }

        } else {
          string TableLeft = "";
          TableLeft = TableLeft + cur_s + cur_string[0];
          Table[TableLeft] = cur_string;
        }
      }
      if (FIRST[cur_s].count('@') > 0) {
        for (auto it1 = FOLLOW[cur_s].begin(); it1 != FOLLOW[cur_s].end();
             it1++) {
          string TableLeft = "";
          TableLeft = TableLeft + cur_s + *it1;
          Table[TableLeft] = "@";
        }
      }
    }
    /*
        检查出错信息：即表格中没有出现过的
    */

    for (auto it = Vn.begin(); it != Vn.end(); it++) {
      for (auto it1 = Vt_temp.begin(); it1 != Vt_temp.end(); it1++) {
        string TableLeft = "";
        TableLeft = TableLeft + *it + *it1;
        if (!Table.count(TableLeft)) {
          Table[TableLeft] = "error";
        }
      }
    }

    /*请编程实现以下功能
    ***************************************************************************************
        显示Table，例如格式打印：cout << *it << "->" << setw(7) << Table[iter];
    */
    cout << "显示table表：" << endl << endl;
  }

  /*
      每一次分析一个输入串
      Sign为符号栈,出栈字符为x
      输入字符串当前字符为a
  */
  bool AnalyzePredict(string inputstring) {
    stack<char> Sign;
    Sign.push('#');
    Sign.push(S);
    int StringPtr = 0;
    char a = inputstring[StringPtr++];
    bool flag = true;
    while (flag) {
      char x = Sign.top();
      Sign.pop();
      // 如果是终结符,直接移出符号栈
      if (Vt.count(x)) {
        if (x == a)
          a = inputstring[StringPtr++];
        else
          return false;
      } else {
        /*请编程实现以下功能
        ***************************************************************************************
        */
        // 第一步：如果不是终结符，如果是末尾符号

        // 第二步：如果是非终结符，需要移进操作
      }
    }
    return true;
  }

  /*
      消除左递归算法蓝书P124
  */
  void remove_left_recursion() {
    string tempVn = "";
    for (auto it = Vn.begin(); it != Vn.end(); it++) {
      tempVn += *it;
    }

    for (int i = 0; i < tempVn.length(); i++) {
      char pi = tempVn[i];
      /*请编程实现消除左递归的替换代入操作
       ***************************************************************************************
       */

      remove_left_gene(pi);
    }
  }

  /*
      消除直接左递归
  */
  void remove_left_gene(char c) {
    char NewVn;
    for (int i = 0; i < 26; i++) {
      NewVn = i + 'A';
      if (!Vn.count(NewVn)) {
        break;
      }
    }
    bool isaddNewVn = 0;
    for (auto it = P[c].begin(); it != P[c].end(); it++) {
      string right = *it;

      if (right[0] == c) {
        isaddNewVn = 1;

        break;
      }
    }
    if (isaddNewVn) {
      set<string> NewPRight; // 加入了新非终结符NewVn的右部
      set<string> NewPNewVn; // 新非终结符的右部
      for (auto it = P[c].begin(); it != P[c].end(); it++) {
        string right = *it;
        if (right[0] != c) {
          right += NewVn; // TA
          NewPRight.insert(right);
        } else {
          right = right.substr(1); //+TA
          right += NewVn;
          NewPNewVn.insert(right);
        }
      }
      Vn.insert(NewVn);
      NewPNewVn.insert("@");
      P[NewVn] = NewPNewVn;
      P[c] = NewPRight;
    }
  }

  void ShowByTogether() {
    for (auto it = Vn.begin(); it != Vn.end(); it++) {
      cout << *it << "->";
      char c = *it;
      for (auto it1 = P[c].begin(); it1 != P[c].end(); it1++) {
        if (it1 == P[c].begin())
          cout << *it1;
        else
          cout << "|" << *it1;
      }
      cout << endl;
    }
  }
};

int main() {
  /*
  文法测试
  E->T|E+T;
  T->F|T*F;
  F->i|(E);

  A->+TA|@;
  B->*FB|@;
  E->TA;
  F->(E)|i;
  T->FB;
  直接将上面两个测试样例放在parse_test1.txt和parse_test2.txt中
  */
  string filename_gramer = "parse_test1.txt";
  Grammar *grammar = new Grammar(filename_gramer);
  cout << "/-------------------------没有消除左递归----------------------------"
          "-/"
       << endl;
  cout << "规格显示：" << endl;
  grammar->ShowByTogether();
  cout << endl;
  grammar->getFirst();
  cout << endl;
  grammar->getFollow();
  cout << endl;
  grammar->getTable();

  cout << "/-------------------------------------------------------------------"
          "-/"
       << endl
       << endl
       << endl;

  cout << "/-------------------------已经消除左递归----------------------------"
          "-/"
       << endl;
  /*
  消除左递归之后的判断
  */
  grammar->remove_left_recursion();
  cout << "规格显示：" << endl;
  cout << endl;
  grammar->ShowByTogether();
  grammar->getFirst();
  cout << endl;
  grammar->getFollow();
  cout << endl;
  grammar->getTable();

  cout << "/-------------------------------------------------------------------"
          "-/"
       << endl
       << endl
       << endl;
  cout
      << "/-------------------------对词法进行分析----------------------------/"
      << endl;
  /*
      目前的想法是使用第一个实验lex.cpp文件分离出不同的单词，然后对单词进行操作：
      如果单词为+，*，等于他本身，否则等于i;
      以下直接使用lex.cpp运行后的输出文本out1.txt（见Parsing文件夹内）
  */
  string filename = "D:\\Parsing\\out1.txt";
  ifstream in(filename);
  char buffer[200];
  string inf = "";
  int num = 0;
  //	cout << "文法分析结果为：" << endl << endl;
  if (in.is_open()) {
    while (!in.eof()) {
      in.getline(buffer, 100);
      //			cout << buffer << endl;
      inf += buffer;
      num++;
    }
  }
  string row = "";
  for (int i = 0; i < num - 1; i++) {
    int ptr = i * 13;
    string temp = "";
    for (int j = 1; j <= 5; j++) {
      if (inf[j + ptr] == ' ')
        continue;
      else
        temp += inf[ptr + j];
    }
    if (temp == "+" || temp == "-" || temp == "*" || temp == "/" ||
        temp == ">" || temp == "<" || temp == "=" || temp == "(" ||
        temp == ")") {
      row += temp;
    } else {
      if (temp == ";") {
        row += "#";
        cout << "当前row为:   " << row << endl;

        if (grammar->AnalyzePredict(row)) {
          cout << "                                         正确" << endl;
        } else
          cout << "                                         错误" << endl;
        row = "";
      } else {
        row += "i";
      }
    }
  }
  cout << "/-------------------------------------------------------------------"
          "-/"
       << endl
       << endl
       << endl;
  system("pause");
  return 0;
}
