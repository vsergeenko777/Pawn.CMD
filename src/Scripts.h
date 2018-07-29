#ifndef SCRIPTS_H_
#define SCRIPTS_H_

namespace Scripts {
    struct Command;

    using CmdArray = std::vector<std::string>;
    using CommandMap = std::unordered_map<std::string, Command>;

    struct Command {
        cell addr;
        unsigned int flags;
        bool is_alias;
    };

    struct Script {
        Command &GetCommand(const std::string &name) {
            const auto iter = cmds.find(name);

            if (iter == cmds.end()) {
                throw std::runtime_error{"command '" + name + "' not found"};
            }

            return iter->second;
        }

        AMX *amx;
        cell opct_addr, opcr_addr, opcp_addr, on_init_addr;
        std::deque<cell> init_flags_and_aliases_addresses;
        CommandMap cmds;
    };

    std::list<Script> scripts;
    std::unordered_set<std::shared_ptr<CmdArray>> cmd_array_set;

    std::shared_ptr<CmdArray> GetCmdArray(cell ptr) {
        const auto iter = std::find_if(cmd_array_set.begin(), cmd_array_set.end(), [ptr](const std::shared_ptr<CmdArray> &p) {
            return reinterpret_cast<cell>(p.get()) == ptr;
        });

        if (iter == cmd_array_set.end()) {
            throw std::runtime_error{"invalid array handle"};
        }

        return *iter;
    }

    Script &GetScript(AMX *amx) {
        const auto iter = std::find_if(scripts.begin(), scripts.end(), [amx](const Script &script) {
            return script.amx == amx;
        });

        if (iter == scripts.end()) {
            throw std::runtime_error{"amx not found"};
        }

        return *iter;
    }

    void ProcessCommand(cell playerid, const char *cmdtext) {
        if (
            !cmdtext
            || cmdtext[0] != '/'
        ) {
            return;
        }

        int i = 1;
        while (
            cmdtext[i]
            && cmdtext[i] != ' '
        ) {
            i++;
        }
        const auto cmd = Settings::case_insensitivity ? 
            boost::locale::to_lower(&cmdtext[1], &cmdtext[i]) : 
            std::string{&cmdtext[1], &cmdtext[i]};

        while (cmdtext[i] == ' ') {
            i++;
        } // skip excess spaces before params
        const char *params = &cmdtext[i];

        CommandMap::const_iterator iter_cmd{};
        cell addr_cmdtext{}, addr_cmd_name{}, addr_params{}, retval{}, flags{};
        bool command_exists{};
        for (const auto &script : scripts) {
            if (script.opct_addr) {
                amx_PushString(script.amx, &addr_cmdtext, nullptr, cmdtext, 0, 0);
                amx_Push(script.amx, playerid);
                Functions::ExecAmxPublic(script.amx, &retval, script.opct_addr);
                amx_Release(script.amx, addr_cmdtext);

                if (retval == 1) {
                    break;
                }
            }

            if (command_exists = ((iter_cmd = script.cmds.find(cmd)) != script.cmds.end())) {
                flags = iter_cmd->second.flags;
            }

            if (script.opcr_addr) {
                amx_Push(script.amx, flags);
                amx_PushString(script.amx, &addr_params, nullptr, params, 0, 0);
                amx_PushString(script.amx, &addr_cmd_name, nullptr, cmd.c_str(), 0, 0);
                amx_Push(script.amx, playerid);
                Functions::ExecAmxPublic(script.amx, &retval, script.opcr_addr);
                amx_Release(script.amx, addr_cmd_name);
                amx_Release(script.amx, addr_params);

                if (!retval) {
                    continue;
                }
            }

            if (command_exists) {
                amx_PushString(script.amx, &addr_params, nullptr, params, 0, 0);
                amx_Push(script.amx, playerid);
                Functions::ExecAmxPublic(script.amx, &retval, iter_cmd->second.addr);
                amx_Release(script.amx, addr_params);
            } else {
                retval = -1;
            }

            if (script.opcp_addr) {
                amx_Push(script.amx, flags);
                amx_Push(script.amx, retval);
                amx_PushString(script.amx, &addr_params, nullptr, params, 0, 0);
                amx_PushString(script.amx, &addr_cmd_name, nullptr, cmd.c_str(), 0, 0);
                amx_Push(script.amx, playerid);
                Functions::ExecAmxPublic(script.amx, &retval, script.opcp_addr);
                amx_Release(script.amx, addr_cmd_name);
                amx_Release(script.amx, addr_params);
            }

            if (retval == 1) {
                break;
            }
        }
    }

    static void Load(AMX *amx) {
        cell include_version{}, is_gamemode{};
        if (
            Functions::GetAmxPublicVar(amx, Settings::kPublicVarNameVersion, include_version)
            && Functions::GetAmxPublicVar(amx, Settings::kPublicVarNameIsGamemode, is_gamemode)
        ) {
            if (include_version != PAWNCMD_INCLUDE_VERSION) {
                Logger::instance()->Write("[%s] %s: mismatch between the plugin and include versions", Settings::kName, __FUNCTION__);
            }
        }

        int num_publics{};
        amx_NumPublics(amx, &num_publics);
        if (!num_publics) {
            return;
        }

        Script script{};
        script.amx = amx;

        for (int index{}; index < num_publics; ++index) {
            std::string public_name = Functions::GetAmxPublicName(amx, index);
            std::smatch match;
            if (std::regex_match(public_name, match, std::regex{R"(pc_cmd_(\w+))"})) {
                auto cmd_name = match[1].str();

                if (Settings::case_insensitivity) {
                    cmd_name = boost::locale::to_lower(cmd_name);
                }

                Command command{};
                command.addr = Functions::GetAmxPublicAddr(amx, index);

                script.cmds[cmd_name] = command;
            } else if (std::regex_match(public_name, std::regex{R"(pc_alias_\w+)"})) {
                script.init_flags_and_aliases_addresses.push_back(Functions::GetAmxPublicAddr(amx, index));
            } else if (std::regex_match(public_name, std::regex{R"(pc_flags_\w+)"})) {
                script.init_flags_and_aliases_addresses.push_front(Functions::GetAmxPublicAddr(amx, index));
            } else if (Settings::legacy_opct_support && public_name == "OnPlayerCommandText") {
                script.opct_addr = Functions::GetAmxPublicAddr(amx, index);
            } else if (public_name == "OnPlayerCommandReceived") {
                script.opcr_addr = Functions::GetAmxPublicAddr(amx, index);
            } else if (public_name == "OnPlayerCommandPerformed") {
                script.opcp_addr = Functions::GetAmxPublicAddr(amx, index);
            } else if (public_name == "PC_OnInit") {
                script.on_init_addr = Functions::GetAmxPublicAddr(amx, index);
            }
        }

        is_gamemode ? scripts.push_back(script) : scripts.push_front(script);
    }

    void Unload(AMX *amx) {
        scripts.remove_if([amx](const Script &script) {
            return script.amx == amx;
        });
    }
}

#endif // SCRIPTS_H_
