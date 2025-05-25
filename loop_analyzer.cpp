#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <regex>
#include <vector>
#include <algorithm>
using namespace std;

// Helper function to remove comments
string removeComments(string code) {
    // Remove single-line comments
    code = regex_replace(code, regex(R"(//[^\n]*)"), "");
    // Remove multi-line comments
    code = regex_replace(code, regex(R"(/\*[\s\S]*?\*/)"), "");
    return code;
}

vector<string> extractVariables(const string& condition) {
    vector<string> vars;
    regex varRegex(R"(\b([a-zA-Z_]\w*)\s*([<>!=]=?|[\+\-\*/])\s*([^&\|^]+))");
    smatch match;
    
    string::const_iterator searchStart(condition.cbegin());
    while (regex_search(searchStart, condition.cend(), match, varRegex)) {
        vars.push_back(match[1]);
        searchStart = match.suffix().first;
    }
    return vars;
}

string analyzeManipulation(const string& loopBody, const string& condition, const vector<string>& vars) {
    if (vars.empty()) return "?";

    string code = loopBody + " " + condition;
    string norm = regex_replace(code, regex(R"(\s+)"), "");
    string complexity = "1";

    for (const auto& var : vars) {
        // Escape the variable for regex
        string escapedVar = regex_replace(var, regex(R"([.^$|()\\[\]{}*+?])"), R"(\\$&)");

        // O(1) patterns (no change)
        regex noChange(escapedVar + R"((=[^;]+;))"); // Assignment but not using itself

        // O(n) patterns
        regex incDec1(escapedVar + R"((\+\+|--))");              // y++, y--
        regex incDec2(R"((\+\+|--))" + escapedVar);              // ++y, --y
        regex plusEqual(escapedVar + R"(\+=(\d+|)" + escapedVar + R"())"); // y += 1, y += x
        regex minusEqual(escapedVar + R"(-=(\d+|)" + escapedVar + R"())"); // y -= 1, y -= x
        regex addSame(escapedVar + R"(=)" + escapedVar + R"([\+-](\d+|)" + escapedVar + R"())"); // y = y + 1, y = y + x
        regex subSame(escapedVar + R"(=)" + escapedVar + R"(-(\d+|)" + escapedVar + R"())"); // y = y - 1, y = y - x

        // O(logn) patterns
        regex mulEqual(escapedVar + R"(\*=(\d+|)" + escapedVar + R"())"); // y *= 2, y *= x
        regex divEqual(escapedVar + R"(/=(\d+|)" + escapedVar + R"())");  // y /= 2, y /= x
        regex mulSame(escapedVar + R"(=)" + escapedVar + R"(\*(\d+|)" + escapedVar + R"())"); // y = y * 2, y = y * x
        regex divSame(escapedVar + R"(=)" + escapedVar + R"(/(\d+|)" + escapedVar + R"())"); // y = y / 2, y = y / x
        regex rightShift(escapedVar + R"(>>=\s*\d+)");                 // y >>= 1
        regex leftShift(escapedVar + R"(<<=\s*\d+)");                  // y <<= 1

        // Binary search/merge sort patterns
        regex midPattern(escapedVar + R"(=\s*(low|high)\s*([+-])\s*((low|high)\s*[/]\s*2|\d+\s*[/]\s*2))"); // mid = low + (high-low)/2
        regex binaryUpdate1(R"((low|high)\s*=\s*)" + escapedVar + R"(\s*([+-])\s*\d*)"); // low = mid + 1
        regex binaryUpdate2(R"((low|high)\s*=\s*)" + escapedVar); // high = mid

        // Check patterns in order of complexity
        if (regex_search(norm, incDec1) || regex_search(norm, incDec2) ||
            regex_search(norm, plusEqual) || regex_search(norm, minusEqual) ||
            regex_search(norm, addSame) || regex_search(norm, subSame)) {
            if (complexity == "1") complexity = "n";
            else if (complexity == "logn") complexity = "nlogn";
        }
        else if (regex_search(norm, mulEqual) || regex_search(norm, divEqual) ||
                 regex_search(norm, mulSame) || regex_search(norm, divSame) ||
                 regex_search(norm, rightShift) || regex_search(norm, leftShift) ||
                 regex_search(norm, midPattern) || regex_search(norm, binaryUpdate1) ||
                 regex_search(norm, binaryUpdate2)) {
            if (complexity == "1") complexity = "logn";
            else if (complexity == "n") complexity = "nlogn";
        }
        else if (!regex_search(norm, noChange)) {
            complexity = "?";
        }
    }

    return complexity;
}

