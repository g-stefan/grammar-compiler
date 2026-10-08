// Created by Grigore Stefan <g_stefan@yahoo.com>
// Public domain (Unlicense) <http://unlicense.org>
// SPDX-FileCopyrightText: 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: Unlicense

messageAction("generate");

// ---

Shell.setenv("PATH", Shell.realPath(Shell.getcwd()) + "\\output\\bin;" + Shell.getenv("PATH"));

// Regenerate source/XYO/GrammarCompiler.Application/Code from input/Compiler.grammar

runInPath("source", function() {
	exitIf(Shell.system("grammar-compiler @../input/Code.Generate.arguments"));
});
