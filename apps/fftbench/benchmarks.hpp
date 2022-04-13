#pragma once

// from https://www.fftw.org/fftw3_doc/One_002dDimensional-DFTs-of-Real-Data.html
// In many practical applications, the input data in[i] are purely real numbers, in which case the DFT output satisfies the “Hermitian” redundancy:
// out[i] is the conjugate of out[n-i]. It is possible to take advantage of these circumstances in order to achieve roughly a factor of two
// improvement in both speed and memory usage. In exchange for these speed and space advantages, the user sacrifices some of the simplicity of
// FFTW’s complex transforms. First of all, the input and output arrays are of different sizes and types: the input is n real numbers, while the
// output is n/2+1 complex numbers (the non-redundant outputs); this also requires slight “padding” of the input array for in-place transforms.
// Second, the inverse transform (complex to real) has the side-effect of overwriting its input array, by default. Neither of these inconveniences
// should pose a serious problem for users, but it is important to be aware of them.

static void OpenCVBenchmark(benchmark::State& state, std::vector<f32> input)
{
  std::vector<f32> outputf;
  for (auto _ : state)
    cv::dft(input, outputf);
}

static void FFTWEstimateBenchmark(benchmark::State& state, std::vector<f32> input)
{
  f32* inputAligned = fftwf_alloc_real(input.size());
  fftwf_complex* outputAligned = fftwf_alloc_complex(input.size() / 2 + 1);
  fftwf_plan plan = fftwf_plan_dft_r2c_1d(input.size(), inputAligned, outputAligned, FFTW_ESTIMATE);
  std::memcpy(inputAligned, input.data(), input.size() * sizeof(f32));
  for (auto _ : state)
    fftwf_execute(plan);
  fftwf_destroy_plan(plan);
  fftwf_free(inputAligned);
  fftwf_free(outputAligned);
  fftwf_cleanup();
}

static void FFTWMeasureBenchmark(benchmark::State& state, std::vector<f32> input)
{
  f32* inputAligned = fftwf_alloc_real(input.size());
  fftwf_complex* outputAligned = fftwf_alloc_complex(input.size() / 2 + 1);
  fftwf_plan plan = fftwf_plan_dft_r2c_1d(input.size(), inputAligned, outputAligned, FFTW_MEASURE);
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
  const pocketfft::shape_t shapeInput(input.size());
  const pocketfft::stride_t strideInput(sizeof(f32));
  const pocketfft::stride_t strideOutput(sizeof(std::complex<f32>));
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

void RegisterBenchmarks()
{
  static constexpr auto expmin = 8;  // 8=512
  static constexpr auto expmax = 24; // 24=16M
  static const auto exponents = Linspace<i32>(expmin, expmax, expmax - expmin + 1);

  for (const auto exponent : exponents)
  {
    const auto timeunit = exponent > 18 ? benchmark::kMillisecond : benchmark::kMicrosecond;
    const auto size = 1 << exponent;
    const auto input = GenerateRandomVector(size);

    benchmark::RegisterBenchmark(fmt::format("{:>8} | OpenCV-IPP ccs", size).c_str(), OpenCVBenchmark, input)->Unit(timeunit);
    benchmark::RegisterBenchmark(fmt::format("{:>8} | FFTW est r2c", size).c_str(), FFTWEstimateBenchmark, input)->Unit(timeunit);
    benchmark::RegisterBenchmark(fmt::format("{:>8} | FFTW mes r2c", size).c_str(), FFTWMeasureBenchmark, input)->Unit(timeunit);
    benchmark::RegisterBenchmark(fmt::format("{:>8} | PocketFFT", size).c_str(), PocketFFTBenchmark, input)->Unit(timeunit);
    benchmark::RegisterBenchmark(fmt::format("{:>8} | PFFFT", size).c_str(), PFFFTBenchmark, input)->Unit(timeunit);
  }
}