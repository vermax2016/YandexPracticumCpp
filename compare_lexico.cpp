#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

bool comp(char s1, char s2) {
    return tolower(s1) < tolower(s2);
}
int main() {
    int n;
    cin >> n;
    vector<string> words;
    for(int i = 0; i < n; ++i) {
        string word;
        cin >> word;
        words.push_back(word);
    }
    sort(words.begin(), words.end(), [](string& Word1, string& word2)
        {
            return lexicographical_compare(Word1.begin(), Word1.end(), word2.begin(), word2.end(), comp);
        });
    for(string word : words) {
        cout << word << " ";
    }
    cout << "\n";
}
