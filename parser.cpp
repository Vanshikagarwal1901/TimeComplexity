#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;

// Comment removal without regex
void removeComments(string& line, bool& inBlockComment) {
    if (inBlockComment) {
        size_t endPos = line.find("*/");
        if (endPos != string::npos) {
            line = line.substr(endPos + 2);
            inBlockComment = false;
            removeComments(line, inBlockComment);
        } else {
            line.clear();
            return;
        }
    }

    size_t blockStart = line.find("/*");
    if (blockStart != string::npos) {
        size_t blockEnd = line.find("*/", blockStart + 2);
        if (blockEnd != string::npos) {
            line.erase(blockStart, blockEnd - blockStart + 2);
            removeComments(line, inBlockComment);
        } else {
            line.erase(blockStart);
            inBlockComment = true;
        }
    }

    size_t lineComment = line.find("//");
    if (lineComment != string::npos) {
        line.erase(lineComment);
    }

    // Trim whitespace
    line.erase(0, line.find_first_not_of(" \t"));
    line.erase(line.find_last_not_of(" \t") + 1);
}

// Extract variable name from while condition
string extractWhileVar(const string& line) {
    size_t whilePos = line.find("while");
    if (whilePos == string::npos) return "";
    
    size_t openParen = line.find('(', whilePos);
    if (openParen == string::npos) return "";
    
    size_t closeParen = line.find(')', openParen);
    if (closeParen == string::npos) return "";
    
    string condition = line.substr(openParen + 1, closeParen - openParen - 1);
    
    // Find first valid variable name in condition
    size_t varStart = condition.find_first_not_of(" (&|!=");
    if (varStart == string::npos) return "";
    
    size_t varEnd = condition.find_first_of(" )&|!=<", varStart);
    if (varEnd == string::npos) varEnd = condition.length();
    
    string var = condition.substr(varStart, varEnd - varStart);
    return var;
}

// Check if line updates a variable
bool updatesVariable(const string& line, const string& var) {
    if (var.empty()) return false;
    
    // Check for var++ or var--
    if (line.find(var + "++") != string::npos || 
        line.find(var + "--") != string::npos) {
        return true;
    }
    
    // Check for ++var or --var
    if (line.find("++" + var) != string::npos || 
        line.find("--" + var) != string::npos) {
        return true;
    }
    
    // Check for var = ... or var +=, var -= etc.
    size_t assignPos = line.find(var);
    while (assignPos != string::npos) {
        // Check if this is actually our variable (not part of another word)
        bool isWholeWord = true;
        if (assignPos > 0 && isalnum(line[assignPos - 1])) isWholeWord = false;
        if (assignPos + var.length() < line.length() && 
            isalnum(line[assignPos + var.length()])) isWholeWord = false;
            
        if (isWholeWord) {
            size_t opPos = line.find_first_of("=+-*/%&|^", assignPos + var.length());
            if (opPos != string::npos && line[opPos] == '=') {
                return true;
            }
        }
        assignPos = line.find(var, assignPos + 1);
    }
    
    return false;
}

// Check if line contains a function call
bool containsFunctionCall(const string& line, const vector<string>& functions) {
    for (const string& func : functions) {
        size_t callPos = line.find(func + "(");
        if (callPos != string::npos) {
            // Check if it's really a function call and not a declaration
            if (line.find(';', callPos) != string::npos || 
                line.find('}', callPos) != string::npos) {
                return true;
            }
        }
    }
    return false;
}

// Check if line starts a loop
bool isLoopStart(const string& line) {
    return line.find("for(") != string::npos || 
           line.find("while(") != string::npos ||
           line.find("for (") != string::npos ||
           line.find("while (") != string::npos;
}

