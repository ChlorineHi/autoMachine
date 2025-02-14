#include <iostream>
#include <vector>
#include <stack>
#include <map>
#include <queue>
#include <set>
#include <algorithm>
#include <sstream>
using namespace std;
typedef unsigned long long ll;

// 复制所有必要的结构体和函数声明
bool isOper(char ch);
ll Fast_pow(ll a, ll b, ll p);

struct ENFAState {
    bool isEnd;
    int ind;
    map<char, vector<ENFAState *>> trans;
    vector<ENFAState *> emptytrans;
    ENFAState(bool isEnd) : isEnd(isEnd) {}
};

void AddTrans(ENFAState *States, ENFAState *to, char ch);

struct ENFANode {
    ENFAState *start;
    ENFAState *end;
    ENFANode() {}
    ENFANode(ENFAState *start, ENFAState *end) : start(start), end(end) {
        end->isEnd = true;
        AddTrans(start, end, ' ');
    }
    ENFANode(ENFAState *start, ENFAState *end, char symbol) : start(start), end(end) {
        end->isEnd = true;
        AddTrans(start, end, symbol);
    }
};

void init(string RL, string &_RL);
ENFANode *Build(string _RL, vector<ENFAState *> &ENFAStates);
ENFANode *concats(ENFANode *first, ENFANode *second);
ENFANode *unions(ENFANode *first, ENFANode *second, vector<ENFAState *> &ENFAStates);
ENFANode *closure(ENFANode *node, vector<ENFAState *> &ENFAStates);

struct NFAState {
    int ind;
    bool isEnd;
    map<char, vector<NFAState *>> trans;
    map<char, vector<ENFAState *>> transENFA;
    vector<ENFAState *> ENFAStates;
    NFAState() { isEnd = false; }
};

struct DFAState {
    int ind;
    bool isEnd;
    map<char, vector<DFAState *>> trans;
    map<char, set<NFAState *>> transNFA;
    set<NFAState *> NFAStates;
    DFAState() { isEnd = false; }
    DFAState(bool isEnd) : isEnd(isEnd) {}
};

class ENFA {
public:
    string RL;
    string _RL;
    ENFANode *node;
    vector<ENFAState *> ENFAStates;

    ENFA() {}
    ENFA(string RL) : RL(RL), _RL("") {
        init(RL, _RL);
        node = Build(_RL, ENFAStates);
    }
};

class NFA {
public:
    map<ll, int> M;
    vector<NFAState *> NFAStates;

    NFA(ENFA enfa);
};

class DFA {
public:
    vector<DFAState *> DFAStates;
    map<ll, int> M;

    DFA(NFA &nfa);
};

// 以下是一些关键函数的声明和实现
bool isOper(char ch) {
    return ch == '*' || ch == '+' || ch == '.';
}

void init(string RL, string &_RL) { 
    stack<char> oper;
    map<char, int> priority; // 操作顺序 闭包> 连接 > 并
    priority['*'] = 3;
    priority['+'] = 1;
    priority['.'] = 2;
    string poss = "";
    for (int i = 0; i < RL.length(); i++) { 
        poss += RL[i];
        if (i + 1 < RL.length())
            if ((RL[i] == '0' || RL[i] == '1') && ((RL[i + 1] == '0' || RL[i + 1] == '1') || RL[i + 1] == '('))
            {
                poss += '.';
            }
            else if (RL[i] == ')' && (RL[i + 1] == '0' || RL[i + 1] == '1'))
            {
                poss += '.';
            }
            else if (RL[i] == '*' && (RL[i + 1] == '(' || RL[i + 1] == '0' || RL[i + 1] == '1'))
            {
                poss += '.';
            }
            else if (RL[i] == ')' && (RL[i + 1] == '('))
            { 
                poss += '.';
            }
    }
    RL = poss;

    for (auto ch : RL) { 
        if (ch == '(')
        {
            oper.push(ch);
        }
        else if (ch == ')')
        {
            while (oper.top() != '(')
            {
                _RL += oper.top();
                oper.pop();
            }
            oper.pop();
        }
        else if (ch == '*' || ch == '+' || ch == '.')
        {
            while (!oper.empty())
            {
                if (priority[ch] <= priority[oper.top()])
                {
                    _RL += oper.top();
                    oper.pop();
                }
                else
                {
                    break;
                }
            }
            oper.push(ch);
        }
        else
        {
            _RL += ch;
        }
    }
    while (!oper.empty())
    {
        _RL += oper.top();
        oper.pop();
    }
}