int findMatchingBrace(const string& code, int start) {
    int balance = 1;
    for (int i = start + 1; i < code.size(); ++i) {
        if (code[i] == '{') balance++;
        if (code[i] == '}') balance--;
        if (balance == 0) return i;
    }
    return -1;
}

string analyzeLoop(const string& loopCode);

string analyzeWhileLoop(const string& loopCode) {
    regex whileRegex(R"(while\s*\(([^)]+)\)\s*\{)");
    smatch match;

    if (regex_search(loopCode, match, whileRegex)) {
        string condition = match[1];
        vector<string> vars = extractVariables(condition);

        size_t startBody = loopCode.find("{", match.position());
        size_t endBody = findMatchingBrace(loopCode, startBody);
        if (startBody == string::npos || endBody == string::npos) return "?";

        string body = loopCode.substr(startBody + 1, endBody - startBody - 1);

        // Check for nested loops
        regex innerLoop(R"((while|for)\s*\()");
        string innerComplexity = "1";
        string::const_iterator searchStart(body.cbegin());
        while (regex_search(searchStart, body.cend(), innerLoop)) {
            int pos = distance(body.cbegin(), searchStart);
            int innerStart = body.find("{", pos);
            int innerEnd = findMatchingBrace(body, innerStart);
            if (innerStart == string::npos || innerEnd == string::npos) break;
            
            string innerLoopCode = body.substr(pos, innerEnd - pos + 1);
            string currentInnerComp = analyzeLoop(innerLoopCode);
            
            // Combine complexities
            if (innerComplexity == "1") innerComplexity = currentInnerComp;
            else if (innerComplexity == "n" && currentInnerComp == "n") innerComplexity = "n^2";
            else if (innerComplexity == "logn" && currentInnerComp == "logn") innerComplexity = "log^2n";
            else if ((innerComplexity == "n" && currentInnerComp == "logn") || 
                    (innerComplexity == "logn" && currentInnerComp == "n")) innerComplexity = "nlogn";
            else innerComplexity += "*" + currentInnerComp;
            
            searchStart = body.cbegin() + innerEnd + 1;
        }

        string outerComp = analyzeManipulation(body, condition, vars);
        
        // Combine with inner complexity
        if (innerComplexity != "1") {
            if (outerComp == "1") outerComp = innerComplexity;
            else if (outerComp == "n" && innerComplexity == "n") outerComp = "n^2";
            else if (outerComp == "logn" && innerComplexity == "logn") outerComp = "log^2n";
            else if ((outerComp == "n" && innerComplexity == "logn") || 
                    (outerComp == "logn" && innerComplexity == "n")) outerComp = "nlogn";
            else outerComp += "*" + innerComplexity;
        }

        return outerComp.empty() ? "?" : outerComp;
    }

    return "?";
}

