#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <sstream>
#include <regex>
#include <fstream>  // Added for file reading
using namespace std;
struct LoopNode 
{
    string loopType;
    string loopHeader;
    string complexity;
    vector<LoopNode*> children;
};
string getLoopComplexity(const string& header) 
{
    if (regex_search(header, regex("\\w+\\s*\\*=\\s*\\d+")) ||
        regex_search(header, regex("\\w+\\s*=\\s*\\w+\\s*\\*\\s*\\d+")) ||
        regex_search(header, regex("\\w+\\s*/=\\s*\\d+")) ||
        regex_search(header, regex("\\w+\\s*=\\s*\\w+\\s*/\\s*\\d+"))) {
        return "O(log n)";
    }
    if (regex_search(header, regex("\\+\\+") ) ||
        regex_search(header, regex("--") ) ||
        regex_search(header, regex("\\+=\\s*1")) ||
        regex_search(header, regex("=\\s*\\w+\\s*\\+\\s*1"))) 
    {
        return "O(n)";
    }
    return "O(n)"; // Default fallback
}
void printComplexity(LoopNode* node, int depth = 0) {
    for (int i = 0; i < depth; ++i) cout << "  ";
    cout << node->loopType << " - " << node->complexity << endl;
    for (auto* child : node->children) {
        printComplexity(child, depth + 1);
    }
}
string multiplyComplexities(const string& a, const string& b) {
    if (a == "O(1)") return b;
    if (b == "O(1)") return a;
    if ((a == "O(n)" && b == "O(log n)") || (a == "O(log n)" && b == "O(n)"))
        return "O(n log n)";
    if (a == "O(n)" && b == "O(n)")
        return "O(n^2)";
    if (a == "O(log n)" && b == "O(log n)")
        return "O(log^2 n)";
    if (a == "O(n^2)" && b == "O(log n)")
        return "O(n^2 log n)";
    if (a == "O(n log n)" && b == "O(n)")
        return "O(n^2 log n)";
    return "O(n * log n)";
}
string calculateTotalComplexity(LoopNode* node) {
    string total = node->complexity;
    for (auto* child : node->children) {
        string childComp = calculateTotalComplexity(child);
        total = multiplyComplexities(total, childComp);
    }
    return total;
}
int main() 
{
    string filename = "code.txt"; // The file containing the code
    ifstream file(filename);  // Open the file
    if (!file.is_open()) {
        cerr << "Error: Could not open the file." << endl;
        return 1;
    }
    stringstream ss;
    ss << file.rdbuf();  // Read the entire file content into stringstream
    file.close();  // Close the file
    string line;
    stack<LoopNode*> loopStack;
    vector<LoopNode*> topLevel; 
    while (getline(ss, line)) {
        if (line.find("for") != string::npos || line.find("while") != string::npos) {
            LoopNode* node = new LoopNode;
            node->loopType = (line.find("for") != string::npos) ? "for" : "while";
            node->loopHeader = line;
            node->complexity = getLoopComplexity(line);
            loopStack.push(node);
        } else if (line.find("}") != string::npos && !loopStack.empty()) {
            LoopNode* node = loopStack.top(); loopStack.pop();
            if (!loopStack.empty()) {
                loopStack.top()->children.push_back(node);
            } else {
                topLevel.push_back(node);
            }
        }
    }
    for (auto* node : topLevel) {
        string final = calculateTotalComplexity(node);
        cout << final << endl;
    }

    return 0;
}
