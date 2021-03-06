//
//  main.cpp
//  snake
//
//  Created by flexih on 2020/1/19.
//  Copyright © 2020 flexih. All rights reserved.
//

#include "Bin.h"
#include "Linkmap.hpp"
#include "Output.hpp"
#include "cxxopts.hpp"
#include "utility.hpp"

using namespace snake;

int main(int argc, char * argv[]) {
    auto useJson = false;
    cxxopts::Options options("snake", "🐍 Snake, Yet Another Mach-O Unused ObjC Selector/Class/Protocol Detector\n");
    options.custom_help("[-dscp] [-l path] path/to/binary ...");
    options.positional_help("");
    options.add_options()
    ("s,selector", "Unused selectors")
    ("c,class", "Unused classes")
    ("p,protocol", "Unused protocoles")
    ("d,duplicate", "Duplicate selectors")
    ("l,linkmap", "Linkmap file, which has selector size, library name", cxxopts::value<std::string>())
    ("i,input", "Mach-O binary", cxxopts::value<std::string>())
    ("j,json", "Output json format", cxxopts::value<bool>(useJson))
    ("help", "Print help")
    ;
    options.parse_positional({"input"});
    std::string machoPath, linkmapPath;
    uint8_t function = 0;
    try {
        auto result = options.parse(argc, argv);
        if (result.count("help")) {
            std::cout << options.help() << std::endl;
            exit(0);
        }
        if (result.count("input")) {
            machoPath = trimPath(result["input"].as<std::string>());
        } else {
            throw cxxopts::option_not_present_exception("mach-o");
        }
        if (result.count("linkmap")) {
            linkmapPath = trimPath(result["linkmap"].as<std::string>());
        }
        if (result.count("s")) {
            function = 's';
        } else if (result.count("c")) {
            function = 'c';
        } else if (result.count("p")) {
            function = 'p';
        } else if (result.count("d")) {
            function = 'd';
        } else {
            throw cxxopts::option_not_present_exception("-dscp");
        }
        Bin bin;
        bin.read(machoPath);
        Linkmap linkmap;
        linkmap.read(linkmapPath);
        auto arch = bin.arch();
        if (arch == nullptr) {
            std::cout << "snake: " << "mach_header.filetype != MH_EXECUTE" << std::endl;
            exit(1);
        }
        switch (function) {
            case 's': {
                auto selectorsUnused = arch->ObjCSelectorsUnused();
                std::cout << (useJson ? Output::json.unusedSelectors(selectorsUnused, linkmap).str() + "\n" : Output::raw.unusedSelectors(selectorsUnused, linkmap).str());
                break;
            }
            case 'c': {
                auto classesUnused = arch->ObjCClassesUnused();
                std::cout << (useJson ? Output::json.unusedClasses(classesUnused, linkmap).str() + "\n" : Output::raw.unusedClasses(classesUnused, linkmap).str());
                break;
            }
            case 'p': {
                auto protocolsUnsued = arch->ObjCProtocolsUnused();
                std::cout << (useJson ? Output::json.unusedProtocols(protocolsUnsued, linkmap).str() + "\n" : Output::raw.unusedProtocols(protocolsUnsued, linkmap).str());
                break;
            }
            case 'd': {
                auto duplicateSelectors = arch->ObjCDuplicateSelectors();
                std::cout << Output::raw.duplicatSelectors(duplicateSelectors, linkmap).str() << std::endl;
                break;
            }
            default:
                break;
        }
    } catch (const cxxopts::OptionException& e) {
        std::cout << "snake: " << e.what() << std::endl;
        std::cout << options.help() << std::endl;
        exit(1);
    }
    return 0;
}