string analyzeForLoop(const string& loopCode) {
    regex forRegex(R"(for\s*\(\s*(.*?)\s*;\s*(.*?)\s*;\s*(.*?)\s*\)\s*\{)");
    smatch match;

    if (regex_search(loopCode, match, forRegex)) {
        string init = match[1];
        string condition = match[2];
        string update = match[3];
        
        vector<string> vars = extractVariables(condition);
        if (vars.empty()) {
            // Try to extract from update
            regex varUpdate(R"(([a-zA-Z_]\w*)\s*([+\-*/])=|\+\+|\--)");
            smatch varMatch;
            if (regex_search(update, varMatch, varUpdate)) {
                vars.push_back(varMatch[1]);
            }
        }

        size_t startBody = loopCode.find("{", match.position());
        size_t endBody = findMatchingBrace(loopCode, startBody);
        if (startBody == string::npos || endBody == string::npos) return "?";

        string body = loopCode.substr(startBody + 1, endBody - startBody - 1);

        // Include update in the body for analysis
        string fullBody = body + ";" + update + ";";
        
        // Check for nested loops
        regex innerLoop(R"((while|for)\s*\()");
        string innerComplexity = "1";
        string::const_iterator searchStart(body.cbegin());
        while (regex_search(searchStart, body.cend(), innerLoop)) {
            int pos = distance(body.cbegin(), searchStart);
            int innerStart = body.find("{", pos);
            int innerEnd = findMatchingBrace(body, innerStart);
            if (innerStart == string::npos || innerEnd == string::npos) break;
            
            string innerLoopCode = body.substr(pos, innerEnd - pos + 1);
            string currentInnerComp = analyzeLoop(innerLoopCode);
            
            // Combine complexities
            if (innerComplexity == "1") innerComplexity = currentInnerComp;
            else if (innerComplexity == "n" && currentInnerComp == "n") innerComplexity = "n^2";
            else if (innerComplexity == "logn" && currentInnerComp == "logn") innerComplexity = "log^2n";
            else if ((innerComplexity == "n" && currentInnerComp == "logn") || 
                    (innerComplexity == "logn" && currentInnerComp == "n")) innerComplexity = "nlogn";
            else innerComplexity += "*" + currentInnerComp;
            
            searchStart = body.cbegin() + innerEnd + 1;
        }

        string outerComp = analyzeManipulation(fullBody, condition, vars);
        
        // Combine with inner complexity
        if (innerComplexity != "1") {
            if (outerComp == "1") outerComp = innerComplexity;
            else if (outerComp == "n" && innerComplexity == "n") outerComp = "n^2";
            else if (outerComp == "logn" && innerComplexity == "logn") outerComp = "log^2n";
            else if ((outerComp == "n" && innerComplexity == "logn") || 
                    (outerComp == "logn" && innerComplexity == "n")) outerComp = "nlogn";
            else outerComp += "*" + innerComplexity;
        }

        return outerComp.empty() ? "?" : outerComp;
    }

    return "?";
}

string analyzeLoop(const string& loopCode) {
    if (loopCode.find("while") != string::npos) {
        return analyzeWhileLoop(loopCode);
    } else if (loopCode.find("for") != string::npos) {
        return analyzeForLoop(loopCode);
    }
    return "?";
}

string analyzeCode(const string& code) {
    regex loopRegex(R"((while|for)\s*\()");
    smatch match;
    string complexity = "1";
    
    string::const_iterator searchStart(code.cbegin());
    while (regex_search(searchStart, code.cend(), match, loopRegex)) {
        int pos = distance(code.cbegin(), searchStart);
        int loopStart = code.find("{", pos);
        int loopEnd = findMatchingBrace(code, loopStart);
        if (loopStart == string::npos || loopEnd == string::npos) break;
        
        string loopCode = code.substr(pos, loopEnd - pos + 1);
        string currentComp = analyzeLoop(loopCode);
        
        // Combine complexities
        if (complexity == "1") complexity = currentComp;
        else if (complexity == "n" && currentComp == "n") complexity = "n^2";
        else if (complexity == "logn" && currentComp == "logn") complexity = "log^2n";
        else if ((complexity == "n" && currentComp == "logn") || 
                (complexity == "logn" && currentComp == "n")) complexity = "nlogn";
        else complexity += "*" + currentComp;
        
        searchStart = code.cbegin() + loopEnd + 1;
    }
    
    return complexity.empty() ? "1" : complexity;
}

int main() {
    int t;
    cout << "Enter number of test cases: ";
    cin >> t;
    cin.ignore(); // Ignore newline after reading t
    
    while(t--) {
        string filename;
        cout << "Enter the C++ source file name: ";
        getline(cin, filename);

        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Error: Cannot open file " << filename << endl;
            continue;
        }

        stringstream buffer;
        buffer << file.rdbuf();
        string code = buffer.str();

        code = removeComments(code);
        string result = analyzeCode(code);
        cout << "\n📊 Estimated Time Complexity: O(" << result << ")\n\n";
    }

    return 0;
}