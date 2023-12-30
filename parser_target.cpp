#include "stdlib.h"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <stack>
#include <string.h>
#include <string>

// namespace fs = std::filesystem;
using namespace std;
const int maxnlen = 1e4;

class Grammar {
  // @ ���� �� ��
private:
  set<char> Vn; // ���ս��
  set<char> Vt; // �ս��
  char S;
  map<char, set<string>> P;
  map<char, set<char>> FIRST;
  map<char, set<char>> FOLLOW;
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
      cout << "�ķ�  �ļ���ʧ��" << endl;
      exit(1);
    }
    char *buffer = new char[maxnlen];
    in.getline(buffer, maxnlen, '#'); // ??����Ҫtxt�����#��??
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

    // ���Vn��Vt��set
    cout << "Vn: ";
    for (auto item : Vn) {
      cout << item << ", ";
    }
    cout << endl;

    cout << "Vt: ";
    for (auto item : Vt) {
      cout << item << ", ";
    }
    cout << endl;
  }

  void setHead(char c) { S = c; }

  // �������ʽ
  void add(string s) {
    char s1 = s[0];
    string s2 = "";
    int num = 0;
    for (int i = 0; i < s.length(); i++) {
      // ��Ӳ���ʽ�Ҳ�
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
    cout << "��ǰ�����ķ�Ϊ��" << endl << endl;
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
    // �жϵ�������
    int iter = 4;
    while (iter--) {
      for (set<char>::iterator it = Vn.begin(); it != Vn.end(); it++) {
        char left = *it;
        /*����ʵ�����¹���
        ***************************************************************************************
        cur_s->cur_string[0]
            a�ӵ�A��FIRST��
        cur_s->cur_string[0]
            B��FITRST���ӵ�A��FIRST��
        */

        bool have_null = true;
        for (auto right : P[left]) {
          for (char V : right) {
            if (isVt(V)) {
              FIRST[left].insert(V);

              if (V != '@') {
                break;
              }
            } else {
              FIRST[left].insert(FIRST[V].begin(), FIRST[V].end());

              if (FIRST[V].count('@') > 0) {
                FIRST[left].insert('@');
              } else {
                break;
              }
            }
          }
        }
      }
    }
    // ���FIRST��
    cout << "FIRST��Ϊ��" << endl << endl;
    for (auto V : Vn) {
      cout << "FIRST(" << V << ") : ";
      for (auto val : FIRST[V]) {
        cout << val << " ";
      }
      cout << endl;
    }
  }

  void getFollow() {
    FOLLOW.clear();
    FOLLOW[S].insert('$');
    // �жϵ�������
    int iter = 4;
    while (iter--) {
      for (set<char>::iterator it = Vn.begin(); it != Vn.end(); it++) {
        char left = *it;
        /*����ʵ�����¹���
        ***************************************************************************************
        cur_s->cur_string[0]
        a�ӵ�A��FIRST��
        cur_s->cur_string[0]
        B��FITRST���ӵ�A��FIRST��
        */
        for (const auto &right_string : P[left]) {
          for (size_t i = 1; i < right_string.length(); ++i) {
            // ��һ����B->Ac����c�ӵ�A��follow��

            if (isVt(right_string[i]) && isVn(right_string[i - 1])) {
              FOLLOW[right_string[i - 1]].insert(right_string[i]);
            }

            // �ڶ�����B->AC����C��first���ӵ�A��follow��
            if (isVn(right_string[i]) && isVn(right_string[i - 1])) {
              set<char> temp_first;
              for (char V : FIRST[right_string[i]]) {
                if (V != '@') {
                  temp_first.insert(V);
                }
              }
              FOLLOW[right_string[i - 1]].insert(temp_first.begin(),
                                                 temp_first.end());
            }
          }

          // ��������AC/ACKΪ�����������������B->AC��B->ACK(K��first������@)����B��follow�����뵽C��follow��
          FOLLOW[right_string.back()].insert(FOLLOW[left].begin(),
                                             FOLLOW[left].end());

          if (FIRST[right_string.back()].count('@') > 0 &&
              isVn(right_string[right_string.length() - 2])) {
            FOLLOW[right_string[right_string.length() - 2]].insert(
                FOLLOW[left].begin(), FOLLOW[left].end());
          }
        }
      }
    }
    // ���FOLLOW��
    cout << "FOLLOW��Ϊ��" << endl << endl;
    for (auto cur_s : Vn) {
      cout << "FOLLOW(" << cur_s << ") : ";
      for (auto val : FOLLOW[cur_s]) {
        cout << val << " ";
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
    Vt_temp.insert('$');
    for (auto it = Vn.begin(); it != Vn.end(); it++) {
      char cur_s = *it;
      for (auto it1 = P[cur_s].begin(); it1 != P[cur_s].end(); it1++) {
        /*
        ����ʽΪ
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
        ��������Ϣ���������û�г��ֹ���
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

    /*����ʵ�����¹���
    ***************************************************************************************
        ��ʾTable�������ʽ��ӡ��cout << *it << "->" << setw(7) << Table[iter];
    */
    cout << "��ʾtable��" << endl << endl;

    for (const auto &item : Table) {
      char Vn_item = item.first[0];
      char Vt_item = item.first[1];
      string exp = item.second;
      cout << "��ǰ " << Vn_item;
      cout << " ��һ���������� " << Vt_item;
      cout << " ,�Ҳ��Ƶ���:  ";
      cout << Vn_item << " -> " << exp;
      cout << endl;
    }
  }

  /*
      ÿһ�η���һ�����봮
      SignΪ����ջ,��ջ�ַ�Ϊx
      �����ַ�����ǰ�ַ�Ϊa
  */
  bool AnalyzePredict(string inputstring) {
    string Sign; // �ַ�����ʾ����ջ
    Sign += '$';
    Sign += S;
    int StringPtr = 0;

    cout << setw(20) << right << "ջ " << setw(20) << left << "�ַ�ʣ������"
         << setw(20) << left << "����ʽ" << endl;

    // char a = inputstring[StringPtr++];
    char a = inputstring[StringPtr];
    bool flag = true;
    while (flag) {

      cout << endl
           << setw(20) << right << Sign + "  " << setw(20) << left
           << inputstring.substr(StringPtr, inputstring.length() - StringPtr);

      char x = Sign[Sign.length() - 1];
      Sign = Sign.substr(0, Sign.length() - 1);
      // ������ս��,ֱ���Ƴ�����ջ
      if (Vt.count(x)) {
        if (x == a) {
          a = inputstring[++StringPtr];
        } else if (x == '@') {
          continue;
        } else {
          return false;
        }
      } else {
        /*����ʵ�����¹���
        ***************************************************************************************
        */
        if (x == '$') { // ��һ������������ս���������ĩβ����
          if (a == '$') {
            cout << "�ɹ���������";
            return true;
          }
          cout << inputstring.substr(StringPtr)
               << " index: " << inputstring.size() << endl;
          return false;
        }
        // �ڶ���������Ƿ��ս������Ҫ�ƽ�����
        string reduc_str = string(1, x).append(1, a);

        if (Table[reduc_str] == "error") {
          cout << "�ƽ�����: " << reduc_str << endl;
          return false;
        }

        Sign.append(Table[reduc_str].rbegin(), Table[reduc_str].rend());

        cout << reduc_str[0] << " -> " << Table[reduc_str];

        // a = inputstring[StringPtr++];
      }
    }
    return true;
  }

  /*
      ������ݹ��㷨����P124
  */
  void remove_left_recursion() {
    for (auto pi = Vn.begin(); pi != Vn.end(); ++pi) {
      for (auto pj = Vn.begin(); pj != pi; ++pj) {
        set<string> temp_pi_set;
        for (const auto &right_single : P[*pi]) {
          if (right_single.find(*pj) == 0) { // �������ʽ��pj��ʼ
            for (const auto &to_replace : P[*pj]) {
              string replaced = right_single;
              replaced.replace(0, 1, to_replace); // �滻pjΪpj�Ĳ���ʽ
              temp_pi_set.insert(replaced);
            }
          } else {
            temp_pi_set.insert(right_single);
          }
        }
        P[*pi] = temp_pi_set;
      }
      remove_left_gene(*pi);
    }
  }

  /*
      ����ֱ����ݹ�
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
      set<string> NewPRight; // �������·��ս��NewVn���Ҳ�
      set<string> NewPNewVn; // �·��ս�����Ҳ�
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
      Vt.insert('@');
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

  bool isVn(char V) {
    if (Vn.find(V) != Vn.end()) {
      return true;
    } else {
      return false;
    }
  }

  bool isVt(char V) {
    if (Vt.find(V) != Vt.end()) {
      return true;
    } else {
      return false;
    }
  }
};

int main() {
  /*
  �ķ�����
  E->T|E+T;
  T->F|T*F;
  F->i|(E);

  A->+TA|@;
  B->*FB|@;
  E->TA;
  F->(E)|i;
  T->FB;
  ֱ�ӽ���������������������parse_test1.txt��parse_test2.txt��
  */

  string filename_gramer = "parse_test1.txt";

  // ����׼����ض��� parse_result.txt�ļ�
  //  freopen((pwd + "parse_result1.txt").c_str(), "w", stdout);
  //    freopen("parse_result2.txt", "w", stdout);

  Grammar *grammar = new Grammar(filename_gramer);
  cout << "/-------------------------û��������ݹ�----------------------------"
          "-/"
       << endl;
  cout << "�����ʾ��" << endl;
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

  cout << "/-------------------------�Ѿ�������ݹ�----------------------------"
          "-/"
       << endl;
  /*
  ������ݹ�֮����ж�
  */
  grammar->remove_left_recursion();
  cout << "�����ʾ��" << endl;
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
      << "/-------------------------�Դʷ����з���----------------------------/"
      << endl;
  /*
      Ŀǰ���뷨��ʹ�õ�һ��ʵ��lex.cpp�ļ��������ͬ�ĵ��ʣ�Ȼ��Ե��ʽ��в�����
      �������Ϊ+��*�������������������i;
      ����ֱ��ʹ��lex.cpp���к������ı�out1.txt����Parsing�ļ����ڣ�
  */
  // string filename = pwd + "\\Parsing\\out1.txt";
  string filename = "out2.txt";

  ifstream in(filename);
  char buffer[200];
  string inf = "";
  int num = 0;
  //	cout << "�ķ��������Ϊ��" << endl << endl;
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
        row += "$";
        cout << "��ǰ���봮rowΪ:   " << row << endl << endl;

        if (grammar->AnalyzePredict(row)) {
          cout << endl
               << "                                                            "
                  " ����ȷ"
               << endl;
        } else
          cout << endl
               << "                                                            "
                  " ������"
               << endl;
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
