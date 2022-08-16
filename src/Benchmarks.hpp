#pragma once
#include "Precompiled.hpp"

// from https://www.fftw.org/fftw3_doc/One_002dDimensional-DFTs-of-Real-Data.html
// In many practical applications, the input data in[i] are purely real numbers, in which case the DFT output satisfies the “Hermitian” redundancy:
// out[i] is the conjugate of out[n-i]. It is possible to take advantage of these circumstances in order to achieve roughly a factor of two
// improvement in both speed and memory usage. In exchange for these speed and space advantages, the user sacrifices some of the simplicity of
// FFTW’s complex transforms. First of all, the input and output arrays are of different sizes and types: the input is n real numbers, while the
// output is n/2+1 complex numbers (the non-redundant outputs); this also requires slight “padding” of the input array for in-place transforms.
// Second, the inverse transform (complex to real) has the side-effect of overwriting its input array, by default. Neither of these inconveniences
// should pose a serious problem for users, but it is important to be aware of them.

static void FFTWBenchmark(benchmark::State& state, std::vector<f32> input, u32 flags)
{
  f32* inputAligned = fftwf_alloc_real(input.size());
  fftwf_complex* outputAligned = fftwf_alloc_complex(input.size() / 2 + 1);
  fftwf_plan plan = fftwf_plan_dft_r2c_1d(input.size(), inputAligned, outputAligned, flags);
  std::memcpy(inputAligned, input.data(), input.size() * sizeof(f32));
  for (auto _ : state)
    fftwf_execute(plan);
  fftwf_destroy_plan(plan);
  fftwf_free(inputAligned);
  fftwf_free(outputAligned);
  fftwf_cleanup();
}

static void PocketFFTBenchmark(benchmark::State& state, std::vector<f32> input)
{
  const pocketfft::shape_t shapeInput{input.size()};
  const pocketfft::stride_t strideInput{sizeof(f32)};
  const pocketfft::stride_t strideOutput{sizeof(std::complex<f32>)};
  const size_t axis = 0;
  const size_t nthreads = 1;
  const f32 factor = 1;
  std::vector<std::complex<f32>> output(input.size() / 2 + 1);
  for (auto _ : state)
    pocketfft::r2c(shapeInput, strideInput, strideOutput, axis, pocketfft::FORWARD, input.data(), output.data(), factor, nthreads);
}

static void PFFFTBenchmark(benchmark::State& state, std::vector<f32> input)
{
  pffft::Fft<f32> fft(input.size());

  if (!fft.isValid())
    throw std::invalid_argument(fmt::format("Error: transformation length {} is not decomposable into small prime factors. Next valid transform size is: {}; next power of 2 is: {}", input.size(),
        pffft::Fft<f32>::nearestTransformSize(input.size()), pffft::Fft<f32>::nextPowerOfTwo(input.size())));

  pffft::AlignedVector<f32> inputAligned = fft.valueVector();
  pffft::AlignedVector<std::complex<f32>> outputAligned = fft.spectrumVector();
  std::memcpy(inputAligned.data(), input.data(), input.size() * sizeof(f32));
  for (auto _ : state)
    fft.forward(inputAligned, outputAligned);
}

#ifdef ENABLE_KFR
static void KFRBenchmark(benchmark::State& state, std::vector<f32> input)
{
  kfr::univector<f32> in(input.size());
  kfr::univector<kfr::complex<f32>> out(input.size());
  kfr::dft_plan_real<f32> plan(input.size());
  std::memcpy(in.data(), input.data(), input.size() * sizeof(f32));
  kfr::univector<kfr::u8> temp(plan.temp_size);

  for (auto _ : state)
    plan.execute(out, in, temp);
}
#endif

#ifdef ENABLE_OPENCV
static void OpenCVBenchmark(benchmark::State& state, std::vector<f32> input)
{
  std::vector<f32> outputf;
  for (auto _ : state)
    cv::dft(input, outputf);
}
#endif

#ifdef ENABLE_IPP
static void IPPBenchmark(benchmark::State& state, std::vector<f32> input, IppHintAlgorithm hint)
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

  for (auto _ : state)
    ippsDFTFwd_RToCCS_32f(pSrc, pDst, pDFTSpec, pDFTWorkBuf);

  if (pDFTWorkBuf)
    ippFree(pDFTWorkBuf);
  if (pDFTSpec)
    ippFree(pDFTSpec);
  ippFree(pSrc);
  ippFree(pDst);
}
#endif

void RegisterBenchmarks()
{
  static constexpr auto expmin = 8;  // 8=512
  static constexpr auto expmax = 24; // 24=16M
  static const auto exponents = Linspace<i32>(expmin, expmax, expmax - expmin + 1);

  for (const auto exponent : exponents)
  {
    const auto timeunit = benchmark::kMillisecond;
    const auto size = 1 << exponent;
    const auto input = GenerateRandomVector(size);

    benchmark::RegisterBenchmark(fmt::format("{:>8} | FFTW estimate", size).c_str(), FFTWBenchmark, input, FFTW_ESTIMATE)->Unit(timeunit);
    benchmark::RegisterBenchmark(fmt::format("{:>8} | FFTW measure", size).c_str(), FFTWBenchmark, input, FFTW_MEASURE)->Unit(timeunit);
    benchmark::RegisterBenchmark(fmt::format("{:>8} | FFTW patient", size).c_str(), FFTWBenchmark, input, FFTW_PATIENT)->Unit(timeunit);
    benchmark::RegisterBenchmark(fmt::format("{:>8} | PocketFFT", size).c_str(), PocketFFTBenchmark, input)->Unit(timeunit);
    benchmark::RegisterBenchmark(fmt::format("{:>8} | PFFFT", size).c_str(), PFFFTBenchmark, input)->Unit(timeunit);
#ifdef ENABLE_KFR
    benchmark::RegisterBenchmark(fmt::format("{:>8} | KFR", size).c_str(), KFRBenchmark, input)->Unit(timeunit);
#endif
#ifdef ENABLE_OPENCV
    benchmark::RegisterBenchmark(fmt::format("{:>8} | OpenCV", size).c_str(), OpenCVBenchmark, input)->Unit(timeunit);
#endif
#ifdef ENABLE_IPP
    benchmark::RegisterBenchmark(fmt::format("{:>8} | IPP accurate", size).c_str(), IPPBenchmark, input, ippAlgHintAccurate)->Unit(timeunit);
    benchmark::RegisterBenchmark(fmt::format("{:>8} | IPP fast", size).c_str(), IPPBenchmark, input, ippAlgHintFast)->Unit(timeunit);
#endif
  }
}