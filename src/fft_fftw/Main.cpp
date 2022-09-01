#include <exception>
#include <complex>
#include <fftw/api/fftw3.h>

int main(int argc, char** argv)
try
{
  const auto size = 1024;
  const auto flag = FFTW_ESTIMATE;
  const auto nthreads = 1;

  if (not fftwf_init_threads())
    throw std::runtime_error("Failed to initialize FFTW threads");

  fftwf_plan_with_nthreads(nthreads);
  float* inputAligned = fftwf_alloc_real(size);
  fftwf_complex* outputAligned = fftwf_alloc_complex(size / 2 + 1);
  fftwf_plan plan = fftwf_plan_dft_r2c_1d(size, inputAligned, outputAligned, flag);
  fftwf_execute(plan);
  fftwf_destroy_plan(plan);
  fftwf_free(inputAligned);
  fftwf_free(outputAligned);
  fftwf_cleanup_threads();

  return EXIT_SUCCESS;
}
catch (const std::exception& e)
{
  return EXIT_FAILURE;
}
catch (...)
{
  return EXIT_FAILURE;
}
