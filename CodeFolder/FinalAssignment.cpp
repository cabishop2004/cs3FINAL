#include "ChainingHash.h"
#include "ProbingHash.h"
#include "FinalAssignment.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <cassert>

using namespace std;
using namespace chrono;

// Function to detect section headers like I II
bool is_section_header(const string& word) {
    if (word.empty() || word.back() != '.') return false;
    string roman = word.substr(0, word.size() - 1);
    for (char c : roman) {
        if (c != 'I' && c != 'V' && c != 'X') return false;
    }
    return true;
}

string clean_token(const string& token) {
    string temp;
    for (char c : token) {
        if (isalnum(c)) temp += tolower(c);
    }
    return temp;
}

// simple selection sort descending 
template<typename T>
void sort_freq_desc(ResizableArray<pair<T,int>>& arr) {
    for (size_t i = 0; i + 1 < arr.size(); ++i) {
        size_t maxidx = i;
        for (size_t j = i + 1; j < arr.size(); ++j) {
            if (arr[j].second > arr[maxidx].second) maxidx = j;
        }
        if (maxidx != i) swap(arr[i], arr[maxidx]);
    }
}

// simple selection sort ascending
template<typename T>
void sort_freq_asc(ResizableArray<pair<T,int>>& arr) {
    for (size_t i = 0; i + 1 < arr.size(); ++i) {
        size_t minidx = i;
        for (size_t j = i + 1; j < arr.size(); ++j) {
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
        if (h == key_hash && tokens[i] == key) positions.push_back(i + 1);
    }
    return positions;
}

size_t simple_mod_hash(const string& s, size_t hsize) {
    size_t sum = 0;
    for (char c : s) sum += static_cast<unsigned char>(c);
    return sum % hsize;
}

void tokenize(const string& text, ResizableArray<string>& tokens) {
    string token;
    for (char c : text) {
        if (isalnum(c)) {
            token += tolower(c);
        } else if (!token.empty()) {
            tokens.push_back(token);
            token.clear();
        }
    }
    if (!token.empty()) tokens.push_back(token);
}
//sentence counter
size_t count_sentences(const string& text) {
    size_t count = 0;
    for (char c : text) {
        if (c == '.' || c == '!' || c == '?') count++;
    }
    return count;
}

// tests
void run_experiments(const ResizableArray<string>& tokens) {
    const int NUM_RUNS = 10;
    ResizableArray<double> load_factors;
    load_factors.push_back(0.5);
    load_factors.push_back(0.7);
    load_factors.push_back(0.8);
    ResizableArray<size_t> table_sizes;
    table_sizes.push_back(5003);
    table_sizes.push_back(10007);
    table_sizes.push_back(20011);

    cout << "\n=== Experiment 1: Linear Probing with Varying Load Factors ===\n";
    for (size_t i = 0; i < load_factors.size(); ++i) {
        double lf = load_factors[i];
        long long total_time = 0;
        for (int run = 0; run < NUM_RUNS; ++run) {
            ProbingHash<string, int> probe(20011, lf);
            auto start = high_resolution_clock::now();
            for (size_t j = 0; j < tokens.size(); ++j) {
                int v;
                string w = tokens[j];
                probe.find(w, v) ? probe.insert(w, v + 1) : probe.insert(w, 1);
            }
            auto end = high_resolution_clock::now();
            total_time += duration_cast<nanoseconds>(end - start).count();
        }
        cout << "Load factor: " << lf
             << " → Average Time (" << NUM_RUNS << " runs): "
             << (total_time / NUM_RUNS) << " ns\n";
    }

    cout << "\n=== Experiment 2: Chaining with Varying Table Sizes ===\n";
    for (size_t i = 0; i < table_sizes.size(); ++i) {
        size_t sz = table_sizes[i];
        long long total_time = 0;
        for (int run = 0; run < NUM_RUNS; ++run) {
            ChainingHash<string, int> chain(sz);
            auto start = high_resolution_clock::now();
            for (size_t j = 0; j < tokens.size(); ++j) {
                int v;
                string w = tokens[j];
                chain.find(w, v) ? chain.insert(w, v + 1) : chain.insert(w, 1);
            }
            auto end = high_resolution_clock::now();
            total_time += duration_cast<nanoseconds>(end - start).count();
        }
        cout << "Table size: " << sz
             << " → Average Time (" << NUM_RUNS << " runs): "
             << (total_time / NUM_RUNS) << " ns\n";
    }

    cout << "\n=== Experiment 3: Comparing Hash Functions (Chaining) ===\n";
    // Horners Rule
    {
        long long total_time = 0;
        for (int run = 0; run < NUM_RUNS; ++run) {
            ChainingHash<string, int> chain(20011);
            auto start = high_resolution_clock::now();
            for (size_t j = 0; j < tokens.size(); ++j) {
                int v;
                string w = tokens[j];
                chain.find(w, v) ? chain.insert(w, v + 1) : chain.insert(w, 1);
            }
            auto end = high_resolution_clock::now();
            total_time += duration_cast<nanoseconds>(end - start).count();
        }
        cout << "Horner's → Average Time (" << NUM_RUNS << " runs): "
             << (total_time / NUM_RUNS) << " ns\n";
    }
    // Simple Mod Hash
    {
        long long total_time = 0;
        for (int run = 0; run < NUM_RUNS; ++run) {
            ChainingHash<string, int> chain(20011, simple_mod_hash);
            auto start = high_resolution_clock::now();
            for (size_t j = 0; j < tokens.size(); ++j) {
                int v;
                string w = tokens[j];
                chain.find(w, v) ? chain.insert(w, v + 1) : chain.insert(w, 1);
            }
            auto end = high_resolution_clock::now();
            total_time += duration_cast<nanoseconds>(end - start).count();
        }
        cout << "Simple mod hash → Average Time (" << NUM_RUNS << " runs): "
             << (total_time / NUM_RUNS) << " ns\n";
    }

    cout << "\n=== Experiment 4: Collision Handling (Linear Probing) ===\n";
    cout << "Collision resolution uses linear probing: if a collision occurs, probe the next slot using (i + 1) % hsize.\n";
    cout << "This method is based on open addressing, as discussed in class.\n";
}

void menu() {
    cout << "=== Menu === \n"
         << "1. Display 80 most frequent words" << endl
         << "2. Display 80 least frequent words" << endl
         << "3. Search up to 8 keys in 'Engineer’s Thumb'" << endl
         << "4. Count sentences" << endl
         << "5. Run experiments" << endl
         << "0. Exit" << endl
         << "Choice: ";
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <input_file> <output_file>";
        return 1;
    }
    ifstream infile(argv[1]);
    ofstream outfile(argv[2]);
    if (!infile || !outfile) {
        cerr << "Error opening files";
        return 1;
    }

    // Read entire file into a string
    string full_text((istreambuf_iterator<char>(infile)), istreambuf_iterator<char>());

    // Extract Gutenberg body.. might help?
    const string START_MARKER = "*** START OF THIS PROJECT GUTENBERG EBOOK A SCANDAL IN BOHEMIA ***";
    const string END_MARKER   = "*** END OF THIS PROJECT GUTENBERG EBOOK A SCANDAL IN BOHEMIA ***";
    size_t start = full_text.find(START_MARKER);
    size_t end   = full_text.find(END_MARKER);
    string body;
    if (start != string::npos && end != string::npos && end > start) {
        body = full_text.substr(start + START_MARKER.length(),
                                end - (start + START_MARKER.length()));
    } else {
        body = full_text; // fallback to entire text
    }

    // Lowercase everything
    for (size_t i = 0; i < body.size(); ++i) body[i] = tolower(body[i]);

    // Tokenize text
    ResizableArray<string> tokens;
    tokenize(body, tokens);

    // Count sentences
    auto start_sentence_count = high_resolution_clock::now(); // Start timing for sentence count
    size_t sentence_count = count_sentences(body);
    auto end_sentence_count = high_resolution_clock::now(); // End timing for sentence count
    long long sentence_count_runtime_ns = duration_cast<nanoseconds>(end_sentence_count - start_sentence_count).count();

    // Build frequency table
    ResizableArray<pair<string,int>> freq_list;
    for (size_t i = 0; i < tokens.size(); ++i) {
        const string& w = tokens[i];
        bool found = false;
        for (size_t j = 0; j < freq_list.size(); ++j) {
            if (freq_list[j].first == w) {
                freq_list[j].second++;
                found = true;
                break;
            }
        }
        if (!found) freq_list.push_back(make_pair(w, 1));
    }

    
  

    // Insert tokens into hash tables
    const size_t TABLE_SIZE = 20011;
    const double MAX_LOAD = 0.7;
    ChainingHash<string,int> chain_table(TABLE_SIZE);
    ProbingHash<string,int> probe_table(TABLE_SIZE, MAX_LOAD);

    // Insert tokens into hash tables based on sections
    int section = 0;
    int v;
    for (size_t i = 0; i < tokens.size(); ++i) {
        const string& w = tokens[i];

        // Detect section headers and update the section count
        if (is_section_header(w)) {
            section++;
            continue;
        }

        // Insert into the appropriate hash table based on the section
        if (section >= 1 && section <= 6) {
            chain_table.find(w, v) ? chain_table.insert(w, v + 1) : chain_table.insert(w, 1);
        } else if (section >= 7 && section <= 12) {
            probe_table.find(w, v) ? probe_table.insert(w, v + 1) : probe_table.insert(w, 1);
        }
    }

    int choice;
    do {
        menu();
        cin >> choice;
        switch (choice) {
            case 1: {
                ofstream of(argv[2], ios::trunc);
                auto start = high_resolution_clock::now(); // Start timing
                sort_freq_desc(freq_list);
                auto end = high_resolution_clock::now(); // End timing
                of << "Top 80 Words:" << endl;
                for (size_t i = 0; i < 80 && i < freq_list.size(); ++i) {
                    of << freq_list[i].first << ": " << freq_list[i].second << endl;
                }
                of << "Runtime: " << duration_cast<nanoseconds>(end - start).count() << " ns" << endl;
                cout << "Top 80 words written to output file." << endl;
                break;
            }
            case 2: {
                ofstream of(argv[2], ios::trunc);
                auto temp = freq_list;
                auto start = high_resolution_clock::now(); // Start timing
                sort_freq_asc(temp);
                auto end = high_resolution_clock::now(); // End timing
                of << "Bottom 80 Words:" << endl;
                for (size_t i = 0; i < 80 && i < temp.size(); ++i) {
                    of << temp[i].first << ": " << temp[i].second << endl;
                }
                of << "Runtime: " << duration_cast<nanoseconds>(end - start).count() << " ns" << endl;
                cout << "Bottom 80 words written to output file." << endl;
                break;
            }
            case 3: {
                ofstream of(argv[2], ios::trunc);
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Enter up to 8 keys separated by '@@@': ";
                string line;
                getline(cin, line);
                ResizableArray<string> keys;
                size_t pos = 0;
                while ((pos = line.find("@@@")) != string::npos) {
                    keys.push_back(clean_token(line.substr(0, pos)));
                    line.erase(0, pos + 3);
                }
                if (!line.empty()) keys.push_back(clean_token(line));
                auto start = high_resolution_clock::now(); // Start timing
                of << "Key Search Results:" << endl;
                for (size_t k = 0; k < keys.size(); ++k) {
                    auto positions = rabin_karp(tokens, keys[k]);
                    of << "Key '" << keys[k] << "' at positions: ";
                    for (size_t pidx = 0; pidx < positions.size(); ++pidx) {
                        of << positions[pidx] << " ";
                    }
                    of << endl;
                }
                auto end = high_resolution_clock::now(); // End timing
                of << "Runtime: " << duration_cast<nanoseconds>(end - start).count() << " ns" << endl;
                cout << "Key search results written to output file." << endl;
                break;
            }
            case 4: {
                ofstream of(argv[2], ios::trunc);
                of << "Sentence count: " << sentence_count << endl;
                of << "Runtime: " << sentence_count_runtime_ns << " ns" << endl; // Use pre-measured runtime
                cout << "Sentence count written to output file." << endl;
                break;
            }
            case 5: {
                ofstream of(argv[2], ios::trunc);
                streambuf* orig = cout.rdbuf(of.rdbuf());
                run_experiments(tokens); // No runtime output for #5
                cout.rdbuf(orig);
                cout << "Experiments completed. Results written to output file." << endl;
                break;
            }
            case 0:
                cout << "Exiting program." << endl;
                break;
            default:
                cout << "Invalid choice." << endl;
        }
    } while (choice != 0);

    return 0;
}
