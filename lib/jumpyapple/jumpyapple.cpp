#include <optional>
#include <variant>
#include <numeric>
#include <YYToolkit/YYTK_Shared.hpp>
#include "jumpyapple.hpp"

using namespace Aurie;
using namespace YYTK;

namespace jumpyapple {
    std::optional<std::vector<RValue>> to_vector(std::optional<RValue*> value) {
        if (value.has_value()) {
            auto val = value.value();
            if (val->m_Kind == VALUE_ARRAY) {
                return val->ToVector();
            }
        }
        return {};
    }

    std::optional<RValue*> get_ref_member(std::optional<RValue*> value, std::string name) {
        if (!value.has_value()) { return {}; }
        YYTKInterface* yytk_interface = GetInterface();
        if (yytk_interface == nullptr) {
            return {};
        }

        RValue* member_value = nullptr;
        auto status = yytk_interface->GetInstanceMember(value.value()->ToInstance(), name.c_str(), member_value);
        if (!AurieSuccess(status)) {
            return {};
        }
        return member_value;
    }

    std::optional<RValue*> get_ref_member_from_path(std::optional<RValue*> value, std::vector<std::string> names) {
        return std::accumulate(names.begin(), names.end(), value, [](std::optional<RValue*> acc, std::string name) {
            return get_ref_member(acc, name);
            });
    }

    std::optional<RValue> find_menu_by_name(std::optional<RValue*> anchor, std::string_view name) {
        std::vector<std::string> path = { "open_menus", "__buffer" };
        auto maybe_open_menus = get_ref_member_from_path(anchor, path).and_then(to_vector);
        if (maybe_open_menus.has_value()) {
            auto open_menus = maybe_open_menus.value();
            for (size_t i = 0; i < open_menus.size(); i++) {
                auto maybe_name = object_name_of(&open_menus[i]);
                if (maybe_name.has_value() && maybe_name.value() == name) {
                    return open_menus[i];
                }
            }
        }
        // NOTE: A result type would let us tell the caller why/where we failed.
        return {};
    }

    std::optional<std::string> object_name_of(std::optional<RValue*> value) {
        if (!value.has_value()) { return {}; }
        YYTKInterface* yytk_interface = GetInterface();
        if (yytk_interface == nullptr) {
            return {};
        }

        RValue result = yytk_interface->CallBuiltin("instanceof", { *value.value() });
        if (result.m_Kind == VALUE_STRING) {
            return std::string(result.ToCString());
        }
    }

    bool is_instance_of(std::optional<RValue*> value, std::string_view name) {
        if (!value.has_value()) { return {}; }
        return object_name_of(value).value_or("") == name;
    }

    std::optional<std::string> get_function_address(const char* script_name) {
        CScript* func_ptr = nullptr;
        YYTKInterface* yytk_interface = GetInterface();
        if (yytk_interface == nullptr) {
            return {};
        }

        AurieStatus status = yytk_interface->GetNamedRoutinePointer(script_name, reinterpret_cast<PVOID*>(&func_ptr));
        if (!AurieSuccess(status)) {
            return {};
        }

        return std::format("{:#010x}", reinterpret_cast<intptr_t>(func_ptr->m_Functions->m_ScriptFunction));
    }

    std::optional<bool> is_ari_instance(std::optional<RValue*> instance) {
        YYTKInterface* yytk_interface = GetInterface();
        if (yytk_interface == nullptr) {
            return {};
        }

        uint32_t found_members = 0;
        std::vector<std::string> expected_members = {};
        yytk_interface->EnumInstanceMembers(instance.value()->ToInstance(), [&found_members, expected_members](const char* member_name, RValue* member_value) {
            auto itr = std::find(expected_members.begin(), expected_members.end(), member_name);
            if (itr != expected_members.end()) {
                found_members += 1;
            }
            return found_members == expected_members.size();
            });
    }
}
