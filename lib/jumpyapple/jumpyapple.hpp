/**
* jumpyapple's collection of utilities and a thin functional wrapper over YYTK.
*/
#pragma once
#include <optional>
#include <variant>
#include <numeric>

#include <YYToolkit/YYTK_Shared.hpp>

namespace jumpyapple {
    using namespace YYTK;

#pragma region Functional layer
    /**
    * Same as RValue.ToVector(), but with std::optional.
    */
    std::optional<std::vector<RValue>> to_vector(std::optional<RValue*> value);

    /**
    * Same as RValue.GetRefMember(), but with std::optional.
    */
    std::optional<RValue*> get_ref_member(std::optional<RValue*> value, std::string name);

    /**
    * Similar to the `get_ref_member`, but you can pass in a list of names along the path.
    * 
    * Returns the last member on the path. If an error occur anywhere along the path, return the None option of the
    * std::optional.
    */
    std::optional<RValue*> get_ref_member_from_path(std::optional<RValue*> value, std::vector<std::string> names);

    /**
    * Find a menu using its object's name.
    */
    std::optional<RValue> find_menu_by_name(std::optional<RValue*> anchor, std::string_view name);

    /**
    * Call the builtin function `instanceof` to get the object name.
    */
    std::optional<std::string> object_name_of(std::optional<RValue*> value);

    /**
    * Return true if the name from the builtin function `instanceof` is the same as the given name.
    */
    bool is_instance_of(std::optional<RValue*> value, std::string_view name);
#pragma endregion

#pragma region Utilities
    std::optional<std::string> get_function_address(const char* script_name);

    /**
     * Returns true if the given instance is Ari.
     * 
     * It loops over each member of the instance and stop when all expected members are found.
     */
    std::optional<bool> is_ari_instance(std::optional<RValue *> instance);
#pragma endregion
}
