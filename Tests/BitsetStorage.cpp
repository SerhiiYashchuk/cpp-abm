#include "catch.hpp"

#include "Manager.hpp"

using namespace ABM;

using MyComponents = ComponentList<int, float, double, char, bool>;
using Integral = Signature<int, char, bool>;
using Float = Signature<float, double>;
using MySignatures = SignatureList<Integral, Float>;
using MySettings = Settings<MyComponents, MySignatures>;

TEST_CASE("BitsetStorage")
{
  BitsetStorage<MySettings> bitsetStorage;

  const auto & integralBitset = bitsetStorage.getSignatureBitset<Integral>();
  const auto & floatBitset = bitsetStorage.getSignatureBitset<Float>();

  REQUIRE(integralBitset == MySettings::Bitset{"11001"});
  REQUIRE(floatBitset == MySettings::Bitset{"00110"});
}
