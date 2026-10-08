// Grammar Compiler
// Copyright (c) 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Transformer.hpp>

namespace XYO::GrammarCompiler::Application::Transformer {

	using namespace XYO::GrammarCompiler::Application::Grammar;

	// Transformations that change what is matched, also written as comments in the generated code
	static void addNote(Context &context, const String &text) {
		context.rule->notes << text << "\r\n";
		*context.warning << "Note: symbol " << context.rule->value << ": " << text << "\r\n";
	};

	static String removeNotes(const String &text) {
		String retV;
		const char *scan = text.value();
		while (*scan != 0) {
			const char *end = scan;
			for (; (*end != 0) && (*end != '\n'); ++end) {
			};
			if (*end == '\n') {
				++end;
			};
			if (strncmp(scan, "Note: ", 6) != 0) {
				retV.concatenate(scan, end - scan);
			};
			scan = end;
		};
		return retV;
	};

	static void addWarning(Context &context, const String &text) {
		*context.warning << "Warning: symbol " << context.rule->value << ": " << text << "\r\n";
	};

	static GrammarNode *head(GrammarNode *node) {
		if (node->type == GrammarNodeType::Sequence) {
			return node->childHead;
		};
		return node;
	};

	// Consumes node, returns what follows its head, Empty if nothing
	static GrammarNode *tail(GrammarNode *node) {
		if (node->type != GrammarNodeType::Sequence) {
			release(node);
			return newNode(GrammarNodeType::Empty);
		};
		release(GrammarNodeList::pop(node));
		if (node->childHead == nullptr) {
			release(node);
			return newNode(GrammarNodeType::Empty);
		};
		if (node->childHead == node->childTail) {
			GrammarNode *child = GrammarNodeList::pop(node);
			release(node);
			return child;
		};
		return node;
	};

	static bool literalValue(GrammarNode *node, std::string &value) {
		if (node->type != GrammarNodeType::Literal) {
			return false;
		};
		return decodeLiteral(node->value, value);
	};

	static bool startsWith(const std::string &value, const std::string &prefix) {
		if (prefix.length() > value.length()) {
			return false;
		};
		return (value.compare(0, prefix.length(), prefix) == 0);
	};

	static GrammarNode *simplifySequence(GrammarNode *node) {
		GrammarNode *holder = newNode(GrammarNodeType::Unknown);
		GrammarNode *child;
		while ((child = GrammarNodeList::pop(node)) != nullptr) {
			if (child->type == GrammarNodeType::Sequence) {
				moveChildren(holder, child);
				release(child);
				continue;
			};
			if (child->type == GrammarNodeType::Empty) {
				release(child);
				continue;
			};
			GrammarNodeList::pushToTail(holder, child);
		};
		moveChildren(node, holder);
		release(holder);

		if (node->childHead == nullptr) {
			release(node);
			return newNode(GrammarNodeType::Empty);
		};
		if (node->childHead == node->childTail) {
			child = GrammarNodeList::pop(node);
			release(node);
			return child;
		};
		return node;
	};

	// Rule 4 and similar: (A+)? => A*, (A*)* => A*, ...
	static GrammarNode *simplifyLoop(GrammarNode *node) {
		GrammarNode *child = node->childHead;
		if (child->type == GrammarNodeType::Empty) {
			release(node);
			return newNode(GrammarNodeType::Empty);
		};
		if ((child->type != GrammarNodeType::Optional) && (child->type != GrammarNodeType::ZeroOrMore) && (child->type != GrammarNodeType::OneOrMore)) {
			return node;
		};

		bool keepChild = false;
		switch (node->type) {
		case GrammarNodeType::Optional:
			// (A?)? => A? , (A*)? => A* , (A+)? => A*
			keepChild = (child->type != GrammarNodeType::OneOrMore);
			break;
		case GrammarNodeType::ZeroOrMore:
			// (A?)* , (A*)* , (A+)* => A*
			keepChild = (child->type == GrammarNodeType::ZeroOrMore);
			break;
		case GrammarNodeType::OneOrMore:
			// (A?)+ => A* , (A*)+ => A* , (A+)+ => A+
			keepChild = (child->type != GrammarNodeType::Optional);
			break;
		default:
			return node;
		};

		child = GrammarNodeList::pop(node);
		if (keepChild) {
			release(node);
			return child;
		};
		GrammarNode *grandChild = GrammarNodeList::pop(child);
		release(child);
		node->type = GrammarNodeType::ZeroOrMore;
		GrammarNodeList::pushToTail(node, grandChild);
		return node;
	};

