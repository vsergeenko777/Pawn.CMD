#ifndef NATIVES_H_
#define NATIVES_H_

namespace Natives {
    // native PC_Init();
    cell AMX_NATIVE_CALL n_PC_Init(AMX *amx, cell *params) {
        try {
            const auto &script = Scripts::GetScript(amx);

            for (auto addr : script.init_flags_and_aliases_addresses) {
                Functions::ExecAmxPublic(amx, nullptr, addr);
            }

            if (script.on_init_addr) {
                Functions::ExecAmxPublic(amx, nullptr, script.on_init_addr);
            }
        } catch (const std::exception &e) {
            Logger::instance()->Write("[%s] %s: %s", Settings::kName, __FUNCTION__, e.what());

            return 0;
        }

        return 1;
    }

    // native PC_RegAlias(const cmd[], const alias[], ...);
    cell AMX_NATIVE_CALL n_PC_RegAlias(AMX *amx, cell *params) {
        try {
            if (params[0] < (2 * sizeof(cell))) {
                throw std::runtime_error{"number of parameters must not be less than 2"};
            }

            auto &script = Scripts::GetScript(amx);

            Scripts::Command command{};
            for (size_t i = 1; i <= params[0] / sizeof(cell); ++i) {
                auto cmd_name = Functions::GetAmxCmdName(amx, params[i]);

                if (i == 1) {
                    command = script.GetCommand(cmd_name);

                    if (command.is_alias) {
                        throw std::runtime_error{"command '" + cmd_name + "' is an alias"};
                    }

                    command.is_alias = true;
                } else {
                    if (script.cmds.count(cmd_name)) {
                        throw std::runtime_error{"alias '" + cmd_name + "' is occupied"};
                    }

                    script.cmds[cmd_name] = command;
                }
            }
        } catch (const std::exception &e) {
            Logger::instance()->Write("[%s] %s: %s", Settings::kName, __FUNCTION__, e.what());

            return 0;
        }

        return 1;
    }

    // native PC_SetFlags(const cmd[], flags);
    cell AMX_NATIVE_CALL n_PC_SetFlags(AMX *amx, cell *params) {
        try {
            Functions::AssertParams(2, params);

            auto &script = Scripts::GetScript(amx);

            auto cmd_name = Functions::GetAmxCmdName(amx, params[1]);

            auto &command = script.GetCommand(cmd_name);

            if (command.is_alias) {
                throw std::runtime_error{"command '" + cmd_name + "' is an alias"};
            }

            command.flags = params[2];
        } catch (const std::exception &e) {
            Logger::instance()->Write("[%s] %s: %s", Settings::kName, __FUNCTION__, e.what());

            return 0;
        }

        return 1;
    }

    // native PC_GetFlags(const cmd[]);
    cell AMX_NATIVE_CALL n_PC_GetFlags(AMX *amx, cell *params) {
        try {
            Functions::AssertParams(1, params);

            auto &script = Scripts::GetScript(amx);

            auto cmd_name = Functions::GetAmxCmdName(amx, params[1]);

            return script.GetCommand(cmd_name).flags;
        } catch (const std::exception &e) {
            Logger::instance()->Write("[%s] %s: %s", Settings::kName, __FUNCTION__, e.what());
        }

        return 0;
    }

    // native PC_EmulateCommand(playerid, const cmdtext[]);
    cell AMX_NATIVE_CALL n_PC_EmulateCommand(AMX *amx, cell *params) {
        try {
            Functions::AssertParams(2, params);

            Scripts::ProcessCommand(params[1], Functions::GetAmxString(amx, params[2]).c_str());
        } catch (const std::exception &e) {
            Logger::instance()->Write("[%s] %s: %s", Settings::kName, __FUNCTION__, e.what());

            return 0;
        }

        return 1;
    }

    // native PC_RenameCommand(const cmd[], const newname[]);
    cell AMX_NATIVE_CALL n_PC_RenameCommand(AMX *amx, cell *params) {
        try {
            Functions::AssertParams(2, params);

            auto &script = Scripts::GetScript(amx);

            auto cmd_name = Functions::GetAmxCmdName(amx, params[1]);
            auto cmd_newname = Functions::GetAmxCmdName(amx, params[2]);

            auto &command = script.GetCommand(cmd_name);

            if (script.cmds.count(cmd_newname)) {
                throw std::runtime_error{"name '" + cmd_newname + "' is occupied"};
            }

            script.cmds[cmd_newname] = command;

            script.cmds.erase(cmd_name);
        } catch (const std::exception &e) {
            Logger::instance()->Write("[%s] %s: %s", Settings::kName, __FUNCTION__, e.what());

            return 0;
        }

        return 1;
    }

    // native PC_CommandExists(const cmd[]);
    cell AMX_NATIVE_CALL n_PC_CommandExists(AMX *amx, cell *params) {
        try {
            Functions::AssertParams(1, params);

            const auto &script = Scripts::GetScript(amx);

            auto cmd_name = Functions::GetAmxCmdName(amx, params[1]);

            return script.cmds.count(cmd_name);
        } catch (const std::exception &e) {
            Logger::instance()->Write("[%s] %s: %s", Settings::kName, __FUNCTION__, e.what());
        }

        return 0;
    }

