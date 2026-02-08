//Sophia Herrmann

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <cctype>

using namespace std;

const unsigned int MEM_MIN = 0x0000;
const unsigned int MEM_MAX = 0xFFFF;

struct Pair {
    unsigned int base;
    unsigned int limit;
    unsigned int start;
    unsigned int end;
};

string toHex4(unsigned int v) {
    std::ostringstream oss;
    oss << "0x" << std::uppercase << std::hex << setw(4) << setfill('0') << (v & 0xFFFF);
    return oss.str();
}

bool parseHexToken(const string &tok, unsigned int &val) {
    string s = tok;
    // trim
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a==string::npos) return false;
    size_t b = s.find_last_not_of(" \t\r\n");
    s = s.substr(a, b-a+1);
    if (s.size() >= 2 && s[0]=='0' && (s[1]=='x' || s[1]=='X')) s = s.substr(2);
    if (s.empty() || s.size() > 4) return false;
    for (char c : s) if (!isxdigit((unsigned char)c)) return false;
    try {
        val = (unsigned int)stoul(s, nullptr, 16);
    } catch(...) {
        return false;
    }
    return true;
}

int main() {
    ifstream f1("26S_COP4610_Project1_InputFile1.txt");
    if (!f1) {
        cerr << "Error: cannot open file\n";
        return 1;
    }

    vector<Pair> unused;
    string a,b;
    while (f1 >> a >> b) {
        unsigned int base=0, limit=0;
        if (!parseHexToken(a, base)) {
            cerr << "Error: invalid base " << a << "\n";
            return 1;
        }
        if (!parseHexToken(b, limit)) {
            cerr << "Error: invalid limit " << b << "\n";
            return 1;
        }
        if (limit == 0) {
            cerr << "Error: limit cannot be zero\n";
            return 1;
        }
        unsigned long long endcalc = (unsigned long long)base + (unsigned long long)limit - 1ULL;
        if (endcalc > MEM_MAX) {
            cerr << "Error: base+limit-1 out of range\n";
            return 1;
        }
        Pair p;
        p.base = base;
        p.limit = limit;
        p.start = base;
        p.end = (unsigned int)endcalc;
        unused.push_back(p);
    }
    f1.close();

    for (size_t i=1;i<unused.size();++i) {
        if (unused[i].base < unused[i-1].base) {
            cerr << "Error: base registers must be in increasing order\n";
            return 1;
        }
        if (unused[i].start <= unused[i-1].end) {
            cerr << "Error: overlapping unused intervals\n";
            return 1;
        }
    }

    cout << "This program creates the memory map table informing about used and unused areas\n";
    cout << "in the system memory once it is informed about the base and limit register\n";
    cout << "contents for the free/unused slots in the memory at the moment.\n\n";
    cout << "These base and limit values are being read from the file containing user input.\n\n";
    cout << "These are the values in hexadecimal number system:\n\n";

    cout << "The table of ununsed memory areas\n\n";
    cout << left << setw(18) << "Base Register" << "Limit Register\n";
    for (size_t i=0;i<unused.size();++i) {
        cout << left << setw(18) << toHex4(unused[i].base) << toHex4(unused[i].limit) << "\n";
    }
    cout << "\n";

    vector<pair<unsigned int,unsigned int>> used;
    if (unused.empty()) {
        used.push_back({MEM_MIN, MEM_MAX});
    } else {
        if (unused.front().start > MEM_MIN) {
            used.push_back({MEM_MIN, unused.front().start - 1});
        }
        for (size_t i=0;i+1<unused.size();++i) {
            unsigned int end1 = unused[i].end;
            unsigned int start2 = unused[i+1].start;
            if (end1 + 1 <= start2 - 1) {
                used.push_back({end1 + 1, start2 - 1});
            }
        }
        if (unused.back().end < MEM_MAX) {
            used.push_back({unused.back().end + 1, MEM_MAX});
        }
    }

    struct Seg { bool used; unsigned int top; unsigned int bottom; };
    vector<Seg> segs;
    unsigned int cur = MEM_MAX;
    int iu = (int)unused.size() - 1;
    int iv = (int)used.size() - 1;
    while (true) {
        bool matched = false;
        if (iu >= 0) {
            Pair &p = unused[iu];
            if (cur >= p.start && cur <= p.end) {
                segs.push_back({false, cur, p.start});
                if (p.start == 0) break;
                cur = p.start - 1;
                iu--;
                matched = true;
            }
        }
        if (matched) continue;
        if (iv >= 0) {
            auto &q = used[iv];
            if (cur >= q.first && cur <= q.second) {
                segs.push_back({true, cur, q.first});
                if (q.first == 0) break;
                cur = q.first - 1;
                iv--;
                matched = true;
            }
        }
        if (matched) continue;
        if (cur == 0) break;
        cur--;
    }

    cout << "The Complete Memory Map table is:\n\n";
    for (size_t i=0;i<segs.size();++i) {
        if (segs[i].used) {
            cout << left << setw(8) << "Used:" << toHex4(segs[i].top) << "\n";
            cout << left << setw(8) << "" << toHex4(segs[i].bottom) << "\n";
        } else {
            cout << left << setw(8) << "Unused:" << toHex4(segs[i].top) << "\n";
            cout << left << setw(8) << "" << toHex4(segs[i].bottom) << "\n";
        }
    }
    cout << "\n";

    ifstream f2("26S_COP4610_Project1_InputFile2.txt");
    if (!f2) {
        cerr << "Error: cannot open file\n";
        return 1;
    }
    cout << "Next, opening file 2 for stack operations ...\n\n";

    string startTok;
    if (!(f2 >> startTok)) {
        cerr << "Error: missing starting location\n";
        return 1;
    }
    unsigned int procStart = 0;
    if (!parseHexToken(startTok, procStart)) {
        cerr << "Error: invalid starting location\n";
        return 1;
    }

    bool found = false;
    unsigned int procLow=0, procHigh=0;
    for (size_t i=0;i<used.size();++i) {
        if (procStart >= used[i].first && procStart <= used[i].second) {
            found = true;
            procLow = used[i].first;
            procHigh = used[i].second;
            break;
        }
    }
    if (!found) {
        cerr << "Error: starting location not in any used area\n";
        return 1;
    }

    unsigned int SP = procHigh;

    cout << left << setw(20) << "Process starting" << "location: " << toHex4(procLow) << ";\n";
    cout << left << setw(20) << "" << "Process last location: " << toHex4(procHigh) << "; SP: " << toHex4(SP) << "\n";

    string line;
    getline(f2, line);
    while (getline(f2, line)) {
        // trim
        size_t s = line.find_first_not_of(" \t\r\n");
        if (s==string::npos) continue;
        size_t e = line.find_last_not_of(" \t\r\n");
        string t = line.substr(s, e-s+1);
        if (t.empty()) continue;
        // check for end
        string low = t;
        for (char &c : low) c = (char)tolower((unsigned char)c);
        if (low.find("end of") != string::npos) {
            cout << left << setw(20) << " " << "End of instructions ...\n";
            break;
        }
  
        unsigned int size = 0;
        if (low.find("int ") != string::npos || low.find("int\t") != string::npos) size = 4;
        else if (low.find("double ") != string::npos || low.find("double\t") != string::npos) size = 8;
        else if (low.find("char ") != string::npos || low.find("char\t") != string::npos) size = 1;
        else {
            cout << left << setw(20) << "Instruction:" << t << " SP: " << toHex4(SP) << "\n";
            continue;
        }
        if (SP < size) {
            cerr << "Error: stack overflow\n";
            return 1;
        }
        SP = SP - size;
        cout << left << setw(20) << "Instruction:" << t << " SP: " << toHex4(SP) << "\n";
    }

    f2.close();

    cout << "\nCompleted with exit code: 0\n";
    return 0;
}

