#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

bool Comp(char s1, char s2) {
    return tolower(s1) < tolower(s2);
}
int main() {
    int n;
    cin >> n;
    vector<string> Words;
    for(int i = 0; i < n; ++i) {
        string Word;
        cin >> Word;
        Words.push_back(Word);
    }
    sort(Words.begin(), Words.end(), [](string& Word1, string& Word2)
        {
            return lexicographical_compare(Word1.begin(), Word1.end(), Word2.begin(), Word2.end(), Comp);
        });
    for(string Word : Words) {
        cout << Word << " ";
    }
    cout << "\n";
}
