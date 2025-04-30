// Created by Grigore Stefan <g_stefan@yahoo.com>
// Public domain (Unlicense) <http://unlicense.org>
// SPDX-FileCopyrightText: 2022-2025 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: Unlicense

messageAction("test");

// ---

Shell.mkdirRecursivelyIfNotExists("output/test");
Shell.mkdirRecursivelyIfNotExists("temp");

// ---

Shell.setenv("PATH", Shell.realPath(Shell.getcwd()) + "\\output\\bin;" + Shell.getenv("PATH"));

runInPath("output/test", function() {
	exitIf(Shell.system("grammar-compiler @../../test/Test.Generate.arguments"));	
});

// Rebuild grammar compiler with generated code and run

exitIf(Shell.system("fabricare --workspace=test/Test.Generate.fabricare.json"));

runInPath("output/test", function() {
	exitIf(Shell.system("grammar-compiler.test @../../test/Test.Generate.Check.arguments"));
});

