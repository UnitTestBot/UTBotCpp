import * as vs from 'vscode';
import { ValidationType } from "../proto-ts/util_pb";

export async function getPredicate(type: ValidationType): Promise<string | undefined> {
    let result = undefined, selection = undefined, predicate = undefined;
    if (type === ValidationType.STRING) {
        selection = [
            {
                label: '==',
                description: 'equal to prompted value'
            }
        ];
    } else {
        selection = [
            {
                label: '==',
                description: 'equal to prompted value'
            },
            {
                label: '!=',
                description: 'not equal to prompted value'
            },
            {
                label: '<=',
                description: 'less or equal to prompted value'
            },
            {
                label: '>=',
                description: 'greater or equal to prompted value'
            },
            {
                label: '<',
                description: 'less than prompted value'
            },
            {
                label: '>',
                description: 'greater than prompted value'
            },
        ];
    }
    result = await vs.window.showQuickPick(selection, {
        placeHolder: 'Return value should be',
        onDidSelectItem: (item: vs.QuickPickItem) => {
            if (item) {
                predicate = item.label;
            }
        }
    });
    if (!result) {
        return undefined;
    } else {
        return predicate;
    }
}

const defaultReturnValues = new Map<ValidationType, string>([
    [ValidationType.INT8_T, '0'],
    [ValidationType.INT16_T, '0'],
    [ValidationType.INT32_T, '0'],
    [ValidationType.INT64_T, '0'],
    [ValidationType.UINT8_T, '0'],
    [ValidationType.UINT16_T, '0'],
    [ValidationType.UINT32_T, '0'],
    [ValidationType.UINT64_T, '0'],
    [ValidationType.CHAR, 'a'],
    [ValidationType.FLOAT, '1.0'],
    [ValidationType.STRING, 'abacaba'],
]);
function integerInBounds(value: string, bounds: [BigInt, BigInt] | undefined): boolean {
    if (!bounds) {
        return false;
    }
    return (bounds[0] <= BigInt(value) && BigInt(value) <= bounds[1]);
}

function intBoundsBySize(size: number, signed: boolean): [BigInt, BigInt] {
    if (signed) {
        return [BigInt(0), BigInt(Math.pow(2, size) - 1)];
    } else {
        return [BigInt(-Math.pow(2, size)), BigInt(Math.pow(2, size) - 1)];
    }
}

const validationTypeName = new Map<ValidationType, string>([
    [ValidationType.INT8_T, "int8_t"],
    [ValidationType.INT16_T, "int16_t"],
    [ValidationType.INT32_T, "int32_t"],
    [ValidationType.INT64_T, "int64_t"],
    [ValidationType.UINT8_T, "uint8_t"],
    [ValidationType.UINT16_T, "uint16_t"],
    [ValidationType.UINT32_T, "uint32_t"],
    [ValidationType.UINT64_T, "uint64_t"]
]);

const integerBounds = new Map<ValidationType, [BigInt, BigInt]>([
    [ValidationType.INT8_T, intBoundsBySize(8, false)],
    [ValidationType.INT16_T, intBoundsBySize(16, false)],
    [ValidationType.INT32_T, intBoundsBySize(32, false)],
    [ValidationType.INT64_T, intBoundsBySize(64, false)],
    [ValidationType.UINT8_T, intBoundsBySize(8, true)],
    [ValidationType.UINT16_T, intBoundsBySize(16, true)],
    [ValidationType.UINT32_T, intBoundsBySize(32, true)],
    [ValidationType.UINT64_T, intBoundsBySize(64, true)]
]);

function intValidationFunc(validationType: ValidationType):  (value: string) => string | undefined | null | Thenable<string | undefined | null> {
    return (value: string): string | undefined | null | Thenable<string | undefined | null> => {
        if (value.match('^-?(([1-9][0-9]*)|0)$')) {
            if (integerInBounds(value, integerBounds.get(validationType))) {
                return null;
            } else {
                return 'Value does not fit into C ' + validationTypeName.get(validationType) + ' type';
            }
        } else {
            return 'Value is not an integer';
        }
    };
}

const returnValueValidators = new Map<ValidationType, (value: string) => string | undefined | null | Thenable<string | undefined | null>>([
    [ValidationType.INT8_T, intValidationFunc(ValidationType.INT8_T)],
    [ValidationType.INT16_T, intValidationFunc(ValidationType.INT16_T)],
    [ValidationType.INT32_T, intValidationFunc(ValidationType.INT32_T)],
    [ValidationType.INT64_T, intValidationFunc(ValidationType.INT64_T)],
    [ValidationType.UINT8_T, intValidationFunc(ValidationType.UINT8_T)],
    [ValidationType.UINT16_T, intValidationFunc(ValidationType.UINT16_T)],
    [ValidationType.UINT32_T, intValidationFunc(ValidationType.UINT32_T)],
    [ValidationType.UINT64_T, intValidationFunc(ValidationType.UINT64_T)],
    [ValidationType.CHAR, (value: string): string | undefined | null | Thenable<string | undefined | null> => {
        if (value.length === 1) {
            return null;
        } else {
            const escapeSequences = [
                '\\\'',
                '\\"',
                '\\?',
                '\\\\',
                '\\a',
                '\\b',
                '\\f',
                '\\n',
                '\\r',
                '\\t',
                '\\v'
            ];
            if (!escapeSequences.includes(value)) {
                return 'Value is not a character';
            } else {
                return null;
            }
        }
    }],
    [ValidationType.FLOAT, (value: string): string | undefined | null | Thenable<string | undefined | null> => {
        if (value.match('^-?([1-9][0-9]*)[.]([0-9]*)$')) {
            if (value.length < 15) {
                return null;
            } else {
                return 'Value does not fit into C float type.';
            }
        } else {
            return 'Value is not floating-point';
        }
    }],
    [ValidationType.STRING, (value: string): string | undefined | null | Thenable<string | undefined | null> => {
        if (value.length > 32) {
            return 'String is too long';
        } else {
            return null;
        }
    }]
  ]);

export async function getReturnValue(type: ValidationType): Promise<string | undefined> {
    if (type === ValidationType.BOOL) {
        return vs.window.showQuickPick(['true', 'false'], {
            placeHolder: 'Enter value to which constraint should be applied'
        });
    } else {
        return vs.window.showInputBox({
            prompt: 'Enter value to which constraint should be applied',
            value: defaultReturnValues.get(type),
            validateInput: returnValueValidators.get(type)
        });
    }
}
