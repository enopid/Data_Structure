
#include "MyVector.h"

using namespace std;

int main() {
	MyVector<string> vec(3, "Init");
	for(int i=0; i < 20; ++i, vec.print_info()) {
		vec.push_back(to_string(i + 1) + "val");
	}
	vec.print_elements();
}