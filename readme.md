# 编译原理课程设计 - 凌妙根老师版本

设计并实现一个 LL(1)语法分析器，实现对算术文法 G[E]:E->E+T|T T->T\*F|F F->(E)|i 所定义的符号串进行识别。

例子：符号串 abc+age+80 为文法所定义的句子。符号串(abc-80(\*s5)不是文法所定义的句子。\_)

## 实验过程

### 1、 学习基础知识.doc 文档，阅读 parser_my.cpp 中的代码，理解程序的结构。

在我们通读代码之后，可以大致理解 Grammer 类的设计。

在整个输入文法、修正左递归并计算预测表和最后的预测分析的过程中，这些状态是需要被用到和修改的：

- Vn 和 Vt 存储文法中的非终结符和终结符。

- S 起始符号，一直看到的大 S。

- P 产生式，所有的产生式都在里面

- FIRST 和 FOLLOW LL1 要用到的 FIRST 集和 FOLLOW 集

- Table 预测分析表，驱动了预测分析器

它提供了以下方法：

- setHead、add、print 设置头部、添加产生式、打印文法。

- getFirst 和 getFollow 计算文法的 FIRST 集和 FOLLOW 集。

- getTable 构造预测分析表。

- AnalyzePredict 预测分析器，进行预测分析。

- remove_left_recursion 和 remove_left_gene 消除左递归。后者是消除直接左递归，已经被完成。

- ShowByTogether 以一定格式显示文法。

- isVn 和 isVt 判断一个字符是否为非终结符或终结符。

最后，我们做的是这些事情：

- 实例化 Grammar 对象，传递文法位置并读取文法内容。

- 先不消除左递归，计算 FIRST 集和 FOLLOW 集、构造解析表。

- 消除左递归，再计算 FIRST 和 FOLLOW 集，构造预测分析表。

- 读取处理过的词法分析结果，使用预测分析器进行语法分析器。

### 2、 检测左递归，如果有则进行消除，实现 remove_left_recursion()函数；消除直接左递归（已实现）。

主要思路很明确：首先解决所有间接左递归问题，然后针对每个非终结符调用 remove_left_gene 函数来解决可能的直接左递归问题。这种方法可以确保文法不包含任何形式的左递归。

间接左递归消除，具体来说，先外层循环遍历所有非终结符 ，记 pi。然后，检查以先前非终结符开始的产生式：内层循环遍历直到当前的非终结符 pi，也就是遍历所有在 pi 之前的非终结符 pj。

接下来进行替换。如果 pi 的某个产生式以 pj 开始，说明存在间接左递归。那么，我们替换该产生式中的 pj 为 pj 的所有产生式，这就消除了 pi 和 pj 之间的间接左递归。

然后，用 remove_left_gene 函数来解决可能的直接左递归问题，我们就完成了这部分工作。

具体代码：

```cpp
void remove_left_recursion() {
for (auto pi = Vn.begin(); pi != Vn.end(); ++pi) {
for (auto pj = Vn.begin(); pj != pi; ++pj) {
set<string> temp_pi_set;
for (const auto& right_single: P[*pi]) {
if (right_single.find(*pj) == 0) { // 如果产生式以 pj 开始
for (const auto& to_replace: P[*pj]) {
string replaced = right_single;
replaced.replace(0, 1, to_replace); // 替换 pj 为 pj 的产生式
temp_pi_set.insert(replaced);
}
} else {
temp_pi_set.insert(right_single);
}
}
P[*pi] = temp_pi_set;
}
remove_left_gene(\*pi);
}
}
```

### 3、 求解 FIRST 集和 FOLLOW 集，分别实现 getFirst()和 getFollow()函数；

**GetFirst()**

这里做的事情很简单，就是获取某一个产生式右边的所有可能首终结符。

遍历就可以了。对于文法中的每个非终结符 left，遍历其所有产生式右侧 right：

如果 right 中的字符 V 是终结符（isVt(V)），则将其加入到 left 的 FIRST 集中。

如果遇到的终结符不是空串标识符 '@'，则停止当前产生式的处理，因为终结符后面的符号不会影响当前的 FIRST 集。

```cpp
if (isVt(V)) {
FIRST[left].insert(V);
if (V != '@') {
break;
}
}
```

如果 right 中的字符 V 是非终结符，则将 V 的 FIRST 集合并到 left 的 FIRST 集中。

如果 V 的 FIRST 集包含空串 '@'，则也将空串加入到 left 的 FIRST 集中。这表示从 V 开始的产生式可以导出空串。

如果 V 的 FIRST 集不包含空串，停止当前产生式的处理。这是因为在不产生空串的情况下，后续的符号不会影响当前的 FIRST 集。

