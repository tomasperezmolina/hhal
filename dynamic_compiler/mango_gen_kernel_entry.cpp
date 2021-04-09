#include "dynamic_compiler/mango_gen_kernel_entry.h"
#include <vector>
#include <string>
#include <fstream>
#include <assert.h>
#include <types.h>

struct parameter {
    bool pointer = false;
    std::string type;
    std::string name;

    std::string join() {
        return type + (pointer ? "" : " ") + name;
    }
};

const std::string int_types[8] = {"int", "unsigned int", "signed int", "long", "signed long", "unsigned long", "int32_t", "uint32_t"};

bool check_int_type(std::string type) {
    for(int i = 0; i < 8; ++i) {
        if(type == int_types[i])
            return true;
    }
    return false;
}

bool generate_entrypoint(std::string file_path, std::string output_path, hhal::Unit arch) {
    int arch_i = 0;
    if(arch == hhal::Unit::GN)
        arch_i = 6;
    //else if (arch == mango::mango_unit_type_t::PEAK)
    //    arch_i = 5;
    assert(arch_i != 0 && "Invalid architecture");

    std::ifstream file(file_path.c_str());
    std::string str;

    //Search mango_gen_entrypoint pragma
    //If it is not present, the entrypoint is not generated
    bool entry_pragma_search = true;
    while (entry_pragma_search && getline(file, str)) {
        if(str == "#pragma mango_gen_entrypoint") {
            entry_pragma_search = false;
        }
    }
    if(entry_pragma_search) return false;

    bool pragma_search = true;
    while (pragma_search && getline(file, str)) {
        if(str == "#pragma mango_kernel") {
            pragma_search = false;
        }
    }
    assert(!pragma_search && "Missing mango_kernel pragma");

    getline(file, str);
    file.close();

    // kernel proto
    std::vector<parameter> params;
    std::string kernel_name;
    int i = 0;
    char c;
    c = str[i];
    while(c != ' ') { //kernel type
        c = str[++i];
    }
    c = str[++i];
    while(c != '(') { //kernel name
        kernel_name.push_back(c);
        c = str[++i];
    }
    c = str[++i];
    while(c != ')') { //kernel params
        while(c == ' ') c = str[++i];

        parameter param;
        while(c != ' ' && c != '*') { //param type
            param.type.push_back(c);
            c = str[++i];
        }
        while(c == ' ' || c == '*') { //param is pointer
            if(c == '*') {
                param.pointer = true;
                param.type.append(" *");
            }
            c = str[++i];
        }
        while(c != ',' && c != ')') { //param name
            param.name.push_back(c);
            c = str[++i];
        }
        params.push_back(param);

        if(c == ',') c = str[++i];
    }
    std::string kernel_proto = str.substr(0, ++i);

    std::ofstream out_file(output_path);
    //includes
    out_file << "#include \"dev/mango_hn.h\"\n"
                "#include <stdlib.h>\n";
    //extern kernel
    out_file << "extern " << kernel_proto << ";\n\n";
    //main
    out_file << "int main(int argc, char **argv) {\n"
	            "\tmango_init(argv);\n";

    //params
    for(unsigned int j = 0; j < params.size(); ++j) {
        parameter param = params[j];
        if(param.pointer) {
            out_file << "\t" << param.join() << " = (" << param.type << ")mango_memory_map(strtol(argv[" << arch_i << "],NULL,16));\n";
        } else if (param.type == "mango_event_t") {
            out_file << "\t" << param.join() << ";\n";
            out_file << "\t" << param.name << ".vaddr = (uint32_t *)mango_memory_map(strtol(argv[" << arch_i << "],NULL,16));\n";
        } else if (param.type == "float") {
            out_file << "\t" << param.join() << " = strtof(argv[" << arch_i << "],NULL,16);\n";
        } else if (param.type == "double") {
            out_file << "\t" << param.join() << " = strtod(argv[" << arch_i << "],NULL,16);\n";
        } else if (check_int_type(param.type)) {
            out_file << "\t" << param.join() << " = strtol(argv[" << arch_i << "],NULL,16);\n";
        } else {
            assert(false && "Unrecognized type");
        }
        ++arch_i;
    }

    //kernel call
    out_file << "\n\t" << kernel_name << "(";
    for(unsigned int j = 0; j < params.size(); ++j) {
        out_file << params[j].name;
        if(j < params.size()-1)
            out_file << ", ";
    }
    out_file << ");\n\n";

    //mango close
    out_file << "\tmango_close(42);\n";
    out_file << "}";
    out_file.close();

    return true;
}

/*
int main(int argc, char **argv) {
    std::string arch = argv[2];
    std::string file_path = argv[1];

    std::string output_path = file_path.substr(0, file_path.find_last_of(".")) + "_entry.c";
    generate_entrypoint(file_path, output_path, arch);
}
*/
