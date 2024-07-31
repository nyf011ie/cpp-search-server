#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

struct Document {
    int id;
    double relevance;
};

class SearchServer {
public:
    void SetDocumentCount(int count) {
        document_count_ = count;
    }

    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document) {
        vector<string> words_in_document = SplitIntoWordsNoStop(document);
        double tf = CalculateTF(words_in_document);
        for(const string &word : words_in_document) {
            word_to_document_freqs_[word][document_id] += tf;
        }
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const Query query_words = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query_words);

        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 return lhs.relevance > rhs.relevance;
             });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

private:
    struct Query {
        set<string> plus;
        set<string> minus;
    };

    int document_count_ = 0;
    map<string, map<int, double>> word_to_document_freqs_;
    set<string> stop_words_;

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    Query ParseQuery(const string& text) const {
        Query query_words;
        for (const string& word : SplitIntoWordsNoStop(text)) {
            if (word[0] == '-' && !IsStopWord(word.substr(1))) {
                query_words.minus.insert(word.substr(1));
            } else {
                query_words.plus.insert(word);
            }
        }
        return query_words;
    }

    vector<Document> FindAllDocuments(const Query& query_words) const {
        vector<Document> matched_documents;
        map<int,double> document_to_relevance;

        for(const auto &word : query_words.plus) {
            if(word_to_document_freqs_.count(word) != 0) {
                double idf = CalculateIDF(word);
                for(auto const &[document_id, tf] : word_to_document_freqs_.at(word)) {
                    document_to_relevance[document_id] += idf * tf;
                }
            }
        }

        for(const auto &word : query_words.minus) {
            if(word_to_document_freqs_.count(word) != 0) {
                for(const auto&[id, data] : word_to_document_freqs_.at(word)) {
                    document_to_relevance.erase(id);
                }
            }
        }

        for (const auto &[id, relevance] : document_to_relevance) {
            matched_documents.push_back({id, (double)relevance});
        }

        return matched_documents;
    }

    double CalculateIDF(const string &word) const {
        return log((double) document_count_ / word_to_document_freqs_.at(word).size());
    }

    double CalculateTF(const vector<string> words_in_document) const {
        return 1.0 / words_in_document.size();
    }
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());
    const int document_count = ReadLineWithNumber();

    search_server.SetDocumentCount(document_count);
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }

    return search_server;
}

int main() {
    const SearchServer search_server = CreateSearchServer();
    const string query = ReadLine();

    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "
             << "relevance = "s << relevance << " }"s << endl;
    }
}