这样，我们完成了 GetFirst 部分。

```cpp
else {
FIRST[left].insert(FIRST[V].begin(), FIRST[V].end());
if (FIRST[V].count('@') > 0) {
FIRST[left].insert('@');
} else {
 break;
}
}
```

**GetFollow()**

我们这一步要做的事情是：得到某个特定非终结符之后可能出现的终结符集合。

我们得到的提示是通过三个步骤处理三个情况。通过逐个处理产生式中的符号组合，我们能够准确地计算出每个非终结符的 FOLLOW 集。

步骤 1：处理产生式形如 B->Ac 的情况

这种情况下，我们需要将终结符 c 加入到非终结符 A 的 FOLLOW 集。

实现它很简单。遍历每个产生式的右侧，如果某个位置的符号是终结符（isVt(right_string[i])），并且它前面的符号是非终结符（isVn(right_string[i - 1])），则将这个终结符添加到前一个非终结符的 FOLLOW 集中。

```cpp
// 第一步：B->Ac，将 c 加到 A 的 follow 集
if (isVt(right_string[i]) && isVn(right_string[i - 1])) {
FOLLOW[right_string[i - 1]].insert(right_string[i]);
}
```

步骤 2：处理产生式形如 B->AC 的情况
为了处理这种情形，我们需要将非终结符 C 的 FIRST 集（不包括空串）加入到非终结符 A 的 FOLLOW 集。
遍历每个产生式的右侧，如果连续两个符号都是非终结符（isVn(right_string[i]) && isVn(right_string[i - 1])），则将后一个非终结符的 FIRST 集（除去空串）合并到前一个非终结符的 FOLLOW 集中。

```cpp
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
```

步骤 3：处理产生式末尾的非终结符

如果产生式的最后一个符号是非终结符，将左侧非终结符 B 的 FOLLOW 集添加到这个最后的非终结符的 FOLLOW 集中；如果产生式的倒数第二个符号是非终结符，并且最后一个符号的 FIRST 集包含空串，也将 B 的 FOLLOW 集添加到倒数第二个非终结符的 FOLLOW 集中。

为了做到这点，我们要先将产生式左侧非终结符 B（即 left）的 FOLLOW 集合并到产生式最右侧符号的 FOLLOW 集中。然后，如果最右侧符号的 FIRST 集含有空串，并且倒数第二个符号是非终结符，也将 B 的 FOLLOW 集添加到这个倒数第二个非终结符的 FOLLOW 集中。

```cpp
// 第三步：AC/ACK 为最后两个或者三个，B->AC，B->ACK(K 的 first 集含有@)，将 B 的 follow 集加入到 C 的 follow 集
FOLLOW[right_string.back()].insert(FOLLOW[left].begin(),FOLLOW[left].end());
if (FIRST[right_string.back()].count('@') > 0 && isVn(right_string[right_string.length() - 2]) {
FOLLOW[right_string[right_string.length() - 2]].insert(
FOLLOW[left].begin(), FOLLOW[left].end());
}
```

通过以上步骤，我们完成了 GetFirst() 和 GetFollow() 部分。

### 4、 理解构建 LL(1)分析表 getTable()中的代码，实现格式打印分析表的功能（显示 Table 部分）；

这一部分，我们要完成的工作是要构建预测分析表。预测分析表是驱动后文预测分析器的最关键的信息，预测分析器通过预测分析表进行预测，并通过下一个词法单元精确推导每一个产生式。

预测分析表的构建已经完成，我们只需要打印它就可以了。最为简单的办法是输出这样格式的的日志：

当前 A 下一个词素若是 $ ,右侧推导出: A -> @  
当前 A 下一个词素若是 ( ,右侧推导出: A -> error  
当前 A 下一个词素若是 ) ,右侧推导出: A -> @  
当前 A 下一个词素若是 \* ,右侧推导出: A -> error  
当前 A 下一个词素若是 + ,右侧推导出: A -> +TA  
当前 A 下一个词素若是 @ ,右侧推导出: A -> @

我们也确实很容易实现这部分日志

遍历 Table，first 和 second 就是 map 的键值对，键的字符串就是当前非终结符和下一个词素，值就是表达式。
所以，这是一个容易打印的过程。代码如下：

```cpp
cout << "显示 table 表：" << endl << endl;

    for (const auto &item : Table) {
      char Vn_item = item.first[0];
      char Vt_item = item.first[1];
      string exp = item.second;
      cout << "当前 " << Vn_item;
      cout << " 下一个词素若是 " << Vt_item;
      cout << " ,右侧推导出:  ";
      cout << Vn_item << " -> " << exp;
      cout << endl;
    }
```

