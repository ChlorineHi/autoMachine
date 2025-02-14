#include <iostream>
#include <vector>
#include <stack>
#include <map>
#include <queue>
#include <set>
#include <algorithm>
using namespace std;
typedef unsigned long long ll;

bool isOper(char ch);

ll Fast_pow(ll a, ll b, ll p);

struct ENFAState
{
    bool isEnd;                           // 是否为终结点
    int ind;                              // NFA对应的编号
    map<char, vector<ENFAState *>> trans; // 非空转移	//map<key,value>
    vector<ENFAState *> emptytrans;       // 空转移状态
    ENFAState(bool isEnd) : isEnd(isEnd) {}
};

void AddTrans(ENFAState *States, ENFAState *to, char ch); // 连边

struct ENFANode
{                     // 整个状态的关系用Node表示 便于进行各种递归操作
    ENFAState *start; // 开始结点
    ENFAState *end;   // 结束结点
    ENFANode()
    {
    }
    ENFANode(ENFAState *start, ENFAState *end) : start(start), end(end)
    {
        end->isEnd = true;
        AddTrans(start, end, ' ');
    }
    ENFANode(ENFAState *start, ENFAState *end, char symbol) : start(start), end(end)
    {
        end->isEnd = true;
        AddTrans(start, end, symbol);
    }
};

void init(string RL, string &_RL); // 中缀转后缀

ENFANode *Build(string _RL, vector<ENFAState *> &ENFAStates); // 后缀转ENFA

ENFANode *concats(ENFANode *first, ENFANode *second);

ENFANode *unions(ENFANode *first, ENFANode *second, vector<ENFAState *> &ENFAStates);

ENFANode *closure(ENFANode *node, vector<ENFAState *> &ENFAStates);

struct ENFA
{
    string RL;  // 正则表达式
    string _RL; // 正则表达式所对应的后缀表达式
    ENFANode *node;
    vector<ENFAState *> ENFAStates; // 所有的NFAState
    ENFA() {}
    ENFA(string RL) : RL(RL), _RL("")
    {
        init(RL, _RL);
        node = Build(_RL, ENFAStates);
    }
};

// 上面是ENFA

struct NFAState
{
    int ind;
    bool isEnd;
    map<char, vector<NFAState *>> trans;      // 该结点通过字符所能到达的NFA状态
    map<char, vector<ENFAState *>> transENFA; // 该结点通过字符所能到达的ENFA状态
    vector<ENFAState *> ENFAStates;
    NFAState() { isEnd = false; }
};

void initNFA(vector<NFAState *> &states, ENFA &enfa, map<ll, int> &M);

struct NFA
{
    map<ll, int> M;
    vector<NFAState *> NFAStates;
    NFA(ENFA enfa)
    {
        initNFA(NFAStates, enfa, M);
    }
};

void GetNeibors(ENFAState *state, vector<bool> &vis, NFAState *nfa, bool _);

void BFSNFA(ENFAState *state, vector<bool> &vis, vector<NFAState *> &NFAStates);

void transNFA(vector<NFAState *> states, vector<bool> vis, map<ll, int> &M);

void initNFA(vector<NFAState *> &states, ENFA &enfa, map<ll, int> &M);

// NFA

struct DFAState
{
    int ind;
    bool isEnd;
    map<char, vector<DFAState *>> trans; // 该结点通过字符所能到达的NFA状态
    map<char, set<NFAState *>> transNFA; // 该结点通过字符所能到达的ENFA状态
    set<NFAState *> NFAStates;           // set 不会存重复的元素
    DFAState() { isEnd = false; }
    DFAState(bool isEnd) : isEnd(isEnd) {}
};

void initDFA(vector<DFAState *> &states, NFA &nfa, map<ll, int> &M);

void getNextNFA(DFAState *state, NFAState *ns);

void bfsDFA(NFAState *state, vector<DFAState *> &DFAstate);

void transDFA(vector<DFAState *> &states, map<ll, int> &M);

void initDFA(vector<DFAState *> &states, NFA &nfa, map<ll, int> &M);

