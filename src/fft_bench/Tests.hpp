#pragma once
#include "Precompiled.hpp"

std::vector<f32> GetFFTVector(f32* input, usize size)
{
  std::vector<f32> output(size);
  for (usize i = 0; i < output.size(); ++i)
    output[i] = input[i];
  return output;
}

std::vector<f32> FFTWTest(std::vector<f32> input, u32 flags)
{
  f32* inputAligned = fftwf_alloc_real(input.size());
  fftwf_complex* outputAligned = fftwf_alloc_complex(input.size() / 2 + 1);
  fftwf_plan plan = fftwf_plan_dft_r2c_1d(input.size(), inputAligned, outputAligned, flags);
  std::memcpy(inputAligned, input.data(), input.size() * sizeof(f32));
  fftwf_execute(plan);
  const auto output = GetFFTVector(std::bit_cast<f32*>(outputAligned), input.size() + 2);
  fftwf_destroy_plan(plan);
  fftwf_free(inputAligned);
  fftwf_free(outputAligned);
  fftwf_cleanup();
  return output;
}

std::vector<f32> PocketFFTTest(std::vector<f32> input)
{
  const pocketfft::shape_t shapeInput{input.size()};
  const pocketfft::stride_t strideInput{sizeof(f32)};
  const pocketfft::stride_t strideOutput{sizeof(std::complex<f32>)};
  const size_t axis = 0;
  const size_t nthreads = 1;
  const f32 factor = 1;
  std::vector<std::complex<f32>> output(input.size() / 2 + 1);
  pocketfft::r2c(shapeInput, strideInput, strideOutput, axis, pocketfft::FORWARD, input.data(), output.data(), factor, nthreads);
  return GetFFTVector(std::bit_cast<f32*>(output.data()), input.size() + 2);
}

std::vector<f32> PFFFTTest(std::vector<f32> input)
{
  pffft::Fft<f32> fft(input.size());

  if (!fft.isValid())
    throw std::invalid_argument(fmt::format("Error: transformation length {} is not decomposable into small prime factors. Next valid transform size is: {}; next power of 2 is: {}", input.size(),
        pffft::Fft<f32>::nearestTransformSize(input.size()), pffft::Fft<f32>::nextPowerOfTwo(input.size())));

  pffft::AlignedVector<f32> inputAligned = fft.valueVector();
  pffft::AlignedVector<std::complex<f32>> outputAligned = fft.spectrumVector();
  std::memcpy(inputAligned.data(), input.data(), input.size() * sizeof(f32));
  fft.forward(inputAligned, outputAligned);
  return GetFFTVector(std::bit_cast<f32*>(outputAligned.data()), input.size() + 2);
}

#ifdef ENABLE_KFR
std::vector<f32> KFRTest(std::vector<f32> input)
{
  kfr::univector<f32> in(input.size());
  kfr::univector<kfr::complex<f32>> out(input.size());
  kfr::dft_plan_real<f32> plan(input.size());
  std::memcpy(in.data(), input.data(), input.size() * sizeof(f32));
  kfr::univector<u8> temp(plan.temp_size);
  plan.execute(out, in, temp);
  return GetFFTVector(std::bit_cast<f32*>(out.data()), input.size() + 2);
}
#endif

#ifdef ENABLE_OPENCV
std::vector<f32> OpenCVTest(std::vector<f32> input)
{
  std::vector<f32> output;
  cv::dft(input, output);
  return output;
}
#endif

