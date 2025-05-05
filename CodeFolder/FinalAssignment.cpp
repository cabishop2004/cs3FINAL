#include "ChainingHash.h"
#include "ProbingHash.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <cassert>

using namespace std;
using namespace chrono;

template<typename T>
class ResizableArray {
public:
    ResizableArray() : data(nullptr), cap(0), len(0) {}
    ~ResizableArray() { delete[] data; }
    void push_back(const T& val) {
        if (len == cap) resize();
        data[len++] = val;
    }
    T& operator[](size_t idx) { assert(idx < len); return data[idx]; }
    const T& operator[](size_t idx) const { assert(idx < len); return data[idx]; }
    size_t size() const { return len; }
private:
    void resize() {
        cap = (cap == 0 ? 10 : cap * 2);
        T* newdata = new T[cap];
        for (size_t i = 0; i < len; ++i) newdata[i] = data[i];
        delete[] data;
        data = newdata;
    }
    T* data;
    size_t cap;
    size_t len;
};

string clean_token(const string& token) {
    string temp;
    for (size_t i = 0; i < token.size(); ++i) {
        if (i + 1 < token.size() && token[i] == '-' && token[i+1] == '-') { ++i; continue; }
        char c = token[i];
        if (isalnum(c) || c == '-') temp += tolower(c);
    }
    return temp;
}

size_t count_sentences(const ResizableArray<string>& tokens) {
    size_t sentences = 0;
    for (size_t i = 0; i < tokens.size(); ++i) {
        const string& tok = tokens[i];
        for (char c : tok) {
            if (c == '.' || c == '!' || c == '?') sentences++;
        }
    }
    return sentences;
}

// Simple selection sort descending by count
void sort_freq_desc(ResizableArray<pair<string,int>>& arr) {
    for (size_t i = 0; i + 1 < arr.size(); ++i) {
        size_t maxidx = i;
        for (size_t j = i+1; j < arr.size(); ++j) {
            if (arr[j].second > arr[maxidx].second) maxidx = j;
        }
        if (maxidx != i) swap(arr[i], arr[maxidx]);
    }
}

// Simple selection sort ascending by count
void sort_freq_asc(ResizableArray<pair<string,int>>& arr) {
    for (size_t i = 0; i + 1 < arr.size(); ++i) {
        size_t minidx = i;
        for (size_t j = i+1; j < arr.size(); ++j) {
            if (arr[j].second < arr[minidx].second) minidx = j;
        }
        if (minidx != i) swap(arr[i], arr[minidx]);
    }
}

ResizableArray<size_t> rabin_karp(const ResizableArray<string>& tokens, const string& key) {
    ResizableArray<size_t> positions;
    size_t key_hash = 0;
    for (char c : key) key_hash = key_hash * 31 + (unsigned char)c;
    for (size_t i = 0; i < tokens.size(); ++i) {
        size_t h = 0;
        for (char c : tokens[i]) h = h * 31 + (unsigned char)c;
        if (h == key_hash && tokens[i] == key) positions.push_back(i+1);
    }
    return positions;
}

