#ifndef SETTINGS_H_
#define SETTINGS_H_

namespace Settings {
    constexpr char
        *kName = "Pawn.CMD",
        *kVersion = "3.2.0",
        *kConfigFile = "plugins/pawncmd.cfg",
        *kPublicVarNameVersion = "_pawncmd_version",
        *kPublicVarNameIsGamemode = "_pawncmd_is_gamemode",
#ifdef _WIN32
        *kOpctPattern =
        "\x83\xEC\x08"          /*sub esp,0x8*/ \
        "\x53"                  /*push ebx*/ \
        "\x8B\x5C\x24\x14"      /*mov ebx,DWORD PTR [esp+0x14]*/ \
        "\x55"                  /*push ebp*/ \
        "\x8B\x6C\x24\x14"      /*mov ebp,DWORD PTR [esp+0x14]*/ \
        "\x56"                  /*push esi*/ \
        "\x33\xF6"              /*xor esi,esi*/ \
        "\x57"                  /*push edi*/ \
        "\x8B\xF9"              /*mov edi,ecx*/ \
        "\x89\x74\x24\x10"      /*mov DWORD PTR [esp+0x10],esi*/ \
        "\x8B\x04\xB7"          /*mov eax,DWORD PTR [edi+esi*4]*/ \
        "\x85\xC0",             /*test eax,eax*/
        *kOpctMask = "xxxxxxxxxxxxxxxxxxxxxxxxxxxx";
#else
        *kOpctPattern =
        "\x55"                          /*push ebp*/ \
        "\x89\xE5"                      /*mov ebp,esp*/ \
        "\x57"                          /*push edi*/ \
        "\x56"                          /*push esi*/ \
        "\x53"                          /*push ebx*/ \
        "\x83\xEC\x2C"                  /*sub esp,0x2c*/ \
        "\x8B\x75\x08"                  /*mov esi,DWORD PTR [ebp+0x8]*/ \
        "\xC7\x45\xE4\x00\x00\x00\x00"  /*mov DWORD PTR [ebp-0x1c],0x0*/ \
        "\x8B\x7D\x10"                  /*mov edi,DWORD PTR [ebp+0x10]*/ \
        "\x89\xF3"                      /*mov ebx,esi*/ \
        "\xEB\x14",                     /*jmp 0x2e*/
        *kOpctMask = "xxxxxxxxxxxxxxxxxxxxxxxxxx";
#endif
    bool
        case_insensitivity{},
        legacy_opct_support{};

    std::string locale_name;

    void Read() {
        std::fstream{kConfigFile, std::fstream::out | std::fstream::app};

        boost::property_tree::ptree tree;
        boost::property_tree::read_ini(kConfigFile, tree);

        case_insensitivity = tree.get<bool>("CaseInsensitivity", true);
        legacy_opct_support = tree.get<bool>("LegacyOpctSupport", true);

        locale_name = tree.get<std::string>("LocaleName", "");
    }
}

#endif // SETTINGS_H_
