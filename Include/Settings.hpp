#ifndef ABM_SETTINGS_HPP
#define ABM_SETTINGS_HPP

#define BRIGAND_NO_BOOST_SUPPORT

#include <type_traits>
#include <bitset>

#include "brigand.hpp"

namespace ABM
{
// Type aliases
template<typename... TArgs>
using ComponentList = brigand::list<TArgs...>;

template<typename... TArgs>
using SignatureList = brigand::list<TArgs...>;

template<typename... TArgs>
using Signature = brigand::list<TArgs...>;

// Settings
template<typename TComponentList, typename TSignatureList>
struct Settings
{
  using ComponentList = typename TComponentList::list;
  using SignatureList = typename TSignatureList::list;

  /**
   * @brief Determines if a given type is registered as a Component
   */
  template<typename TComponent>
  static constexpr bool isComponent() noexcept
  {
    using TFind = brigand::find<ComponentList,
      std::is_same<brigand::_1, TComponent>>;

    return !std::is_same<TFind, brigand::empty_sequence>();
  }

  /**
   * @brief Determines if a given type is registered as a Signature
   */
  template<typename TSignature>
  static constexpr bool isSignature() noexcept
  {
    using TFind = brigand::find<SignatureList,
      std::is_same<brigand::_1, brigand::pin<TSignature>>>;

    return !std::is_same<TFind, brigand::empty_sequence>();
  }

  /**
   * @brief Returns the ID of a given Component
   */
  template<typename TComponent>
  static constexpr std::size_t componentID() noexcept
  {
    static_assert(isComponent<TComponent>(), "T is not a component");

    return brigand::index_of<ComponentList, TComponent>();
  }

  /**
   * @brief Returns the ID of a given Signature
   */
  template<typename TSignature>
  static constexpr std::size_t signatureID() noexcept
  {
    static_assert(isSignature<TSignature>(), "T is not a signature");

    return brigand::index_of<SignatureList, brigand::pin<TSignature>>();
  }

  /**
   * @brief Returns total number of registered Components
   */
  static constexpr std::size_t componentCount() noexcept
  {
    return brigand::size<ComponentList>();
  }

  /**
   * @brief Returns total number of registered Signatures
   */
  static constexpr std::size_t signatureCount() noexcept
  {
    return brigand::size<SignatureList>();
  }

  using Bitset = std::bitset<componentCount()>;
};
}

#endif
