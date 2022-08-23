#include "Benchmarks.hpp"
#include "Tests.hpp"

static void Init()
{
  fftwf_init_threads();
#ifdef ENABLE_IPP
  ippInit();
  ippSetNumThreads(1);
  const auto libVersion = ippGetLibVersion();
  fmt::print("IPP version: {} {}\n", libVersion->Name, libVersion->Version);
#endif
}

// --benchmark_out_format={json|console|csv}
// --benchmark_out=<filename>
// --benchmark_out_format=csv --benchmark_out=../data/fftbench.csv
int main(int argc, char** argv)
try
{
  Init();

  static constexpr usize testSize = 1024;
  if constexpr (testSize > 0)
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
