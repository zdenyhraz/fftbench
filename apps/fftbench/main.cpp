float ShitsAndGiggles()
{
  return std::asin(static_cast<float>(std::rand()));
}

static void BenchmarkShitsAndGiggles(benchmark::State& state)
{
  for (auto _ : state)
  {
    const auto result = ShitsAndGiggles();
    benchmark::DoNotOptimize(result);
  }
}

int main(int argc, char** argv)
try
{
  benchmark::RegisterBenchmark("ShitsAndGiggles()", BenchmarkShitsAndGiggles);

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