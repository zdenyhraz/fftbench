float Sine()
{
  return std::asin(static_cast<float>(std::rand()));
}

float Power(int power)
{
  return std::pow(static_cast<float>(std::rand()), power);
}

static void BenchmarkPower(benchmark::State& state, int power)
{
  for (auto _ : state)
  {
    const auto result = Power(power);
    benchmark::DoNotOptimize(result);
  }
}

static void BenchmarkSine(benchmark::State& state)
{
  for (auto _ : state)
  {
    const auto result = Sine();
    benchmark::DoNotOptimize(result);
  }
}

int main(int argc, char** argv)
try
{
  benchmark::RegisterBenchmark("Sine", BenchmarkSine);
  for (int power : {0, 1, 2, 3, 4, 5, 6})
    benchmark::RegisterBenchmark(fmt::format("Power{}", power).c_str(), BenchmarkPower, power);

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