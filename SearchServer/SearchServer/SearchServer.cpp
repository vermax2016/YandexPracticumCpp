#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <algorithm>
#include <utility>
#include <execution>
#include <cmath>

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
    double relevance;
    int rating;
};

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED
};

class SearchServer {
public:

    vector<string> SplitIntoWords(const string& text) const {
        vector<string> words;
        string word;
        for (const char c : text) {
            if (c == ' ') {
                words.push_back(word);
                word = "";
            }
            else {
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

    void AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings) {
        vector<string> split_into_words = SplitIntoWordsNoStop(document);
        double split_into_word_size = split_into_words.size();

        ++document_count_;

        for (const string& word : split_into_words) {
            double freq_word_in_doc = count(split_into_words.begin(), split_into_words.end(), word);
            double tf = freq_word_in_doc / split_into_word_size;

            word_to_documents_freqs_[word].insert({ document_id, tf });
        }
        rating_document.emplace(document_id, ComputeAverageRating(ratings));
        document_status.emplace(document_id, status);
    }

    vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus status = DocumentStatus::ACTUAL) const {
        vector<Document> find_top_documents = FindAllDocuments(raw_query);
        sort(execution::par, find_top_documents.begin(),           //
            find_top_documents.end(),                              //
            [](const Document& lhs, const Document& rhs)           //
            {
                return lhs.relevance > rhs.relevance;
            });
        vector<Document> top_document_status;
        for (auto [id, relevance, rating] : find_top_documents) {
            if (document_status.at(id) == status) {
                top_document_status.push_back({
                    id,
                    relevance,
                    rating });
            }
        }

        if (top_document_status.size() > MAX_RESULT_DOCUMENT_COUNT) {
            top_document_status.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return top_document_status;
    }

private:
    map<string, map<int, double>> word_to_documents_freqs_;
    set<string> stop_words_;
    int document_count_ = 0;

    map<int, int> rating_document;

    map<int, DocumentStatus> document_status;

    struct Query {
        vector<string> PlusWords;
        vector<string> MinusWords;
    };

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
        vector<string> query_words = SplitIntoWordsNoStop(text);
        for (const string& word : query_words) {
            if (word[0] == '-') {
                string NoMinus = word.substr(1);
                query.MinusWords.push_back(NoMinus);
            }
            else {
                query.PlusWords.push_back(word);
            }
        }
        return query;
    }

    double IdfWordsRequest(const string& Word) const {
        return log(document_count_ * 1.0 / word_to_documents_freqs_.at(Word).size());
    }

    bool HasDocumentGreaterRelevance(const Document& lhs, const Document& rhs) {
        return lhs.relevance > rhs.relevance;
    }

    static int ComputeAverageRating(const vector<int>& ratings) {
        if (ratings.empty()) return 0;

        int summ_ratings = 0;
        for (const int rating : ratings) {
            summ_ratings += rating;
        }
        return summ_ratings / static_cast<int>(ratings.size());
    }

    vector<Document> FindAllDocuments(const string& query) const {
        Query Query_words = ParseQuery(query);
        const vector<string> query_words_plus = Query_words.PlusWords;
        const vector<string> query_words_minus = Query_words.MinusWords;
        map<int, double> document_to_relevance;

        for (const string& word : query_words_plus) {
            if (word_to_documents_freqs_.count(word) == 0) {
                continue;
            }

            const double idf = IdfWordsRequest(word);
            for (const auto [document_id, tf] : word_to_documents_freqs_.at(word)) {
                document_to_relevance[document_id] += tf * idf;
            }
        }
        if (!query_words_minus.empty()) {
            for (const string& word : query_words_minus) {
                if (word_to_documents_freqs_.count(word) == 0) {
                    continue;
                }
                for (const auto& [document_id, tf] : word_to_documents_freqs_.at(word)) {
                    document_to_relevance.erase(document_id);
                }
            }
        }
        
        vector<Document> found_documents;
        for (auto [id, relevance] : document_to_relevance) {
            found_documents.push_back({
                id,
                relevance,
                rating_document.at(id) });
        }
        return found_documents;
    }
};

void PrintDocument(const Document& document) {
    cout << "{ "s
        << "document_id = "s << document.id << ", "s
        << "relevancce = "s << document.relevance << ", "s
        << "rating = "s << document.rating
        << " }"s << endl;
}

int main() {
    SearchServer search_server;
    search_server.SetStopWords("и в на"s);

    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });

    cout << "ACTUAL:"s << endl;
    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s)) {
        PrintDocument(document);
    }

    cout << "BANNED:"s << endl;
    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::BANNED)) {
        PrintDocument(document);
    }

    return 0;
}
