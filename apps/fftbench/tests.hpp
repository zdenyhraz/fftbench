#pragma once

std::vector<f32> GetFFTVector(f32* input, usize size)
{
  std::vector<f32> output(size);
  for (usize i = 0; i < output.size(); ++i)
    output[i] = input[i];
  return output;
}

std::vector<f32> OpenCVTest(std::vector<f32> input)
{
  std::vector<f32> output;
  cv::dft(input, output);
  return output;
}

std::vector<f32> FFTWEstimateTest(std::vector<f32> input)
{
  f32* inputAligned = fftwf_alloc_real(input.size());
  fftwf_complex* outputAligned = fftwf_alloc_complex(input.size() / 2 + 1);
  fftwf_plan plan = fftwf_plan_dft_r2c_1d(input.size(), inputAligned, outputAligned, FFTW_ESTIMATE);
  std::memcpy(inputAligned, input.data(), input.size() * sizeof(f32));
  fftwf_execute(plan);
  const auto output = GetFFTVector(std::bit_cast<f32*>(outputAligned), input.size() + 2);
  fftwf_destroy_plan(plan);
  fftwf_free(inputAligned);
  fftwf_free(outputAligned);
  fftwf_cleanup();
  return output;
}

std::vector<f32> FFTWMeasureTest(std::vector<f32> input)
{
  f32* inputAligned = fftwf_alloc_real(input.size());
  fftwf_complex* outputAligned = fftwf_alloc_complex(input.size() / 2 + 1);
  fftwf_plan plan = fftwf_plan_dft_r2c_1d(input.size(), inputAligned, outputAligned, FFTW_MEASURE);
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

void PrintFFT(const std::vector<f32>& fft)
{
  fmt::print("[");
  for (usize i = 0; i < fft.size(); ++i)
  {
    fmt::print("{:.2f}", fft[i]);
    if (i < fft.size() - 1)
      fmt::print(", ");
  }
  fmt::print("] ");
}

void CheckEqual(const std::string& name, const std::vector<f32>& fftref, const std::vector<f32>& fft)
{
  fmt::print("Checking {} ... ", name);

  if (std::abs(static_cast<i32>(fft.size()) - static_cast<i32>(fftref.size())) > 2)
    throw std::runtime_error(fmt::format("{} size differs: {} != {}", name, fft.size(), fftref.size()));

  static constexpr f32 tolerance = 1e-6;
  static constexpr usize pad = 2;
  bool ok = true;

  for (usize i = pad; i < std::min(fft.size(), fftref.size()) - pad; ++i)
    if ((std::abs(fft[i] - fftref[i]) > tolerance) and (std::abs(fft[i] - fftref[i - 1]) > tolerance) and (std::abs(fft[i] - fftref[i + 1]) > tolerance))
      ok = false;

  fmt::print("{} ", ok ? "OK" : "NOK");
  if (fft.size() <= 34)
    PrintFFT(fft);
  fmt::print("\n");
}

void RunTests()
{
  const auto input = GenerateRandomVector(32);
  const auto fftref = FFTWEstimateTest(input);
  const bool showref = false;

  if (showref)
  {
    fmt::print("Reference FFT: ");
    PrintFFT(fftref);
    fmt::print("\n");
  }

  if (std::ranges::count_if(fftref, [](const auto& x) { return x != std::complex<f32>{0, 0}; }) == 0)
    throw std::runtime_error("Invalid reference FFT");

  CheckEqual("OpenCV-IPP ccs", fftref, OpenCVTest(input));
  CheckEqual("FFTW est r2c", fftref, FFTWEstimateTest(input));
  CheckEqual("FFTW mes r2c", fftref, FFTWMeasureTest(input));
  CheckEqual("PocketFFT", fftref, PocketFFTTest(input));
  CheckEqual("PFFFT", fftref, PFFFTTest(input));
}