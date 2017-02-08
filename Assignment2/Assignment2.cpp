#include <string>
#include <vector>
#include <cctype>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <sstream>

/// Collection of helper function to act on strings.
namespace String {
    // Trims a string from the left.
    void ltrim(std::string& s) {
        // Erase from the beginning of the string to wherever the first non-whitespace character is.
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](char a) {
            return !std::isspace((int)a);
        }));
    }
    // Trims a string from the right.
    void rtrim(std::string& s) {
        // Erase from the end of the string to wherever the last non-whitespace character is.
        s.erase(std::find_if(s.rbegin(), s.rend(), [](char a) {
            return !std::isspace((int)a);
        }).base(), s.end()); // base returns the string iterator of the found reverse iterator.
    }
    // Trims a string from both ends.
    void trim(std::string& s) {
        ltrim(s);
        rtrim(s);
    }
}

namespace Input {
    // Compares two strings in a case-insensitive manner.
    bool icompare(const std::string& a, const std::string& b) {
        // Simple shortcut, two strings must be the same length to be the same.
        if (a.length() != b.length()) return false;

        // Not-so-efficient algorithm that equates each pair of characters with the same index in the string objects' char arrays one at a time.
        return std::equal(a.begin(), a.end(), b.begin(), [](char a, char b) {
            // Case insensitive so simply force characters to their lower-case form.
            return std::tolower(a) == std::tolower(b);
        });
    }

    // Gets a user choice between two sets of valid responses. Returns 1 where the user response was in the first set and -1 where the response was in the second set.
    int getBetweenTwoStringSetOptions(std::string message, const std::vector<std::string>& firstStringSet, const std::vector<std::string>& secondStringSet) {
        int response;
        std::string line;
        // Grab a line from the cin buffer.
        while (std::getline(std::cin, line)) {
            // Iterate over valid true strings and compare them with the grabbed line in a case-insensitive manner.
            const auto& itTrue = std::find_if(firstStringSet.begin(), firstStringSet.end(), [&line](const std::string& a) {
                return icompare(a, line);
            });
            // Iterate over valid false strings and compare them with the grabbed line in a case-insensitive manner.
            const auto& itFalse = std::find_if(secondStringSet.begin(), secondStringSet.end(), [&line](const std::string& a) {
                return icompare(a, line);
            });

            // If either iterator is not pointing at the end of their respective string arrays, then the grabbed line must have matched one of the valid strings in one of those arrays.
            if (itTrue != firstStringSet.end()) {
                response = 1;
                break;
            } else if (itFalse != secondStringSet.end()) {
                response = -1;
                break;
            }

            std::cout << "Sorry, the value you inputted was not valid." << std::endl;
            std::cout << message;
        }
        // Can only get here via the if block, but returning inside the if block makes VS throw a warning, I don't like warnings.
        return response;
    }

    // Gets a bool from the user.
    bool getBool() {
        static const std::vector<std::string> validTrues = {
            "yes",
            "y",
            "true",
            "1"
        };
        static const std::vector<std::string> validFalses = {
            "no",
            "n",
            "false",
            "0"
        };

        return getBetweenTwoStringSetOptions("Yay, or nay? [y/n]:\n", validTrues, validFalses) == 1;
    }
}

/// Model that holds the charge data.
class ChargeDataModel {
public:
    ChargeDataModel() {}
    ~ChargeDataModel() {
        dispose();
    }

    void init(std::string filepath = "millikan.dat") {
        m_filepath = filepath;
    }
    void dispose() {
        // Close file is still open.
        if (m_file.is_open()) {
            m_file.close();
        }

        // Clear up memory.
        if (m_charges != nullptr) {
            delete[] m_charges;
            m_charges = nullptr;
        }
    }
    
    double* getChargeData(unsigned int& size) {
        // If we haven't yet loaded data from the file, do so.
        if (m_charges == nullptr) {
            loadDataFromFile(size);
        }
        return m_charges;
    }
private:
    bool openFile() {
        m_file.open(m_filepath, std::ios::in);
        return m_file.is_open();
    }

