#include "benchmarks.hpp"
#include "tests.hpp"

// --benchmark_out_format={json|console|csv}
// --benchmark_out=<filename>
int main(int argc, char** argv)
try
{
  static constexpr bool tests = true;
  static constexpr usize testSize = 1024;

  if constexpr (tests)
    RunTests(testSize);

  RegisterBenchmarks();

  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();

  return EXIT_SUCCESS;
}
catch (const std::exception& e)
{
  fmt::print("Error: {}\n", e.what());
  return EXIT_FAILURE;
}
catch (...)
{
  fmt::print("Error: Unknown error\n");
  return EXIT_FAILURE;
}
