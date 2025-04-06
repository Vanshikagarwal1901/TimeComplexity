#include<bits/stdc++.h>
using namespace std;

string removeFunctions(string code){
    int pos = 0;
    while ((pos = code.find("void", pos)) != string::npos) {
        size_t end = code.find("{", pos + 4);
        if (end == string::npos) break;
        code.erase(pos, end - pos + 1);
    }
    pos = 0;
    while ((pos = code.find("int", pos)) != string::npos) {
        size_t end = code.find("{", pos + 3);
        if (end == string::npos) break;
        code.erase(pos, end - pos + 1);
    }
    pos = 0;
    while ((pos = code.find("float", pos)) != string::npos) {
        size_t end = code.find("{", pos + 5);
        if (end == string::npos) break;
        code.erase(pos, end - pos + 1);
    }
    pos = 0;
    while ((pos = code.find("double", pos)) != string::npos) {
        size_t end = code.find("{", pos + 6);
        if (end == string::npos) break;
        code.erase(pos, end - pos + 1);
    }
    pos = 0;
    while ((pos = code.find("char", pos)) != string::npos) {
        size_t end = code.find("{", pos + 4);
        if (end == string::npos) break;
        code.erase(pos, end - pos + 1);
    }
    pos = 0;
    while ((pos = code.find("bool", pos)) != string::npos) {
        size_t end = code.find("{", pos + 4);
        if (end == string::npos) break;
        code.erase(pos, end - pos + 1);
    }
    pos = 0;
    while ((pos = code.find("string", pos)) != string::npos) {
        size_t end = code.find("{", pos + 6);
        if (end == string::npos) break;
        code.erase(pos, end - pos + 1);
    }
    pos = 0;
    while ((pos = code.find("vector", pos)) != string::npos) {
        size_t end = code.find("{", pos + 6);
        if (end == string::npos) break;
        code.erase(pos, end - pos + 1);
    }
    pos = 0;
    while ((pos = code.find("map", pos)) != string::npos) {
        size_t end = code.find("{", pos + 3);
        if (end == string::npos) break;
        code.erase(pos, end - pos + 1);
    }
    pos = 0;
    while ((pos = code.find("set", pos)) != string::npos) {
        size_t end = code.find("{", pos + 3);
        if (end == string::npos) break;
        code.erase(pos, end - pos + 1);
    }
    pos = 0;
    while ((pos = code.find("unordered_map", pos)) != string::npos) {
        size_t end = code.find("{", pos + 14);
        if (end == string::npos) break;
        code.erase(pos, end - pos + 1);
    }
    pos = 0;
    while ((pos = code.find("unordered_set", pos)) != string::npos) {
        size_t end = code.find("{", pos + 13);
        if (end == string::npos) break;
        code.erase(pos, end - pos + 1);
    }
    pos = 0;
    while ((pos = code.find("list", pos)) != string::npos) {
        size_t end = code.find("{", pos + 4);
        if (end == string::npos) break;
        code.erase(pos, end - pos + 1);
    }
    pos = 0;
    while ((pos = code.find("deque", pos)) != string::npos) {
        size_t end = code.find("{", pos + 5);
        if (end == string::npos) break;
        code.erase(pos, end - pos + 1);
    }
    pos = 0;
    while ((pos = code.find("stack", pos)) != string::npos) {
        size_t end = code.find("{", pos + 5);
        if (end == string::npos) break;
        code.erase(pos, end - pos + 1);
    }
    pos = 0;
    while ((pos = code.find("queue", pos)) != string::npos) {
        size_t end = code.find("{", pos + 5);
        if (end == string::npos) break;
        code.erase(pos, end - pos + 1);
    }
    pos = 0;
    while ((pos = code.find("priority_queue", pos)) != string::npos) {
        size_t end = code.find("{", pos + 15);
        if (end == string::npos) break;
        code.erase(pos, end - pos + 1);
    }
    pos = 0;
    while ((pos = code.find("pair", pos)) != string::npos) {
        size_t end = code.find("{", pos + 4);
        if (end == string::npos) break;
        code.erase(pos, end - pos + 1);
    }
    pos = 0;
    while ((pos = code.find("tuple", pos)) != string::npos) {
        size_t end = code.find("{", pos + 5);
        if (end == string::npos) break;
        code.erase(pos, end - pos + 1);
    }
    pos = 0;
    while ((pos = code.find("function", pos)) != string::npos) {
        size_t end = code.find("{", pos + 8);
        if (end == string::npos) break;
        code.erase(pos, end - pos + 1);
    }
    pos = 0;
    while ((pos = code.find("template", pos)) != string::npos) {
        size_t end = code.find("{", pos + 8);
        if (end == string::npos) break;
        code.erase(pos, end - pos + 1);
    }
    return code;
}

