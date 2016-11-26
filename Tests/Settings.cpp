#include "Settings.hpp"

namespace ABM
{
// Lets create a list of components and some signatures
using MyComponents = ComponentList<int, float, double, char, bool>;

using Integral = Signature<int, char, bool>;
using Float = Signature<float, double>;

using MySignatures = SignatureList<Integral, Float>;

using MySettings = Settings<MyComponents, MySignatures>;

// Now we will test our "type traits"
namespace Tests
{
static_assert(MySettings::componentCount() == 5, "Incorrect components count");
static_assert(MySettings::signatureCount() == 2, "Incorrect signature count");

static_assert(MySettings::isComponent<int>(), "int should be a component");
static_assert(MySettings::isComponent<float>(), "float should be a component");
static_assert(MySettings::isComponent<double>(), "double should be a component");
static_assert(MySettings::isComponent<char>(), "char should be a component");
static_assert(MySettings::isComponent<bool>(), "bool should be a component");

static_assert(MySettings::isSignature<Integral>(), "Integral should be a signature");
static_assert(MySettings::isSignature<Float>(), "Float should be a signature");

static_assert(MySettings::componentID<int>() == 0, "Wrong int ID");
static_assert(MySettings::componentID<float>() == 1, "Wrong float ID");
static_assert(MySettings::componentID<double>() == 2, "Wrong double ID");
static_assert(MySettings::componentID<char>() == 3, "Wrong char ID");
static_assert(MySettings::componentID<bool>() == 4, "Wrong bool ID");

static_assert(MySettings::signatureID<Integral>() == 0, "Wrong Integral ID");
static_assert(MySettings::signatureID<Float>() == 1, "Wrong Float ID");
}

}
