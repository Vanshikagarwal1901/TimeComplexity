#include <iostream>
#include <fstream>
#include <regex>
#include <stack>
#include <string>
#include <cmath>
using namespace std;

int loopDepth = 0;
int logLoops = 0;
bool hasRecursionLinear = false;
bool hasRecursionLog = false;
bool hasRecursionBinary = false;
string functionName = "";

bool isLogarithmicLoop(string loopLine) {
    regex logPattern("i\\s*=\\s*i\\s*[/\\*]=\\s*\\d+");
    return regex_search(loopLine, logPattern);
}

void analyzeLine(const string& line) {
    static regex forPattern("for\\s*\\((.*)\\)");
    static regex whilePattern("while\\s*\\((.*)\\)");
    static regex functionPattern("void\\s+(\\w+)\\s*\\(");
    static regex recursiveCallPattern("(\\w+)\\s*\\(");

    smatch match;
    string trimmedLine = regex_replace(line, regex("^\\s+|\\s+$"), "");

    // Detect function name
    if (regex_search(trimmedLine, match, functionPattern)) {
        functionName = match[1];
    }

    // Detect recursive calls
    if (!functionName.empty() && regex_search(trimmedLine, match, recursiveCallPattern)) {
        string calledFunc = match[1];
        if (calledFunc == functionName) {
            if (trimmedLine.find("n - 1") != string::npos)
                hasRecursionLinear = true;
            else if (trimmedLine.find("n / 2") != string::npos)
                hasRecursionBinary = true;
            else if (trimmedLine.find("n") != string::npos)
                hasRecursionLog = true;
        }
    }

    // Detect for-loops
    if (regex_search(trimmedLine, match, forPattern)) {
        loopDepth++;
        string loopBody = match[1];
        if (isLogarithmicLoop(loopBody)) {
            logLoops++;
        }
    }

    // Detect while-loops
    if (regex_search(trimmedLine, match, whilePattern)) {
        loopDepth++;
        if (isLogarithmicLoop(trimmedLine)) {
            logLoops++;
        }
    }
}

string getLoopComplexity(int depth, int logCount) {
    if (depth == 0) return "O(1)";
    int poly = depth - logCount;
    if (poly == 0) return "O(log n)";
    if (logCount == 0) return "O(n^" + to_string(poly) + ")";
    return "O(n^" + to_string(poly) + " * log^" + to_string(logCount) + " n)";
}

string getRecursionComplexity() {
    if (hasRecursionBinary) return "O(n log n)";
    if (hasRecursionLog) return "O(log n)";
    if (hasRecursionLinear) return "O(n)";
    return "O(1)";
}

string combineComplexities(const string& loopC, const string& recurC) {
    if (loopC == "O(1)") return recurC;
    if (recurC == "O(1)") return loopC;

    // Extract expressions inside O(...) brackets
    string loopExpr = loopC.substr(2, loopC.length() - 3);
    string recurExpr = recurC.substr(2, recurC.length() - 3);

    return "O(" + loopExpr + " + " + recurExpr + ")";
}


int main() {
    string fileName;
    cout << "Enter C++ source code filename (with .cpp): ";
    cin >> fileName;

    ifstream file(fileName);
    if (!file.is_open()) {
        cerr << "Failed to open file.\n";
        return 1;
    }

    string line;
    while (getline(file, line)) {
        analyzeLine(line);
    }

    string loopComplexity = getLoopComplexity(loopDepth, logLoops);
    string recursionComplexity = getRecursionComplexity();
    string finalComplexity = combineComplexities(loopComplexity, recursionComplexity);

    cout << "\n📊 Estimated Worst-case Time Complexity: " << finalComplexity << endl;

    return 0;
}
