// Grammar Compiler
// Copyright (c) 2022 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Application.hpp>
#include <XYO/GrammarCompiler.Application/Processor.hpp>
#include <XYO/GrammarCompiler.Application/Code/ParserTable.hpp>
#include <XYO/GrammarCompiler.Application/Debug.hpp>

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
		       "    --usage                        this info\n"
		       "    --license                      show license\n"
		       "    --version                      show version\n");
		printf("\n");
	};

	void Application::showLicense() {
		printf("%s", GrammarCompiler::Application::License::license());
	};

	void Application::showVersion() {
		printf("version %s build %s [%s]\n", GrammarCompiler::Application::Version::version(), GrammarCompiler::Application::Version::build(), GrammarCompiler::Application::Version::datetime());
	};

	void Application::initMemory() {
		String::initMemory();
		TDynamicArray<String>::initMemory();
		Parser::initMemory();
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

		for (i = 1; i < cmdN; ++i) {
			if (StringCore::beginWith(cmdS[i], "@")) {
				String content;
				if (Shell::fileGetContents(&cmdS[i][1], content)) {
					int cmdNX;
					char **cmdSX;
					int m;
					Shell::mainArgsSet(content, cmdNX, cmdSX);
					for (m = 0; m < cmdNX; ++m) {
						cmdLine.push(cmdSX[m]);
					};
					Shell::mainArgsDelete(cmdNX, cmdSX);
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
				if (String::indexOf(opt, "=", 0, optIndex)) {
					optValue = String::substring(opt, optIndex + 1);
					opt = String::substring(opt, 0, optIndex);
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
				continue;
			};
		};

		// ---

		if (grammarFile.length() == 0) {
			printf("Error: No grammar file specified!\r\n");
			return 1;
		};

		if (sourceHeaderFile.length() == 0) {
			printf("Error: No source header file specified!\r\n");
			return 1;
		};

		if (sourceNamespace.length() == 0) {
			printf("Error: No source namespace specified!\r\n");
			return 1;
		};

		// ---

		Parser parser;
		Processor processor;

		if (!parser.parseFile(grammarFile, Code::parserTable)) {
			processor.tokenList = parser.tokenList;
			printf("Error: Syntax error line: %zd!\r\n", processor.getLineCount());
			return 1;
		};

		processor.tokenList = parser.tokenList;

		if (!processor.prepareCode()) {
			printf("Error: Preparing code!\r\n");
			return 1;
		};

		if (!processor.saveCode(sourceNamespace,
		                        sourceIncludeGuard,
		                        sourceIncludePath,
		                        sourceHeaderFile,
		                        outputFolder)) {
			printf("Error: Saving code!\r\n");
			return 1;
		};

		return 0L;
	};
};

#ifndef XYO_GRAMMARCOMPILER_APPLICATION_LIBRARY
XYO_APPLICATION_MAIN(XYO::GrammarCompiler::Application::Application);
#endif
