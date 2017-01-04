#ifndef ABM_MANAGER_HPP
#define ABM_MANAGER_HPP

#define VALUE_TYPE(T) typename decltype(T)::type

#include <vector>
#include <tuple>
#include <cassert>

#include "Settings.hpp"
#include "Agent.hpp"

namespace ABM
{
// Bitset storage
template<typename TSettings>
class BitsetStorage
{
public:
  using Settings = TSettings;
  using ComponentList = typename Settings::ComponentList;
  using SignatureList = typename Settings::SignatureList;
  using Bitset = typename Settings::Bitset;

  /**
   * @brief Returns a corresponding bitset for a given Signature
   */
  template<typename TSignature>
  auto & getSignatureBitset() noexcept
  {
    return std::get<Settings::template signatureID<TSignature>()>(bitsets);
  }

  /**
   * @brief Returns a corresponding bitset for a given Signature
   */
  template<typename TSignature>
  const auto & getSignatureBitset() const noexcept
  {
    return std::get<Settings::template signatureID<TSignature>()>(bitsets);
  }

private:
  using BitsetList = brigand::filled_list<Bitset, Settings::signatureCount()>;
  using BitsetTuple = brigand::as_tuple<BitsetList>;

  /**
   * @brief Initializes a bitset for a given Signature
   */
  template<typename TSignature>
  void initializeBitset() noexcept
  {
    auto & bitset = getSignatureBitset<TSignature>();

    brigand::for_each<TSignature>([this, & bitset](auto component){
      bitset[Settings::template componentID<VALUE_TYPE(component)>()] = true;
    });
  }

  BitsetTuple bitsets;

public:
  BitsetStorage()
  {
    brigand::for_each<SignatureList>([this](auto signature){
      this->initializeBitset<VALUE_TYPE(signature)>();
    });
  }
};

// Component storage
template<typename TSettings>
class ComponentStorage
{
public:
  using Settings = TSettings;
  using ComponentList = typename Settings::ComponentList;

  /**
   * @brief Increases capacity of the storage
   */
  void grow(std::size_t newCapacity)
  {
    brigand::for_each<ComponentList>([this, newCapacity](auto component){
      auto & vector = this->getComponentVector<VALUE_TYPE(component)>();

      vector.resize(newCapacity);
    });
  }

  /**
   * @brief Returns specific Component for a given Agent
   */
  template<typename TComponent>
  auto & getComponent(std::size_t index) noexcept
  {
    auto & components = getComponentVector<TComponent>();

    return components[index];
  }

  template<typename TComponent>
  const auto & getComponent(std::size_t index) const noexcept
  {
    const auto & components = getComponentVector<TComponent>();

    return components[index];
  }

private:
  template<typename... TArgs>
  using TupleWrapper = typename std::tuple<std::vector<TArgs>...>;

  using TupleOfVectors = brigand::wrap<ComponentList, TupleWrapper>;

  /**
   * @brief Returns a vector of specific Components
   */
  template<typename TComponent>
  auto & getComponentVector() noexcept
  {
    return std::get<std::vector<TComponent>>(componentVectors);
  }

  template<typename TComponent>
  const auto & getComponentVector() const noexcept
  {
    return std::get<std::vector<TComponent>>(componentVectors);
  }

  TupleOfVectors componentVectors;
};

// Manager
template<typename TSettings>
class Manager
{
public:
  using Settings = TSettings;
  using ComponentList = typename Settings::ComponentList;
  using SignatureList = typename Settings::SignatureList;
  using Bitset = typename Settings::Bitset;

  /**
   * @brief Checks if an Agent with a given index has a specific Component
   */
  template<typename TComponent>
  bool hasComponent(std::size_t index) const noexcept
  {
    const auto & agent = getAgent(index);

    return agent.bitset[Settings::template componentID<TComponent>()];
  }

  /**
   * @brief Creates and adds a specific Component for an Agent with a given index
   */
  template<typename TComponent, typename... TArgs>
  auto & addComponent(std::size_t index, TArgs &&... args) noexcept
  {
    auto & agent = getAgent(index);
    auto & component = components.template getComponent<TComponent>(agent.dataIndex);
    component = TComponent(std::forward<decltype(args)>(args)...);

    agent.bitset[Settings::template componentID<TComponent>()] = true;

    return component;
  }

  /**
   * @brief Returns a specific Component of an Agent with a given index
   */
  template<typename TComponent>
  auto & getComponent(std::size_t index) noexcept
  {
    assert(hasComponent<TComponent>(index));

    auto & agent = getAgent(index);

    return components.template getComponent<TComponent>(agent.dataIndex);
  }

  template<typename TComponent>
  const auto & getComponent(std::size_t index) const noexcept
  {
    assert(hasComponent<TComponent>(index));

    const auto & agent = getAgent(index);

    return components.template getComponent<TComponent>(agent.dataIndex);
  }