std::vector<f32> IPPTest(std::vector<f32> input, IppHintAlgorithm hint)
{
  const int N = input.size();
  const auto flag = IPP_FFT_NODIV_BY_ANY;
  auto pSrc = ippsMalloc_32f(N);
  auto pDst = ippsMalloc_32f(N);
  std::memcpy(pSrc, input.data(), input.size() * sizeof(f32));
  int sizeDFTSpec, sizeDFTInitBuf, sizeDFTWorkBuf;
  ippsDFTGetSize_R_32f(N, flag, hint, &sizeDFTSpec, &sizeDFTInitBuf, &sizeDFTWorkBuf);
  auto pDFTSpec = (IppsDFTSpec_R_32f*)ippsMalloc_8u(sizeDFTSpec);
  auto pDFTInitBuf = ippsMalloc_8u(sizeDFTInitBuf);
  auto pDFTWorkBuf = ippsMalloc_8u(sizeDFTWorkBuf);
  ippsDFTInit_R_32f(N, flag, hint, pDFTSpec, pDFTInitBuf);
  if (pDFTInitBuf)
    ippFree(pDFTInitBuf);

  ippsDFTFwd_RToCCS_32f(pSrc, pDst, pDFTSpec, pDFTWorkBuf);
  const auto output = GetFFTVector(std::bit_cast<f32*>(pDst), input.size() + 2);

  if (pDFTWorkBuf)
    ippFree(pDFTWorkBuf);
  if (pDFTSpec)
    ippFree(pDFTSpec);

  ippFree(pSrc);
  ippFree(pDst);

  return output;
}

void PrintFFT(const std::vector<f32>& fft, const std::string& prefix, const std::string& suffix)
{
  fmt::print("{}\n[", prefix);
  for (usize i = 0; i < fft.size(); ++i)
  {
    fmt::print("{:.1e}", fft[i]);
    if (i < fft.size() - 1)
      fmt::print(", ");
  }
  fmt::print("]{}", suffix);
}

void CheckEqual(const std::string& name, const std::vector<f32>& fftref, const std::vector<f32>& fft)
{
  fmt::print("Checking {} ... ", name);

  if (std::abs(static_cast<i32>(fft.size()) - static_cast<i32>(fftref.size())) > 2)
    throw std::runtime_error(fmt::format("{} size differs: {} != {}", name, fft.size(), fftref.size()));

  static constexpr f32 tolerance = 1e-3;
  static constexpr usize pad = 2;
  f64 maxdiff = 0;
  for (usize i = pad; i < std::min(fft.size(), fftref.size()) - pad; ++i)
  {
    const f64 diff1 = std::abs(static_cast<f64>(fft[i]) - fftref[i]);
    const f64 diff2 = std::abs(static_cast<f64>(fft[i]) - fftref[i - 1]);
    const f64 diff3 = std::abs(static_cast<f64>(fft[i]) - fftref[i + 1]);
    maxdiff = std::max(maxdiff, std::min({diff1, diff2, diff3}));
  }

  const bool ok = maxdiff <= tolerance;
  fmt::print("{}, maxdiff: {:.2e}\n", ok ? "OK" : "NOK", maxdiff);
  if (not ok)
  {
    if (fft.size() <= 128)
    {
      PrintFFT(fftref, "Reference: ", "\n");
      PrintFFT(fft, fmt::format("{}: ", name), "\n");
    }
    throw std::runtime_error(fmt::format("{} did not pass the tests ", name));
  }
}

void RunTests(usize size)
{
  std::srand(std::time(nullptr));

  const auto input = GenerateRandomVector(size);
  const auto fftref = FFTWTest(input, FFTW_MEASURE);

  if (std::ranges::count_if(fftref, [](const auto& x) { return x != std::complex<f32>{0, 0}; }) == 0)
    throw std::runtime_error("Invalid reference FFT");

  CheckEqual("FFTW estimate", fftref, FFTWTest(input, FFTW_ESTIMATE));
  CheckEqual("FFTW measure", fftref, FFTWTest(input, FFTW_MEASURE));
  CheckEqual("FFTW patient", fftref, FFTWTest(input, FFTW_PATIENT));

  CheckEqual("IPP accurate", fftref, IPPTest(input, ippAlgHintAccurate));
  CheckEqual("IPP fast", fftref, IPPTest(input, ippAlgHintFast));

  CheckEqual("PocketFFT", fftref, PocketFFTTest(input));

  CheckEqual("PFFFT", fftref, PFFFTTest(input));

#ifdef ENABLE_KFR
  CheckEqual("KFR", fftref, KFRTest(input));
#endif

#ifdef ENABLE_OPENCV
  CheckEqual("OpenCV", fftref, OpenCVTest(input));
#endif
}
