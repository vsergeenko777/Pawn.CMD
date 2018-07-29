#ifndef HOOKS_H_
#define HOOKS_H_

#ifdef THISCALL
#undef THISCALL
#endif

#ifdef _WIN32
#define THISCALL __thiscall
#else
#define THISCALL
#endif

namespace Hooks {
    std::shared_ptr<urmem::hook> hook_fs__on_player_command_text;

    struct InternalHooks {
        static int THISCALL CFilterScripts__OnPlayerCommandText(void *_this, cell playerid, const char *cmdtext) {
            Scripts::ProcessCommand(playerid, cmdtext);

            return 1;
        }
    };

    void Init(void *addr_in_server) {
        urmem::sig_scanner scanner;
        urmem::address_t opct_addr{};

        if (
            !scanner.init(addr_in_server)
            || !scanner.find(Settings::kOpctPattern, Settings::kOpctMask, opct_addr)
            || !opct_addr
        ) {
            throw std::runtime_error{"CFilterScripts::OnPlayerCommandText not found"};
        }

        const auto dest = urmem::get_func_addr(&InternalHooks::CFilterScripts__OnPlayerCommandText);

        hook_fs__on_player_command_text = std::make_shared<urmem::hook>(opct_addr, dest);
    }
}

#endif // HOOKS_H_
