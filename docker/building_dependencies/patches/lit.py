#!/usr/bin/python3
# -*- coding: utf-8 -*-
import os
import re
import sys
import lit.util

from lit.main import main
from lit.formats.googletest import GoogleTest

pass_re = re.compile(r'\[  PASSED  \] \d+ tests?.')

def execute(self, test, litConfig):
    testPath,testName = os.path.split(test.getSourcePath())
    while not os.path.exists(testPath):
        # Handle GTest parametrized and typed tests, whose name includes
        # some '/'s.
        testPath, namePrefix = os.path.split(testPath)
        testName = namePrefix + '/' + testName

    cmd = [testPath, '--gtest_filter=' + testName]
    cmd = self.maybeAddPythonToCmd(cmd)
    if litConfig.useValgrind:
        cmd = litConfig.valgrindArgs + cmd

    if litConfig.noExecute:
        return lit.Test.PASS, ''

    try:
        out, err, exitCode = lit.util.executeCommand(
            cmd, env=test.config.environment,
            timeout=litConfig.maxIndividualTestTime)
    except lit.util.ExecuteCommandTimeoutException:
        return (lit.Test.TIMEOUT,
                'Reached timeout of {} seconds'.format(
                    litConfig.maxIndividualTestTime)
                )

    if exitCode:
        return lit.Test.FAIL, out + err

    #passing_test_line = '[  PASSED  ] 1 test.'
    if not pass_re.search(out):
        msg = ('Unable to find %r in gtest output:\n\n%s%s' %
               (passing_test_line, out, err))
        return lit.Test.UNRESOLVED, msg

    return lit.Test.PASS, ''


if __name__ == '__main__':
    GoogleTest.execute = execute
    sys.argv[0] = re.sub(r'(-script\.pyw|\.exe)?$', '', sys.argv[0])
    sys.exit(main())
