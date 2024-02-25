#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <algorithm>
#include <utility>
#include <execution>
#include <cmath>
#include <numeric>

const int MAX_RESULT_DOCUMENT_COUNT = 5;

using namespace std;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

vector<int> ReadRating() {
    int count;
    cin >> count;
    vector<int> ratings;
    for (int i = 0; i < count; ++i) {
        int r;
        cin >> r;
        ratings.push_back(r);
    }
    ReadLine();
    return ratings;
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

   static int ComputeAverageRating(const vector<int>& ratings) {

        int summ_ratings = accumulate(ratings.begin(), ratings.end(), 0);
        int rating_size = ratings.size();
        int rating_middle;
        if (!ratings.empty()) {
            return rating_middle = summ_ratings / rating_size;
        }
        else {
            return 0;
        }
    }

    void AddDocument(int document_id, const string& document, const vector<int>& ratings) {
        vector<string> split_into_words = SplitIntoWordsNoStop(document);
        double split_into_word_size = split_into_words.size();

        ++document_count_;

        rating[document_id] = ComputeAverageRating(ratings);

        for (const string& word : split_into_words) {
            double freq_word_in_doc = count(split_into_words.begin(), split_into_words.end(), word);
            double tf = freq_word_in_doc / split_into_word_size;

            word_to_documents_freqs_[word].insert({ document_id, tf });
        }
    }

    vector<Document> FindTopDocuments(const string& query) const {
        vector<Document> find_top_documents = FindAllDocuments(query);
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

    int FindRating(int document_id) const {
        return rating.at(document_id);
    }

private:
    map<string, map<int, double>> word_to_documents_freqs_;
    set<string> stop_words_;
    int document_count_ = 0;
    map<int, int> rating;

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

        for (const string& word : query_words_minus) {
            if (word_to_documents_freqs_.count(word) == 0) {
                continue;
            }
            for (const auto& [document_id, tf] : word_to_documents_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }

        vector<Document> found_documents;
        for (auto [id, relevance] : document_to_relevance) {
            found_documents.push_back({ id, relevance });
        }
        return found_documents;
    }
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        const string read_document = ReadLine();
        vector<int> read_rating = ReadRating();
        search_server.AddDocument(document_id, read_document, read_rating);
    }
    return search_server;
}

int main() {
    const auto create_search_server = CreateSearchServer();

    const string query = ReadLine();
    for (auto [document_id, relevance] : create_search_server.FindTopDocuments(query)) {

        cout << "{ document_id = "s << document_id << ", relevance = "s << relevance << ", rating = "s << //
            create_search_server.FindRating(document_id) << " }"s << endl;
    }
}
