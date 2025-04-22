#include <iostream>
#include <fstream>
#include <string>
#include <regex>
using namespace std;
void parseCode(const string& inputFile, const string& outputFile) {
    ifstream inFile(inputFile);
    ofstream outFile(outputFile);
    
    if (!inFile.is_open()) {
        cerr << "Error opening input file." << endl;
        return;
    }
    
    if (!outFile.is_open()) {
        cerr << "Error opening output file." << endl;
        return;
    }

    string line;
    regex loopRegex(R"(\b(for|while)\s*\(.*\)\s*\{)");
    regex functionRegex(R"(\b\w+\s+\w+\s*\(.*\)\s*\{)");
    regex recursiveCallRegex(R"(\b\w+\s*\(.*\))");

    while (getline(inFile, line)) {
        // Check for loops
        if (regex_search(line, loopRegex)) {
            outFile << line << endl; // Write the loop line
        }
        // Check for function definitions
        else if (regex_search(line, functionRegex)) {
            outFile << line << endl; // Write the function definition
        }
        // Check for recursive calls
        else if (regex_search(line, recursiveCallRegex)) {
            outFile << line << endl; // Write the recursive call
        }
    }

    inFile.close();
    outFile.close();
}

int main() {
    string inputFile = "code.txt";  // Input file containing the code to parse
    string outputFile = "parsed_code.txt"; // Output file to store parsed code

    parseCode(inputFile, outputFile);

    cout << "Parsing complete. Parsed code written to " << outputFile << endl;

    return 0;
}