	static GrammarNode *simplifyRepeat(GrammarNode *node) {
		GrammarNode *child = node->childHead;
		if (child->type == GrammarNodeType::Empty) {
			release(node);
			return newNode(GrammarNodeType::Empty);
		};
		if ((node->min == 0) && (node->max == 0)) {
			release(node);
			return newNode(GrammarNodeType::Empty);
		};
		if ((node->min == 1) && (node->max == 1)) {
			child = GrammarNodeList::pop(node);
			release(node);
			return child;
		};
		if ((node->min == 0) && (node->max < 0)) {
			node->type = GrammarNodeType::ZeroOrMore;
		} else if ((node->min == 1) && (node->max < 0)) {
			node->type = GrammarNodeType::OneOrMore;
		} else if ((node->min == 0) && (node->max == 1)) {
			node->type = GrammarNodeType::Optional;
		};
		return node;
	};

	// "a" | "ab" => "a" | "a" & "b" , then factored as a common prefix
	static void splitLiteralPrefix(GrammarNode *node) {
		GrammarNode *alternative;
		for (alternative = node->childHead; (alternative != nullptr) && (alternative->next != nullptr); alternative = alternative->next) {
			GrammarNode *next = alternative->next;
			std::string prefix;
			std::string value;
			if (!literalValue(head(alternative), prefix) || !literalValue(head(next), value)) {
				continue;
			};
			if ((prefix.length() == 0) || (prefix.length() >= value.length()) || !startsWith(value, prefix)) {
				continue;
			};

			GrammarNode *rest = newNode(GrammarNodeType::Literal);
			rest->value = encodeLiteral(value.substr(prefix.length()));

			if (next->type == GrammarNodeType::Sequence) {
				release(GrammarNodeList::pop(next));
				GrammarNodeList::push(next, rest);
				GrammarNodeList::push(next, copy(head(alternative)));
				continue;
			};

			GrammarNode *sequence = newNode(GrammarNodeType::Sequence, copy(head(alternative)), rest);
			GrammarNodeList::insertNode(node, next, sequence);
			GrammarNodeList::extract(node, next);
			release(next);
		};
	};

	// Rules 1, 2, 5: A & B | A & C => A & (B | C) , A & B | A => A & B? , A | A & B => A & B?
	// Only neighbouring alternatives, the order of the alternatives is the priority.
	// Returns true if rule 5 changed what is matched
	static bool factor(GrammarNode *node, Context &context) {
		bool retV = false;
		GrammarNode *alternative = node->childHead;
		while (alternative != nullptr) {
			GrammarNode *end = alternative->next;
			size_t count = 1;
			while ((end != nullptr) && isEqual(head(alternative), head(end))) {
				end = end->next;
				++count;
			};
			if (count < 2) {
				alternative = end;
				continue;
			};

			GrammarNode *anchor = alternative->back;
			GrammarNode *group = newNode(GrammarNodeType::Choice);
			GrammarNode *scan = alternative;
			while (scan != end) {
				GrammarNode *next = scan->next;
				GrammarNodeList::extract(node, scan);
				GrammarNodeList::pushToTail(group, scan);
				scan = next;
			};

			GrammarNode *first = copy(head(group->childHead));
			GrammarNode *tails = newNode(GrammarNodeType::Choice);
			bool hasEmpty = false;
			bool isLongestMatch = false;
			GrammarNode *item;
			while ((item = GrammarNodeList::pop(group)) != nullptr) {
				GrammarNode *itemTail = tail(item);
				if (itemTail->type == GrammarNodeType::Empty) {
					hasEmpty = true;
					release(itemTail);
					continue;
				};
				if (hasEmpty) {
					isLongestMatch = true;
				};
				GrammarNodeList::pushToTail(tails, itemTail);
			};
			release(group);

			// rule 5, the shorter alternative is tried last
			if (hasEmpty) {
				GrammarNodeList::pushToTail(tails, newNode(GrammarNodeType::Empty));
			};

			if (isLongestMatch) {
				retV = true;
			};

			GrammarNode *factored = simplify(newNode(GrammarNodeType::Sequence, first, tails), context);
			GrammarNodeList::insertNode(node, anchor, factored);
			alternative = factored->next;
		};
		return retV;
	};

	// An alternative that can not match because a previous one always matches first
	static bool isShadowed(GrammarNode *previous, GrammarNode *alternative) {
		if (isEqual(previous, alternative)) {
			return true;
		};
		std::string prefix;
		std::string value;
		if (literalValue(previous, prefix) && literalValue(head(alternative), value)) {
			return (prefix.length() > 0) && startsWith(value, prefix);
		};
		return false;
	};

