#ifndef ABM_AGENT_HPP
#define ABM_AGENT_HPP

#include "Settings.hpp"

namespace ABM
{
template<typename TSettings>
struct Agent
{
  using Settings = TSettings;
  using Bitset = typename Settings::Bitset;

  std::size_t dataIndex = 0;
  bool alive = true;
  Bitset bitset;
};
}

#endif
