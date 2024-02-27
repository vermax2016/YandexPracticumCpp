#include <cstdint>
#include <iostream>
#include <limits>

using namespace std;

int main() {
    const auto min = numeric_limits<int64_t>::min();
    const auto max = numeric_limits<uint64_t>::max();

    const auto summ = min + max;

    cout << min << endl;
    cout << max << endl;
    cout << summ << endl;
    cout << 2 * min << endl;
    cout << 2 * max << endl;

    // выведите 5 чисел
}