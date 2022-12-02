#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <random>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "ADS_set.h"

using namespace std;

int main() {
    //ADS_set<unsigned> tester{7};
    // ADS_set<unsigned> tester{4, 7, 1, 5, 3, 6, 0, 8, 10, 2, 9, 100, 90, 99, 3, 876, 434, 113};
    ADS_set<unsigned> tester{7, 8, 9, 4, 1, 5, 10};

    cout << "normal iterator with multiple elements\n";
    for (const auto& entry : tester) {
        cout << entry << "\n";
    }

    cout << "special method\n";
    auto pair = tester.x();
    cout << pair.first << ", " << pair.second << "\n";

    cout << "program is over\n";

    return 0;
}