	static void removeShadowed(GrammarNode *node, Context &context) {
		GrammarNode *alternative;
		GrammarNode *next;
		GrammarNode *previous;
		for (alternative = node->childHead; alternative != nullptr; alternative = next) {
			next = alternative->next;
			for (previous = node->childHead; previous != alternative; previous = previous->next) {
				if (isShadowed(previous, alternative)) {
					break;
				};
			};
			if (previous == alternative) {
				continue;
			};
			String text = "alternative ";
			text << toString(alternative) << " can never match, " << toString(previous) << " matches first";
			addWarning(context, text);
			GrammarNodeList::extract(node, alternative);
			release(alternative);
		};
	};

	// Alternatives after one that always matches can never match
	static void removeAfter(GrammarNode *node, GrammarNode *alternative, Context &context) {
		while (alternative->next != nullptr) {
			GrammarNode *next = alternative->next;
			String text = "alternative ";
			text << toString(next) << " can never match, " << toString(alternative) << " always matches";
			addWarning(context, text);
			GrammarNodeList::extract(node, next);
			release(next);
		};
	};

	static GrammarNode *simplifyChoice(GrammarNode *node, Context &context) {
		GrammarNode *holder = newNode(GrammarNodeType::Unknown);
		GrammarNode *child;
		while ((child = GrammarNodeList::pop(node)) != nullptr) {
			if (child->type == GrammarNodeType::Choice) {
				moveChildren(holder, child);
				release(child);
				continue;
			};
			GrammarNodeList::pushToTail(holder, child);
		};
		moveChildren(node, holder);
		release(holder);

		String before = toString(node);
		String notes = context.rule->notes;
		String warning = *context.warning;

		splitLiteralPrefix(node);
		bool isLongestMatch = factor(node, context);
		if (context.rule->notes.length() != notes.length()) {
			// rule 5 applied while factoring
			isLongestMatch = true;
		};
		removeShadowed(node, context);

		GrammarNode *retV = node;

		// A | B | "" => (A | B)?
		for (child = node->childHead; child != nullptr; child = child->next) {
			if (child->type == GrammarNodeType::Empty) {
				break;
			};
		};
		if (child != nullptr) {
			removeAfter(node, child, context);
			GrammarNodeList::extract(node, child);
			release(child);
			if (node->childHead == nullptr) {
				release(node);
				retV = newNode(GrammarNodeType::Empty);
			} else {
				retV = simplify(newNode(GrammarNodeType::Optional, node), context);
			};
		} else if (node->childHead == node->childTail) {
			retV = GrammarNodeList::pop(node);
			release(node);
		};

		if (isLongestMatch) {
			// one note for the choice, instead of the notes of the inner steps
			context.rule->notes = notes;
			*context.warning = warning + removeNotes(context.warning->substring(warning.length()));
			String note = "longest match, ";
			note << before << " => " << toString(retV);
			addNote(context, note);
		};

		return retV;
	};

	GrammarNode *simplify(GrammarNode *node, Context &context) {
		GrammarNode *holder = newNode(GrammarNodeType::Unknown);
		GrammarNode *child;
		while ((child = GrammarNodeList::pop(node)) != nullptr) {
			GrammarNodeList::pushToTail(holder, simplify(child, context));
		};
		moveChildren(node, holder);
		release(holder);

		switch (node->type) {
		case GrammarNodeType::Literal: {
			std::string value;
			if (decodeLiteral(node->value, value) && (value.length() == 0)) {
				release(node);
				return newNode(GrammarNodeType::Empty);
			};
			return node;
		};
		case GrammarNodeType::Sequence:
			return simplifySequence(node);
		case GrammarNodeType::Choice:
			return simplifyChoice(node, context);
		case GrammarNodeType::Optional:
		case GrammarNodeType::ZeroOrMore:
		case GrammarNodeType::OneOrMore:
			return simplifyLoop(node);
		case GrammarNodeType::Repeat:
			return simplifyRepeat(node);
		default:
			break;
		};
		return node;
	};

	void simplifyRule(GrammarNode *rule, Context &context) {
		GrammarNode *expression = GrammarNodeList::pop(rule);
		String last = toString(expression);
		int k;
		for (k = 0; k < 64; ++k) {
			expression = simplify(expression, context);
			String current = toString(expression);
			if (current == last) {
				break;
			};
			last = current;
		};
		GrammarNodeList::pushToTail(rule, expression);
	};

	static bool isSelf(GrammarNode *node, GrammarNode *rule) {
		return (node->type == GrammarNodeType::Reference) && (node->value == rule->value);
	};

