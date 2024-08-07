import js from "@eslint/js";
import globals from "globals";
import html from "eslint-plugin-html"
import stylisticJs from '@stylistic/eslint-plugin-js'

// to not complelty change up the whole style at once and introduce huge rebasing problems disable a handfull of recommended rules for now
const commonRules = {
    ...js.configs.recommended.rules,
    "no-undef": "off",
    "no-unused-vars": "off",

    // to not change any logic in the first lint intigration set a few rules to warn instead of error
    "no-empty": "warn",
    "no-unsafe-negation": "warn",
    "no-constant-binary-expression": "warn",
    "valid-typeof": "warn",
    "no-control-regex": "warn",
    "no-redeclare": "warn",
    "no-constant-condition": "warn",
    "no-useless-escape": "warn",

    // style rules:
    "@stylistic/js/function-call-spacing": ["error", "never"],
    "@stylistic/js/key-spacing": "error",
    "@stylistic/js/keyword-spacing": "error",
    "@stylistic/js/no-extra-semi": "error",
    "@stylistic/js/no-whitespace-before-property": "error",
    "@stylistic/js/space-before-blocks": "error",
    "@stylistic/js/space-infix-ops": "error"
}

export default [
    {
        files: [
            "tools/**/*.js",
            "wled00/data/**/*.js",
            "eslint.config.js"
        ],
        linterOptions: {
            reportUnusedDisableDirectives: "error",
        },
        languageOptions: {
            globals: {
                ...globals.browser
            }
        },
        plugins: {
            "@stylistic/js": stylisticJs
        },
        rules: {
            ...commonRules
        }
    },
    {
        files: [
            "tools/**/*.js",
        ],
        languageOptions: {
            sourceType: "commonjs"
        },
        plugins: {
            "@stylistic/js": stylisticJs
        },
        rules: {
            ...commonRules
        }
    },
    {
        files: [
            "tools/**/*.htm",
            "wled00/data/**/*.htm"
        ],
        plugins: { 
            html,
            "@stylistic/js": stylisticJs
        },
        rules: {
            ...commonRules
        }
    },
];