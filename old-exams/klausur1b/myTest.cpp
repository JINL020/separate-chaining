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
    ADS_set<unsigned> tester{10, 101, 99};
    // tester.dump();

    cout << "testing y()\n";
    tester.erase(101);
    tester.erase(99);
    cout << tester.y() << "\n";

    // ADS_set<unsigned> tester1{7};
    // // tester.dump();

    // cout << "normal iterator with only 1 element\n";
    // for (const auto& entry : tester1) {
    //     cout << entry << "\n";
    // }

    // cout << "special iterator with only 1 element\n";
    // for (auto i = tester1.y(); i != tester1.end(); ++i) {
    //     cout << *i << "\n";
    // }

    cout << "program is over\n";

    return 0;
}