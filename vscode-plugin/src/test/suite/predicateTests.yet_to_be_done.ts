import * as assert from 'assert';
import * as path from 'path';
import { Commands } from '../../config/commands';
import { executeCommand } from '../../utils/utils';
import {
    checkCoverageJson,
    checkDirectoryWithTestsExists,
    checkTestFileNotEmpty,
    checkTestFilesGenerated,
} from '../helper';

suite('"Generate Tests For Project" Test Suite', () => {
    const projectName = 'c-example-mini';
    const projectPath = path.resolve(__dirname, 
            '../../../../', 'integration-tests', projectName);

    // eslint-disable-next-line @typescript-eslint/no-unused-vars
    async function checkAll(): Promise<void> {
        assert.ok(checkDirectoryWithTestsExists(projectPath));
        assert.ok(checkTestFilesGenerated(projectPath, ['basic_functions']));
        assert.ok(checkTestFileNotEmpty(projectPath));
        await executeCommand(Commands.RunAllTestsAndShowCoverage);
        assert.ok(checkCoverageJson(path.join(projectPath, 'build')));
    }

	test('Test 0', async () => {
        //TODO
    });
});
