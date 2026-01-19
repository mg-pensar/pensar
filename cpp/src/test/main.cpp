// author : Mauricio Gomes
// license: MIT (https://opensource.org/licenses/MIT)

#include "../../../unit_test/src/test.hpp"
#include "../../../cpp/src/sysinfo.hpp"

#include "../../../cpp/src/s.hpp"

int main()
{
	namespace pd = pensar_digital::cpplib;
    //unsigned int threads = pd::cpu_threads();
    //std::cout << "Estimated number of CPU threads: " << threads << std::endl;
    namespace test = pensar_digital::unit_test;
    test::all_tests ().run ();
 }
