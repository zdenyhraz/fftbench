#include <exception>
#include <complex>
#include <filesystem>
#include <fstream>
#include <fftw/api/fftw3.h>
#include <fmt/format.h>
#include "utils/Timer.hpp"

using Clock = std::chrono::high_resolution_clock;
using Timepoint = std::chrono::time_point<Clock>;
using Duration = std::chrono::milliseconds;

template <typename T>
int64_t GetDuration(auto tic, auto toc)
{
  return std::chrono::duration_cast<T>(toc - tic).count();
}

void WriteCSVfield(std::ofstream& file, auto value)
{
  file << value << ",";
}

void WriteCSVNewline(std::ofstream& file)
{
  file << std::endl;
}

int main(int argc, char** argv)
try
{
  if (not fftwf_init_threads())
    throw std::runtime_error("Failed to initialize FFTW threads");

  const auto nthreads = 1;
  const auto flag = FFTW_PATIENT;
  const auto exponentMin = 8;  // 8
  const auto exponentMax = 24; // 24
  const auto dataPath = std::filesystem::current_path().parent_path() / "data";
  std::ofstream file(dataPath / "wisdom.csv");

  fftwf_plan_with_nthreads(nthreads);
  WriteCSVfield(file, "size");
  WriteCSVfield(file, "create plan and export wisdom [ms]");
  WriteCSVfield(file, "import wisdom and create plan [ms]");
  WriteCSVfield(file, "execute plan [ms]");
  WriteCSVNewline(file);

  for (auto exponent = exponentMin; exponent <= exponentMax; ++exponent)
  {
    fmt::print("> 2^{} FFT\n", exponent);
    const auto size = 1 << exponent;
    const auto wisdomPath = dataPath / fmt::format("wisdom_{}.fftw", size);
    auto* inputAligned = fftwf_alloc_real(size);
    auto* outputAligned = fftwf_alloc_complex(size / 2 + 1);
    Timepoint tic, toc;
    WriteCSVfield(file, size);

    {
      TIMER(" - create plan and export wisdom");
      tic = Clock::now();
      fftwf_plan plan_wisdom = fftwf_plan_dft_r2c_1d(size, inputAligned, outputAligned, flag);
      if (not fftwf_export_wisdom_to_filename(wisdomPath.c_str()))
        throw std::runtime_error("Failed to export FFTW wisdom");
      toc = Clock::now();
      WriteCSVfield(file, GetDuration<Duration>(tic, toc));
      fftwf_destroy_plan(plan_wisdom);
    }

    fftwf_forget_wisdom();
    fftwf_plan plan;

    {
      TIMER(" - import wisdom and create plan");
      tic = Clock::now();
      if (not fftwf_import_wisdom_from_filename(wisdomPath.c_str()))
        throw std::runtime_error("Failed to import FFTW wisdom");
      plan = fftwf_plan_dft_r2c_1d(size, inputAligned, outputAligned, flag);
      toc = Clock::now();
      WriteCSVfield(file, GetDuration<Duration>(tic, toc));
    }

    {
      TIMER(" - execute plan");
      tic = Clock::now();
      fftwf_execute(plan);
      toc = Clock::now();
      WriteCSVfield(file, GetDuration<Duration>(tic, toc));
    }

    WriteCSVNewline(file);
    fftwf_destroy_plan(plan);
    fftwf_free(inputAligned);
    fftwf_free(outputAligned);
  }

  fftwf_cleanup_threads();
  return EXIT_SUCCESS;
}
catch (const std::exception& e)
{
  fmt::print("Error: {}\n", e.what());
  return EXIT_FAILURE;
}
catch (...)
{
  fmt::print("Error: {}\n", "Unknown error");
  return EXIT_FAILURE;
}