// Check if line starts a function
bool isFunctionStart(const string& line, string& funcName) {
    size_t openParen = line.find('(');
    size_t closeParen = line.find(')');
    size_t openBrace = line.find('{');
    
    if (openParen == string::npos || closeParen == string::npos || openBrace == string::npos)
        return false;
    if (openParen >= closeParen || closeParen >= openBrace)
        return false;
    
    // Extract function name
    size_t nameStart = line.substr(0, openParen).find_last_of(" *&");
    if (nameStart == string::npos) nameStart = 0;
    else nameStart++;
    
    funcName = line.substr(nameStart, openParen - nameStart);
    return !funcName.empty();
}

void parseCode(const string& inputFile, const string& outputFile) {
    ifstream inFile(inputFile);
    ofstream outFile(outputFile);

    if (!inFile.is_open() || !outFile.is_open()) {
        cerr << "Error opening files." << endl;
        return;
    }

    string line;
    bool inBlockComment = false;
    vector<string> functions;
    vector<string> loopVars;
    vector<int> braceStack;
    vector<bool> inLoopBody;
    vector<string> loopBodyLines;
    vector<string> currentLoopVars;

    while (getline(inFile, line)) {
        removeComments(line, inBlockComment);
        if (line.empty()) continue;

        // Track brace level
        size_t openBraces = count(line.begin(), line.end(), '{');
        size_t closeBraces = count(line.begin(), line.end(), '}');
        
        // Update brace stack
        for (size_t i = 0; i < openBraces; i++) {
            braceStack.push_back(1);
            if (!inLoopBody.empty()) inLoopBody.back() = true;
        }
        for (size_t i = 0; i < closeBraces; i++) {
            if (!braceStack.empty()) braceStack.pop_back();
            if (!inLoopBody.empty() && braceStack.size() < inLoopBody.size()) {
                // End of loop body - process collected lines
                string loopVar = currentLoopVars.back();
                bool hasUpdates = false;
                bool hasFunctionCalls = false;
                
                // Check for important content in loop body
                for (const string& bodyLine : loopBodyLines) {
                    if (!loopVar.empty() && updatesVariable(bodyLine, loopVar)) {
                        hasUpdates = true;
                    }
                    if (containsFunctionCall(bodyLine, functions)) {
                        hasFunctionCalls = true;
                    }
                    if (isLoopStart(bodyLine)) {
                        hasFunctionCalls = true; // Treat nested loops as important
                    }
                }
                
                // Output loop body if it has important content
                if (hasUpdates || hasFunctionCalls) {
                    outFile << loopBodyLines.front() << endl; // Loop start line
                    for (size_t i = 1; i < loopBodyLines.size(); i++) {
                        const string& bodyLine = loopBodyLines[i];
                        if (updatesVariable(bodyLine, loopVar) ||
                            containsFunctionCall(bodyLine, functions) ||
                            isLoopStart(bodyLine) ||
                            bodyLine.find('}') != string::npos) {
                            outFile << bodyLine << endl;
                        }
                    }
                }
                
                loopBodyLines.clear();
                currentLoopVars.pop_back();
                inLoopBody.pop_back();
            }
        }

        // Check for function definitions
        string funcName;
        if (isFunctionStart(line, funcName)) {
            functions.push_back(funcName);
            outFile << line << endl;
            continue;
        }

        // Check for loop starts
        if (isLoopStart(line) && !braceStack.empty()) {
            string loopVar;
            if (line.find("while") != string::npos) {
                loopVar = extractWhileVar(line);
            }
            currentLoopVars.push_back(loopVar);
            inLoopBody.push_back(false);
            loopBodyLines.push_back(line);
            continue;
        }

        // If we're collecting loop body, add the line
        if (!loopBodyLines.empty()) {
            loopBodyLines.push_back(line);
        }
        // Otherwise check for other constructs to keep
        else {
            if (containsFunctionCall(line, functions)) {
                outFile << line << endl;
            }
            else if (line.find('}') != string::npos) {
                outFile << line << endl;
            }
        }
    }

    inFile.close();
    outFile.close();
}

int main() {
    string inputFile = "code.txt";
    string outputFile = "parsed_code.txt";

    parseCode(inputFile, outputFile);

    cout << "Parsing complete. Results written to " << outputFile << endl;
    return 0;
}