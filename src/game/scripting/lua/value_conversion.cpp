#include <stdinc.hpp>
#include "value_conversion.hpp"

namespace scripting::lua
{
	namespace
	{
		struct array_value
		{
			int index;
			script_value value;
		};

		sol::lua_value entity_to_array(lua_State* state, unsigned int id)
		{
			auto table = sol::table::create(state);

			std::unordered_map<std::string, array_value> values;

			auto current = game::scr_VarGlob[0].objectVariableChildren[id].firstChild;

			auto idx = 1;

			while (current)
			{
				const auto var = game::scr_VarGlob[0].childVariableValue[current];

				if (var.type == game::SCRIPT_NONE)
				{
					current = var.nextSibling;
					continue;
				}

				const auto string_value = (unsigned __int8)var.name_lo + (var.k.keys.name_hi << 8);
				const auto* str = game::SL_ConvertToString(string_value);

				std::string key = string_value < 0x40000 && str
					? str
					: std::to_string(idx++);

				game::VariableValue variable{};
				variable.type = var.type;
				variable.u = var.u.u;

				array_value value;
				value.index = current;
				value.value = script_value(variable);

				values[key] = value;

				current = var.nextSibling;
			}

			auto metatable = sol::table::create(state);

			metatable[sol::meta_function::new_index] = [state, values](const sol::table t, const sol::this_state s,
				const sol::lua_value& key_value, const sol::lua_value& value)
			{
				const auto key = key_value.is<int>()
					? std::to_string(key_value.as<int>())
					: key_value.as<std::string>();

				if (values.find(key) == values.end())
				{
					return;
				}

				const auto variable = convert(value).get_raw();
				const auto i = values.at(key).index;

				game::scr_VarGlob->childVariableValue[i].type = (char)variable.type;
				game::scr_VarGlob->childVariableValue[i].u.u = variable.u;
			};

			metatable[sol::meta_function::index] = [state, values](const sol::table t, const sol::this_state s,
				const sol::lua_value& key_value)
			{
				const auto key = key_value.is<int>()
					? std::to_string(key_value.as<int>())
					: key_value.as<std::string>();

				if (values.find(key) == values.end())
				{
					return sol::lua_value{};
				}

				return convert(s, values.at(key).value);
			};

			metatable[sol::meta_function::length] = [values]()
			{
				return values.size();
			};

			table["getkeys"] = [values]()
			{
				std::vector<std::string> _keys;

				for (const auto& entry : values)
				{
					_keys.push_back(entry.first);
				}

				return _keys;
			};

			table[sol::metatable_key] = metatable;

			return table;
		}
	}

	script_value convert(const sol::lua_value& value)
	{
		if (value.is<int>())
		{
			return {value.as<int>()};
		}

		if (value.is<unsigned int>())
		{
			return {value.as<unsigned int>()};
		}

		if (value.is<bool>())
		{
			return {value.as<bool>()};
		}

		if (value.is<double>())
		{
			return {value.as<double>()};
		}

		if (value.is<float>())
		{
			return {value.as<float>()};
		}

		if (value.is<std::string>())
		{
			return {value.as<std::string>()};
		}

		if (value.is<entity>())
		{
			return {value.as<entity>()};
		}

		if (value.is<vector>())
		{
			return {value.as<vector>()};
		}

		return {};
	}

	sol::lua_value convert(lua_State* state, const script_value& value)
	{
		if (value.is<int>())
		{
			return {state, value.as<int>()};
		}

		if (value.is<float>())
		{
			return {state, value.as<float>()};
		}

		if (value.is<std::string>())
		{
			return {state, value.as<std::string>()};
		}

		if (value.is<std::vector<script_value>>())
		{
			return entity_to_array(state, value.get_raw().u.uintValue);
		}
		
		if (value.is<entity>())
		{
			return {state, value.as<entity>()};
		}

		if (value.is<vector>())
		{
			return {state, value.as<vector>()};
		}

		return {state, {}};
	}
}