int countLoopsAndRecursion(const string& code) {
    int loopCount = 0;
    int recursionCount = 0;

    size_t pos = 0;

    while ((pos = code.find("for(", pos)) != string::npos) {
        loopCount++;
        pos += 4; // Move past "for("
    }

    pos = 0; 
    
    while ((pos = code.find("while(", pos)) != string::npos) {
        loopCount++;
        pos += 6; 
    }

    pos = 0; 
    while ((pos = code.find("functionName(", pos)) != string::npos) {
        recursionCount++;
        pos += 13; 
    }

    cout << "Number of loops: " << loopCount << endl;
    cout << "Number of recursive calls: " << recursionCount << endl;
    return loopCount + recursionCount; 
}

int main(){
    ifstream file("code.txt");
    if (!file.is_open()) {
        cout << "Error opening file" << endl;
        return 1;
    }
    string code((istreambuf_iterator<char>(file)),istreambuf_iterator<char>());
    file.close();
    // Remove comments
    size_t pos = 0;
    while ((pos = code.find("/*", pos)) != string::npos) {
        size_t end = code.find("*/", pos + 2);
        if (end == string::npos) break;
        code.erase(pos, end - pos + 2);
    }
    pos = 0;
    while ((pos = code.find("//", pos)) != string::npos) {
        size_t end = code.find("\n", pos + 2);
        if (end == string::npos) break;
        code.erase(pos, end - pos);
    }
    // Remove string literals
    pos = 0;
    while ((pos = code.find("\"", pos)) != string::npos) {
        size_t end = code.find("\"", pos + 1);
        if (end == string::npos) break;
        code.erase(pos, end - pos + 1);
    }
    // Remove character literals
    pos = 0;
    while ((pos = code.find("'", pos)) != string::npos) {
        size_t end = code.find("'", pos + 1);
        if (end == string::npos) break;
        code.erase(pos, end - pos + 1);
    }
    // Remove preprocessor directives
    pos = 0;
    while ((pos = code.find("#", pos)) != string::npos) {
        size_t end = code.find("\n", pos + 1);
        if (end == string::npos) break;
        code.erase(pos, end - pos + 1);
    }
    // Remove empty lines
    pos = 0;
    while ((pos = code.find("\n\n", pos)) != string::npos) {
        code.erase(pos, 1);
    }
    // Remove extra spaces
    pos = 0;
    while ((pos = code.find("  ", pos)) != string::npos) {
        code.erase(pos, 1);
    }
    // Remove leading and trailing spaces
    pos = 0;
    while ((pos = code.find(" ", pos)) != string::npos) {
        if (pos == 0 || code[pos - 1] == '\n') {
            code.erase(pos, 1);
        } else {
            pos++;
        }
    }
    pos = code.length() - 1;
    while (pos > 0 && (code[pos] == ' ' || code[pos] == '\n')) {
        code.erase(pos, 1);
        pos--;
    } 

    cout<<"Code after removing functions and comments:\n"<<code<<endl;
    cout<<"Code Complexity: "<<countLoopsAndRecursion(code)<<endl;

    return 0;
}