  /**
   * @brief Removes a specific Component from an Agent with a given index
   */
  template<typename TComponent>
  void deleteComponent(std::size_t index) noexcept
  {
    auto & agent = getAgent(index);

    agent.bitset[Settings::template componentID<TComponent>()] = false;
  }

  /**
   * @brief Creates a new agent and returns its index
   */
  std::size_t createIndex()
  {
    growIfNeeded();

    const auto newIndex = nextSize++;

    assert(!isAlive(newIndex));

    auto & agent = agents[newIndex];

    agent.alive = true;
    agent.bitset.reset();

    return newIndex;
  }

  /**
   * @brief Checks if an Agent with a given index is alive
   * @param index - index of an Agent
   */
  bool isAlive(std::size_t index) const noexcept
  {
    return getAgent(index).alive;
  }

  /**
   * @brief Kills an Agent with a given index
   * @param index - index of an Agent
   */
  void kill(std::size_t index) noexcept
  {
    getAgent(index).alive = false;
  }

  /**
   * Resets a storage and all agents
   */
  void clear() noexcept
  {
    for (std::size_t i = 0; i < capacity; ++i)
    {
      auto & agent = agents[i];

      agent.dataIndex = i;
      agent.alive = false;
      agent.bitset.reset();
    }

    size = 0;
    nextSize = 0;
  }

  /**
   * @brief Refreshes all agents based on their status
   */
  void refresh() noexcept
  {
    if (nextSize == 0)
    {
      size = 0;

      return;
    }

    size = nextSize = refreshImpl();
  }

  /**
   * @brief Checks if an Agent with a given index matches specific Signature
   */
  template<typename TSignature>
  bool matchesSignature(std::size_t index) const noexcept
  {
    const auto & agent = getAgent(index);
    const auto & signatureBitset = signatureBitsets.template getSignatureBitset<TSignature>();

    return (agent.bitset & signatureBitset) == signatureBitset;
  }

  /**
   * @brief Helper function that executes a given functor for all agents
   */
  template<typename TFunc>
  void forAgents(TFunc && func) noexcept
  {
    for (std::size_t i = 0; i < size; ++i)
    {
      func(i);
    }
  }

  /**
   * @brief Helper functions that executes a given functor for each agent
   * that matches specific Signature
   */
  template<typename TSignature, typename TFunc>
  void forAgentsMatching(TFunc && func) noexcept
  {
    forAgents([this, & func](std::size_t index){
      if (matchesSignature<TSignature>(index))
      {
        func(index);
      }
    });
  }

  /**
   * @brief Returns actual number of active agents
   */
  std::size_t getAgentsCount() const noexcept
  {
    return size;
  }

  /**
   * @brief Returns curent capacity of a storage
   */
  std::size_t getCapacity() const noexcept
  {
    return capacity;
  }

private:
  /**
   * @brief Returns an Agent with a given index
   * @param index - index of an Agent
   */
  auto & getAgent(std::size_t index) noexcept
  {
    assert(index < nextSize);

    return agents[index];
  }

  const auto & getAgent(std::size_t index) const noexcept
  {
    assert(index < nextSize);

    return agents[index];
  }

  /**
   * @brief Increases capacity of agents' storage
   */
  void growTo(std::size_t newCapacity)
  {
    assert(newCapacity > capacity);

    agents.resize(newCapacity);
    components.grow(newCapacity);

    for (std::size_t i = capacity; i < newCapacity; ++i)
    {
      auto & agent = agents[i];

      agent.dataIndex = i;
      agent.alive = false;
      agent.bitset.reset();
    }

    capacity = newCapacity;
  }

  /**
   * @brief Increases capacity if there's not enough space
   */
  void growIfNeeded()
  {
    if (capacity > nextSize)
    {
      return;
    }

    growTo((capacity + 10) * 2);
  }

  /**
   * @brief Refresh implementation.
   * Basically it sorts all the agents in a way that alive go to the begining
   * and dead - after them
   */
  std::size_t refreshImpl() noexcept
  {
    std::size_t iDead = 0;
    std::size_t iAlive = nextSize - 1;

    while (true)
    {
      for ( ; true; ++iDead)
      {
        if (iDead > iAlive)
        {
          return iDead;
        }

        if (!agents[iDead].alive)
        {
          break;
        }
      }

      for ( ; true; --iAlive)
      {
        if (agents[iAlive].alive)
        {
          break;
        }

        if (iAlive <= iDead)
        {
          return iDead;
        }
      }

      assert(agents[iAlive].alive);
      assert(!agents[iDead].alive);

      std::swap(agents[iAlive], agents[iDead]);

      ++iDead;
      --iAlive;
    }

    return iDead;
  }

  std::size_t capacity = 0;
  std::size_t size = 0;
  std::size_t nextSize = 0;

  std::vector<Agent<Settings>> agents;
  ComponentStorage<Settings> components;
  BitsetStorage<Settings> signatureBitsets;
};
}

#endif