	// Rule 3: E = E & A | B => E = B & A*
	bool removeLeftRecursion(GrammarNode *rule, Context &context, String &error) {
		GrammarNode *expression = rule->childHead;
		GrammarNode *scan;
		bool found = false;
		if (expression->type == GrammarNodeType::Choice) {
			for (scan = expression->childHead; scan != nullptr; scan = scan->next) {
				if (isSelf(head(scan), rule)) {
					found = true;
				};
			};
		} else {
			found = isSelf(head(expression), rule);
		};
		if (!found) {
			return true;
		};

		expression = GrammarNodeList::pop(rule);
		if (expression->type != GrammarNodeType::Choice) {
			expression = newNode(GrammarNodeType::Choice, expression);
		};

		GrammarNode *base = newNode(GrammarNodeType::Choice);
		GrammarNode *recursive = newNode(GrammarNodeType::Choice);
		GrammarNode *alternative;
		while ((alternative = GrammarNodeList::pop(expression)) != nullptr) {
			if (!isSelf(head(alternative), rule)) {
				GrammarNodeList::pushToTail(base, alternative);
				continue;
			};
			GrammarNode *alternativeTail = tail(alternative);
			if (alternativeTail->type == GrammarNodeType::Empty) {
				String text = "alternative ";
				text << rule->value << " has no effect";
				addWarning(context, text);
				release(alternativeTail);
				continue;
			};
			GrammarNodeList::pushToTail(recursive, alternativeTail);
		};
		release(expression);

		if (base->childHead == nullptr) {
			release(base);
			release(recursive);
			error = "Symbol ";
			error << rule->value << " is left recursive and has no alternative to start with";
			return false;
		};

		GrammarNode *result;
		if (recursive->childHead == nullptr) {
			release(recursive);
			result = base;
		} else {
			result = newNode(GrammarNodeType::Sequence, base, newNode(GrammarNodeType::ZeroOrMore, recursive));
		};
		GrammarNodeList::pushToTail(rule, result);
		simplifyRule(rule, context);

		String note = "left recursion, ";
		note << rule->source << " => " << toString(rule->childHead);
		addNote(context, note);
		return true;
	};

	// Can succeed without consuming input
	bool isNullable(GrammarNode *node) {
		GrammarNode *scan;
		switch (node->type) {
		case GrammarNodeType::Empty:
		case GrammarNodeType::Optional:
		case GrammarNodeType::ZeroOrMore:
		case GrammarNodeType::Not:
			return true;
		case GrammarNodeType::Literal: {
			std::string value;
			return decodeLiteral(node->value, value) && (value.length() == 0);
		};
		case GrammarNodeType::Reference:
			return (node->rule != nullptr) && node->rule->isNullable;
		case GrammarNodeType::Sequence:
			for (scan = node->childHead; scan != nullptr; scan = scan->next) {
				if (!isNullable(scan)) {
					return false;
				};
			};
			return true;
		case GrammarNodeType::Choice:
			for (scan = node->childHead; scan != nullptr; scan = scan->next) {
				if (isNullable(scan)) {
					return true;
				};
			};
			return false;
		case GrammarNodeType::OneOrMore:
			return isNullable(node->childHead);
		case GrammarNodeType::Repeat:
			return (node->min == 0) || isNullable(node->childHead);
		default:
			break;
		};
		return false;
	};

	// Never fails
	bool isAlwaysSuccess(GrammarNode *node) {
		GrammarNode *scan;
		switch (node->type) {
		case GrammarNodeType::Empty:
		case GrammarNodeType::Optional:
		case GrammarNodeType::ZeroOrMore:
			return true;
		case GrammarNodeType::Literal: {
			std::string value;
			return decodeLiteral(node->value, value) && (value.length() == 0);
		};
		case GrammarNodeType::Reference:
			return (node->rule != nullptr) && node->rule->isAlwaysSuccess;
		case GrammarNodeType::Sequence:
			for (scan = node->childHead; scan != nullptr; scan = scan->next) {
				if (!isAlwaysSuccess(scan)) {
					return false;
				};
			};
			return true;
		case GrammarNodeType::Choice:
			for (scan = node->childHead; scan != nullptr; scan = scan->next) {
				if (isAlwaysSuccess(scan)) {
					return true;
				};
			};
			return false;
		case GrammarNodeType::OneOrMore:
			return isAlwaysSuccess(node->childHead);
		case GrammarNodeType::Repeat:
			return (node->min == 0) || isAlwaysSuccess(node->childHead);
		default:
			break;
		};
		return false;
	};

