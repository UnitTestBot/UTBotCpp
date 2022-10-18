# How run script: python generate.py {number_of_functions}

import sys

def main(function_count) :
    print('Number of functions: {}'.format(function_count))
    res_file = open('func{}.c'.format(function_count), 'w')
    res_file.write('\n'.join(['/*',
                              ' * This file is automatically generated by UnitTestBot. For further information see https://github.com/UnitTestBot/UTBotCpp',
                              ' */\n\n']))
    res_file.write('// function_count {}\n'.format(function_count))
    for i in range(function_count) :
        res_file.write('\n'.join(['int fun_{0} (int a) {{'.format(i),
                       '    if (a >= 0) {',
                       '        return a;'.format(i),
                       '    }',
                       '    return -1 * a;',
                       '}\n\n']))


if __name__ == '__main__':
    if(len(sys.argv) > 1):
        function_count = int(sys.argv[1])
    else:
        function_count = 100
    main(function_count)