struct DFA
{
    vector<DFAState *> DFAStates;
    map<ll, int> M;
    DFA(NFA &nfa)
    {
        initDFA(DFAStates, nfa, M);
    }
};

// DFA

struct DFASet
{
    int ind;
    vector<DFAState *> DFAStates;
    map<int, bool> vis;
};

void merge(vector<int> &DSU, int a, int b);

int findx(vector<int> DSU, int x);

int findSet(int ind, vector<DFASet *> sset);

void MinimizeDFA(DFA &dfa);

void RearrangeID(DFA &dfa);

void showDFA(DFA dfa);

// DFA最小化

int main()
{
    string str;
    cin >> str;
    ENFA node(str);
    NFA nfa(node);
    DFA dfa(nfa);
    MinimizeDFA(dfa);
    RearrangeID(dfa);
    showDFA(dfa);
    return 0;
}

ll Fast_pow(ll a, ll b, ll p)
{ // 快速幂 用于算 a^b （a的b次方）%p 复杂度为log2N  原理见CSDN
    ll res = 1;
    while (b)
    {
        if (b & 1)
            res = (res * a) % p;
        a = (a * a) % p;
        b >>= 1;
    }
    return res % p;
}
// O（N）
ll calcSetHash(NFAState *NFAstate)
{                            // 计算DFAState的哈希值 用于标记DFAState是否被访问过，哈希值只与DFAState中所存的NFAState的编号有关
    ll res = 0;              // 只要尽量保证不同编号和数量的结点所算出的哈希值不同即可
    ll mod = 11451419198107; // NFAState 只跟 size 还有 每个ENFAState的Ind
    for (auto it : NFAstate->ENFAStates)
    {
        // it->ENFAState 对应一个状态
        // 7^(ind+1)
        res += NFAstate->ENFAStates.size() * Fast_pow(7, it->ind + 1, mod);
    }
    return res;
}

ll calcSetHash(DFAState *DFAstate)
{               // 计算DFAState的哈希值 用于标记DFAState是否被访问过，哈希值只与DFAState中所存的NFAState的编号有关
    ll res = 0; // 只要尽量保证不同编号和数量的结点所算出的哈希值不同即可
    ll mod = 11451419198107;
    for (auto it : DFAstate->NFAStates)
    {
        res += DFAstate->NFAStates.size() * Fast_pow(7, it->ind + 1, mod);
    }
    return res;
}

bool isOper(char ch)
{ // 判断符号是否为操作符
    if (ch == '*' || ch == '.' || ch == '+')
        return true;
    return false;
}

