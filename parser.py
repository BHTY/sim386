#function declaration auto_parser
import re

def tokenize(string):
    tokens = re.findall("[__#a-zA-Z][__#a-zA-Z0-9]*|->|\".*\"|==|@|!=|>=|<=|&&|\\|\\||_?\\d+|[+\-*/=!<>()[\\]{},.;:$&\\|]|'.*'|\".*\"", string)
    return tokens

tokens_to_ignore = ["WINUSERAPI", "WINAPI", "_In_", "_In_opt_", "("]

function_decl = """WINUSERAPI
LRESULT
WINAPI
DispatchMessageA(
    _In_ CONST MSG *lpMsg);"""

class Argument:
    def __init__(self, tokens):
        self.name = tokens[-1]
        self.value_type = " ".join(tokens[:-1])
        self.is_pointer = (tokens[0][0:2] == "LP") or "*" in tokens


def generate_fn_call(name, args):
    out_string = "{}(".format(name)

    for i in args:
        if i.is_pointer:
            arg_name = "_" + i.name
        else:
            arg_name = i.name
        out_string += "({}){}, ".format(i.value_type, arg_name)

    return out_string[:-2] + ");"


def generate_print_string(name, args):
    out_string = '"\\nCalling {}('.format(name)

    for i in args:
        out_string += "%p, "

    out_string = out_string[:-2] + ')", '

    for i in args:
        out_string += i.name + ", "

    return out_string[:-2]


def parse(toks):
    return_value = None
    function_name = None
    args = []
    current_arg = []
    
    for i in toks:
        if i in tokens_to_ignore:
            continue
        
        if return_value == None: #looking for return value
            if i.lower() == "void": return_value = 0
            else: return_value = 1
            continue

        elif function_name == None:
            function_name = i
            continue

        elif i == ",":
            args.append(Argument(current_arg))
            current_arg = []

        elif i == ")": #we're done here
            args.append(Argument(current_arg))
            break

        else:
            current_arg.append(i)

    function_text = "uint32_t thunk_{}(i386* cpu){{\n".format(function_name)
    current_addr = 4

    for i in args:
        function_text += "\tuint32_t {} = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + {});\n".format(i.name, current_addr)
        current_addr += 4
        pass

    for i in args:
        if i.is_pointer:
            function_text += "\t{} {} = ({})virtual_to_physical_addr(cpu, {});\n".format(i.value_type, "_" + i.name, i.value_type, i.name)

    function_text += "\tprintf({});\n".format(generate_print_string(function_name, args))

    if return_value:
        function_text += "\treturn (uint32_t){}".format(generate_fn_call(function_name, args))
    else:
        function_text += "\t{}\n\treturn;".format(generate_fn_call(function_name, args))

    function_text += "\n}"

    return function_text
        
print(parse(tokenize(function_decl)))
