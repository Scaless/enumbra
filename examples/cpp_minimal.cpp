
#include "enumbra_minimal.hpp"

void test_minimal() {

}

// Include both headers to make sure they work together and no 
// multiple include funny business happens
#include "enumbra_test.hpp"
#include "enumbra_minimal.hpp"
#include "enumbra_test.hpp"
#include "enumbra_minimal.hpp"

void test_together() {

}