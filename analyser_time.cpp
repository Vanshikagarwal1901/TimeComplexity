#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <stack>
#include <unordered_map>
#include <regex>
#include <cctype> // For std::isspace
#include <filesystem> // For checking if file exists
using namespace std;
namespace fs = std::filesystem;

struct LoopInfo {
    string type; // for/while
    string condition;
    string operation;
    int depth;
};

struct RecursionInfo {
    string baseCaseCondition;
    string recursiveCall;
    int problemReductionFactor; // 1: linear, 2: halving, etc.
    bool isTailRecursive;
};

struct FunctionInfo {
    string name;
    vector<string> calls;
    bool isRecursive = false;
    vector<LoopInfo> loops;
    bool isLogRecursion = false;
    RecursionInfo recursionInfo;
    vector<string> bodyLines;
};

unordered_map<string, FunctionInfo> functions;

// Trim function implementation
string trim(const string& str) {
    auto start = str.begin();
    while (start != str.end() && std::isspace(*start)) {
        ++start;
    }
    
    auto end = str.end();
    do {
        --end;
    } while (end != start && std::isspace(*end));
    
    return string(start, end + 1);
}

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

// Check if line is a conditional statement
bool isConditional(const string& line) {
    return line.find("if(") != string::npos || 
           line.find("if (") != string::npos ||
           line.find("else if(") != string::npos ||
           line.find("else if (") != string::npos ||
           line.find("else") != string::npos ||
           line.find("switch(") != string::npos ||
           line.find("switch (") != string::npos;
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

bool parseAndWriteCode(const string& inputFile, const string& outputFile) {
    // Check if input file exists
    if (!fs::exists(inputFile)) {
        cerr << "Error: Input file '" << inputFile << "' does not exist." << endl;
        return false;
    }

    ifstream inFile(inputFile);
    if (!inFile.is_open()) {
        cerr << "Error: Cannot open input file '" << inputFile << "' for reading." << endl;
        return false;
    }

    ofstream outFile(outputFile);
    if (!outFile.is_open()) {
        cerr << "Error: Cannot open output file '" << outputFile << "' for writing." << endl;
        inFile.close();
        return false;
    }

    cout << "Successfully opened input file: " << inputFile << endl;
    cout << "Successfully opened output file: " << outputFile << endl;

    string line;
    bool inBlockComment = false;
    vector<string> functions;
    vector<string> loopVars;
    stack<char> braceStack;
    vector<bool> inLoopBody;
    vector<string> loopBodyLines;
    vector<string> currentLoopVars;
    int conditionalDepth = 0;
    int lineCount = 0;

    while (getline(inFile, line)) {
        lineCount++;
        removeComments(line, inBlockComment);
        if (line.empty()) continue;

        // Track brace level using stack
        for (char c : line) {
            if (c == '{') {
                braceStack.push(c);
                if (!inLoopBody.empty()) inLoopBody.back() = true;
            }
            else if (c == '}') {
                if (!braceStack.empty()) {
                    braceStack.pop();
                    // Check if we're exiting a loop body
                    if (!inLoopBody.empty() && braceStack.size() < inLoopBody.size()) {
                        // Process collected loop body lines
                        string loopVar = currentLoopVars.back();
                        bool hasUpdates = false;
                        bool hasFunctionCalls = false;
                        bool hasConditionals = false;
                        
                        for (const string& bodyLine : loopBodyLines) {
                            if (!loopVar.empty() && updatesVariable(bodyLine, loopVar)) {
                                hasUpdates = true;
                            }
                            if (containsFunctionCall(bodyLine, functions)) {
                                hasFunctionCalls = true;
                            }
                            if (isConditional(bodyLine)) {
                                hasConditionals = true;
                            }
                            if (isLoopStart(bodyLine)) {
                                hasFunctionCalls = true;
                            }
                        }
                        
                        if (hasUpdates || hasFunctionCalls || hasConditionals) {
                            outFile << loopBodyLines.front() << endl;
                            for (size_t i = 1; i < loopBodyLines.size(); i++) {
                                const string& bodyLine = loopBodyLines[i];
                                if (updatesVariable(bodyLine, loopVar) ||
                                    containsFunctionCall(bodyLine, functions) ||
                                    isConditional(bodyLine) ||
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
            }
        }

        // Check for function definitions
        string funcName;
        if (isFunctionStart(line, funcName)) {
            functions.push_back(funcName);
            outFile << line << endl;
            continue;
        }

        // Check for conditional statements
        if (isConditional(line)) {
            conditionalDepth = braceStack.size();
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

    // Verify all braces were properly closed
    if (!braceStack.empty()) {
        cerr << "Warning: Unbalanced braces detected in input file!" << endl;
    }

    inFile.close();
    outFile.close();
    cout << "Parsed " << lineCount << " lines from input file" << endl;
    cout << "Parsed code written to " << outputFile << endl;
    
    // Verify the output file was created and has content
    ifstream checkOut(outputFile);
    if (!checkOut.is_open()) {
        cerr << "Error: Cannot verify output file was created." << endl;
        return false;
    }
    
    string checkLine;
    int outputLines = 0;
    while (getline(checkOut, checkLine)) {
        outputLines++;
    }
    checkOut.close();
    
    cout << "Output file contains " << outputLines << " lines." << endl;
    return true;
}

string extractFunctionName(const string& line) {
    smatch match;
    regex funcPattern(R"((?:\w+[\s*&]+)+(\w+)\s*\([^\)]*\)\s*\{)");
    if (regex_search(line, match, funcPattern)) {
        return match[1];
    }
    
    // Try a simpler pattern if the complex one fails
    regex simplePattern(R"((\w+)\s*\([^\)]*\)\s*\{)");
    if (regex_search(line, match, simplePattern)) {
        return match[1];
    }
    return "";
}

bool containsBinarySearchCondition(const string& condition) {
    return condition.find("l <= r") != string::npos ||
           condition.find("left <= right") != string::npos ||
           condition.find("low <= high") != string::npos ||
           condition.find("start <= end") != string::npos ||
           condition.find("i < n") != string::npos ||
           condition.find("i >= 0") != string::npos ||
           condition.find("while") != string::npos;
}

bool containsBinarySearchOperation(const string& operation, const vector<string>& bodyLines) {
    static regex midUpdatePattern(R"((l|r|left|right|low|high|start|end)\s*=\s*(mid|((l|r|left|right|low|high|start|end)\s*[-+]\s*1)))");
    if (regex_search(operation, midUpdatePattern)) return true;

    static regex assignmentPattern(R"((l|r|left|right|low|high|start|end)\s*=\s*(mid\s*[\+\-]\s*1|(\w+)\s*[/]\s*2))");
    for (const auto& line : bodyLines) {
        if (regex_search(line, assignmentPattern)) return true;
    }

    static regex divStepPattern(R"((\w+)\s*[/]=\s*2)");
    for (const auto& line : bodyLines) {
        if (regex_search(line, divStepPattern)) return true;
    }

    static regex divAssignPattern(R"((\w+)\s*=\s*\1\s*[/]\s*2\s*;?)");
    for (const auto& line : bodyLines) {
        if (regex_search(line, divAssignPattern)) return true;
    }

    return false;
}

LoopInfo extractLoopInfo(const string& line, int depth, ifstream& inFile) {
    LoopInfo info;
    info.depth = depth;
    if (line.find("for") != string::npos)
        info.type = "for";
    else
        info.type = "while";

    smatch match;
    regex forPattern(R"(for\s*\(([^;]+);([^;]+);([^\)]+)\))");
    regex whilePattern(R"(while\s*\(([^\)]+)\))");

    vector<string> bodyLines;
    streampos pos = inFile.tellg();

    if (regex_search(line, match, forPattern)) {
        info.condition = trim(match[2]);
        info.operation = trim(match[3]);
    } else if (regex_search(line, match, whilePattern)) {
        info.condition = trim(match[1]);
        info.operation = "";

        string tempLine;
        for (int i = 0; i < 5 && getline(inFile, tempLine); ++i) {
            string trimmedLine = trim(tempLine);
            if (!trimmedLine.empty())
                bodyLines.push_back(trimmedLine);
        }
        inFile.clear();
        inFile.seekg(pos);

        if (containsBinarySearchCondition(info.condition) && containsBinarySearchOperation("", bodyLines)) {
            info.operation = "log";
        }
    }

    static regex divStepPattern(R"((\w+)\s*[/]=\s*2)");
    if (regex_search(info.operation, divStepPattern)) {
        info.operation = "log";
    }

    return info;
}

bool isRecursiveCall(const string& line, const string& functionName) {
    regex callPattern("\\b" + functionName + R"(\s*\()");
    return regex_search(line, callPattern);
}

RecursionInfo analyzeRecursionPattern(const string& functionName, const vector<string>& bodyLines) {
    RecursionInfo info;
    info.problemReductionFactor = 1;
    info.isTailRecursive = false;

    // Find base case
    for (const auto& line : bodyLines) {
        if ((line.find("if (") != string::npos || line.find("if(") != string::npos) &&
            line.find("return") != string::npos) {
            info.baseCaseCondition = line;
            break;
        }
    }

    // Find recursive calls and analyze reduction
    for (const auto& line : bodyLines) {
        if (line.find(functionName + "(") != string::npos) {
            info.recursiveCall = line;
            
            // Check tail recursion
            if (line.find("return " + functionName + "(") != string::npos) {
                info.isTailRecursive = true;
            }

            // Analyze problem reduction
            if (line.find("n - 1") != string::npos || line.find("n-1") != string::npos) {
                info.problemReductionFactor = 1;
            } else if (line.find("n / 2") != string::npos || line.find("n/2") != string::npos) {
                info.problemReductionFactor = 2;
            } else if (line.find("mid") != string::npos) {
                info.problemReductionFactor = 2;
            } else if (line.find("n - 2") != string::npos || line.find("n-2") != string::npos) {
                info.problemReductionFactor = 1; // Still linear
            }
        }
    }

    return info;
}

string estimateRecursionComplexity(const RecursionInfo& info) {
    if (info.problemReductionFactor == 1) {
        return "O(n)";
    } else if (info.problemReductionFactor == 2) {
        return "O(log n)";
    } else if (info.isTailRecursive) {
        return "O(n)"; // Tail recursion with default linear
    } else {
        return "O(n) or worse";
    }
}

bool analyzeParsedCode(const string& parsedFile) {
    ifstream inFile(parsedFile);
    if (!inFile.is_open()) {
        cerr << "Error: Cannot open parsed file '" << parsedFile << "' for analysis." << endl;
        return false;
    }

    cout << "Successfully opened parsed file for analysis: " << parsedFile << endl;
    
    functions.clear();

    string line;
    stack<string> context;
    string currentFunction = "";
    int currentDepth = 0;
    vector<string> currentFunctionBody;

    while (getline(inFile, line)) {
        line = trim(line);
        if (line.empty()) continue;

        string potentialFuncName = extractFunctionName(line);
        if (!potentialFuncName.empty()) {
            currentFunction = potentialFuncName;
            functions[currentFunction] = FunctionInfo{currentFunction};
            context.push("function");
            currentDepth = 0;
            currentFunctionBody.clear();
            continue;
        }

        if (!currentFunction.empty()) {
            currentFunctionBody.push_back(line);
        }

        if ((line.find("for") != string::npos || line.find("while") != string::npos) && !currentFunction.empty()) {
            LoopInfo info = extractLoopInfo(line, currentDepth, inFile);
            functions[currentFunction].loops.push_back(info);
            context.push("loop");
            currentDepth++;
            continue;
        }

        if (!currentFunction.empty()) {
            for (auto& functionPair : functions) {
                const auto& calledFuncName = functionPair.first;
                auto& calledFuncInfo = functionPair.second;

                regex callPattern("\\b" + calledFuncName + R"(\s*\()");
                if (regex_search(line, callPattern)) {
                    if (currentFunction == calledFuncName) {
                        calledFuncInfo.calls.push_back(calledFuncName);
                        if (calledFuncName == currentFunction) {
                            calledFuncInfo.isRecursive = true;
                        }
                    }
                }
            }
        }

        if (line == "}" && !context.empty()) {
            string last = context.top();
            context.pop();
            if (last == "loop") currentDepth--;
            if (last == "function") {
                FunctionInfo& finfo = functions[currentFunction];
                finfo.bodyLines = currentFunctionBody;
                
                if (finfo.isRecursive) {
                    finfo.recursionInfo = analyzeRecursionPattern(currentFunction, currentFunctionBody);
                    if (finfo.recursionInfo.problemReductionFactor == 2) {
                        finfo.isLogRecursion = true;
                    }
                }
                currentFunction = "";
                currentFunctionBody.clear();
            }
        }
    }

    inFile.close();

    cout << "Finished reading parsed file. Analyzing functions..." << endl;
    cout << "Found " << functions.size() << " functions to analyze." << endl;

    bool anyRecursion = false;
    for (auto it = functions.begin(); it != functions.end(); ++it) {
        const string& fname = it->first;
        FunctionInfo& finfo = it->second;

        cout << "\nFunction: " << fname << "\n";

        int maxDepth = 0;
        int logCount = 0;
        for (const auto& loop : finfo.loops) 
        {
            maxDepth = max(maxDepth, loop.depth + 1);
            if (loop.operation == "log" || regex_search(loop.operation, regex(R"([/]=|/|\=)"))) {
                logCount++;
            }
        }
        if (finfo.isRecursive) {
            anyRecursion = true;
            cout << "Recursive Function Detected: " << fname << "()\n";
            
            cout << "--- Recursion Analysis ---\n";
            if (!finfo.recursionInfo.baseCaseCondition.empty()) {
                cout << "Base case: " << finfo.recursionInfo.baseCaseCondition << "\n";
            }
            if (!finfo.recursionInfo.recursiveCall.empty()) {
                cout << "Recursive call: " << finfo.recursionInfo.recursiveCall << "\n";
            }
            cout << "Problem reduction factor: " << finfo.recursionInfo.problemReductionFactor << "\n";
            cout << "Tail recursive: " << (finfo.recursionInfo.isTailRecursive ? "Yes" : "No") << "\n";
            
            cout << "--- Time Complexity Estimation ---\n";
            cout << "Estimated Complexity: " << estimateRecursionComplexity(finfo.recursionInfo) << "\n";
            continue;
        }

        cout << "--- Time Complexity Estimation ---\n";
        if (logCount > 0 && maxDepth == 1)
            cout << "Estimated Complexity: O(log n)\n";
        else if (logCount > 0 && maxDepth > 1)
            cout << "Estimated Complexity: O(n^" << maxDepth - logCount << " * log n)\n";
        else if (maxDepth > 0)
            cout << "Estimated Complexity: O(n^" << maxDepth << ")\n";
        else
            cout << "Estimated Complexity: O(1)\n";
    }

    if (!anyRecursion) {
        cout << "\nNo recursive functions detected.\n";
    }
    
    return true;
}

int main() {
    cout << "Current working directory content:" << endl;
    for (const auto& entry : fs::directory_iterator(".")) {
        cout << "  " << entry.path().filename().string() << endl;
    }
    
    string inputFile = "code.txt";
    string parsedFile = "parsed_code.txt";
    
    cout << "\nChecking if input file exists: " << inputFile << endl;
    if (fs::exists(inputFile)) {
        cout << "Input file exists!" << endl;
    } else {
        cout << "Input file does not exist! Creating a sample file for testing." << endl;
        // Create a sample code.txt file
        ofstream sampleCode(inputFile);
        if (sampleCode.is_open()) {
            sampleCode << "int factorial(int n) {" << endl;
            sampleCode << "    if (n <= 1) return 1;" << endl;
            sampleCode << "    return n * factorial(n-1);" << endl;
            sampleCode << "}" << endl;
            sampleCode << endl;
            sampleCode << "int binarySearch(int arr[], int l, int r, int x) {" << endl;
            sampleCode << "    while (l <= r) {" << endl;
            sampleCode << "        int mid = l + (r - l) / 2;" << endl;
            sampleCode << "        if (arr[mid] == x) return mid;" << endl;
            sampleCode << "        if (arr[mid] < x) l = mid + 1;" << endl;
            sampleCode << "        else r = mid - 1;" << endl;
            sampleCode << "    }" << endl;
            sampleCode << "    return -1;" << endl;
            sampleCode << "}" << endl;
            sampleCode.close();
            cout << "Sample file created!" << endl;
        } else {
            cout << "Failed to create sample file!" << endl;
        }
    }
    
    cout << "\nStarting analysis..." << endl;
    if (!parseAndWriteCode(inputFile, parsedFile)) {
        cerr << "Failed to parse and write code." << endl;
        return 1;
    }
    
    cout << "\nParsing completed. Starting analysis..." << endl;
    if (!analyzeParsedCode(parsedFile)) {
        cerr << "Failed to analyze parsed code." << endl;
        return 1;
    }
    
    cout << "\nAnalysis completed successfully!" << endl;
    return 0;
}