void AddTrans(ENFAState *States, ENFAState *to, char ch) {
    if (ch == ' ') {
        States->emptytrans.push_back(to);
    } else {
        States->trans[ch].push_back(to);
    }
}

ENFANode* concats(ENFANode *first, ENFANode *second) {
    if (!first) return second;
    if (!second) return first;

    AddTrans(first->end, second->start, ' ');
    first->end = second->end;
    return first;
}

ENFANode* unions(ENFANode *first, ENFANode *second, vector<ENFAState *> &ENFAStates) {
    if (!first) return second;
    if (!second) return first;

    ENFAState *start = new ENFAState(false);
    ENFAState *end = new ENFAState(true);
    ENFAStates.push_back(start);
    ENFAStates.push_back(end);

    AddTrans(start, first->start, ' ');
    AddTrans(start, second->start, ' ');
    AddTrans(first->end, end, ' ');
    AddTrans(second->end, end, ' ');

    return new ENFANode(start, end);
}

ENFANode* closure(ENFANode *node, vector<ENFAState *> &ENFAStates) {
    if (!node) return nullptr;

    ENFAState *start = new ENFAState(false);
    ENFAState *end = new ENFAState(true);
    ENFAStates.push_back(start);
    ENFAStates.push_back(end);

    AddTrans(start, node->start, ' ');
    AddTrans(start, end, ' ');
    AddTrans(node->end, node->start, ' ');
    AddTrans(node->end, end, ' ');

    return new ENFANode(start, end);
}

ENFANode* Build(string _RL, vector<ENFAState *> &ENFAStates) {
    stack<ENFANode *> st;
    for (char ch : _RL) {
        if (isOper(ch)) {
            if (ch == '*') {
                if (!st.empty()) {
                    ENFANode *top = st.top();
                    st.pop();
                    st.push(closure(top, ENFAStates));
                }
            } else if (ch == '+') {
                if (st.size() >= 2) {
                    ENFANode *second = st.top(); st.pop();
                    ENFANode *first = st.top(); st.pop();
                    st.push(unions(first, second, ENFAStates));
                }
            } else if (ch == '.') {
                if (st.size() >= 2) {
                    ENFANode *second = st.top(); st.pop();
                    ENFANode *first = st.top(); st.pop();
                    st.push(concats(first, second));
                }
            }
        } else {
            ENFAState *start = new ENFAState(false);
            ENFAState *end = new ENFAState(true);
            ENFAStates.push_back(start);
            ENFAStates.push_back(end);
            AddTrans(start, end, ch);
            st.push(new ENFANode(start, end, ch));
        }
    }
    return st.top();
}

// NFA 构造函数的简化实现
NFA::NFA(ENFA enfa) {
    // 这里只是一个简化的实现，实际转换过程非常复杂
    for (auto state : enfa.ENFAStates) {
        NFAState* nfaState = new NFAState();
        nfaState->isEnd = state->isEnd;
        NFAStates.push_back(nfaState);
    }
}

// DFA 构造函数的简化实现
DFA::DFA(NFA &nfa) {
    // 这里只是一个简化的实现，实际转换过程非常复杂
    for (auto state : nfa.NFAStates) {
        DFAState* dfaState = new DFAState();
        dfaState->isEnd = state->isEnd;
        DFAStates.push_back(dfaState);
    }
}

// 主函数，支持命令行参数
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "用法: " << argv[0] << " <正则表达式>" << endl;
        return 1;
    }

    string regex = argv[1];
    
    try {
        // 创建 ENFA
        ENFA enfa(regex);
        
        // 转换为 NFA
        NFA nfa(enfa);
        
        // 转换为 DFA
        DFA dfa(nfa);
        
        // 输出详细信息
        cout << "正则表达式: " << regex << endl;
        cout << "ENFA状态数: " << enfa.ENFAStates.size() << endl;
        cout << "NFA状态数: " << nfa.NFAStates.size() << endl;
        cout << "DFA状态数: " << dfa.DFAStates.size() << endl;
        
        // 可以添加更多输出逻辑
    }
    catch (const exception& e) {
        cerr << "处理错误: " << e.what() << endl;
        return 1;
    }

    return 0;
}

// 注意：这里省略了大量复杂的实现细节，需要逐步补全
