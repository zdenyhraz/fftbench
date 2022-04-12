#pragma once

template <typename T>
constexpr std::vector<T> Linspace(f64 start, f64 end, usize n)
{
  std::vector<T> vec(n);
  for (i32 i = 0; i < n; ++i)
    vec[i] = start + static_cast<f64>(i) / (n - 1) * (end - start);
  return vec;
}