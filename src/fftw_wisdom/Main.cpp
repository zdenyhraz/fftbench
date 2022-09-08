#include <exception>
#include <complex>
#include <filesystem>
#include <fftw/api/fftw3.h>
#include <fmt/format.h>
#include "utils/Timer.hpp"

int main(int argc, char** argv)
try
{
  const auto size = 1 << 24;
  const auto nthreads = 1;
  const auto flag = FFTW_MEASURE;
  const auto path = std::filesystem::current_path().parent_path() / fmt::format("data/wisdom_{}.fftw", size);

  if (not fftwf_init_threads())
    throw std::runtime_error("Failed to initialize FFTW threads");

  fftwf_plan_with_nthreads(nthreads);
  float* inputAligned = fftwf_alloc_real(size);
  fftwf_complex* outputAligned = fftwf_alloc_complex(size / 2 + 1);

  {
    TIMER("Create plan and export wisdom");
    fftwf_plan plan = fftwf_plan_dft_r2c_1d(size, inputAligned, outputAligned, flag);
    if (not fftwf_export_wisdom_to_filename(path.c_str()))
      throw std::runtime_error("Failed to export FFTW wisdom");
    fftwf_destroy_plan(plan);
  }

  fftwf_forget_wisdom();

  {
    TIMER("Import wisdom and create plan");
    if (not fftwf_import_wisdom_from_filename(path.c_str()))
      throw std::runtime_error("Failed to import FFTW wisdom");
    fftwf_plan plan = fftwf_plan_dft_r2c_1d(size, inputAligned, outputAligned, flag);
    // fftwf_execute(plan);
    // fftwf_destroy_plan(plan);
  }

  fftwf_free(inputAligned);
  fftwf_free(outputAligned);
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
