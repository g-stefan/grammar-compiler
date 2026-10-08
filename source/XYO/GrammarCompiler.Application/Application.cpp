// Grammar Compiler
// Copyright (c) 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Application.hpp>
#include <XYO/GrammarCompiler.Application/Grammar.hpp>
#include <XYO/GrammarCompiler.Application/Transformer.hpp>
#include <XYO/GrammarCompiler.Application/Generator.hpp>
#include <XYO/GrammarCompiler.Application/Code/ParserTable.hpp>

#include <XYO/GrammarCompiler.Application/Copyright.hpp>
#include <XYO/GrammarCompiler.Application/License.hpp>
#include <XYO/GrammarCompiler.Application/Version.hpp>

namespace XYO::GrammarCompiler::Application {

	void Application::showUsage() {
		printf("Grammar Compiler - Compile grammar to c++ source code\n");
		showVersion();
		printf("%s\n\n", GrammarCompiler::Application::Copyright::copyright());

		printf("%s",
		       "options:\n"
		       "    --help                         this info\n"
		       "    --usage                        this info\n"
		       "    --license                      show license\n"
		       "    --version                      show version\n"
		       "    --grammar=file                 grammar to compile\n"
		       "    --source-header=file           text at the beginning of each generated file\n"
		       "    --source-namespace=name        namespace of the generated code\n"
		       "    --source-include-guard=name    prefix of the include guards\n"
		       "    --source-include-path=path     include path of the generated files\n"
		       "    --output-folder=path           where to write the generated files\n"
		       "    --output-grammar=file          write the optimized grammar,\n"
		       "                                   without --source-namespace no code is generated\n"
		       "    @file                          read options from file\n");
		printf("\n");
	};

	void Application::showLicense() {
		printf("%s", GrammarCompiler::Application::License::license().c_str());
	};

	void Application::showVersion() {
		printf("version %s build %s [%s]\n", GrammarCompiler::Application::Version::version(), GrammarCompiler::Application::Version::build(), GrammarCompiler::Application::Version::datetime());
	};

	void Application::initMemory() {
		String::initMemory();
		TDynamicArray<String>::initMemory();
		Parser::initMemory();
		GrammarNodeList::initMemory();
	};

	int Application::main(int cmdN, char *cmdS[]) {
		int i;
		String opt;
		size_t optIndex;
		String optValue;
		TDynamicArray<String> cmdLine;

		String grammarFile;
		String sourceHeaderFile;
		String sourceIncludeGuard;
		String sourceIncludePath;
		String sourceNamespace;
		String outputFolder;
		String outputGrammarFile;

		for (i = 1; i < cmdN; ++i) {
			if (StringCore::beginWith(cmdS[i], "@")) {
				String content;
				if (Shell::fileGetContents(&cmdS[i][1], content)) {
					XYO::System::ShellArguments shellArguments;
					int m;
					shellArguments.set(content);
					for (m = 0; m < shellArguments.cmdN; ++m) {
						cmdLine.push(shellArguments.cmdS[m]);
					};
					continue;
				};
				printf("Error: file not found - %s\n", &cmdS[i][1]);
				return 1;
			};
			cmdLine.push(cmdS[i]);
		};

		for (i = 0; i < cmdLine.length(); ++i) {
			if (StringCore::beginWith(cmdLine[i], "--")) {
				opt = cmdLine[i].index(2);
				optValue = "";
				if (opt.indexOf("=", 0, optIndex)) {
					optValue = opt.substring(optIndex + 1);
					opt = opt.substring(0, optIndex);
				};
				if (opt == "help") {
					showUsage();
					return 0;
				};
				if (opt == "usage") {
					showUsage();
					return 0;
				};
				if (opt == "license") {
					showLicense();
					return 0;
				};
				if (opt == "version") {
					showVersion();
					return 0;
				};
				if (opt == "grammar") {
					grammarFile = optValue;
					continue;
				};
				if (opt == "source-header") {
					sourceHeaderFile = optValue;
					continue;
				};
				if (opt == "source-namespace") {
					sourceNamespace = optValue;
					continue;
				};
				if (opt == "source-include-guard") {
					sourceIncludeGuard = optValue;
					continue;
				};
				if (opt == "source-include-path") {
					sourceIncludePath = optValue;
					continue;
				};
				if (opt == "output-folder") {
					outputFolder = optValue;
					continue;
				};
				if (opt == "output-grammar") {
					outputGrammarFile = optValue;
					continue;
				};
				continue;
			};
		};

		// ---

		if (grammarFile.length() == 0) {
			printf("Error: No grammar file specified!\r\n");
			return 1;
		};

		bool generateCode = (sourceNamespace.length() > 0) || (outputGrammarFile.length() == 0);

		if (generateCode) {
			if (sourceHeaderFile.length() == 0) {
				printf("Error: No source header file specified!\r\n");
				return 1;
			};

			if (sourceNamespace.length() == 0) {
				printf("Error: No source namespace specified!\r\n");
				return 1;
			};
		};

		// ---

		if (!Shell::fileExists(grammarFile)) {
			printf("Error: Grammar file not found - %s\r\n", grammarFile.value());
			return 1;
		};

		Parser parser;
		try {
			if (!parser.parseFile(grammarFile, Code::parserTable)) {
				printf("Error: Syntax error line: %zu!\r\n", parser.getErrorLine());
				return 1;
			};
		} catch (const std::exception &e) {
			// parser-stack-overflow, nesting too deep
			printf("Error: %s line: %zu!\r\n", e.what(), parser.getErrorLine());
			return 1;
		};

		String error;
		String warning;
		GrammarNode *grammar = nullptr;
		bool isOk = Grammar::build(parser.tokenList, grammar, error) &&
		            Grammar::validate(grammar, error, warning) &&
		            Transformer::process(grammar, error, warning);

		if (warning.length() > 0) {
			printf("%s", warning.value());
		};

		if (isOk && (outputGrammarFile.length() > 0)) {
			if (!Shell::filePutContents(outputGrammarFile, Grammar::toStringGrammar(grammar))) {
				error = "Unable to write file ";
				error << outputGrammarFile;
				isOk = false;
			};
		};

		if (isOk && generateCode) {
			Generator generator;
			generator.grammar = grammar;
			isOk = generator.saveCode(sourceNamespace,
			                          sourceIncludeGuard,
			                          sourceIncludePath,
			                          sourceHeaderFile,
			                          outputFolder);
			error = generator.error;
		};

		Grammar::release(grammar);

		if (!isOk) {
			printf("Error: %s\r\n", (error.length() > 0) ? error.value() : "Saving code!");
			return 1;
		};

		return 0;
	};
};

#ifndef XYO_GRAMMARCOMPILER_APPLICATION_LIBRARY
XYO_APPLICATION_MAIN(XYO::GrammarCompiler::Application::Application);
#endif
