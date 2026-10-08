// Created by Grigore Stefan <g_stefan@yahoo.com>
// Public domain (Unlicense) <http://unlicense.org>
// SPDX-FileCopyrightText: 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: Unlicense

messageAction("test");

// ---

Shell.mkdirRecursivelyIfNotExists("output/test");
Shell.mkdirRecursivelyIfNotExists("output/test/Transform");
Shell.mkdirRecursivelyIfNotExists("temp");

// ---

Shell.setenv("PATH", Shell.realPath(Shell.getcwd()) + "\\output\\bin;" + Shell.getenv("PATH"));

function compareFile(fileA, fileB) {
	if (Shell.fileGetContents(fileA) != Shell.fileGetContents(fileB)) {
		messageError("different content: " + fileA + " , " + fileB);
		return false;
	};
	return true;
};

function compareFolder(pathA, pathB) {
	var fileListA = Shell.getFileList(pathA + "/*");
	var fileListB = Shell.getFileList(pathB + "/*");
	if (fileListA.length != fileListB.length) {
		messageError("different number of files: " + pathA + " , " + pathB);
		return false;
	};
	for (var file of fileListA) {
		var fileName = Shell.getFileName(file);
		if (!compareFile(pathA + "/" + fileName, pathB + "/" + fileName)) {
			return false;
		};
	};
	return true;
};

// Generated parser must be the same as the one used to build the grammar compiler

runInPath("output/test", function() {
	exitIf(Shell.system("grammar-compiler @../../test/Test.Generate.arguments"));
});

exitIf(!compareFolder("source/XYO/GrammarCompiler.Application/Code", "output/test/XYO/GrammarCompiler.Application/Code"), "generated parser is not up to date, run: fabricare generate");

// Rebuild grammar compiler with generated code and run

exitIf(Shell.system("fabricare --workspace=test/Test.Generate.fabricare.json"));

runInPath("output/test", function() {
	exitIf(Shell.system("grammar-compiler.test @../../test/Test.Generate.Check.arguments"));
});

exitIf(!compareFolder("output/test/XYO/GrammarCompiler.Application/Code", "output/test/Check/XYO/GrammarCompiler.Application/Code"), "rebuilt grammar compiler generates a different parser");

// Grammar transformations, the optimized grammar must not change when optimized again

var transformTests = ["Rule1", "Rule2", "Rule3", "Rule4", "Rule5", "Prefix", "Loops", "Literal", "LiteralShadowed", "Duplicate", "AlwaysMatches", "LeftRecursion", "Assign", "Nested"];

runInPath("output/test/Transform", function() {
	for (var name of transformTests) {
		Console.writeLn("- transform: " + name);
		exitIf(Shell.system("grammar-compiler --grammar=../../../test/Transform/" + name + ".grammar --output-grammar=" + name + ".grammar"));
		exitIf(!compareFile(name + ".grammar", "../../../test/Transform/" + name + ".expected"), "unexpected transformation: " + name);
		exitIf(Shell.system("grammar-compiler --grammar=" + name + ".grammar --output-grammar=" + name + ".again.grammar"));
		exitIf(!compareFile(name + ".grammar", name + ".again.grammar"), "transformation is not stable: " + name);
	};
});

// The grammar written without optimizations is optimized to the same grammar

runInPath("output/test/Transform", function() {
	exitIf(Shell.system("grammar-compiler --grammar=../../../input/Compiler.grammar --output-grammar=Compiler.grammar"));
	exitIf(Shell.system("grammar-compiler --grammar=../../../input/Compiler.not-optimized.grammar --output-grammar=Compiler.not-optimized.grammar"));
	exitIf(!compareFile("Compiler.grammar", "Compiler.not-optimized.grammar"), "Compiler.not-optimized.grammar is not optimized as Compiler.grammar");
});

// Invalid grammars must be rejected

var errorTests = ["Undefined", "Duplicate", "AssignBeforeDefinition", "NoParserTable", "CountInterval", "Syntax", "LeftRecursionIndirect", "LeftRecursionNoBase", "ReservedName", "Keyword", "NulLiteral", "NulInterval", "StackOverflow"];

runInPath("output/test", function() {
	for (var name of errorTests) {
		Console.writeLn("- expected error: " + name);
		exitIf(!Shell.system("grammar-compiler --grammar=../../test/Error/" + name + ".grammar --source-header=../../test/Test.Source.Header.hpp --source-namespace=TestError --source-include-guard=TEST_ERROR --output-folder=Error/" + name), "grammar accepted: " + name);
	};
});

// Parsers generated from test grammars

var parserTests = ["Loop", "Drift", "Backtrack", "Recursive", "Group", "OrAssign", "AndAssign", "Negation", "Interval", "Count", "LeftRecursion", "LongestMatch", "LiteralLongestMatch"];

function generateParser(grammar, name) {
	exitIf(Shell.system("grammar-compiler --grammar=" + grammar + " --source-header=../../test/Test.Source.Header.hpp --source-namespace=TestParser::" + name + " --source-include-guard=TEST_PARSER_" + name + " --source-include-path=Parser/" + name + " --output-folder=Parser/" + name));
};

runInPath("output/test", function() {
	for (var name of parserTests) {
		generateParser("../../test/Parser/" + name + ".grammar", name);
	};
	generateParser("../../input/Compiler.not-optimized.grammar", "NotOptimized");
});

exitIf(Shell.system("fabricare --workspace=test/Test.Parser.fabricare.json"));

runInPath("output/test", function() {
	exitIf(Shell.system("grammar-compiler.test.parser"));
});
