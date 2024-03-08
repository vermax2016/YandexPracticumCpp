#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <algorithm>
#include <utility>
#include <execution>
#include <cmath>
#include <tuple>

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
    int GetDocumentCount() {
        return document_count_;
    }

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
        documents_.emplace(document_id,
            DocumentData{ ComputeAverageRating(ratings),
            status
            });
    }

    vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus status = DocumentStatus::ACTUAL) const {
        const Query query = ParseQuery(raw_query);
        auto find_top_documents = FindAllDocuments(query, status);
        sort(execution::par, find_top_documents.begin(),           //
            find_top_documents.end(),                              //
            [](const Document& lhs, const Document& rhs)           //
            {
                return lhs.relevance > rhs.relevance;
            });
        if (find_top_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            find_top_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return find_top_documents;
    }

    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query, int document_id) const {
        const Query query = ParseQuery(raw_query);
        tuple<vector<string>, DocumentStatus> words_document;
        vector<string> words;
        for (const string& word : query.PlusWords) {
            for (const auto [id, _] : word_to_documents_freqs_.at(word)) {
                if (id == document_id) {
                    words.push_back(word);
                }
            }
        }
        return { words, documents_.at(document_id).status };
    }

private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    map<string, map<int, double>> word_to_documents_freqs_;
    set<string> stop_words_;
    int document_count_ = 0;
    map<int, DocumentData> documents_;

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

    vector<Document> FindAllDocuments(const Query& query, DocumentStatus status) const {
        map<int, double> document_to_relevance;
        for (const string& word : query.PlusWords) {
            if (word_to_documents_freqs_.count(word) == 0) {
                continue;
            }
            const double idf = IdfWordsRequest(word);
            for (const auto [document_id, tf] : word_to_documents_freqs_.at(word)) {
                if (documents_.at(document_id).status == status) {
                    document_to_relevance[document_id] += tf * idf;
                }
            }
        }
        if (!query.MinusWords.empty()) {
            for (const string& word : query.MinusWords) {
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
                documents_.at(id).rating});
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

void PrintMatchDocumentResult(int document_id, const vector<string>& words, DocumentStatus status) {
    cout << "{ "s
        << "document_id = "s << document_id << ", "s
        << "status = "s << static_cast<int>(status) << ", "s
        << "words ="s;
    for (const string& word : words) {
        cout << ' ' << word;
    }
    cout << "}"s << endl;
}

int main() {
    setlocale(LC_ALL, "Russian");
    SearchServer search_server;
    search_server.SetStopWords("и в на"s);

    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });

    const int document_count = search_server.GetDocumentCount();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        const auto [words, status] = search_server.MatchDocument("пушистый кот"s, document_id);
        PrintMatchDocumentResult(document_id, words, status);
    }
}