    // native PC_DeleteCommand(const cmd[]);
    cell AMX_NATIVE_CALL n_PC_DeleteCommand(AMX *amx, cell *params) {
        try {
            Functions::AssertParams(1, params);

            auto &script = Scripts::GetScript(amx);

            auto cmd_name = Functions::GetAmxCmdName(amx, params[1]);

            script.GetCommand(cmd_name);

            return script.cmds.erase(cmd_name);
        } catch (const std::exception &e) {
            Logger::instance()->Write("[%s] %s: %s", Settings::kName, __FUNCTION__, e.what());
        }

        return 0;
    }

    // native CmdArray:PC_GetCommandArray();
    cell AMX_NATIVE_CALL n_PC_GetCommandArray(AMX *amx, cell *params) {
        try {
            const auto &script = Scripts::GetScript(amx);

            const auto cmd_array = std::make_shared<Scripts::CmdArray>();

            for (const auto &cmd : script.cmds) {
                if (cmd.second.is_alias) {
                    continue;
                }

                cmd_array->push_back(cmd.first);
            }

            Scripts::cmd_array_set.insert(cmd_array);

            return reinterpret_cast<cell>(cmd_array.get());
        } catch (const std::exception &e) {
            Logger::instance()->Write("[%s] %s: %s", Settings::kName, __FUNCTION__, e.what());
        }

        return 0;
    }

    // native CmdArray:PC_GetAliasArray(const cmd[]);
    cell AMX_NATIVE_CALL n_PC_GetAliasArray(AMX *amx, cell *params) {
        try {
            Functions::AssertParams(1, params);

            auto &script = Scripts::GetScript(amx);

            auto cmd_name = Functions::GetAmxCmdName(amx, params[1]);

            auto &command = script.GetCommand(cmd_name);

            if (command.is_alias) {
                throw std::runtime_error{"command '" + cmd_name + "' is an alias"};
            }

            const auto cmd_array = std::make_shared<Scripts::CmdArray>();

            for (const auto &alias : script.cmds) {
                if (
                    alias.second.addr != command.addr
                    || !alias.second.is_alias
                ) {
                    continue;
                }

                cmd_array->push_back(alias.first);
            }

            Scripts::cmd_array_set.insert(cmd_array);

            return reinterpret_cast<cell>(cmd_array.get());
        } catch (const std::exception &e) {
            Logger::instance()->Write("[%s] %s: %s", Settings::kName, __FUNCTION__, e.what());
        }

        return 0;
    }

    // native PC_GetArraySize(CmdArray:arr);
    cell AMX_NATIVE_CALL n_PC_GetArraySize(AMX *amx, cell *params) {
        try {
            Functions::AssertParams(1, params);

            return Scripts::GetCmdArray(params[1])->size();
        } catch (const std::exception &e) {
            Logger::instance()->Write("[%s] %s: %s", Settings::kName, __FUNCTION__, e.what());
        }

        return 0;
    }

    // native PC_FreeArray(&CmdArray:arr);
    cell AMX_NATIVE_CALL n_PC_FreeArray(AMX *amx, cell *params) {
        try {
            Functions::AssertParams(1, params);

            cell *cptr{};

            if (amx_GetAddr(amx, params[1], &cptr) != AMX_ERR_NONE) {
                throw std::runtime_error{"invalid param reference"};
            }

            Scripts::cmd_array_set.erase(Scripts::GetCmdArray(*cptr));

            *cptr = 0;

            return 1;
        } catch (const std::exception &e) {
            Logger::instance()->Write("[%s] %s: %s", Settings::kName, __FUNCTION__, e.what());
        }

        return 0;
    }

    // native PC_GetCommandName(CmdArray:arr, index, name[], size = sizeof name);
    cell AMX_NATIVE_CALL n_PC_GetCommandName(AMX *amx, cell *params) {
        try {
            Functions::AssertParams(4, params);

            const auto cmd_array = Scripts::GetCmdArray(params[1]);

            const auto index = static_cast<size_t>(params[2]);

            Functions::SetAmxString(amx, params[3], cmd_array->at(index).c_str(), params[4]);

            return 1;
        } catch (const std::exception &e) {
            Logger::instance()->Write("[%s] %s: %s", Settings::kName, __FUNCTION__, e.what());
        }

        return 0;
    }

    void Register(AMX *amx) {
        const std::vector<AMX_NATIVE_INFO> natives{
            {"PC_Init", &n_PC_Init},

            {"PC_RegAlias", &n_PC_RegAlias},
            {"PC_SetFlags", &n_PC_SetFlags},
            {"PC_GetFlags", &n_PC_GetFlags},
            {"PC_EmulateCommand", &n_PC_EmulateCommand},
            {"PC_RenameCommand", &n_PC_RenameCommand},
            {"PC_CommandExists", &n_PC_CommandExists},
            {"PC_DeleteCommand", &n_PC_DeleteCommand},

            {"PC_GetCommandArray", &n_PC_GetCommandArray},
            {"PC_GetAliasArray", &n_PC_GetAliasArray},
            {"PC_GetArraySize", &n_PC_GetArraySize},
            {"PC_FreeArray", &n_PC_FreeArray},
            {"PC_GetCommandName", &n_PC_GetCommandName},
        };

        amx_Register(amx, natives.data(), natives.size());
    }
}

#endif // NATIVES_H_
