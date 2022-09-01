#include <exception>
#include <complex>
#include <ipp.h>

int main(int argc, char** argv)
try
{
  const auto size = 1 << 24;
  const auto nthreads = 1;
  const auto flag = IPP_FFT_NODIV_BY_ANY;
  const auto hint = ippAlgHintFast;

  ippInit();
  ippSetNumThreads(nthreads);
  auto input = ippsMalloc_32f(size);
  auto output = ippsMalloc_32f(size);
  int planSize, bufferInitSize, bufferSize;
  ippsDFTGetSize_R_32f(size, flag, hint, &planSize, &bufferInitSize, &bufferSize);
  auto plan = (IppsDFTSpec_R_32f*)ippsMalloc_8u(planSize);
  auto bufferInit = ippsMalloc_8u(bufferInitSize);
  ippsDFTInit_R_32f(size, flag, hint, plan, bufferInit);
  ippFree(bufferInit);
  auto buffer = ippsMalloc_8u(bufferSize);
  ippsDFTFwd_RToCCS_32f(input, output, plan, buffer);
  ippFree(buffer);
  ippFree(plan);
  ippFree(input);
  ippFree(output);

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
