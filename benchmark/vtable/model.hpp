// Copyright Louis Dionne 2017
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#ifndef BENCHMARK_VTABLE_MODEL_HPP
#define BENCHMARK_VTABLE_MODEL_HPP

#include <dyno.hpp>
#include <benchmark/benchmark.h>
#include <memory>
#include <utility>
using namespace dyno::literals;


struct Concept : decltype(dyno::requires(
  dyno::CopyConstructible{},
  dyno::Swappable{},
  dyno::Destructible{},
  dyno::Storable{},
  "f1"_dyno = dyno::function<void(dyno::T&)>,
  "f2"_dyno = dyno::function<void(dyno::T&)>,
  "f3"_dyno = dyno::function<void(dyno::T&)>,
  "f4"_dyno = dyno::function<void(dyno::T&)>
)) { };

template <typename T>
auto const dyno::default_concept_map<Concept, T> = dyno::make_concept_map(
  "f1"_dyno = [](T& self) { ++self; benchmark::DoNotOptimize(self); },
  "f2"_dyno = [](T& self) { ++self; benchmark::DoNotOptimize(self); },
  "f3"_dyno = [](T& self) { ++self; benchmark::DoNotOptimize(self); },
  "f4"_dyno = [](T& self) { ++self; benchmark::DoNotOptimize(self); }
);

template <typename VTablePolicy>
struct model {
  template <typename T>
  explicit model(T t)
    : poly_{std::move(t)}
  { }

  void f1() { poly_.virtual_("f1"_dyno)(poly_); }
  void f2() { poly_.virtual_("f2"_dyno)(poly_); }
  void f3() { poly_.virtual_("f3"_dyno)(poly_); }
  void f4() { poly_.virtual_("f4"_dyno)(poly_); }

private:
  dyno::poly<Concept, dyno::local_storage<8>, VTablePolicy> poly_;
};

struct inheritance_tag { };

template <>
struct model<inheritance_tag> {
  template <typename T>
  explicit model(T t)
    : self_{std::make_unique<model_t<T>>(std::move(t))}
  { }

  void f1() { self_->f1(); }
  void f2() { self_->f2(); }
  void f3() { self_->f3(); }
  void f4() { self_->f4(); }

private:
  struct concept_t {
    virtual void f1() = 0;
    virtual void f2() = 0;
    virtual void f3() = 0;
    virtual void f4() = 0;
    virtual ~concept_t() { }
  };

  template <typename T>
  struct model_t : concept_t {
    model_t(T v) : value_{std::move(v)} { }
    void f1() override { ++value_; benchmark::DoNotOptimize(value_); }
    void f2() override { ++value_; benchmark::DoNotOptimize(value_); }
    void f3() override { ++value_; benchmark::DoNotOptimize(value_); }
    void f4() override { ++value_; benchmark::DoNotOptimize(value_); }
    T value_;
  };

  std::unique_ptr<concept_t> self_;
};

#endif // BENCHMARK_VTABLE_MODEL_HPP
