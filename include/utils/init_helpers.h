#pragma once

/**
 * @brief Helper to initialize multiple subsystems with a single call
 * 
 * @note  
 * All subsystems must implement a begin() method with no parameters 
 * 
 * @tparam Obj - Type of subsystem
 * @param subSystem - Reference to subsystems to initialize
 * @return auto 
 */
template<typename... Obj>
void initSubSystems(Obj&... subSystem)
{
  ((void)subSystem.begin(),...);
}
