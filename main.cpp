#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <algorithm>
#include <utility>
#include <execution>

const int MAX_RESULT_DOCUMENT_COUNT = 5;

using namespace std;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
    cin >> result;
    ReadLine();
    return result;
}

struct Document {
    int id;
    int relevance;
};

struct Query {
  vector<string> PlusWords;
  vector<string> MinusWords;
};

bool HasDocumentGreaterRelevance(const Document& lhs, const Document& rhs) {
		return lhs.relevance > rhs.relevance;
}

class SearchServer {
public:
    vector<string> SplitIntoWords(const string& text) const {
        vector<string> words;
        string word;
        for (const char c : text) {
            if (c == ' ') {
                words.push_back(word);
                word = "";
            } else {
                word += c;
            }
        }
        words.push_back(word);

        return words;
    }

    void SetStopWords(const string& text) {
        
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document) {
        for (const string& word : SplitIntoWordsNoStop(document)) {
        word_to_documents_[word].insert(document_id);
        }
    }

    vector<Document> FindTopDocuments(const string& query) const {
        auto find_top_documents = FindAllDocuments(query);
        sort(execution::par, find_top_documents.begin(), find_top_documents.end(), [](const Document& lhs, const Document& rhs)
            {
            return lhs.relevance > rhs.relevance;
            });
       
        if(find_top_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            find_top_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return find_top_documents;
    }

private:
    map<string, set<int>> word_to_documents_;
    set<string> stop_words_;
    
    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (stop_words_.count(word) == 0) {
            words.push_back(word);
            }
        }
        return words;
    }
    
    Query ParseQuery(const string& text) const {
        Query query;
        
        for(const string& word : SplitIntoWordsNoStop(text)){
            if (word[0] == '-'){
                string NoMinus = word.substr(1);
                query.MinusWords.push_back(NoMinus);
            } else {
                query.PlusWords.push_back(word);
            }
        }
        
        return query;
    }
    
    vector<Document> FindAllDocuments(const string& query) const {
        const vector<string> query_words_plus = ParseQuery(query).PlusWords;
        const vector<string> query_words_minus = ParseQuery(query).MinusWords;
        map<int, int> document_to_relevance;

        for (const string& word : query_words_plus) {
            if (word_to_documents_.count(word) == 0) {
                continue;
            }
            
            for (const int document_id : word_to_documents_.at(word)) {
            ++document_to_relevance[document_id];
            }
        }
        
        for (const string& word : query_words_minus) {
            if (word_to_documents_.count(word) == 0) {
                continue;
            }
            
            for (const int document_id : word_to_documents_.at(word)) {
            document_to_relevance.erase(document_id);
            }
        }
        
        vector<Document> found_documents;
        for (auto [id, relevance] : document_to_relevance) {
            found_documents.push_back({id, relevance});
        }
        return found_documents;
    }
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());
    // Read documents
    const int document_count = ReadLineWithNumber();
    for(int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }

    return search_server;
}

int main() {
    const auto create_search_server = CreateSearchServer();
    
    const string query = ReadLine();
    for (auto [document_id, relevance] : create_search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", relevance = "s << relevance << " }"s << endl;
    }
}