void menu() {
    cout << "=== Menu === \n"
         << "1. Display 80 most frequent words" << endl
         << "2. Display 80 least frequent words" << endl
         << "3. Search up to 8 keys in 'Engineer’s Thumb'" << endl
         << "4. Count sentences" << endl
         << "0. Exit" << endl
         << "Choice: ";
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <input_file> <output_file>";
        return 1;
    }
    ifstream infile(argv[1]); ofstream outfile(argv[2]);
    if (!infile || !outfile) {
        cerr << "Error opening files";
        return 1;
    }

    const size_t TABLE_SIZE = 20011;
    const double MAX_LOAD = 0.7;
    ChainingHash<string,int> chain_table(TABLE_SIZE);
    ProbingHash<string,int> probe_table(TABLE_SIZE, MAX_LOAD);

    ResizableArray<string> tokens;
    ResizableArray<pair<string,int>> freq_list;

    int choice;
    infile.clear(); infile.seekg(0);
                // reset
                tokens = ResizableArray<string>();
                freq_list = ResizableArray<pair<string,int>>();
                string word;
                auto start = high_resolution_clock::now();
                while (infile >> word) {
                    string w = clean_token(word);
                    if (w.empty()) continue;
                    tokens.push_back(w);
                    int v;
                    chain_table.find(w,v) ? chain_table.insert(w, v+1) : chain_table.insert(w,1);
                    probe_table.find(w,v) ? probe_table.insert(w, v+1) : probe_table.insert(w,1);
                }
                auto end = high_resolution_clock::now();
                outfile << "Load/Process Time: "
                        << duration_cast<nanoseconds>(end-start).count()
                        << " ns";
               
    do {
        menu(); cin >> choice;
        switch (choice) {
            
            case 1: {
                // build freq_list from tokens
                // first, get unique words and counts
                // naive: for each token, update freq_list
                for (size_t i = 0; i < tokens.size(); ++i) {
                    bool found = false;
                    for (size_t j = 0; j < freq_list.size(); ++j) {
                        if (freq_list[j].first == tokens[i]) {
                            freq_list[j].second++;
                            found = true;
                            break;
                        }
                    }
                    if (!found) freq_list.push_back(make_pair(tokens[i], 1));
                }
                sort_freq_desc(freq_list);
                cout << "Top 80 Words:" << endl;
                for (size_t i = 0; i < 80 && i < freq_list.size(); ++i)
                    cout << freq_list[i].first << ": " << freq_list[i].second << endl;
                break;
            }
            case 2: {
                // lowest frequencies
                // reuse freq_list or rebuild
                ResizableArray<pair<string,int>> temp = freq_list;
                sort_freq_asc(temp);
                cout << "Bottom 80 Words:" << endl;
                for (size_t i = 0; i < 80 && i < temp.size(); ++i)
                    cout << temp[i].first << ": " << temp[i].second << endl;
                     // Rebuild freq_list from scratch
    freq_list = ResizableArray<pair<string,int>>();
    for (size_t i = 0; i < tokens.size(); ++i) {
        bool found = false;
        for (size_t j = 0; j < freq_list.size(); ++j) {
                if (freq_list[j].first == tokens[i]) {
                freq_list[j].second++;
                found = true;
                break;
            }
        }
        if (!found) freq_list.push_back(make_pair(tokens[i],1));
    }
    sort_freq_asc(freq_list);
    cout << "Bottom 80 Words:" << endl;
    for (size_t i = 0; i < 80 && i < freq_list.size(); ++i)
        cout << freq_list[i].first << ": " << freq_list[i].second << endl;
     break;
                break;
            }
            case 3: {
                infile.clear(); infile.seekg(0);
                // naive skip to work IX
                ResizableArray<string> work9;
                size_t count = 0;
                string word;
                while (infile >> word) {
                    if (++count > 5000) work9.push_back(clean_token(word));
                }
                cout << "Enter up to 8 keys separated by '@@@': ";
                string line;
                getline(cin, line);
                getline(cin, line);
                ResizableArray<string> keys;
                size_t pos = 0;
                while ((pos = line.find("@@@")) != string::npos) {
                    keys.push_back(clean_token(line.substr(0, pos)));
                    line.erase(0, pos + 3);
                }
                keys.push_back(clean_token(line));
                for (size_t i = 0; i < keys.size(); ++i) {
                    auto positions = rabin_karp(work9, keys[i]);
                    cout << "Key '"<<keys[i]<<"' at positions: ";
                    for (size_t p = 0; p < positions.size(); ++p) cout << positions[p] << " ";
                    cout << endl;
                }
                break;
            }
            case 4: {
                size_t sentences = count_sentences(tokens);
                cout << "Sentence count: "<<sentences<<endl;
                break;
            }
            case 0:
                cout << "Exiting."<<endl;
                break;
            default:
                cout << "Invalid choice."<<endl;
        }
    } while (choice != 0);
    return 0;
}
