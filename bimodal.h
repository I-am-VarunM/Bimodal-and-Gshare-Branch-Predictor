#ifndef BIMODAL_H
#define BIMODAL_H

#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

class bimodal {
private:
    std::vector<int> branch_predictor_table;
    int size;
    unsigned long predictions;
    unsigned long mispredictions;

public:
    void initialize(int m) {
        size = 1 << m;
        branch_predictor_table.resize(size, 2);  // Initialize all counters to 2 (weakly taken)
        predictions = 0;
        mispredictions = 0;
    }

    int update_table(unsigned long address, int actual_branch_value) {
        predictions++;
        unsigned long index = (address >> 2) & (size - 1);
        int predicted_value = branch_predictor_table[index] >= 2 ? 1 : 0;

        if (predicted_value != actual_branch_value) {
            mispredictions++;
        }

        if (actual_branch_value == 1 && branch_predictor_table[index] < 3) {
            branch_predictor_table[index]++;
        } else if (actual_branch_value == 0 && branch_predictor_table[index] > 0) {
            branch_predictor_table[index]--;
        }

        return predicted_value;
    }

    void print_output() {
        std::cout << "OUTPUT" << std::endl;
        std::cout << " number of predictions:    " << predictions << std::endl;
        std::cout << " number of mispredictions: " << mispredictions << std::endl;
        std::cout << std::fixed << std::setprecision(2);
        std::cout << " misprediction rate:       " << (mispredictions * 100.0 / predictions) << "%" << std::endl;
        std::cout << "FINAL BIMODAL CONTENTS" << std::endl;
        for (int i = 0; i < size; i++) {
            std::cout << " " << i << "\t" << branch_predictor_table[i] << std::endl;
        }
    }
};

#endif // BIMODAL_H