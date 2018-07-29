#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

namespace Functions {
    int SetAmxString(AMX *amx, cell amx_addr, const char *source, int max) {
        cell *dest = reinterpret_cast<cell *>(amx->base + static_cast<int>(reinterpret_cast<AMX_HEADER *>(amx->base)->dat + amx_addr));
        cell *start = dest;

        while (max-- && *source) {
            *dest++ = static_cast<cell>(*source++);
        }
        *dest = 0;

        return dest - start;
    }

    std::string GetAmxString(AMX *amx, cell amx_addr) {
        int len{};
        cell *addr{};

        if (
            amx_GetAddr(amx, amx_addr, &addr) == AMX_ERR_NONE
            && amx_StrLen(addr, &len) == AMX_ERR_NONE
            && len
        ) {
            len++;

            std::unique_ptr<char[]> buf{new char[len]{}};

            if (
                buf
                && amx_GetString(buf.get(), addr, 0, len) == AMX_ERR_NONE
            ) {
                return buf.get();
            }
        }

        return {};
    }

    std::string GetAmxCmdName(AMX *amx, cell amx_addr) {
        auto cmd_name = GetAmxString(amx, amx_addr);

        if (Settings::case_insensitivity) {
            cmd_name = boost::locale::to_lower(cmd_name);
        }

        return cmd_name;
    }

    bool GetAmxPublicVar(AMX *amx, const char *name, cell &out) {
        cell addr{}, *phys_addr{};

        if (
            amx_FindPubVar(amx, name, &addr) == AMX_ERR_NONE
            && amx_GetAddr(amx, addr, &phys_addr) == AMX_ERR_NONE
        ) {
            out = *phys_addr;

            return true;
        }

        return false;
    }

    int ExecAmxPublic(AMX *amx, cell *retval, cell addr) {
        const auto hdr = reinterpret_cast<AMX_HEADER *>(amx->base);
        const auto cip = hdr->cip;

        hdr->cip = addr;
        int result = amx_Exec(amx, retval, AMX_EXEC_MAIN);
        hdr->cip = cip;

        return result;
    }

    cell GetAmxPublicAddr(AMX *amx, int index) {
        const auto hdr = reinterpret_cast<AMX_HEADER *>(amx->base);
        const auto func = reinterpret_cast<AMX_FUNCSTUB *>(
            reinterpret_cast<unsigned char *>(hdr)
            + static_cast<size_t>(hdr->publics)
            + static_cast<size_t>(index) * hdr->defsize
        );

        return func->address;
    }

    std::string GetAmxPublicName(AMX *amx, int index) {
        char public_name[32]{};

        amx_GetPublic(amx, index, public_name);

        return public_name;
    }

    void AssertParams(int count, cell *params) {
        if (params[0] != (count * sizeof(cell))) {
            throw std::runtime_error{"number of parameters must be equal to " + std::to_string(count)};
        }
    }
}

#endif // FUNCTIONS_H_
