#include <exception>
#include <complex>
#include <ipp.h>

int main(int argc, char** argv)
try
{
  const auto size = 1024;
  const auto flag = IPP_FFT_NODIV_BY_ANY;
  const auto nthreads = 1;
  const auto hint = ippAlgHintFast;

  ippInit();
  ippSetNumThreads(nthreads);
  auto pSrc = ippsMalloc_32f(size);
  auto pDst = ippsMalloc_32f(size);
  int sizeDFTSpec, sizeDFTInitBuf, sizeDFTWorkBuf;
  ippsDFTGetSize_R_32f(size, flag, hint, &sizeDFTSpec, &sizeDFTInitBuf, &sizeDFTWorkBuf);
  auto pDFTSpec = (IppsDFTSpec_R_32f*)ippsMalloc_8u(sizeDFTSpec);
  auto pDFTInitBuf = ippsMalloc_8u(sizeDFTInitBuf);
  auto pDFTWorkBuf = ippsMalloc_8u(sizeDFTWorkBuf);
  ippsDFTInit_R_32f(size, flag, hint, pDFTSpec, pDFTInitBuf);
  if (pDFTInitBuf)
    ippFree(pDFTInitBuf);

  ippsDFTFwd_RToCCS_32f(pSrc, pDst, pDFTSpec, pDFTWorkBuf);

  if (pDFTWorkBuf)
    ippFree(pDFTWorkBuf);
  if (pDFTSpec)
    ippFree(pDFTSpec);
  ippFree(pSrc);
  ippFree(pDst);

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
