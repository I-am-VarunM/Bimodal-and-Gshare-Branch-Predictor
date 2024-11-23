#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <string>
#include <iomanip>
#include <algorithm>

// Base class for branch predictors
class BranchPredictor {
protected:
    std::vector<uint8_t> predictionTable; // Table of 2-bit saturating counters
    uint32_t m; // Number of low-order PC bits used for indexing

public:
    BranchPredictor(uint32_t m) : m(m) {
        predictionTable.resize(1 << m, 2); // Initialize all entries to 2 (weakly taken)
    }

    virtual ~BranchPredictor() {}

    // Pure virtual functions to be implemented by derived classes
    virtual uint32_t getIndex(uint32_t pc) = 0;
    virtual bool predict(uint32_t pc) = 0;
    virtual void update(uint32_t pc, bool taken) = 0;

    // Getter for the prediction table
    const std::vector<uint8_t>& getPredictionTable() const {
        return predictionTable;
    }
};

// Bimodal predictor implementation (n = 0)
class BimodalPredictor : public BranchPredictor {
public:
    BimodalPredictor(uint32_t m) : BranchPredictor(m) {}

    // Calculate index using only the PC
    uint32_t getIndex(uint32_t pc) override {
        return (pc >> 2) & ((1 << m) - 1); // Use bits m+1 through 2 of PC
    }

    // Predict based on the 2-bit counter value
    bool predict(uint32_t pc) override {
        uint32_t index = getIndex(pc);
        return predictionTable[index] > 1; // Predict taken if counter >= 2
    }

    // Update the 2-bit saturating counter
    void update(uint32_t pc, bool taken) override {
        uint32_t index = getIndex(pc);
        if (taken && predictionTable[index] < 3) {
            predictionTable[index]++;
        } else if (!taken && predictionTable[index] > 0) {
            predictionTable[index]--;
        }
    }
};

// Gshare predictor implementation (n > 0)
class GsharePredictor : public BranchPredictor {
private:
    uint32_t n; // Number of global history bits
    uint32_t globalHistory; // Global branch history register

public:
    GsharePredictor(uint32_t m, uint32_t n) : BranchPredictor(m), n(n), globalHistory(0) {
        std::fill(predictionTable.begin(), predictionTable.end(), 2);
    }

    // Calculate index using PC and global history
    uint32_t getIndex(uint32_t pc) override {
        uint32_t pcBits = (pc >> 2) & ((1 << m) - 1);
        uint32_t upperPCBits = pcBits >> (m - n);
        uint32_t lowerPCBits = pcBits & ((1 << (m - n)) - 1);
        uint32_t xorResult = upperPCBits ^ (globalHistory & ((1 << n) - 1));
        return (xorResult << (m - n)) | lowerPCBits;
    }

    // Predict based on the 2-bit counter value
    bool predict(uint32_t pc) override {
        uint32_t index = getIndex(pc);
        return predictionTable[index] > 1; // Predict taken if counter >= 2
    }

    // Update the 2-bit saturating counter and global history
    void update(uint32_t pc, bool taken) override {
        uint32_t index = getIndex(pc);
        if (taken && predictionTable[index] < 3) {
            predictionTable[index]++;
        } else if (!taken && predictionTable[index] > 0) {
            predictionTable[index]--;
        }
        // Update global history register
        globalHistory = (globalHistory >> 1) | ((taken ? 1 : 0) << (n - 1));
    }
};

// Class to run the branch predictor simulation
class BranchPredictorSimulator {
private:
    BranchPredictor* predictor;
    uint64_t totalBranches;
    uint64_t mispredictions;
    std::string predictorType;
    uint32_t m, n;
    std::string traceFile;

public:
    BranchPredictorSimulator(BranchPredictor* p, const std::string& type, uint32_t m, uint32_t n, const std::string& trace)
        : predictor(p), totalBranches(0), mispredictions(0), predictorType(type), m(m), n(n), traceFile(trace) {}

    // Run the simulation using the trace file
    void runSimulation() {
        std::ifstream file(traceFile);
        if (!file.is_open()) {
            std::cerr << "Error opening file: " << traceFile << std::endl;
            return;
        }

        std::string line;
        while (std::getline(file, line)) {
            uint32_t pc;
            char outcome;
            sscanf(line.c_str(), "%x %c", &pc, &outcome);

            bool actualOutcome = (outcome == 't');
            bool prediction = predictor->predict(pc);

            if (prediction != actualOutcome) {
                mispredictions++;
            }

            predictor->update(pc, actualOutcome);
            totalBranches++;
        }

        file.close();
    }

    // Print simulation results
    void printResults() {
        // Print command
        std::cout << "COMMAND" << std::endl;
        std::cout << "./bpsim " << predictorType << " " << m;
        if (predictorType == "gshare") {
            std::cout << " " << n;
        }
        std::cout << " " << traceFile << std::endl;

        // Print output statistics
        std::cout << "OUTPUT" << std::endl;
        std::cout << "number of predictions:\t\t" << totalBranches << std::endl;
        std::cout << "number of mispredictions:\t" << mispredictions << std::endl;
        std::cout << std::fixed << std::setprecision(2);
        double mispredictionRate = (double)mispredictions / totalBranches * 100.0;
        std::cout << "misprediction rate:\t\t" << mispredictionRate << "%" << std::endl;

        // Print final predictor contents
        std::cout << "FINAL " << (predictorType == "gshare" ? "GSHARE" : "BIMODAL") << " CONTENTS" << std::endl;
        const auto& table = predictor->getPredictionTable();
        for (size_t i = 0; i < table.size(); ++i) {
            std::cout << i << "\t" << (int)table[i] << std::endl;
        }
    }
};

int main(int argc, char* argv[]) {
    // Check command line arguments
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <bimodal|gshare> <M> [<N>] <trace_file>" << std::endl;
        return 1;
    }

    std::string predictorType = argv[1];
    uint32_t m = std::stoi(argv[2]);
    std::string traceFile;

    BranchPredictor* predictor;
    BranchPredictorSimulator* simulator;

    // Create appropriate predictor based on command line arguments
    if (predictorType == "bimodal") {
        if (argc != 4) {
            std::cerr << "Usage for bimodal: " << argv[0] << " bimodal <M> <trace_file>" << std::endl;
            return 1;
        }
        predictor = new BimodalPredictor(m);
        traceFile = argv[3];
        simulator = new BranchPredictorSimulator(predictor, predictorType, m, 0, traceFile);
    } else if (predictorType == "gshare") {
        if (argc != 5) {
            std::cerr << "Usage for gshare: " << argv[0] << " gshare <M> <N> <trace_file>" << std::endl;
            return 1;
        }
        uint32_t n = std::stoi(argv[3]);
        predictor = new GsharePredictor(m, n);
        traceFile = argv[4];
        simulator = new BranchPredictorSimulator(predictor, predictorType, m, n, traceFile);
    } else {
        std::cerr << "Invalid predictor type. Use 'bimodal' or 'gshare'." << std::endl;
        return 1;
    }

    // Run simulation and print results
    simulator->runSimulation();
    simulator->printResults();

    // Clean up
    delete simulator;
    delete predictor;
    return 0;
}