	void computeRuleProperties(GrammarNode *grammar) {
		GrammarNode *rule;
		bool changed = true;
		while (changed) {
			changed = false;
			for (rule = grammar->childHead; rule != nullptr; rule = rule->next) {
				if (!rule->isNullable && isNullable(rule->childHead)) {
					rule->isNullable = true;
					changed = true;
				};
				if (!rule->isAlwaysSuccess && isAlwaysSuccess(rule->childHead)) {
					rule->isAlwaysSuccess = true;
					changed = true;
				};
			};
		};
	};

	static bool visitRule(GrammarNode *rule, GrammarNode *caller, String &error);

	// Rules called before any input is consumed
	static bool visitExpression(GrammarNode *node, GrammarNode *rule, String &error) {
		GrammarNode *scan;
		switch (node->type) {
		case GrammarNodeType::Reference:
			return visitRule(node->rule, rule, error);
		case GrammarNodeType::Sequence:
			for (scan = node->childHead; scan != nullptr; scan = scan->next) {
				if (!visitExpression(scan, rule, error)) {
					return false;
				};
				if (!isNullable(scan)) {
					break;
				};
			};
			return true;
		case GrammarNodeType::Choice:
		case GrammarNodeType::Optional:
		case GrammarNodeType::ZeroOrMore:
		case GrammarNodeType::OneOrMore:
		case GrammarNodeType::Repeat:
		case GrammarNodeType::Not:
			for (scan = node->childHead; scan != nullptr; scan = scan->next) {
				if (!visitExpression(scan, rule, error)) {
					return false;
				};
			};
			return true;
		default:
			break;
		};
		return true;
	};

	// rule->rule is the caller while visiting
	static bool visitRule(GrammarNode *rule, GrammarNode *caller, String &error) {
		if (rule->mark == 2) {
			return true;
		};
		if (rule->mark == 1) {
			String path = rule->value;
			GrammarNode *scan;
			for (scan = caller; (scan != nullptr) && (scan != rule); scan = scan->rule) {
				String item = scan->value;
				item << " -> " << path;
				path = item;
			};
			error = "Left recursion ";
			error << rule->value << " -> " << path;
			return false;
		};
		rule->mark = 1;
		rule->rule = caller;
		bool retV = visitExpression(rule->childHead, rule, error);
		rule->mark = 2;
		rule->rule = nullptr;
		return retV;
	};

	bool checkLeftRecursion(GrammarNode *grammar, String &error) {
		GrammarNode *rule;
		bool retV = true;
		for (rule = grammar->childHead; rule != nullptr; rule = rule->next) {
			if (!visitRule(rule, nullptr, error)) {
				retV = false;
				break;
			};
		};
		for (rule = grammar->childHead; rule != nullptr; rule = rule->next) {
			rule->mark = 0;
			rule->rule = nullptr;
		};
		return retV;
	};

	void removeUnreachable(GrammarNode *node, Context &context) {
		GrammarNode *scan;
		for (scan = node->childHead; scan != nullptr; scan = scan->next) {
			removeUnreachable(scan, context);
		};
		if (node->type != GrammarNodeType::Choice) {
			return;
		};
		for (scan = node->childHead; scan != nullptr; scan = scan->next) {
			if (isAlwaysSuccess(scan)) {
				removeAfter(node, scan, context);
				return;
			};
		};
	};

	void checkLoops(GrammarNode *node, Context &context) {
		GrammarNode *scan;
		for (scan = node->childHead; scan != nullptr; scan = scan->next) {
			checkLoops(scan, context);
		};
		bool isLoop = (node->type == GrammarNodeType::ZeroOrMore) || (node->type == GrammarNodeType::OneOrMore) || ((node->type == GrammarNodeType::Repeat) && (node->max < 0));
		if (isLoop && isNullable(node->childHead)) {
			String text;
			text << toString(node) << " repeats an expression that can match empty input";
			addWarning(context, text);
		};
	};

	bool process(GrammarNode *grammar, String &error, String &warning) {
		Context context;
		GrammarNode *rule;

		context.warning = &warning;

		for (rule = grammar->childHead; rule != nullptr; rule = rule->next) {
			context.rule = rule;
			simplifyRule(rule, context);
		};

		for (rule = grammar->childHead; rule != nullptr; rule = rule->next) {
			context.rule = rule;
			if (!removeLeftRecursion(rule, context, error)) {
				return false;
			};
		};

		computeRuleProperties(grammar);

		if (!checkLeftRecursion(grammar, error)) {
			return false;
		};

		for (rule = grammar->childHead; rule != nullptr; rule = rule->next) {
			context.rule = rule;
			removeUnreachable(rule->childHead, context);
			simplifyRule(rule, context);
			checkLoops(rule->childHead, context);
		};

		return true;
	};

};