void init(string RL, string &_RL)
{ // 正则语言中缀表达式转后缀表达式
    stack<char> oper;
    map<char, int> priority; // 操作顺序 闭包> 连接 > 并
    priority['*'] = 3;
    priority['+'] = 1;
    priority['.'] = 2;
    string poss = "";
    for (int i = 0; i < RL.length(); i++)
    { // 由于正则表达式中的连接操作是省略的 故首先需要处理表达式加上连接符号
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
            { // bug2 形如)(这种之间也需要加乘号连接符
                poss += '.';
            }
    }
    RL = poss;

    for (auto ch : RL)
    { // 中缀转后缀过程 采用栈实现
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

void AddTrans(ENFAState *States, ENFAState *to, char ch)
{
    if (ch == ' ')
    { // 空转移
        States->emptytrans.push_back(to);
    }
    else
    { // 非空转移 ch 为转移条件
        States->trans[ch].push_back(to);
    }
}

ENFANode *concats(ENFANode *first, ENFANode *second)
{ // 连接 前者尾结点与后者首结点直接使用空转移接上即可
    AddTrans(first->end, second->start, ' ');
    first->end->isEnd = false;
    first->end = second->end;
    second->end = NULL;
    second->start = NULL;
    delete second;
    second = NULL;
    return first;
}

ENFANode *unions(ENFANode *first, ENFANode *second, vector<ENFAState *> &ENFAStates)
{ // 并 原理见论文
    ENFAState *starts = new ENFAState(false);
    starts->ind = ENFAStates.size() + 1;
    ENFAState *endd = new ENFAState(true);
    ENFAStates.push_back(starts);
    endd->ind = ENFAStates.size() + 1;
    ENFAStates.push_back(endd);
    AddTrans(starts, first->start, ' ');
    AddTrans(starts, second->start, ' ');
    AddTrans(first->end, endd, ' ');
    AddTrans(second->end, endd, ' ');
    first->end->isEnd = false;
    second->end->isEnd = false;
    first->start = NULL;
    first->end = NULL;
    second->start = NULL;
    second->end = NULL;
    delete first;
    delete second;
    first = NULL;
    second = NULL;
    ENFANode *node = new ENFANode();
    node->start = starts;
    node->end = endd;
    return node;
}

ENFANode *closure(ENFANode *node, vector<ENFAState *> &ENFAStates)
{ // 闭包 原理见论文
    ENFAState *starts = new ENFAState(false);
    starts->ind = ENFAStates.size() + 1;
    ENFAStates.push_back(starts);
    ENFAState *end = new ENFAState(true);
    end->ind = ENFAStates.size() + 1;
    ENFAStates.push_back(end);
    AddTrans(starts, end, ' ');
    AddTrans(starts, node->start, ' ');
    AddTrans(node->end, end, ' ');
    AddTrans(node->end, node->start, ' ');
    node->end->isEnd = false;
    node->start = NULL;
    node->end = NULL;
    delete node;
    node = NULL;
    ENFANode *res = new ENFANode();
    res->start = starts;
    res->end = end;
    return res;
}

ENFANode *Build(string _RL, vector<ENFAState *> &ENFAStates)
{ // 构造ENFA

    if (_RL == "")
    {
        return new ENFANode(new ENFAState(false), new ENFAState(true));
    }

    stack<ENFANode *> ss; // 实际的过程是一个递归的过程 采用栈实现

    for (auto ch : _RL)
    {

        if (ch == '+')
        { // 并操作取两个元素
            ENFANode *l1 = ss.top();
            ss.pop();
            ENFANode *l2 = ss.top();
            ss.pop();
            ss.push(unions(l1, l2, ENFAStates));
        }
        else if (ch == '*')
        { // 闭包操作取一个元素
            ENFANode *l1 = ss.top();
            ss.pop();
            ss.push(closure(l1, ENFAStates));
        }
        else if (ch == '.')
        { // 连接操作 从栈中pop两个元素
            ENFANode *l1 = ss.top();
            ss.pop();
            ENFANode *l2 = ss.top();
            ss.pop();
            ss.push(concats(l2, l1));
        }
        else
        {                                         // 1/0
            ENFAState *s1 = new ENFAState(false); // 如果不是操作符 则 新建两个状态 通过该符号进行连接   A ->B
            ENFAState *s2 = new ENFAState(true);
            s1->ind = ENFAStates.size() + 1;
            ENFAStates.push_back(s1);
            s2->ind = ENFAStates.size() + 1;
            ENFAStates.push_back(s2);
            ENFANode *node = new ENFANode(s1, s2, ch);
            ss.push(node);
        }
    }

    ENFANode *tmp = ss.top();

    ss.pop();
    return tmp;
}

void GetNeibors(ENFAState *state, vector<bool> &vis, NFAState *nfa, bool _)
{
    if (vis[state->ind])
        return;
    vis[state->ind] = true;
    if (state->isEnd)
        nfa->isEnd = true;

    nfa->ENFAStates.push_back(state);

    for (auto it : state->emptytrans)
    {
        if (!vis[it->ind])
        {
            GetNeibors(it, vis, nfa, _);
        }
    }

    if (_)
        for (auto [x, y] : state->trans)
        {
            for (auto t : y)
            {
                nfa->transENFA[x].push_back(t);
            }
        }
}

void BFSNFA(ENFAState *state, vector<bool> &vis, vector<NFAState *> &NFAStates)
{
    queue<NFAState *> que;
    NFAState *ns = new NFAState();
    fill(vis.begin(), vis.end(), false);
    map<ll, bool> tar;
    GetNeibors(state, vis, ns, 1);
    que.push(ns);
    while (!que.empty())
    {
        NFAState *nss = que.front();
        que.pop();
        if (tar.count(calcSetHash(nss)))
            continue;
        tar.insert({calcSetHash(nss), true});
        NFAStates.push_back(nss);
        for (auto [x, y] : nss->transENFA)
        {
            for (auto t : y)
            {
                NFAState *nsc = new NFAState();
                fill(vis.begin(), vis.end(), false);
                GetNeibors(t, vis, nsc, 1);
                if (!tar.count(calcSetHash(nsc)))
                {
                    que.push(nsc);
                }
                else
                {
                    delete nsc;
                }
            }
        }
    }
}

void transNFA(vector<NFAState *> states, vector<bool> vis, map<ll, int> &M)
{
    int cnt = 0;
    for (auto it : states)
    { // 逐一访问各个结点
        ll hash = calcSetHash(it);
        it->ind = cnt;
        M[hash] = cnt;
        cnt++;
    }
    for (auto it : states)
    {
        for (auto tt : it->transENFA['0'])
        {
            NFAState *tmp = new NFAState();
            fill(vis.begin(), vis.end(), false);
            GetNeibors(tt, vis, tmp, 0);
            ll hash = calcSetHash(tmp);
            if (M.count(hash))
            {
                it->trans['0'].push_back(states[M[hash]]);
            }
            delete tmp;
        }
        for (auto tt : it->transENFA['1'])
        {
            NFAState *tmp = new NFAState();
            fill(vis.begin(), vis.end(), false);
            GetNeibors(tt, vis, tmp, 0);
            ll hash = calcSetHash(tmp);
            if (M.count(hash))
            {
                it->trans['1'].push_back(states[M[hash]]);
            }
            delete tmp;
        }
    }
}

void initNFA(vector<NFAState *> &states, ENFA &enfa, map<ll, int> &M)
{
    vector<bool> vis(enfa.ENFAStates.size() + 5);
    fill(vis.begin(), vis.end(), false);
    BFSNFA(enfa.node->start, vis, states);
    transNFA(states, vis, M);
}

void getNextNFA(DFAState *state, NFAState *ns)
{
    for (auto [x, y] : ns->trans)
    {
        for (auto t : y)
            state->transNFA[x].insert(t);
    }
}

void bfsDFA(NFAState *state, vector<DFAState *> &DFAstate)
{ // BFS 创建DFAState 原理见论文
    queue<DFAState *> que;
    DFAState *ds = new DFAState();
    map<ll, bool> tar;
    ds->NFAStates.insert(state);
    if (state->isEnd)
        ds->isEnd = true;
    que.push(ds);
    while (!que.empty())
    {
        DFAState *dss = que.front();
        que.pop();
        if (tar.count(calcSetHash(dss)))
            continue;                         // 若该结点被标记 则continue
        tar.insert({calcSetHash(dss), true}); // 否则将该DFAState作为新状态 加入集合

        for (auto it : dss->NFAStates)
        {
            getNextNFA(dss, it);
        }

        DFAstate.push_back(dss);

        for (auto [x, y] : dss->transNFA)
        { // 遍历该DFA状态通过字符转移所能达到的所有NFA状态
            DFAState *dsc = new DFAState();
            for (auto ir : y)
            {
                if (ir->isEnd)
                    dsc->isEnd = true;
                dsc->NFAStates.insert(ir);
            }

            if (!tar.count(calcSetHash(dsc)))
            {
                que.push(dsc);
            }
            else
            {
                delete dsc;
            }
        }
    }
}

void transDFA(vector<DFAState *> &states, map<ll, int> &M)
{
    int cnt = 0;
    for (auto it : states)
    { // 逐一访问各个结点
        ll hash = calcSetHash(it);
        it->ind = ++cnt;
        M[hash] = cnt;
    }
    for (auto it : states)
    {
        DFAState *tmp = new DFAState();
        for (auto t : it->transNFA['0'])
        {
            tmp->NFAStates.insert(t);
        }
        ll hash1 = calcSetHash(tmp);
        if (M.count(hash1))
            it->trans['0'].push_back(states[M[hash1] - 1]);
        delete tmp;

        tmp = new DFAState();

        for (auto t : it->transNFA['1'])
        {
            tmp->NFAStates.insert(t);
        }
        ll hash2 = calcSetHash(tmp);
        if (M.count(hash2))
            it->trans['1'].push_back(states[M[hash2] - 1]);
        delete tmp;
    }
    // 添加陷阱状态
    DFAState *qt = NULL;
    for (auto it : states)
    {
        if (it->trans['0'].size() == 0)
        {
            if (qt == NULL)
                qt = new DFAState();
            it->trans['0'].push_back(qt);
        }
        if (it->trans['1'].size() == 0)
        {
            if (qt == NULL)
                qt = new DFAState();
            it->trans['1'].push_back(qt);
        }
    }
    if (qt != NULL)
    {
        qt->trans['0'].push_back(qt);
        qt->trans['1'].push_back(qt);
        qt->ind = states.size() + 1;
        M[calcSetHash(qt)] = qt->ind;
        states.push_back(qt);
    }
}

void initDFA(vector<DFAState *> &states, NFA &nfa, map<ll, int> &M)
{ // 初始化DFA
    vector<NFAState *> NFAStates = nfa.NFAStates;
    bfsDFA(NFAStates[0], states); // bfs从开始NFA状态开始创建DFA
    transDFA(states, M);          // 给DFA中的各个结点连边
}

/*
    并查集

    {1},{2},{3},{4},{5},{6},{7}

    {1,2}  {3,5} {4,6,7};


    f[8]={0,1,2,3,4,5,6,7};
    f[i]=i;

    f[2]=1;

    f[8]={0,1,1,3,4,5,6,7};

    f[2]==f[1];

    f[8]={0,3,4,3,6,5,6,7};

    合并 1 2

    f[1]=3;
    f1=3;

    f[3]=3;

    f[2]=4;
    f[4]=6;
    f[6]=6;

    f2=6;
    f[f2]=f1;

    f[8]={0,3,4,3,6,5,3,7};

    f[2]=4;
    f[4]=6;
    f[6]=3;
    f[3]=3;

    f[i]==f[j];

    i=f[i];

*/

int findSet(int ind, vector<DFASet *> sset)
{ // 通过结点ID查找其所属的集合
    for (auto it : sset)
    {
        if (it->vis[ind])
            return it->ind;
    }
    return -2;
}

int findx(vector<int> DSU, int x)
{ // 查找 x 的父节点 x==f[x]
    while (x != DSU[x])
        x = DSU[x];
    return x;
}

void merge(vector<int> &DSU, int a, int b)
{
    int fa = findx(DSU, a), fb = findx(DSU, b);
    if (fa <= fb)
    {
        DSU[fb] = fa;
    }
    else
    {
        DSU[fa] = fb;
    }
}

void MinimizeDFA(DFA &dfa)
{                          // 最小化DFA  树形分割法 原理见论文
    vector<DFASet *> sset; // 分割后的集合
    vector<DFAState *> &DFAStates = dfa.DFAStates;

    DFASet *s1 = new DFASet(), *s2 = new DFASet();

    s1->ind = 1;
    s2->ind = 2;

    for (auto it : DFAStates)
    { // 首先 将终态结点作为一个集合 末态结点作为一个集合 存入sset中
        if (!it->isEnd)
        {
            s1->DFAStates.push_back(it);
            s1->vis[it->ind] = true;
        }
        else
        {
            s2->DFAStates.push_back(it);
            s2->vis[it->ind] = true;
        }
    }

    sset.push_back(s1);
    sset.push_back(s2);

    int lastsize = 0;

    while (lastsize != sset.size())
    {                           // 循环终止的条件为 分割后sset的大小不再变化
        lastsize = sset.size(); // 每次进入循环后更新大小

        for (int i = 0; i < sset.size(); i++)
        { // 遍历sset中的每一个集合
            auto DFAStatess = sset[i]->DFAStates;
            if (DFAStatess.size() <= 1)
                continue; // 如果该集合的大小为1 则肯定是sset中一个集合的子集 可以直接跳过
            int ind = -1;
            vector<bool> ss(DFAStates.size() + 2);
            fill(ss.begin(), ss.end(), false);
            bool isSplit = false;
            for (auto t : DFAStatess)
            { // 遍历集合中的元素 判断集合是否可以分割

                if (t->trans['0'].size() == 0)
                {
                    ss[t->ind] = true;
                    isSplit = true;
                    break;
                }

                for (auto t0 : t->trans['0'])
                {
                    int nxt = findSet(t0->ind, sset);
                    // cout<<t0->ind<<' ';
                    if (ind == -1)
                    { // 以第一个元素通过0能转移的DFA所在的集合为标记 判断后续的元素所在的集合与第一个是否相同 来判断其是否能分割
                        ind = nxt;
                    }
                    else
                    { // 如果其通过结点所找到的点存在不同的话 说明该结点可以分割 将该节点标记
                        if (ind != nxt)
                        {
                            ss[t->ind] = true;
                            isSplit = true;
                        }
                    }
                }
            }
            if (isSplit)
            { // 如果可以分割的话 则将标记的结点 和未标记的结点存入两个集合 存入sset
                DFASet *ds1 = new DFASet(), *ds2 = new DFASet();
                for (auto ii : DFAStatess)
                {
                    if (!ss[ii->ind])
                    {
                        ds1->DFAStates.push_back(ii);
                        ds1->vis[ii->ind] = true;
                    }
                    else
                    {
                        ds2->DFAStates.push_back(ii);
                        ds2->vis[ii->ind] = true;
                    }
                }

                DFASet *tmp = sset[i];
                sset[i] = ds1;
                ds1->ind = tmp->ind;
                delete tmp;
                tmp = NULL;
                ds2->ind = sset.size() + 1;
                sset.push_back(ds2);
            }
        }

        for (int i = 0; i < sset.size(); i++)
        { // 同理 对 集合 以 1的转移进行分割
            auto DFAStatess = sset[i]->DFAStates;
            if (DFAStatess.size() <= 1)
                continue;
            int ind = -1;
            vector<bool> ss(DFAStates.size() + 2);
            fill(ss.begin(), ss.end(), false);
            bool isSplit = false;
            for (auto t : DFAStatess)
            {
                if (t->trans['0'].size() == 0)
                {
                    ss[t->ind] = true;
                    isSplit = true;
                    break;
                }
                for (auto t0 : t->trans['1'])
                {
                    int nxt = findSet(t0->ind, sset);
                    if (ind == -1)
                    {
                        ind = nxt;
                    }
                    else
                    {
                        if (ind != nxt)
                        {
                            ss[t->ind] = true;
                            isSplit = true;
                        }
                    }
                }
            }
            if (isSplit)
            {
                DFASet *ds1 = new DFASet(), *ds2 = new DFASet();
                for (auto ii : DFAStatess)
                {
                    if (!ss[ii->ind])
                    {
                        ds1->DFAStates.push_back(ii);
                        ds1->vis[ii->ind] = true;
                    }
                    else
                    {
                        ds2->DFAStates.push_back(ii);
                        ds2->vis[ii->ind] = true;
                    }
                }
                DFASet *tmp = sset[i];
                sset[i] = ds1;
                ds1->ind = tmp->ind;
                delete tmp;
                tmp = NULL;
                ds2->ind = sset.size() + 1;
                sset.push_back(ds2);
            }
        }
    }

    vector<int> DSU(DFAStates.size() + 1); // 并查集 将
    for (int i = 1; i < DSU.size(); i++)
        DSU[i] = i;
    vector<int> del;
    for (auto ds : sset)
    { // 将每个set中的元素放入一个并查集 标记为同一状态
        for (int i = 0; i + 1 < ds->DFAStates.size(); i++)
        {

            merge(DSU, ds->DFAStates[i]->ind, ds->DFAStates[i + 1]->ind);
        }
    }

    for (auto &it : DFAStates)
    {
        for (auto &[x, y] : it->trans)
        {
            // 将所有结点通过符号转移到的下一个状态 全部指向该状态所对应的父状态
            auto tt = y[0];
            int fa = findx(DSU, tt->ind);
            if (fa != (tt->ind))
            {
                y.erase(y.begin());
                y.push_back(DFAStates[fa - 1]);
            }
        }
    }

    // del vector<DFAStates*>
    for (auto it : DFAStates)
    { // 再将同一并查集通过符号能达到的状态全部赋给父状态的转移
        int fx = findx(DSU, it->ind);
        if (fx != it->ind)
        { // 若该状态不是其所对应的并查集的首个状态 则需要将该状态标记为删除
            del.push_back(it->ind);
            if (it->isEnd)
                DFAStates[fx]->isEnd = true;
            for (auto &[x, y] : it->trans)
            {
                for (auto r : y)
                {
                    bool contains = false;
                    int tt = findx(DSU, r->ind);
                    for (auto g : DFAStates[fx - 1]->trans[x])
                    {
                        if (g->ind == DFAStates[tt - 1]->ind)
                        {
                            contains = true;
                        }
                    }
                    if (!contains)
                    { // 若父节点不包含该种转移才加 否则会重复转移
                        DFAStates[fx - 1]->trans[x].push_back(DFAStates[tt - 1]);
                    }
                }
            }
        }
    }

    int cnt = 0;

    for (auto it : del)
    { // 挂掉的点 erase后 下标会改变 需要对ind的偏移进行修正 否则会index out of bound
        int indd = it - 1 - cnt;
        DFAStates.erase(DFAStates.begin() + indd);
        cnt++;
    }

    for (int i = 0; i < DFAStates.size(); i++)
    {
        DFAStates[i]->ind = i + 1;
    }
}

void showDFA(DFA dfa)
{
    auto DFAStates = dfa.DFAStates;

    auto cmp = [](DFAState *a, DFAState *b)
    {
        return a->ind < b->ind;
    };

    sort(DFAStates.begin(), DFAStates.end(), cmp);
    printf("      0 1\n");
    for (auto it : DFAStates)
    {
        if (it->ind == 1)
            printf("(s%s)", it->isEnd ? ",e" : "");
        else if (it->isEnd)
            printf("(e)");
        printf("q%d ", it->ind - 1);

        printf("q%d ", it->trans['0'][0]->ind - 1);

        printf("q%d\n", it->trans['1'][0]->ind - 1);
    }

    cout << '\n';

    for (int i = 0; i < DFAStates.size(); i++)
    {
        auto it = DFAStates[i];
        if (i != 0)
            printf("\n");
        printf("q%d->0q%d", it->ind - 1, it->trans['0'][0]->ind - 1);
        printf("\n");
        printf("q%d->1q%d", it->ind - 1, it->trans['1'][0]->ind - 1);
        if (it->trans['0'][0]->isEnd)
        {
            printf("\n");
            printf("q%d->%d", it->ind - 1, 0);
        }
        if (it->trans['1'][0]->isEnd)
        {
            printf("\n");
            printf("q%d->%d", it->ind - 1, 1);
        }
        if (it->ind == 1 && it->isEnd)
        {
            printf("\n");
            printf("q0->null \n");
        }
    }
}

void RearrangeID(DFA &dfa)
{

    auto DFAStates = dfa.DFAStates;
    queue<DFAState *> que;
    que.push(DFAStates[0]);
    map<DFAState *, bool> vis;
    int cnt = 1;
    while (!que.empty())
    {
        auto it = que.front();
        que.pop();
        if (vis.count(it))
            continue;
        it->ind = cnt++;
        vis.insert({it, true});
        que.push(it->trans['0'][0]);
        que.push(it->trans['1'][0]);
    }
}