    unsigned int getLineCount() {
        // Make sure the file stream doesn't skip new lines as per default.
        m_file.unsetf(std::ios_base::skipws);

        // Iterates over chars in the file, counting the 
        // number of newline characters.
        unsigned int count = std::count(
            std::istream_iterator<char>(m_file),
            std::istream_iterator<char>(),
            '\n');

        // Return file to prior condition.
        m_file.clear();
        m_file.seekg(0, std::ios::beg);
        m_file.setf(std::ios_base::skipws);

        return count;
    }

    void loadDataFromFile(unsigned int& size) {
        // If file isn't already open, and failed to open on an attempt, exit the program.
        if (!m_file.is_open() && !openFile()) {
            std::cout << "Could not open file: " << m_filepath << "." << std::endl
                      << "Exiting..." << std::endl;
            std::getchar();
            exit(0);
        }

        // Get line count and allocate enough memory for data.
        unsigned int maxSize = getLineCount();
        m_charges = new double[maxSize];

        unsigned int finalSize = maxSize;
        // Iterate over lines in file and validate them.
        for (unsigned int i = 0; i < maxSize; ++i) {
            // Grab the line.
            std::string line;
            std::getline(m_file, line);

            // Trim whitespace.
            String::trim(line);

            // Put it in a stringstream.
            std::stringstream sstream(line);

            // Out put to a double.
            double possibleCharge = -1.0;
            sstream >> possibleCharge;

            // Test that the rest of the stringstream is empty.
            std::string emptyTest;
            sstream >> emptyTest;

            // If either the possible charge has an invalid value, or the empty test string is not empty, fail.
            if (possibleCharge < 0.0 || !emptyTest.empty()) {
                std::cout << "File: " << m_filepath << " has a corrupt data point." << std::endl
                          << "Skipping that data point." << std::endl;
                --finalSize;
                continue;
            }

            // All's well, push the read charge onto the charge array.
            m_charges[i - maxSize + finalSize] = possibleCharge; // Have to compute the difference between maxSize and finalSize 
                                                                 // and take that off so as to have all data points continiguous in the array.
        }
        size = finalSize;
    }

    std::fstream m_file;
    std::string m_filepath;

    double* m_charges = nullptr;
};

namespace DataAnalysis {
    double computeMean(double* data, unsigned int size) {
        double total = 0.0;
        for (unsigned int i = 0; i < size; ++i) {
            total += data[i];
        }
        return total / (double)size;
    }

    double computeStandardDeviation(double* data, unsigned int size, double mean) {
        double total = 0.0;
        for (unsigned int i = 0; i < size; ++i) {
            total += std::pow(data[i] - mean, 2.0);
        }
        return std::sqrt(total / (double)(size - 1));
    }

    double computeStandardErrorInTheMean(double mean, unsigned int size) {
        return mean / std::sqrt((double)size);
    }
}

int main() {
    // TODO(Matthew): Ask for the name of the file(s) they wish to load.
    std::cout << "Welcome to Matt's impetuous charge calculator!" << std::endl;

    std::vector<std::string> filesToLoad;
    bool shouldContinue;
    do {
        std::cout << "Please enter the name of the file you wish to load:" << std::endl;
        std::string temp;
        std::getline(std::cin, temp);
        filesToLoad.push_back(temp);

        std::cout << "Is there another file you'd like to load? [y/n]" << std::endl;
        shouldContinue = Input::getBool();
    } while (shouldContinue);

    ChargeDataModel model; // Just reuse the same model for each.
    for (std::string& file : filesToLoad) {
        model.init(file);

        unsigned int size;
        const auto& data = model.getChargeData(size);

        double mean = DataAnalysis::computeMean(data, size);
        double standardDeviation = DataAnalysis::computeStandardDeviation(data, size, mean);
        double errorInTheMean = DataAnalysis::computeStandardErrorInTheMean(mean, size);

        std::cout << "File read from: " << file << std::endl;
        std::cout << "    The computed mean is:" << std::endl << "        (" << mean << " +/- " << errorInTheMean << ")C" << std::endl;
        std::cout << "    The computed standard deviation is:" << std::endl << "        " << standardDeviation << "C" << std::endl;

        model.dispose();
    }

    std::cout << "Press any key to exit..." << std::endl;
    std::getchar();
    return 0;
}