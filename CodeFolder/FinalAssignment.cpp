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


// Function to detect section headers like "I.", "II.", etc. 
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
    for (size_t i = 0; i < token.size(); ++i) {
        if (i + 1 < token.size() && token[i] == '-' && token[i+1] == '-') { ++i; continue; }
        char c = token[i];
        if (isalnum(c) || c == '-') temp += tolower(c);
    }
    return temp;
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

size_t simple_mod_hash(const string& s, size_t hsize) {
    size_t sum = 0;
    for (char c : s) sum += static_cast<unsigned char>(c);
    return sum % hsize;
}


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
                probe.find(tokens[j], v) ? probe.insert(tokens[j], v + 1) : probe.insert(tokens[j], 1);
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
                chain.find(tokens[j], v) ? chain.insert(tokens[j], v + 1) : chain.insert(tokens[j], 1);
            }
            auto end = high_resolution_clock::now();
            total_time += duration_cast<nanoseconds>(end - start).count();
        }

        cout << "Table size: " << sz
             << " → Average Time (" << NUM_RUNS << " runs): "
             << (total_time / NUM_RUNS) << " ns\n";
    }

    cout << "\n=== Experiment 3: Comparing Hash Functions (Chaining) ===\n";

    // Horner's Rule
    {
        long long total_time = 0;
        for (int run = 0; run < NUM_RUNS; ++run) {
            ChainingHash<string, int> chain(20011);
            auto start = high_resolution_clock::now();
            for (size_t i = 0; i < tokens.size(); ++i) {
                int v;
                chain.find(tokens[i], v) ? chain.insert(tokens[i], v + 1) : chain.insert(tokens[i], 1);
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
            for (size_t i = 0; i < tokens.size(); ++i) {
                int v;
                chain.find(tokens[i], v) ? chain.insert(tokens[i], v + 1) : chain.insert(tokens[i], 1);
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
         << "5. Run experiments" << endl // New menu option
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

    const size_t TABLE_SIZE = 20011;
    const double MAX_LOAD = 0.7;
    ChainingHash<string,int> chain_table(TABLE_SIZE);
    ProbingHash<string,int> probe_table(TABLE_SIZE, MAX_LOAD);

    ResizableArray<string> tokens;
    ResizableArray<pair<string,int>> freq_list;

    int choice;
    infile.clear(); infile.seekg(0);
    tokens = ResizableArray<string>();
    freq_list = ResizableArray<pair<string,int>>();
    string word;
    size_t sentence_count = 0;

    int section = 0;
    while (infile >> word) {
        if (is_section_header(word)) section++;

        for (char c : word) {
            if (c == '.' || c == '!' || c == '?') sentence_count++;
        }

        string w = clean_token(word);
        if (w.empty()) continue;
        tokens.push_back(w);
        int v;
        if (section >= 1 && section <= 6) {
            chain_table.find(w, v) ? chain_table.insert(w, v + 1) : chain_table.insert(w, 1);
        } else if (section >= 7 && section <= 12) {
            probe_table.find(w, v) ? probe_table.insert(w, v + 1) : probe_table.insert(w, 1);
        }
    }



    do {
        menu(); 
        cin >> choice;
        switch (choice) {
            case 1: {
                ofstream outfile(argv[2], ios::trunc);  // Overwrite file
                auto start = high_resolution_clock::now();
                // build freq_list from tokens
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
                    if (!found) freq_list.push_back(make_pair(tokens[i], 1));
                }
                sort_freq_desc(freq_list);
                outfile << "Top 80 Words:" << endl;
                for (size_t i = 0; i < 80 && i < freq_list.size(); ++i)
                    outfile << freq_list[i].first << ": " << freq_list[i].second << endl;
                cout << "Top 80 words written to output file.\n";

                auto end = high_resolution_clock::now();
                long long runtime_ns = duration_cast<nanoseconds>(end - start).count();
                outfile << "Runtime: " << runtime_ns << " ns\n";
                break;
            }
            case 2: {
                ofstream outfile(argv[2], ios::trunc);  // Overwrite file
                auto start = high_resolution_clock::now();

                ResizableArray<pair<string,int>> temp = freq_list;
                sort_freq_asc(temp);
                outfile << "Bottom 80 Words:" << endl;
                for (size_t i = 0; i < 80 && i < temp.size(); ++i)
                    outfile << temp[i].first << ": " << temp[i].second << endl;
                cout << "Bottom 80 words written to output file.\n";

                auto end = high_resolution_clock::now();
                outfile << "Runtime: " << duration_cast<nanoseconds>(end - start).count() << " ns\n";
                break;
            }
            case 3: {
                ofstream outfile(argv[2], ios::trunc);
                auto start = high_resolution_clock::now();
        
                infile.clear(); infile.seekg(0);
                ResizableArray<string> work9;
                size_t count = 0;
                string word;
                while (infile >> word) {
                    if (++count > 5000) work9.push_back(clean_token(word));
                }
                cout << "Enter up to 8 keys separated by '@@@': ";
                string line;
                getline(cin, line); // flush newline
                getline(cin, line);
                ResizableArray<string> keys;
                size_t pos = 0;
                while ((pos = line.find("@@@")) != string::npos) {
                    keys.push_back(clean_token(line.substr(0, pos)));
                    line.erase(0, pos + 3);
                }
                keys.push_back(clean_token(line));
                outfile << "Key Search Results:" << endl;
                for (size_t i = 0; i < keys.size(); ++i) {
                    auto positions = rabin_karp(work9, keys[i]);
                    outfile << "Key '" << keys[i] << "' at positions: ";
                    for (size_t p = 0; p < positions.size(); ++p) outfile << positions[p] << " ";
                    outfile << endl;
                }
                cout << "Key search results written to output file.\n";
        
                auto end = high_resolution_clock::now();
                outfile << "Runtime: " << duration_cast<nanoseconds>(end - start).count() << " ns\n";
                break;
            }
            case 4: {
                ofstream outfile(argv[2], ios::trunc);
                auto start = high_resolution_clock::now();
        
                outfile << "Sentence count: " << sentence_count << endl;
                cout << "Sentence count written to output file.\n";
        
                auto end = high_resolution_clock::now();
                outfile << "Runtime: " << duration_cast<nanoseconds>(end - start).count() << " ns\n";
                break;
            }
        
            case 5: { // Run experiments
                ofstream outfile(argv[2], ios::trunc);
                outfile << "\n=== Running Experiments ===\n";
                streambuf* cout_buf = cout.rdbuf(); // Save original cout buffer
                cout.rdbuf(outfile.rdbuf());       // Redirect cout to outfile
                run_experiments(tokens);           // Ensure experiments are executed immediately
                cout.rdbuf(cout_buf);              // Restore original cout buffer
                outfile.flush();                   // Flush output to ensure it is written immediately
                cout << "Experiments completed. Results written to output file.\n";
                break;
            }
            case 0: {
                cout << "Exiting program." << endl;
                break;
            }
            default:
                cout << "Invalid choice." << endl;
        }
    } while (choice != 0);

    return 0; // Ensure a clean exit
}
