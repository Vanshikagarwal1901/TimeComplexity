#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <regex>
#include <unordered_map>

using namespace std;

// Function to analyze the time complexity of a loop
string analyzeLoop(const string& loop) {
    // Simple analysis for for and while loops
    if (loop.find("for") != string::npos) {
        // Extract the loop variable and range
        regex forRegex(R"(for\s*\(([^;]*);([^;]*);([^)]*)\))");
        smatch match;
        if (regex_search(loop, match, forRegex)) {
            // Assuming the loop runs from 0 to n
            return "O(n)"; // Simplified assumption
        }
    } else if (loop.find("while") != string::npos) {
        return "O(n)"; // Simplified assumption
    }
    return "O(1)"; // Default case
}

// Function to analyze the time complexity of a recursive function
string analyzeRecursion(const string& functionName) {
    // Placeholder for recursive function analysis
    // You would need to implement a more complex analysis here
    return "O(n)"; // Simplified assumption
}

// Function to analyze the code
void analyzeCode(const string& code) {
    // Split the code into lines for analysis
    vector<string> lines;
    stringstream ss(code);
    string line;
    while (getline(ss, line)) {
        lines.push_back(line);
    }

    // Analyze each line for loops and recursive calls
    for (const string& line : lines) {
        if (line.find("for") != string::npos || line.find("while") != string::npos) {
            string complexity = analyzeLoop(line);
            cout << "Loop found: " << line << " - Time Complexity: " << complexity << endl;
        } else if (line.find("void") != string::npos || line.find("int") != string::npos) {
            // Assuming the function name is the first word after return type
            string functionName = line.substr(0, line.find('('));
            string complexity = analyzeRecursion(functionName);
            cout << "Recursive function found: " << functionName << " - Time Complexity: " << complexity << endl;
        }
    }
}

int main() {
    ifstream inputFile("parsed_code.txt");
    if (!inputFile) {
        cerr << "Error opening file!" << endl;
        return 1; // Exit with error code
    }

    stringstream buffer;
    buffer << inputFile.rdbuf(); // Read the file content into the buffer
    string parsedCode = buffer.str(); // Convert buffer to string

    analyzeCode(parsedCode); // Analyze the code

    return 0;
}