### 5、 利用 lex.cpp 文件对输入串先进行词法分析，输出运行结果 out1.txt。

这是我运行的一份 out1.txt 的样例：

```text
< 0, abc>
< +, ->
< 0, age>
< +, ->
< 1, 80>
< ;, ->
```

### 6、 利用构建的 LL(1)分析表，实现对符号串的语法分析功能：AnalyzePredict 函数。

首先，我们先观看整个预测分析器的代码的整体思路：

它先初始化符号栈：将起始符号和结束标志 $ 放入符号栈。起始符号是文法的起始非终结符，而 $ 表示栈的底部和输入字符串的结束。然后读取输入字符串：从输入字符串中读取当前字符，准备进行分析。

这个分析器的主体是一个循环，循环条件是直到符号栈为空或分析完成。

在每次循环中，我们先取栈顶符号：

如果栈顶符号是终结符，则检查它是否与当前输入字符匹配。如果匹配，则从栈中弹出该符号，并读取输入字符串中的下一个字符；如果不匹配，报错并结束分析；

接下来，是栈顶符号非终结符的情况，也就是我们需要完成的任务。

如果栈顶符号是非终结符，我们可以查找预测分析表以确定如何处理。根据当前非终结符和输入字符，在预测分析表中找到对应的产生式，然后用该产生式的右侧替换栈顶的非终结符。

在这里，如果成功有一个产生式，我们应该输出一个日志。但是也可能解析表中找不到匹配项，我们就报错并结束分析。

在这里还有个别特殊情况需要处理，如遇到空串产生式或栈顶符号是特殊标记（如 $）。我们需要进行下一个循环或者判断分析是否完成。

当输入字符串完全匹配并且符号栈清空时，分析成功结束；否则，如果在过程中遇到错误或不匹配情况，分析失败。

这样，我们成功完成了预测分析器的构建。

```cpp
if (x == '$') { // 第一步：如果不是终结符，如果是末尾符号
if (a == '$') {
cout << "成功！！！！";
return true;
}
cout << inputstring.substr(StringPtr)
<< " index: " << inputstring.size() << endl;
return false;
}
// 第二步：如果是非终结符，需要移进操作
string reduc_str = string(1, x).append(1, a);
if (Table[reduc_str] == "error") {
cout << "移进错误: " << reduc_str << endl;
return false;
}
Sign.append(Table[reduc_str].rbegin(), Table[reduc_str].rend());
cout << reduc_str[0] << " -> " << Table[reduc_str];
```

## 四、 运行结果

分工：本人主要负责完成了 1,3,\*的功能。

1. 测试输入符号串：abc+age+80
   测试源文件（根据 parse_test1.txt 或 parse_test2.txt）：  
   E->T|E+T;  
   T->F|T\*F;  
   F->i|(E);  
   分析结果（截图）：

2. 测试输入符号串：(abc-80(*s5)
   测试源文件（根据 parse_test1.txt 或 parse_test2.txt）：  
   E->T|E+T;  
   T->F|T*F;  
   F->i|(E);  
   分析结果（截图）：

## 五、词法分析器的反思

我们得到了这份 lex.cpp 文件。和之前实验一我自己完成的词法分析器对比，两种实现有很多不同。

### 实现过程的不同

本次实验所用的词法分析器，使用面向对象的思路，将词法分析器封装为一个类。相比我的过程式代码，这种方式更加模块化和易于扩展，并且更加符合人的直觉。

另外，本次用到的新词法处理器，它在数值常量的处理上更加细致，它区分了十进制、八进制和十六进制数，而我当时对此没有做区分。

最后，第二份代码在识别不同类型的词素时使用了多个函数，这在一定程度上增加了代码的复杂度，但也提高了代码的可维护性。

### 关于简洁度和可维护性

我自己实现的词法分析器比较简单粗暴，更加直接和紧凑，在处理各类词素时非常直观。但由于没有模块化，所以在扩展性和模块化方面略显不足。

新的词法分析器虽然在代码行数上更多，但其面向对象的结构提供了更好的模块化和易于扩展的特性。不同类型的词素处理被拆分到不同的函数中，这有利于后续的维护和更新。

### 相同点

我们都实现词法分析器的基本功能，即从源代码中识别和提取出词法单元、都能解析包括关键字、操作符、数字常量和标识符的代码、都考虑了在处理输入字符串之前做过滤和预处理，去除了空格或注释。

在主要的分析过程中，我们都通过循环遍历输入字符串，逐一处理每个字符或字符组合，以识别不同的词法单元，都是用有限自动机进行解析。
我们都实现了简单的错误处理，以应对无法识别的字符或字符组合，也都能输出结果。
